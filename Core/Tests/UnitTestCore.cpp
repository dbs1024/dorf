// Copyright (c) Darrin Stewart. All rights reserved.
#include "Core.UnitTest/UnitTest.h"
#include "Core.ArgParser/ArgParser.h"

#include <cstring>

// ---- Valid tests (parseArgs returns true) ----

static void testValidEmptyArgs(UnitTestContext* ctx)
{
	ArgParserDesc desc = {};
	ArgParser parser(desc);
	const char* argv[] = { "prog" };
	UNIT_TEST_EXPECT(ctx, parser.parseArgs(1, argv));
	UNIT_TEST_EXPECT(ctx, parser.getErrorMessage() == nullptr);
}

static void testValidStringArg(UnitTestContext* ctx)
{
	ArgDesc argDesc    = {};
	argDesc.name       = "--name";
	argDesc.argType    = ArgParserArgType::String;
	argDesc.numArgs    = static_cast<unsigned>(ArgParserNumArgs::Auto);

	ArgParserDesc desc = {};
	desc.argDescs      = &argDesc;
	desc.argDescCount  = 1;

	ArgParser parser(desc);
	const char* argv[] = { "prog", "--name", "hello" };
	UNIT_TEST_EXPECT(ctx, parser.parseArgs(3, argv));
	UNIT_TEST_EXPECT(ctx, strcmp(parser.getStringValue("name"), "hello") == 0);
}

static void testValidIntArg(UnitTestContext* ctx)
{
	ArgDesc argDesc    = {};
	argDesc.name       = "--count";
	argDesc.argType    = ArgParserArgType::Int;
	argDesc.numArgs    = static_cast<unsigned>(ArgParserNumArgs::Auto);

	ArgParserDesc desc = {};
	desc.argDescs      = &argDesc;
	desc.argDescCount  = 1;

	ArgParser parser(desc);
	const char* argv[] = { "prog", "--count", "42" };
	UNIT_TEST_EXPECT(ctx, parser.parseArgs(3, argv));
	UNIT_TEST_EXPECT(ctx, parser.getIntValue("count") == 42);
}

static void testValidNegativeIntArg(UnitTestContext* ctx)
{
	ArgDesc argDesc    = {};
	argDesc.name       = "--offset";
	argDesc.argType    = ArgParserArgType::Int;
	argDesc.numArgs    = static_cast<unsigned>(ArgParserNumArgs::Auto);

	ArgParserDesc desc = {};
	desc.argDescs      = &argDesc;
	desc.argDescCount  = 1;

	ArgParser parser(desc);
	const char* argv[] = { "prog", "--offset", "-7" };
	UNIT_TEST_EXPECT(ctx, parser.parseArgs(3, argv));
	UNIT_TEST_EXPECT(ctx, parser.getIntValue("offset") == -7);
}

static void testValidFloatArg(UnitTestContext* ctx)
{
	ArgDesc argDesc    = {};
	argDesc.name       = "--rate";
	argDesc.argType    = ArgParserArgType::Float;
	argDesc.numArgs    = static_cast<unsigned>(ArgParserNumArgs::Auto);

	ArgParserDesc desc = {};
	desc.argDescs      = &argDesc;
	desc.argDescCount  = 1;

	ArgParser parser(desc);
	const char* argv[] = { "prog", "--rate", "1.5" };
	UNIT_TEST_EXPECT(ctx, parser.parseArgs(3, argv));
	UNIT_TEST_EXPECT(ctx, parser.getFloatValue("rate") == 1.5f);
}

static void testValidBoolTrueVariants(UnitTestContext* ctx)
{
	const char* trueVariants[] = { "true", "True", "TRUE", "t", "T", "1" };
	for (const char* variant : trueVariants)
	{
		ArgDesc argDesc    = {};
		argDesc.name       = "--flag";
		argDesc.argType    = ArgParserArgType::Bool;
		argDesc.numArgs    = static_cast<unsigned>(ArgParserNumArgs::Auto);

		ArgParserDesc desc = {};
		desc.argDescs      = &argDesc;
		desc.argDescCount  = 1;

		ArgParser parser(desc);
		const char* argv[] = { "prog", "--flag", variant };
		UNIT_TEST_EXPECT(ctx, parser.parseArgs(3, argv));
		UNIT_TEST_EXPECT(ctx, parser.getBoolValue("flag") == true);
	}
}

