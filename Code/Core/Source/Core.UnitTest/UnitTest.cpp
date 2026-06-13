// Copyright (c) Darrin Stewart. All rights reserved.
#include "Core.UnitTest/UnitTest.h"

#include "Core.Memory/SlabAllocator.h"

#include <cstring>
#include <exception>
#include <new>

#if defined(_WIN32)
#include <Windows.h>
#endif

enum class NodeType : unsigned { None = 0, Suite, Test };

struct ChildRef
{
	NodeType type;
	void*    ptr;
};

constexpr int MaxUnitTestSuites = 1024;
constexpr int MaxUnitTestTests  = 2048;

struct UnitTestSuite
{
	bool                inUse;
	const char*         name;
	UnitTestSuiteHandle parent;
	ChildRef            firstChild;
	ChildRef            lastChild;
	ChildRef            nextSibling;
};

struct UnitTest
{
	bool                inUse;
	const char*         name;
	UnitTestFn          fn;
	UnitTestSuiteHandle parent;
	ChildRef            nextSibling;
};

struct UnitTestContext
{
	ChildRef                firstChild;
	ChildRef                lastChild;
	SlabCache*              suitePool;
	SlabCache*              testPool;
	const UnitTestListener* listener;
	UnitTestSuiteHandle     currentSuite;
	UnitTestHandle          currentTest;
};

static ChildRef& getNextSiblingRef(ChildRef ref)
{
	if (ref.type == NodeType::Suite)
		return static_cast<UnitTestSuite*>(ref.ptr)->nextSibling;
	return static_cast<UnitTest*>(ref.ptr)->nextSibling;
}

static void appendChildTo(ChildRef& firstChild, ChildRef& lastChild, ChildRef newRef)
{
	if (firstChild.type == NodeType::None)
		firstChild = newRef;
	else
		getNextSiblingRef(lastChild) = newRef;
	lastChild = newRef;
}

static void freeChildren(UnitTestContext* ctx, ChildRef first)
{
	ChildRef current = first;
	while (current.type != NodeType::None)
	{
		ChildRef next = getNextSiblingRef(current);
		if (current.type == NodeType::Suite)
		{
			UnitTestSuite* suite = static_cast<UnitTestSuite*>(current.ptr);
			ChildRef childFirst  = suite->firstChild;
			suite->inUse         = false;
			slabCacheFree(ctx->suitePool, suite);
			freeChildren(ctx, childFirst);
		}
		else
		{
			UnitTest* test = static_cast<UnitTest*>(current.ptr);
			test->inUse    = false;
			slabCacheFree(ctx->testPool, test);
		}
		current = next;
	}
}

static void runChildren(UnitTestContext* ctx, ChildRef current, UnitTestSuiteHandle suiteHandle);

static void tryInvokeTest(UnitTestContext* ctx, UnitTestFn fn, UnitTestSuiteHandle suiteHandle, UnitTestHandle testHandle)
{
	try
	{
		fn(ctx);
	}
	catch (const std::exception& e)
	{
		if (ctx->listener && ctx->listener->onTestException)
			ctx->listener->onTestException(suiteHandle, testHandle, e.what());
	}
	catch (...)
	{
		if (ctx->listener && ctx->listener->onTestException)
			ctx->listener->onTestException(suiteHandle, testHandle, "unknown C++ exception");
	}
}

#if defined(_WIN32)
static int crashFilter(UnitTestContext* ctx, UnitTestSuiteHandle suite, UnitTestHandle test, EXCEPTION_POINTERS* exInfo)
{
	if (ctx->listener && ctx->listener->onTestCrash)
		ctx->listener->onTestCrash(suite, test, exInfo);
	return 1; // EXCEPTION_EXECUTE_HANDLER
}
#endif

static void runNode(UnitTestContext* ctx, ChildRef ref, UnitTestSuiteHandle suiteHandle)
{
	if (ref.type == NodeType::Suite)
	{
		UnitTestSuite* suite = static_cast<UnitTestSuite*>(ref.ptr);
		if (ctx->listener && ctx->listener->onSuiteBegin)
			ctx->listener->onSuiteBegin(suite);
		runChildren(ctx, suite->firstChild, suite);
		if (ctx->listener && ctx->listener->onSuiteEnd)
			ctx->listener->onSuiteEnd(suite);
	}
	else
	{
		UnitTest* test = static_cast<UnitTest*>(ref.ptr);
		if (ctx->listener && ctx->listener->onTestBegin)
			ctx->listener->onTestBegin(suiteHandle, test);
		ctx->currentSuite = suiteHandle;
		ctx->currentTest  = test;
		if (test->fn)
		{
#if defined(_WIN32)
			__try
			{
				tryInvokeTest(ctx, test->fn, suiteHandle, test);
			}
			__except(crashFilter(ctx, suiteHandle, test, GetExceptionInformation()))
			{
			}
#else
			tryInvokeTest(ctx, test->fn, suiteHandle, test);
#endif
		}
		ctx->currentSuite = InvalidUnitTestSuiteHandle;
		ctx->currentTest  = InvalidUnitTestHandle;
		if (ctx->listener && ctx->listener->onTestEnd)
			ctx->listener->onTestEnd(suiteHandle, test);
	}
}

