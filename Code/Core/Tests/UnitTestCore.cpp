// Copyright (c) Darrin Stewart. All rights reserved.
#include "Core.UnitTest/UnitTest.h"

void registerArgParserTests(UnitTestContext* ctx);
void registerColorTests(UnitTestContext* ctx);
void registerMathTests(UnitTestContext* ctx);
void registerMiscTests(UnitTestContext* ctx);

extern "C" __declspec(dllexport)
void registerUnitTests(UnitTestContext* ctx)
{
	registerArgParserTests(ctx);
	registerColorTests(ctx);
	registerMathTests(ctx);
	registerMiscTests(ctx);
}
