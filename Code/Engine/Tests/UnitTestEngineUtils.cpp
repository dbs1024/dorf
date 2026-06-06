// Copyright (c) Darrin Stewart. All rights reserved.
#include "Core.UnitTest/UnitTest.h"

void registerXmlDocTests(UnitTestContext* ctx);

extern "C" __declspec(dllexport)
void registerUnitTests(UnitTestContext* ctx)
{
	registerXmlDocTests(ctx);
}