static void runChildren(UnitTestContext* ctx, ChildRef current, UnitTestSuiteHandle suiteHandle)
{
	while (current.type != NodeType::None)
	{
		ChildRef next = getNextSiblingRef(current);
		runNode(ctx, current, suiteHandle);
		current = next;
	}
}

UnitTestResult createUnitTestContext(UnitTestContext** outCtx)
{
	UnitTestContext* ctx = new (std::nothrow) UnitTestContext;
	if (!ctx)
	{
		*outCtx = nullptr;
		return UnitTestResult::OutOfResources;
	}
	memset(ctx, 0, sizeof(*ctx));

	SlabCacheParams suitePoolParams = { MaxUnitTestSuites, sizeof(UnitTestSuite), 1 };
	ctx->suitePool = createSlabCache(suitePoolParams);

	SlabCacheParams testPoolParams = { MaxUnitTestTests, sizeof(UnitTest), 1 };
	ctx->testPool = createSlabCache(testPoolParams);

	*outCtx = ctx;
	return UnitTestResult::Success;
}

void destroyUnitTestContext(UnitTestContext* ctx)
{
	if (!ctx)
		return;
	freeChildren(ctx, ctx->firstChild);
	destroySlabCache(ctx->suitePool);
	destroySlabCache(ctx->testPool);
	delete ctx;
}

void setUnitTestListener(UnitTestContext* ctx, const UnitTestListener* listener)
{
	ctx->listener = listener;
}

UnitTestResult createUnitTestSuite(UnitTestSuiteHandle& outHandle, UnitTestContext* ctx, const char* name, UnitTestSuiteHandle parent)
{
	UnitTestSuite* suite = static_cast<UnitTestSuite*>(slabCacheAlloc(ctx->suitePool));
	if (!suite)
	{
		outHandle = InvalidUnitTestSuiteHandle;
		return UnitTestResult::OutOfResources;
	}

	suite->inUse       = true;
	suite->name        = name;
	suite->parent      = parent;
	suite->firstChild  = { NodeType::None, nullptr };
	suite->lastChild   = { NodeType::None, nullptr };
	suite->nextSibling = { NodeType::None, nullptr };

	ChildRef newRef = { NodeType::Suite, suite };

	if (parent == InvalidUnitTestSuiteHandle)
		appendChildTo(ctx->firstChild, ctx->lastChild, newRef);
	else
		appendChildTo(parent->firstChild, parent->lastChild, newRef);

	outHandle = suite;
	return UnitTestResult::Success;
}

UnitTestResult createUnitTest(UnitTestHandle& outHandle, UnitTestContext* ctx, const char* name, UnitTestFn fn, UnitTestSuiteHandle parent)
{
	UnitTest* test = static_cast<UnitTest*>(slabCacheAlloc(ctx->testPool));
	if (!test)
	{
		outHandle = InvalidUnitTestHandle;
		return UnitTestResult::OutOfResources;
	}

	test->inUse       = true;
	test->name        = name;
	test->fn          = fn;
	test->parent      = parent;
	test->nextSibling = { NodeType::None, nullptr };

	ChildRef newRef = { NodeType::Test, test };

	if (parent == InvalidUnitTestSuiteHandle)
		appendChildTo(ctx->firstChild, ctx->lastChild, newRef);
	else
		appendChildTo(parent->firstChild, parent->lastChild, newRef);

	outHandle = test;
	return UnitTestResult::Success;
}

UnitTestResult runUnitTests(UnitTestContext* ctx)
{
	runChildren(ctx, ctx->firstChild, InvalidUnitTestSuiteHandle);

	return UnitTestResult::Success;
}

void unitTestExpect(UnitTestContext* ctx, int condition, const char* expr, const char* file, int line)
{
	if (ctx->listener && ctx->listener->onTestAssert)
		ctx->listener->onTestAssert(ctx->currentSuite, ctx->currentTest, expr, file, line, condition);
}

const char* getUnitTestSuiteName(UnitTestContext* ctx, UnitTestSuiteHandle suite)
{
	(void)ctx;
	if (suite == InvalidUnitTestSuiteHandle)
		return nullptr;
	return suite->inUse ? suite->name : nullptr;
}

const char* getUnitTestName(UnitTestContext* ctx, UnitTestHandle test)
{
	(void)ctx;
	if (test == InvalidUnitTestHandle)
		return nullptr;
	return test->inUse ? test->name : nullptr;
}
