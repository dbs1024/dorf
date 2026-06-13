// Copyright (c) Darrin Stewart. All rights reserved.
#include "Core.UnitTest/UnitTest.h"
#include "Core.Base/Misc.h"

static void testAlignUpAlreadyAligned(UnitTestContext* ctx)
{
	UNIT_TEST_EXPECT(ctx, alignUp<size_t>(16, 16) == 16);
}

static void testAlignUpRoundsUp(UnitTestContext* ctx)
{
	UNIT_TEST_EXPECT(ctx, alignUp<size_t>(17, 16) == 32);
}

static void testAlignUpZero(UnitTestContext* ctx)
{
	UNIT_TEST_EXPECT(ctx, alignUp<size_t>(0, 16) == 0);
}

static void testClampWithinRange(UnitTestContext* ctx)
{
	UNIT_TEST_EXPECT(ctx, clamp(5, 1, 10) == 5);
}

static void testClampBelowMin(UnitTestContext* ctx)
{
	UNIT_TEST_EXPECT(ctx, clamp(0, 1, 10) == 1);
}

static void testClampAboveMax(UnitTestContext* ctx)
{
	UNIT_TEST_EXPECT(ctx, clamp(11, 1, 10) == 10);
}

void registerMiscTests(UnitTestContext* ctx)
{
	UnitTestSuiteHandle miscSuite;
	createUnitTestSuite(miscSuite, ctx, "Misc", InvalidUnitTestSuiteHandle);

	UnitTestSuiteHandle alignUpSuite;
	createUnitTestSuite(alignUpSuite, ctx, "AlignUp", miscSuite);

	UnitTestHandle h;
	createUnitTest(h, ctx, "AlreadyAligned", testAlignUpAlreadyAligned, alignUpSuite);
	createUnitTest(h, ctx, "RoundsUp",       testAlignUpRoundsUp,       alignUpSuite);
	createUnitTest(h, ctx, "Zero",           testAlignUpZero,           alignUpSuite);

	UnitTestSuiteHandle clampSuite;
	createUnitTestSuite(clampSuite, ctx, "Clamp", miscSuite);

	createUnitTest(h, ctx, "WithinRange", testClampWithinRange, clampSuite);
	createUnitTest(h, ctx, "BelowMin",    testClampBelowMin,    clampSuite);
	createUnitTest(h, ctx, "AboveMax",    testClampAboveMax,    clampSuite);
}
