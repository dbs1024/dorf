// Copyright (c) Darrin Stewart. All rights reserved.
#pragma once

enum class UnitTestResult : unsigned
{
	Success        = 0,
	OutOfResources = 1,
	InvalidArg     = 2,
};

struct UnitTestContext;

using UnitTestSuiteHandle = int;
using UnitTestHandle      = int;
using UnitTestFn          = void (*)(UnitTestContext*);

using UnitTestSuiteBeginFn  = void (*)(UnitTestSuiteHandle);
using UnitTestSuiteEndFn    = void (*)(UnitTestSuiteHandle);
using UnitTestBeginFn       = void (*)(UnitTestSuiteHandle, UnitTestHandle);
using UnitTestEndFn         = void (*)(UnitTestSuiteHandle, UnitTestHandle);
using UnitTestAssertFn      = void (*)(UnitTestSuiteHandle, UnitTestHandle, const char* expr, const char* file, int line, int passed);
using UnitTestExceptionFn   = void (*)(UnitTestSuiteHandle, UnitTestHandle, const char* message);
using UnitTestCrashFn       = void (*)(UnitTestSuiteHandle, UnitTestHandle, void* exceptionInfo);

struct UnitTestListener
{
	UnitTestSuiteBeginFn  onSuiteBegin;
	UnitTestSuiteEndFn    onSuiteEnd;
	UnitTestBeginFn       onTestBegin;
	UnitTestEndFn         onTestEnd;
	UnitTestAssertFn      onTestAssert;
	UnitTestExceptionFn   onTestException;
	UnitTestCrashFn       onTestCrash;
};

constexpr UnitTestSuiteHandle InvalidUnitTestSuiteHandle = 0;
constexpr UnitTestHandle      InvalidUnitTestHandle      = 0;

UnitTestResult createUnitTestContext(UnitTestContext** outCtx);
void           destroyUnitTestContext(UnitTestContext* ctx);

void setUnitTestListener(UnitTestContext* ctx, const UnitTestListener* listener);

UnitTestResult createUnitTestSuite(UnitTestSuiteHandle& outHandle, UnitTestContext* ctx, const char* name, UnitTestSuiteHandle parent);

UnitTestResult createUnitTest(UnitTestHandle& outHandle, UnitTestContext* ctx, const char* name, UnitTestFn fn, UnitTestSuiteHandle parent);

UnitTestResult runUnitTests(UnitTestContext* ctx);

void unitTestExpect(UnitTestContext* ctx, int condition, const char* expr, const char* file, int line);

#define UNIT_TEST_EXPECT(ctx, expr) \
	unitTestExpect(ctx, (expr) ? 1 : 0, #expr, __FILE__, __LINE__)

const char* getUnitTestSuiteName(UnitTestContext* ctx, UnitTestSuiteHandle suite);
const char* getUnitTestName(UnitTestContext* ctx, UnitTestHandle test);
