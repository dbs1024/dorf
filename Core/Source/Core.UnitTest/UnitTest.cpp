// Copyright (c) Darrin Stewart. All rights reserved.
#include "Core.UnitTest/UnitTest.h"

#include "Core.Util/FixedItemPool.h"

#include <cstring>

namespace
{
    enum class NodeType : unsigned { None = 0, Suite, Test };

    struct ChildRef
    {
        NodeType type;
        int      handle;
    };

    constexpr int MaxUnitTestRunners = 4;
    constexpr int MaxUnitTestSuites  = 1024;
    constexpr int MaxUnitTestTests   = 2048;

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

    struct UnitTestRunner
    {
        bool        inUse;
        const char* name;
        ChildRef    firstChild;
        ChildRef    lastChild;
    };

    UnitTestRunner               s_runners[MaxUnitTestRunners] = {};
    int                          s_runnerCount                 = 0;
    FixedItemPoolT<UnitTestSuite> s_suitePool;
    FixedItemPoolT<UnitTest>      s_testPool;

    const UnitTestListener* s_listener = nullptr;

    ChildRef& getNextSiblingRef(ChildRef ref)
    {
        if (ref.type == NodeType::Suite)
            return s_suitePool.getPtr(ref.handle)->nextSibling;
        return s_testPool.getPtr(ref.handle)->nextSibling;
    }

    void appendChildTo(ChildRef& firstChild, ChildRef& lastChild, ChildRef newRef)
    {
        if (firstChild.type == NodeType::None)
            firstChild = newRef;
        else
            getNextSiblingRef(lastChild) = newRef;
        lastChild = newRef;
    }

    void freeChildren(ChildRef first)
    {
        ChildRef current = first;
        while (current.type != NodeType::None)
        {
            ChildRef next = getNextSiblingRef(current);
            if (current.type == NodeType::Suite)
            {
                UnitTestSuite* suite = s_suitePool.getPtr(current.handle);
                ChildRef childFirst  = suite->firstChild;
                suite->inUse         = false;
                s_suitePool.free(current.handle);
                freeChildren(childFirst);
            }
            else
            {
                s_testPool.getPtr(current.handle)->inUse = false;
                s_testPool.free(current.handle);
            }
            current = next;
        }
    }

    void runChildren(ChildRef first, UnitTestRunnerHandle runnerHandle, UnitTestSuiteHandle suiteHandle);

    void runNode(ChildRef ref, UnitTestRunnerHandle runnerHandle, UnitTestSuiteHandle suiteHandle)
    {
        if (ref.type == NodeType::Suite)
        {
            UnitTestSuite* suite = s_suitePool.getPtr(ref.handle);
            if (s_listener && s_listener->onSuiteBegin)
                s_listener->onSuiteBegin(runnerHandle, ref.handle);
            runChildren(suite->firstChild, runnerHandle, ref.handle);
            if (s_listener && s_listener->onSuiteEnd)
                s_listener->onSuiteEnd(runnerHandle, ref.handle);
        }
        else
        {
            UnitTest* test = s_testPool.getPtr(ref.handle);
            if (s_listener && s_listener->onTestBegin)
                s_listener->onTestBegin(runnerHandle, suiteHandle, ref.handle);
            if (test->fn)
                test->fn();
            if (s_listener && s_listener->onTestEnd)
                s_listener->onTestEnd(runnerHandle, suiteHandle, ref.handle);
        }
    }

    void runChildren(ChildRef current, UnitTestRunnerHandle runnerHandle, UnitTestSuiteHandle suiteHandle)
    {
        while (current.type != NodeType::None)
        {
            ChildRef next = getNextSiblingRef(current);
            runNode(current, runnerHandle, suiteHandle);
            current = next;
        }
    }
}

void setUnitTestListener(const UnitTestListener* listener)
{
    s_listener = listener;
}

UnitTestResult createUnitTestRunner(UnitTestRunnerHandle& outHandle, const char* name)
{
    for (int i = 0; i < MaxUnitTestRunners; ++i)
    {
        if (!s_runners[i].inUse)
        {
            if (s_runnerCount == 0)
            {
                s_suitePool.init(MaxUnitTestSuites);
                s_testPool.init(MaxUnitTestTests);
            }
            ++s_runnerCount;
            s_runners[i].inUse = true;
            s_runners[i].name  = name;
            outHandle = i;
            return UnitTestResult::Success;
        }
    }

    outHandle = InvalidUnitTestRunnerHandle;
    return UnitTestResult::OutOfResources;
}