static void testValidBoolFalseVariants(UnitTestContext* ctx)
{
	const char* falseVariants[] = { "false", "False", "FALSE", "f", "F", "0" };
	for (const char* variant : falseVariants)
	{
		ArgDesc argDesc    = {};
		argDesc.name       = "--flag";
		argDesc.argType    = ArgParserArgType::Bool;
		argDesc.numArgs    = static_cast<unsigned>(ArgParserNumArgs::Auto);

		ArgParserDesc desc = {};
		desc.argDescs      = &argDesc;
		desc.argDescCount  = 1;

		ArgParser parser(desc);
		const char* argv[] = { "prog", "--flag", variant };
		UNIT_TEST_EXPECT(ctx, parser.parseArgs(3, argv));
		UNIT_TEST_EXPECT(ctx, parser.getBoolValue("flag") == false);
	}
}

static void testValidShortFlag(UnitTestContext* ctx)
{
	ArgDesc argDesc    = {};
	argDesc.name       = "--name";
	argDesc.flag       = 'n';
	argDesc.argType    = ArgParserArgType::String;
	argDesc.numArgs    = static_cast<unsigned>(ArgParserNumArgs::Auto);

	ArgParserDesc desc = {};
	desc.argDescs      = &argDesc;
	desc.argDescCount  = 1;

	ArgParser parser(desc);
	const char* argv[] = { "prog", "-n", "hello" };
	UNIT_TEST_EXPECT(ctx, parser.parseArgs(3, argv));
	UNIT_TEST_EXPECT(ctx, strcmp(parser.getStringValue("name"), "hello") == 0);
}

static void testValidStoreTrue(UnitTestContext* ctx)
{
	ArgDesc argDesc    = {};
	argDesc.name       = "--verbose";
	argDesc.action     = ArgParserAction::StoreTrue;
	argDesc.numArgs    = static_cast<unsigned>(ArgParserNumArgs::Auto);

	ArgParserDesc desc = {};
	desc.argDescs      = &argDesc;
	desc.argDescCount  = 1;

	ArgParser parser(desc);
	const char* argv[] = { "prog", "--verbose" };
	UNIT_TEST_EXPECT(ctx, parser.parseArgs(2, argv));
	UNIT_TEST_EXPECT(ctx, parser.getBoolValue("verbose") == true);
}

static void testValidStoreFalse(UnitTestContext* ctx)
{
	ArgDesc argDesc    = {};
	argDesc.name       = "--no-verbose";
	argDesc.action     = ArgParserAction::StoreFalse;
	argDesc.numArgs    = static_cast<unsigned>(ArgParserNumArgs::Auto);

	ArgParserDesc desc = {};
	desc.argDescs      = &argDesc;
	desc.argDescCount  = 1;

	ArgParser parser(desc);
	const char* argv[] = { "prog", "--no-verbose" };
	UNIT_TEST_EXPECT(ctx, parser.parseArgs(2, argv));
	UNIT_TEST_EXPECT(ctx, parser.getBoolValue("no-verbose") == false);
}

static void testValidCount(UnitTestContext* ctx)
{
	ArgDesc argDesc    = {};
	argDesc.name       = "--verbose";
	argDesc.action     = ArgParserAction::Count;
	argDesc.numArgs    = static_cast<unsigned>(ArgParserNumArgs::Auto);

	ArgParserDesc desc = {};
	desc.argDescs      = &argDesc;
	desc.argDescCount  = 1;

	ArgParser parser(desc);
	const char* argv[] = { "prog", "--verbose", "--verbose", "--verbose" };
	UNIT_TEST_EXPECT(ctx, parser.parseArgs(4, argv));
	UNIT_TEST_EXPECT(ctx, parser.getIntValue("verbose") == 3);
}

