// Copyright (c) Darrin Stewart. All rights reserved.
#include "Core.UnitTest/UnitTest.h"
#include "Core.Math/Vector3f.h"

static void testVector3fAdd(UnitTestContext* ctx)
{
	Vector3f a = { 1.0f, 2.0f, 3.0f };
	Vector3f b = { 4.0f, 5.0f, 6.0f };
	Vector3f r = a + b;
	UNIT_TEST_EXPECT(ctx, r.x == 5.0f);
	UNIT_TEST_EXPECT(ctx, r.y == 7.0f);
	UNIT_TEST_EXPECT(ctx, r.z == 9.0f);
}

static void testVector3fSubtract(UnitTestContext* ctx)
{
	Vector3f a = { 4.0f, 5.0f, 6.0f };
	Vector3f b = { 1.0f, 2.0f, 3.0f };
	Vector3f r = a - b;
	UNIT_TEST_EXPECT(ctx, r.x == 3.0f);
	UNIT_TEST_EXPECT(ctx, r.y == 3.0f);
	UNIT_TEST_EXPECT(ctx, r.z == 3.0f);
}

static void testVector3fScalarMultiplyRight(UnitTestContext* ctx)
{
	Vector3f a = { 1.0f, 2.0f, 3.0f };
	Vector3f r = a * 2.0f;
	UNIT_TEST_EXPECT(ctx, r.x == 2.0f);
	UNIT_TEST_EXPECT(ctx, r.y == 4.0f);
	UNIT_TEST_EXPECT(ctx, r.z == 6.0f);
}

static void testVector3fScalarMultiplyLeft(UnitTestContext* ctx)
{
	Vector3f a = { 1.0f, 2.0f, 3.0f };
	Vector3f r = 2.0f * a;
	UNIT_TEST_EXPECT(ctx, r.x == 2.0f);
	UNIT_TEST_EXPECT(ctx, r.y == 4.0f);
	UNIT_TEST_EXPECT(ctx, r.z == 6.0f);
}

static void testVector3fAddAssign(UnitTestContext* ctx)
{
	Vector3f a = { 1.0f, 2.0f, 3.0f };
	Vector3f b = { 4.0f, 5.0f, 6.0f };
	a += b;
	UNIT_TEST_EXPECT(ctx, a.x == 5.0f);
	UNIT_TEST_EXPECT(ctx, a.y == 7.0f);
	UNIT_TEST_EXPECT(ctx, a.z == 9.0f);
}

static void testVector3fSubtractAssign(UnitTestContext* ctx)
{
	Vector3f a = { 4.0f, 5.0f, 6.0f };
	Vector3f b = { 1.0f, 2.0f, 3.0f };
	a -= b;
	UNIT_TEST_EXPECT(ctx, a.x == 3.0f);
	UNIT_TEST_EXPECT(ctx, a.y == 3.0f);
	UNIT_TEST_EXPECT(ctx, a.z == 3.0f);
}

static void testVector3fScalarMultiplyAssign(UnitTestContext* ctx)
{
	Vector3f a = { 1.0f, 2.0f, 3.0f };
	a *= 2.0f;
	UNIT_TEST_EXPECT(ctx, a.x == 2.0f);
	UNIT_TEST_EXPECT(ctx, a.y == 4.0f);
	UNIT_TEST_EXPECT(ctx, a.z == 6.0f);
}

void registerMathTests(UnitTestContext* ctx)
{
	UnitTestSuiteHandle mathSuite;
	createUnitTestSuite(mathSuite, ctx, "Math", InvalidUnitTestSuiteHandle);

	UnitTestSuiteHandle vector3fSuite;
	createUnitTestSuite(vector3fSuite, ctx, "Vector3f", mathSuite);

	UnitTestHandle h;
	createUnitTest(h, ctx, "Add",                   testVector3fAdd,                   vector3fSuite);
	createUnitTest(h, ctx, "Subtract",              testVector3fSubtract,              vector3fSuite);
	createUnitTest(h, ctx, "ScalarMultiplyRight",   testVector3fScalarMultiplyRight,   vector3fSuite);
	createUnitTest(h, ctx, "ScalarMultiplyLeft",    testVector3fScalarMultiplyLeft,    vector3fSuite);
	createUnitTest(h, ctx, "AddAssign",             testVector3fAddAssign,             vector3fSuite);
	createUnitTest(h, ctx, "SubtractAssign",        testVector3fSubtractAssign,        vector3fSuite);
	createUnitTest(h, ctx, "ScalarMultiplyAssign",  testVector3fScalarMultiplyAssign,  vector3fSuite);
}
