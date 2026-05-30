// Copyright (c) Darrin Stewart. All rights reserved.
#include "Core.UnitTest/UnitTest.h"

#include "Core.Util/FixedItemPool.h"

#include <cstring>
#include <exception>
#include <new>

#if defined(_WIN32)
#include <Windows.h>
#endif

namespace
{
	enum class NodeType : unsigned { None = 0, Suite, Test };

	struct ChildRef
	{
		NodeType type;
		int      handle;
	};

	constexpr int MaxUnitTestSuites = 1024;
	constexpr int MaxUnitTestTests  = 2048;

	struct UnitTestSuite
	{
		bool        inUse;
		const char* name;
		int         parent;
		ChildRef    firstChild;
		ChildRef    lastChild;
		ChildRef    nextSibling;
	};

	struct UnitTest
	{
		bool        inUse;
		const char* name;
		UnitTestFn  fn;
		int         parent;
		ChildRef    nextSibling;
	};
}

struct UnitTestContext
{
	ChildRef                      firstChild    = { NodeType::None, 0 };
	ChildRef                      lastChild     = { NodeType::None, 0 };
	FixedItemPoolT<UnitTestSuite> suitePool;
	FixedItemPoolT<UnitTest>      testPool;
	const UnitTestListener*       listener      = nullptr;
	UnitTestSuiteHandle           currentSuite  = InvalidUnitTestSuiteHandle;
	UnitTestHandle                currentTest   = InvalidUnitTestHandle;
};

static ChildRef& getNextSiblingRef(UnitTestContext* ctx, ChildRef ref)
{
	if (ref.type == NodeType::Suite)
		return ctx->suitePool.getPtr(ref.handle)->nextSibling;
	return ctx->testPool.getPtr(ref.handle)->nextSibling;
}

static void appendChildTo(UnitTestContext* ctx, ChildRef& firstChild, ChildRef& lastChild, ChildRef newRef)
{
	if (firstChild.type == NodeType::None)
		firstChild = newRef;
	else
		getNextSiblingRef(ctx, lastChild) = newRef;
	lastChild = newRef;
}

