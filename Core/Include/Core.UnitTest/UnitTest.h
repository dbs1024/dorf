// Copyright (c) Darrin Stewart. All rights reserved.
#pragma once

enum class UnitTestResult : unsigned
{
    Success = 0,
    OutOfResources = 1,
    InvalidArg = 2,
};

using UnitTestRunnerHandle = int;
using UnitTestSuiteHandle  = int;
using UnitTestHandle       = int;
using UnitTestFn           = void (*)();

using UnitTestRunnerBeginFn = void (*)(UnitTestRunnerHandle);
using UnitTestRunnerEndFn   = void (*)(UnitTestRunnerHandle);
using UnitTestSuiteBeginFn  = void (*)(UnitTestRunnerHandle, UnitTestSuiteHandle);
using UnitTestSuiteEndFn    = void (*)(UnitTestRunnerHandle, UnitTestSuiteHandle);
using UnitTestBeginFn       = void (*)(UnitTestRunnerHandle, UnitTestSuiteHandle, UnitTestHandle);
using UnitTestEndFn         = void (*)(UnitTestRunnerHandle, UnitTestSuiteHandle, UnitTestHandle);

struct UnitTestListener
{
    UnitTestRunnerBeginFn onRunnerBegin;
    UnitTestRunnerEndFn   onRunnerEnd;
    UnitTestSuiteBeginFn  onSuiteBegin;
    UnitTestSuiteEndFn    onSuiteEnd;
    UnitTestBeginFn       onTestBegin;
    UnitTestEndFn         onTestEnd;
};

constexpr UnitTestRunnerHandle InvalidUnitTestRunnerHandle = -1;
constexpr UnitTestSuiteHandle  InvalidUnitTestSuiteHandle  = -1;
constexpr UnitTestHandle       InvalidUnitTestHandle       = -1;

void setUnitTestListener(const UnitTestListener* listener);

UnitTestResult createUnitTestRunner(UnitTestRunnerHandle& outHandle, const char* name);
void           destroyUnitTestRunner(UnitTestRunnerHandle handle);

UnitTestResult createUnitTestSuite(UnitTestSuiteHandle& outHandle, const char* name, UnitTestRunnerHandle runner, UnitTestSuiteHandle parent);

UnitTestResult createUnitTest(UnitTestHandle& outHandle, const char* name, UnitTestFn fn, UnitTestRunnerHandle runner, UnitTestSuiteHandle parent);

UnitTestResult runUnitTests(UnitTestRunnerHandle runner);

const char* getUnitTestRunnerName(UnitTestRunnerHandle runner);
const char* getUnitTestSuiteName(UnitTestSuiteHandle suite);
const char* getUnitTestName(UnitTestHandle test);