static void testValidAppend(UnitTestContext* ctx)
{
	ArgDesc argDesc    = {};
	argDesc.name       = "--file";
	argDesc.action     = ArgParserAction::Append;
	argDesc.argType    = ArgParserArgType::String;
	argDesc.numArgs    = static_cast<unsigned>(ArgParserNumArgs::Auto);

	ArgParserDesc desc = {};
	desc.argDescs      = &argDesc;
	desc.argDescCount  = 1;

	ArgParser parser(desc);
	const char* argv[] = { "prog", "--file", "a.txt", "--file", "b.txt" };
	UNIT_TEST_EXPECT(ctx, parser.parseArgs(5, argv));
	UNIT_TEST_EXPECT(ctx, parser.getValueCount("file") == 2);
	UNIT_TEST_EXPECT(ctx, strcmp(parser.getStringValue("file", 0), "a.txt") == 0);
	UNIT_TEST_EXPECT(ctx, strcmp(parser.getStringValue("file", 1), "b.txt") == 0);
}

static void testValidDefault(UnitTestContext* ctx)
{
	ArgDesc argDesc       = {};
	argDesc.name          = "--output";
	argDesc.argType       = ArgParserArgType::String;
	argDesc.numArgs       = static_cast<unsigned>(ArgParserNumArgs::Auto);
	argDesc.defaultValue  = "out.txt";

	ArgParserDesc desc = {};
	desc.argDescs      = &argDesc;
	desc.argDescCount  = 1;

	ArgParser parser(desc);
	const char* argv[] = { "prog" };
	UNIT_TEST_EXPECT(ctx, parser.parseArgs(1, argv));
	UNIT_TEST_EXPECT(ctx, strcmp(parser.getStringValue("output"), "out.txt") == 0);
}

static void testValidPositional(UnitTestContext* ctx)
{
	ArgDesc argDesc    = {};
	argDesc.name       = "input";
	argDesc.argType    = ArgParserArgType::String;
	argDesc.numArgs    = 1;

	ArgParserDesc desc = {};
	desc.argDescs      = &argDesc;
	desc.argDescCount  = 1;

	ArgParser parser(desc);
	const char* argv[] = { "prog", "file.txt" };
	UNIT_TEST_EXPECT(ctx, parser.parseArgs(2, argv));
	UNIT_TEST_EXPECT(ctx, strcmp(parser.getStringValue("input"), "file.txt") == 0);
}

static void testValidChoices(UnitTestContext* ctx)
{
	const char* choices[] = { "fast", "slow", nullptr };

	ArgDesc argDesc    = {};
	argDesc.name       = "--mode";
	argDesc.argType    = ArgParserArgType::String;
	argDesc.numArgs    = static_cast<unsigned>(ArgParserNumArgs::Auto);
	argDesc.choices    = choices;

	ArgParserDesc desc = {};
	desc.argDescs      = &argDesc;
	desc.argDescCount  = 1;

	ArgParser parser(desc);
	const char* argv[] = { "prog", "--mode", "fast" };
	UNIT_TEST_EXPECT(ctx, parser.parseArgs(3, argv));
	UNIT_TEST_EXPECT(ctx, strcmp(parser.getStringValue("mode"), "fast") == 0);
}

static void testValidHelp(UnitTestContext* ctx)
{
	ArgParserDesc desc = {};
	ArgParser parser(desc);
	const char* argv[] = { "prog", "--help" };
	UNIT_TEST_EXPECT(ctx, parser.parseArgs(2, argv));
	UNIT_TEST_EXPECT(ctx, parser.shouldPrintHelp());
}

static void testValidZeroOrOnePresent(UnitTestContext* ctx)
{
	ArgDesc argDesc       = {};
	argDesc.name          = "--output";
	argDesc.argType       = ArgParserArgType::String;
	argDesc.numArgs       = static_cast<unsigned>(ArgParserNumArgs::ZeroOrOne);
	argDesc.defaultValue  = "default.txt";

	ArgParserDesc desc = {};
	desc.argDescs      = &argDesc;
	desc.argDescCount  = 1;

	ArgParser parser(desc);
	const char* argv[] = { "prog", "--output", "custom.txt" };
	UNIT_TEST_EXPECT(ctx, parser.parseArgs(3, argv));
	UNIT_TEST_EXPECT(ctx, strcmp(parser.getStringValue("output"), "custom.txt") == 0);
}