static void freeChildren(UnitTestContext* ctx, ChildRef first)
{
	ChildRef current = first;
	while (current.type != NodeType::None)
	{
		ChildRef next = getNextSiblingRef(ctx, current);
		if (current.type == NodeType::Suite)
		{
			UnitTestSuite* suite = ctx->suitePool.getPtr(current.handle);
			ChildRef childFirst  = suite->firstChild;
			suite->inUse         = false;
			ctx->suitePool.free(current.handle);
			freeChildren(ctx, childFirst);
		}
		else
		{
			ctx->testPool.getPtr(current.handle)->inUse = false;
			ctx->testPool.free(current.handle);
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
		UnitTestSuite* suite = ctx->suitePool.getPtr(ref.handle);
		if (ctx->listener && ctx->listener->onSuiteBegin)
			ctx->listener->onSuiteBegin(ref.handle);
		runChildren(ctx, suite->firstChild, ref.handle);
		if (ctx->listener && ctx->listener->onSuiteEnd)
			ctx->listener->onSuiteEnd(ref.handle);
	}
	else
	{
		UnitTest* test = ctx->testPool.getPtr(ref.handle);
		if (ctx->listener && ctx->listener->onTestBegin)
			ctx->listener->onTestBegin(suiteHandle, ref.handle);
		ctx->currentSuite = suiteHandle;
		ctx->currentTest  = ref.handle;
		if (test->fn)
		{
#if defined(_WIN32)
			__try
			{
				tryInvokeTest(ctx, test->fn, suiteHandle, ref.handle);
			}
			__except(crashFilter(ctx, suiteHandle, ref.handle, GetExceptionInformation()))
			{
			}
#else
			tryInvokeTest(ctx, test->fn, suiteHandle, ref.handle);
#endif
		}
		ctx->currentSuite = InvalidUnitTestSuiteHandle;
		ctx->currentTest  = InvalidUnitTestHandle;
		if (ctx->listener && ctx->listener->onTestEnd)
			ctx->listener->onTestEnd(suiteHandle, ref.handle);
	}
}

static void runChildren(UnitTestContext* ctx, ChildRef current, UnitTestSuiteHandle suiteHandle)
{
	while (current.type != NodeType::None)
	{
		ChildRef next = getNextSiblingRef(ctx, current);
		runNode(ctx, current, suiteHandle);
		current = next;
	}
}

UnitTestResult createUnitTestContext(UnitTestContext** outCtx)
{
	UnitTestContext* ctx = new (std::nothrow) UnitTestContext{};
	if (!ctx)
	{
		*outCtx = nullptr;
		return UnitTestResult::OutOfResources;
	}
	ctx->suitePool.init(MaxUnitTestSuites);
	ctx->testPool.init(MaxUnitTestTests);
	*outCtx = ctx;
	return UnitTestResult::Success;
}

void destroyUnitTestContext(UnitTestContext* ctx)
{
	if (!ctx)
		return;
	ctx->suitePool.destroy();
	ctx->testPool.destroy();
	delete ctx;
}

void setUnitTestListener(UnitTestContext* ctx, const UnitTestListener* listener)
{
	ctx->listener = listener;
}

UnitTestResult createUnitTestSuite(UnitTestSuiteHandle& outHandle, UnitTestContext* ctx, const char* name, UnitTestSuiteHandle parent)
{
	UnitTestSuiteHandle slot = ctx->suitePool.alloc();
	if (slot == InvalidFixedItemHandle)
	{
		outHandle = InvalidUnitTestSuiteHandle;
		return UnitTestResult::OutOfResources;
	}

	UnitTestSuite* suite = ctx->suitePool.getPtr(slot);
	suite->inUse         = true;
	suite->name          = name;
	suite->parent        = parent;
	suite->firstChild    = { NodeType::None, 0 };
	suite->lastChild     = { NodeType::None, 0 };
	suite->nextSibling   = { NodeType::None, 0 };

	ChildRef newRef = { NodeType::Suite, slot };

	if (parent == InvalidUnitTestSuiteHandle)
		appendChildTo(ctx, ctx->firstChild, ctx->lastChild, newRef);
	else
		appendChildTo(ctx, ctx->suitePool.getPtr(parent)->firstChild, ctx->suitePool.getPtr(parent)->lastChild, newRef);

	outHandle = slot;
	return UnitTestResult::Success;
}

UnitTestResult createUnitTest(UnitTestHandle& outHandle, UnitTestContext* ctx, const char* name, UnitTestFn fn, UnitTestSuiteHandle parent)
{
	UnitTestHandle slot = ctx->testPool.alloc();
	if (slot == InvalidFixedItemHandle)
	{
		outHandle = InvalidUnitTestHandle;
		return UnitTestResult::OutOfResources;
	}

	UnitTest* test    = ctx->testPool.getPtr(slot);
	test->inUse       = true;
	test->name        = name;
	test->fn          = fn;
	test->parent      = parent;
	test->nextSibling = { NodeType::None, 0 };

	ChildRef newRef = { NodeType::Test, slot };

	if (parent == InvalidUnitTestSuiteHandle)
		appendChildTo(ctx, ctx->firstChild, ctx->lastChild, newRef);
	else
		appendChildTo(ctx, ctx->suitePool.getPtr(parent)->firstChild, ctx->suitePool.getPtr(parent)->lastChild, newRef);

	outHandle = slot;
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
	if (suite < 0 || suite >= MaxUnitTestSuites)
		return nullptr;
	UnitTestSuite* s = ctx->suitePool.getPtr(suite);
	return s->inUse ? s->name : nullptr;
}

const char* getUnitTestName(UnitTestContext* ctx, UnitTestHandle test)
{
	if (test < 0 || test >= MaxUnitTestTests)
		return nullptr;
	UnitTest* t = ctx->testPool.getPtr(test);
	return t->inUse ? t->name : nullptr;
}
