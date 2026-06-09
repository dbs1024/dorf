// Copyright (c) Darrin Stewart. All rights reserved.
#include "Core.UnitTest/UnitTest.h"
#include "Core.Color/Color.h"

static void testColorToVector4f(UnitTestContext* ctx)
{
	Color c = { 255, 0, 0, 255 };
	Vector4f v = colorToVector4f(c);
	UNIT_TEST_EXPECT(ctx, v.x == 1.0f);
	UNIT_TEST_EXPECT(ctx, v.y == 0.0f);
	UNIT_TEST_EXPECT(ctx, v.z == 0.0f);
	UNIT_TEST_EXPECT(ctx, v.w == 1.0f);
}

static void testColorFromVector4f(UnitTestContext* ctx)
{
	Vector4f v = { 1.0f, 0.0f, 0.0f, 1.0f };
	Color c = colorFromVector4f(v);
	UNIT_TEST_EXPECT(ctx, c.r == 255);
	UNIT_TEST_EXPECT(ctx, c.g == 0);
	UNIT_TEST_EXPECT(ctx, c.b == 0);
	UNIT_TEST_EXPECT(ctx, c.a == 255);
}

static void testColorRoundTrip(UnitTestContext* ctx)
{
	Color original = { 128, 64, 32, 255 };
	Color result = colorFromVector4f(colorToVector4f(original));
	UNIT_TEST_EXPECT(ctx, result.r == original.r);
	UNIT_TEST_EXPECT(ctx, result.g == original.g);
	UNIT_TEST_EXPECT(ctx, result.b == original.b);
	UNIT_TEST_EXPECT(ctx, result.a == original.a);
}

void registerColorTests(UnitTestContext* ctx)
{
	UnitTestSuiteHandle colorSuite;
	createUnitTestSuite(colorSuite, ctx, "Color", InvalidUnitTestSuiteHandle);

	UnitTestHandle h;
	createUnitTest(h, ctx, "ColorToVector4f",   testColorToVector4f,   colorSuite);
	createUnitTest(h, ctx, "ColorFromVector4f", testColorFromVector4f, colorSuite);
	createUnitTest(h, ctx, "RoundTrip",         testColorRoundTrip,    colorSuite);
}