static void testValidZeroOrOneAbsent(UnitTestContext* ctx)
{
	ArgDesc argDesc       = {};
	argDesc.name          = "--output";
	argDesc.argType       = ArgParserArgType::String;
	argDesc.numArgs       = static_cast<unsigned>(ArgParserNumArgs::ZeroOrOne);
	argDesc.defaultValue  = "default.txt";

	ArgParserDesc desc = {};
	desc.argDescs      = &argDesc;
	desc.argDescCount  = 1;

	ArgParser parser(desc);
	const char* argv[] = { "prog" };
	UNIT_TEST_EXPECT(ctx, parser.parseArgs(1, argv));
	UNIT_TEST_EXPECT(ctx, strcmp(parser.getStringValue("output"), "default.txt") == 0);
}

static void testValidOneOrMore(UnitTestContext* ctx)
{
	ArgDesc argDesc    = {};
	argDesc.name       = "--file";
	argDesc.action     = ArgParserAction::Append;
	argDesc.argType    = ArgParserArgType::String;
	argDesc.numArgs    = static_cast<unsigned>(ArgParserNumArgs::OneOrMore);

	ArgParserDesc desc = {};
	desc.argDescs      = &argDesc;
	desc.argDescCount  = 1;

	ArgParser parser(desc);
	const char* argv[] = { "prog", "--file", "a.txt", "b.txt", "c.txt" };
	UNIT_TEST_EXPECT(ctx, parser.parseArgs(5, argv));
	UNIT_TEST_EXPECT(ctx, parser.getValueCount("file") == 3);
}

// ---- Invalid tests (parseArgs returns false) ----

static void testInvalidUnrecognizedLongArg(UnitTestContext* ctx)
{
	ArgParserDesc desc = {};
	ArgParser parser(desc);
	const char* argv[] = { "prog", "--unknown" };
	UNIT_TEST_EXPECT(ctx, !parser.parseArgs(2, argv));
	UNIT_TEST_EXPECT(ctx, parser.getErrorMessage() != nullptr);
}

static void testInvalidUnrecognizedFlag(UnitTestContext* ctx)
{
	ArgParserDesc desc = {};
	ArgParser parser(desc);
	const char* argv[] = { "prog", "-z" };
	UNIT_TEST_EXPECT(ctx, !parser.parseArgs(2, argv));
	UNIT_TEST_EXPECT(ctx, parser.getErrorMessage() != nullptr);
}

static void testInvalidMissingRequired(UnitTestContext* ctx)
{
	ArgDesc argDesc    = {};
	argDesc.name       = "--name";
	argDesc.argType    = ArgParserArgType::String;
	argDesc.numArgs    = static_cast<unsigned>(ArgParserNumArgs::Auto);
	argDesc.required   = true;

	ArgParserDesc desc = {};
	desc.argDescs      = &argDesc;
	desc.argDescCount  = 1;

	ArgParser parser(desc);
	const char* argv[] = { "prog" };
	UNIT_TEST_EXPECT(ctx, !parser.parseArgs(1, argv));
	UNIT_TEST_EXPECT(ctx, parser.getErrorMessage() != nullptr);
}

static void testInvalidMissingPositional(UnitTestContext* ctx)
{
	ArgDesc argDesc    = {};
	argDesc.name       = "input";
	argDesc.argType    = ArgParserArgType::String;
	argDesc.numArgs    = 1;

	ArgParserDesc desc = {};
	desc.argDescs      = &argDesc;
	desc.argDescCount  = 1;

	ArgParser parser(desc);
	const char* argv[] = { "prog" };
	UNIT_TEST_EXPECT(ctx, !parser.parseArgs(1, argv));
	UNIT_TEST_EXPECT(ctx, parser.getErrorMessage() != nullptr);
}

static void testInvalidUnexpectedPositional(UnitTestContext* ctx)
{
	ArgParserDesc desc = {};
	ArgParser parser(desc);
	const char* argv[] = { "prog", "unexpected" };
	UNIT_TEST_EXPECT(ctx, !parser.parseArgs(2, argv));
	UNIT_TEST_EXPECT(ctx, parser.getErrorMessage() != nullptr);
}

static void testInvalidIntTypeMismatch(UnitTestContext* ctx)
{
	ArgDesc argDesc    = {};
	argDesc.name       = "--count";
	argDesc.argType    = ArgParserArgType::Int;
	argDesc.numArgs    = static_cast<unsigned>(ArgParserNumArgs::Auto);

	ArgParserDesc desc = {};
	desc.argDescs      = &argDesc;
	desc.argDescCount  = 1;

	ArgParser parser(desc);
	const char* argv[] = { "prog", "--count", "abc" };
	UNIT_TEST_EXPECT(ctx, !parser.parseArgs(3, argv));
	UNIT_TEST_EXPECT(ctx, parser.getErrorMessage() != nullptr);
}