void destroyUnitTestRunner(UnitTestRunnerHandle handle)
{
    if (handle < 0 || handle >= MaxUnitTestRunners || !s_runners[handle].inUse)
        return;

    freeChildren(s_runners[handle].firstChild);
    memset(&s_runners[handle], 0, sizeof(UnitTestRunner));
    --s_runnerCount;

    if (s_runnerCount == 0)
    {
        s_suitePool.destroy();
        s_testPool.destroy();
    }
}

UnitTestResult createUnitTestSuite(UnitTestSuiteHandle& outHandle, const char* name, UnitTestRunnerHandle runner, UnitTestSuiteHandle parent)
{
    if (runner < 0 || runner >= MaxUnitTestRunners || !s_runners[runner].inUse)
    {
        outHandle = InvalidUnitTestSuiteHandle;
        return UnitTestResult::OutOfResources;
    }

    UnitTestSuiteHandle slot = s_suitePool.alloc();
    if (slot == InvalidFixedItemHandle)
    {
        outHandle = InvalidUnitTestSuiteHandle;
        return UnitTestResult::OutOfResources;
    }

    UnitTestSuite* suite = s_suitePool.getPtr(slot);
    suite->inUse         = true;
    suite->name          = name;
    suite->parent        = parent;
    suite->firstChild    = { NodeType::None, 0 };
    suite->lastChild     = { NodeType::None, 0 };
    suite->nextSibling   = { NodeType::None, 0 };

    ChildRef newRef = { NodeType::Suite, slot };

    UnitTestRunner& r = s_runners[runner];
    if (parent == InvalidUnitTestSuiteHandle)
        appendChildTo(r.firstChild, r.lastChild, newRef);
    else
        appendChildTo(s_suitePool.getPtr(parent)->firstChild, s_suitePool.getPtr(parent)->lastChild, newRef);

    outHandle = slot;
    return UnitTestResult::Success;
}

UnitTestResult createUnitTest(UnitTestHandle& outHandle, const char* name, UnitTestFn fn, UnitTestRunnerHandle runner, UnitTestSuiteHandle parent)
{
    if (runner < 0 || runner >= MaxUnitTestRunners || !s_runners[runner].inUse)
    {
        outHandle = InvalidUnitTestHandle;
        return UnitTestResult::OutOfResources;
    }

    UnitTestHandle slot = s_testPool.alloc();
    if (slot == InvalidFixedItemHandle)
    {
        outHandle = InvalidUnitTestHandle;
        return UnitTestResult::OutOfResources;
    }

    UnitTest* test      = s_testPool.getPtr(slot);
    test->inUse         = true;
    test->name          = name;
    test->fn            = fn;
    test->parent        = parent;
    test->nextSibling   = { NodeType::None, 0 };

    ChildRef newRef = { NodeType::Test, slot };

    UnitTestRunner& r = s_runners[runner];
    if (parent == InvalidUnitTestSuiteHandle)
        appendChildTo(r.firstChild, r.lastChild, newRef);
    else
        appendChildTo(s_suitePool.getPtr(parent)->firstChild, s_suitePool.getPtr(parent)->lastChild, newRef);

    outHandle = slot;
    return UnitTestResult::Success;
}

UnitTestResult runUnitTests(UnitTestRunnerHandle runner)
{
    if (runner < 0 || runner >= MaxUnitTestRunners || !s_runners[runner].inUse)
        return UnitTestResult::InvalidArg;

    UnitTestRunner& r = s_runners[runner];

    if (s_listener && s_listener->onRunnerBegin)
        s_listener->onRunnerBegin(runner);

    runChildren(r.firstChild, runner, InvalidUnitTestSuiteHandle);

    if (s_listener && s_listener->onRunnerEnd)
        s_listener->onRunnerEnd(runner);

    return UnitTestResult::Success;
}

const char* getUnitTestRunnerName(UnitTestRunnerHandle runner)
{
    if (runner < 0 || runner >= MaxUnitTestRunners || !s_runners[runner].inUse)
        return nullptr;
    return s_runners[runner].name;
}

const char* getUnitTestSuiteName(UnitTestSuiteHandle suite)
{
    if (suite < 0 || suite >= MaxUnitTestSuites || s_runnerCount == 0)
        return nullptr;
    UnitTestSuite* s = s_suitePool.getPtr(suite);
    return s->inUse ? s->name : nullptr;
}

const char* getUnitTestName(UnitTestHandle test)
{
    if (test < 0 || test >= MaxUnitTestTests || s_runnerCount == 0)
        return nullptr;
    UnitTest* t = s_testPool.getPtr(test);
    return t->inUse ? t->name : nullptr;
}