static void testInvalidIntWithTrailingChars(UnitTestContext* ctx)
{
	ArgDesc argDesc    = {};
	argDesc.name       = "--count";
	argDesc.argType    = ArgParserArgType::Int;
	argDesc.numArgs    = static_cast<unsigned>(ArgParserNumArgs::Auto);

	ArgParserDesc desc = {};
	desc.argDescs      = &argDesc;
	desc.argDescCount  = 1;

	ArgParser parser(desc);
	const char* argv[] = { "prog", "--count", "42abc" };
	UNIT_TEST_EXPECT(ctx, !parser.parseArgs(3, argv));
	UNIT_TEST_EXPECT(ctx, parser.getErrorMessage() != nullptr);
}

static void testInvalidFloatTypeMismatch(UnitTestContext* ctx)
{
	ArgDesc argDesc    = {};
	argDesc.name       = "--rate";
	argDesc.argType    = ArgParserArgType::Float;
	argDesc.numArgs    = static_cast<unsigned>(ArgParserNumArgs::Auto);

	ArgParserDesc desc = {};
	desc.argDescs      = &argDesc;
	desc.argDescCount  = 1;

	ArgParser parser(desc);
	const char* argv[] = { "prog", "--rate", "xyz" };
	UNIT_TEST_EXPECT(ctx, !parser.parseArgs(3, argv));
	UNIT_TEST_EXPECT(ctx, parser.getErrorMessage() != nullptr);
}

static void testInvalidBoolTypeMismatch(UnitTestContext* ctx)
{
	ArgDesc argDesc    = {};
	argDesc.name       = "--flag";
	argDesc.argType    = ArgParserArgType::Bool;
	argDesc.numArgs    = static_cast<unsigned>(ArgParserNumArgs::Auto);

	ArgParserDesc desc = {};
	desc.argDescs      = &argDesc;
	desc.argDescCount  = 1;

	ArgParser parser(desc);
	const char* argv[] = { "prog", "--flag", "maybe" };
	UNIT_TEST_EXPECT(ctx, !parser.parseArgs(3, argv));
	UNIT_TEST_EXPECT(ctx, parser.getErrorMessage() != nullptr);
}

static void testInvalidChoice(UnitTestContext* ctx)
{
	const char* choices[] = { "fast", "slow", nullptr };

	ArgDesc argDesc    = {};
	argDesc.name       = "--mode";
	argDesc.argType    = ArgParserArgType::String;
	argDesc.numArgs    = static_cast<unsigned>(ArgParserNumArgs::Auto);
	argDesc.choices    = choices;

	ArgParserDesc desc = {};
	desc.argDescs      = &argDesc;
	desc.argDescCount  = 1;

	ArgParser parser(desc);
	const char* argv[] = { "prog", "--mode", "turbo" };
	UNIT_TEST_EXPECT(ctx, !parser.parseArgs(3, argv));
	UNIT_TEST_EXPECT(ctx, parser.getErrorMessage() != nullptr);
}

static void testInvalidTooFewValues(UnitTestContext* ctx)
{
	ArgDesc argDesc    = {};
	argDesc.name       = "--point";
	argDesc.action     = ArgParserAction::Append;
	argDesc.argType    = ArgParserArgType::Float;
	argDesc.numArgs    = 2;

	ArgParserDesc desc = {};
	desc.argDescs      = &argDesc;
	desc.argDescCount  = 1;

	ArgParser parser(desc);
	const char* argv[] = { "prog", "--point", "1.0" };
	UNIT_TEST_EXPECT(ctx, !parser.parseArgs(3, argv));
	UNIT_TEST_EXPECT(ctx, parser.getErrorMessage() != nullptr);
}

static void testInvalidOneOrMoreEmpty(UnitTestContext* ctx)
{
	ArgDesc argDesc    = {};
	argDesc.name       = "--files";
	argDesc.action     = ArgParserAction::Append;
	argDesc.argType    = ArgParserArgType::String;
	argDesc.numArgs    = static_cast<unsigned>(ArgParserNumArgs::OneOrMore);

	ArgParserDesc desc = {};
	desc.argDescs      = &argDesc;
	desc.argDescCount  = 1;

	ArgParser parser(desc);
	const char* argv[] = { "prog", "--files" };
	UNIT_TEST_EXPECT(ctx, !parser.parseArgs(2, argv));
	UNIT_TEST_EXPECT(ctx, parser.getErrorMessage() != nullptr);
}

extern "C" __declspec(dllexport)
void registerUnitTests(UnitTestContext* ctx)
{
	UnitTestSuiteHandle argParserSuite;
	createUnitTestSuite(argParserSuite, ctx, "ArgParser", InvalidUnitTestSuiteHandle);

	UnitTestSuiteHandle validSuite;
	createUnitTestSuite(validSuite, ctx, "Valid", argParserSuite);

	UnitTestHandle h;
	createUnitTest(h, ctx, "EmptyArgs",          testValidEmptyArgs,          validSuite);
	createUnitTest(h, ctx, "StringArg",          testValidStringArg,          validSuite);
	createUnitTest(h, ctx, "IntArg",             testValidIntArg,             validSuite);
	createUnitTest(h, ctx, "NegativeIntArg",     testValidNegativeIntArg,     validSuite);
	createUnitTest(h, ctx, "FloatArg",           testValidFloatArg,           validSuite);
	createUnitTest(h, ctx, "BoolTrueVariants",   testValidBoolTrueVariants,   validSuite);
	createUnitTest(h, ctx, "BoolFalseVariants",  testValidBoolFalseVariants,  validSuite);
	createUnitTest(h, ctx, "ShortFlag",          testValidShortFlag,          validSuite);
	createUnitTest(h, ctx, "StoreTrue",          testValidStoreTrue,          validSuite);
	createUnitTest(h, ctx, "StoreFalse",         testValidStoreFalse,         validSuite);
	createUnitTest(h, ctx, "Count",              testValidCount,              validSuite);
	createUnitTest(h, ctx, "Append",             testValidAppend,             validSuite);
	createUnitTest(h, ctx, "Default",            testValidDefault,            validSuite);
	createUnitTest(h, ctx, "Positional",         testValidPositional,         validSuite);
	createUnitTest(h, ctx, "Choices",            testValidChoices,            validSuite);
	createUnitTest(h, ctx, "Help",               testValidHelp,               validSuite);
	createUnitTest(h, ctx, "ZeroOrOnePresent",   testValidZeroOrOnePresent,   validSuite);
	createUnitTest(h, ctx, "ZeroOrOneAbsent",    testValidZeroOrOneAbsent,    validSuite);
	createUnitTest(h, ctx, "OneOrMore",          testValidOneOrMore,          validSuite);

	UnitTestSuiteHandle invalidSuite;
	createUnitTestSuite(invalidSuite, ctx, "Invalid", argParserSuite);

	createUnitTest(h, ctx, "UnrecognizedLongArg",   testInvalidUnrecognizedLongArg,   invalidSuite);
	createUnitTest(h, ctx, "UnrecognizedFlag",       testInvalidUnrecognizedFlag,       invalidSuite);
	createUnitTest(h, ctx, "MissingRequired",        testInvalidMissingRequired,        invalidSuite);
	createUnitTest(h, ctx, "MissingPositional",      testInvalidMissingPositional,      invalidSuite);
	createUnitTest(h, ctx, "UnexpectedPositional",   testInvalidUnexpectedPositional,   invalidSuite);
	createUnitTest(h, ctx, "IntTypeMismatch",        testInvalidIntTypeMismatch,        invalidSuite);
	createUnitTest(h, ctx, "IntWithTrailingChars",   testInvalidIntWithTrailingChars,   invalidSuite);
	createUnitTest(h, ctx, "FloatTypeMismatch",      testInvalidFloatTypeMismatch,      invalidSuite);
	createUnitTest(h, ctx, "BoolTypeMismatch",       testInvalidBoolTypeMismatch,       invalidSuite);
	createUnitTest(h, ctx, "InvalidChoice",          testInvalidChoice,                 invalidSuite);
	createUnitTest(h, ctx, "TooFewValues",           testInvalidTooFewValues,           invalidSuite);
	createUnitTest(h, ctx, "OneOrMoreEmpty",         testInvalidOneOrMoreEmpty,         invalidSuite);
}
