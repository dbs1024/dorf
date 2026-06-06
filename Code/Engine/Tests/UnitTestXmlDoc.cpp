// Copyright (c) Darrin Stewart. All rights reserved.
#include "Core.UnitTest/UnitTest.h"
#include "Engine.Utils.XmlDoc/XmlDoc.h"

// ---- Valid ----

static void testValidSimpleElement(UnitTestContext* ctx)
{
	XmlDocHandle doc;
	UNIT_TEST_EXPECT(ctx, loadXmlString(&doc, "<root/>") == XmlResult::Ok);
	destroyXmlDoc(doc);
}

static void testValidElementWithContent(UnitTestContext* ctx)
{
	XmlDocHandle doc;
	UNIT_TEST_EXPECT(ctx, loadXmlString(&doc, "<root>text content</root>") == XmlResult::Ok);
	destroyXmlDoc(doc);
}

static void testValidNestedElements(UnitTestContext* ctx)
{
	XmlDocHandle doc;
	UNIT_TEST_EXPECT(ctx, loadXmlString(&doc, "<root><child/><child>text</child></root>") == XmlResult::Ok);
	destroyXmlDoc(doc);
}

static void testValidAttributes(UnitTestContext* ctx)
{
	XmlDocHandle doc;
	UNIT_TEST_EXPECT(ctx, loadXmlString(&doc, "<root id=\"1\" name='foo'/>") == XmlResult::Ok);
	destroyXmlDoc(doc);
}

static void testValidXmlDeclaration(UnitTestContext* ctx)
{
	XmlDocHandle doc;
	UNIT_TEST_EXPECT(ctx, loadXmlString(&doc, "<?xml version=\"1.0\" encoding=\"UTF-8\"?><root/>") == XmlResult::Ok);
	destroyXmlDoc(doc);
}

static void testValidComment(UnitTestContext* ctx)
{
	XmlDocHandle doc;
	UNIT_TEST_EXPECT(ctx, loadXmlString(&doc, "<!-- header --><root><!-- body --></root>") == XmlResult::Ok);
	destroyXmlDoc(doc);
}

static void testValidCdata(UnitTestContext* ctx)
{
	XmlDocHandle doc;
	UNIT_TEST_EXPECT(ctx, loadXmlString(&doc, "<root><![CDATA[<not a tag>]]></root>") == XmlResult::Ok);
	destroyXmlDoc(doc);
}

static void testValidEntityRef(UnitTestContext* ctx)
{
	XmlDocHandle doc;
	UNIT_TEST_EXPECT(ctx, loadXmlString(&doc, "<root>&amp;&lt;&gt;</root>") == XmlResult::Ok);
	destroyXmlDoc(doc);
}

// ---- Invalid ----

static void testInvalidEmpty(UnitTestContext* ctx)
{
	XmlDocHandle doc;
	UNIT_TEST_EXPECT(ctx, loadXmlString(&doc, "") == XmlResult::ParseError);
}

static void testInvalidUnclosedTag(UnitTestContext* ctx)
{
	XmlDocHandle doc;
	UNIT_TEST_EXPECT(ctx, loadXmlString(&doc, "<root>") == XmlResult::ParseError);
}

static void testInvalidMismatchedTags(UnitTestContext* ctx)
{
	XmlDocHandle doc;
	UNIT_TEST_EXPECT(ctx, loadXmlString(&doc, "<root></other>") == XmlResult::ParseError);
}

static void testInvalidMultipleRoots(UnitTestContext* ctx)
{
	XmlDocHandle doc;
	UNIT_TEST_EXPECT(ctx, loadXmlString(&doc, "<a/><b/>") == XmlResult::ParseError);
}

static void testInvalidTextBeforeRoot(UnitTestContext* ctx)
{
	XmlDocHandle doc;
	UNIT_TEST_EXPECT(ctx, loadXmlString(&doc, "text<root/>") == XmlResult::ParseError);
}

static void testInvalidUnclosedTagBracket(UnitTestContext* ctx)
{
	XmlDocHandle doc;
	UNIT_TEST_EXPECT(ctx, loadXmlString(&doc, "<root") == XmlResult::ParseError);
}

static void testInvalidUnclosedAttrValue(UnitTestContext* ctx)
{
	XmlDocHandle doc;
	UNIT_TEST_EXPECT(ctx, loadXmlString(&doc, "<root attr=\"unclosed/>") == XmlResult::ParseError);
}

static void testInvalidUnclosedComment(UnitTestContext* ctx)
{
	XmlDocHandle doc;
	UNIT_TEST_EXPECT(ctx, loadXmlString(&doc, "<root><!-- unterminated</root>") == XmlResult::ParseError);
}

static void testInvalidDoubledDashInComment(UnitTestContext* ctx)
{
	XmlDocHandle doc;
	UNIT_TEST_EXPECT(ctx, loadXmlString(&doc, "<root><!-- bad -- comment --></root>") == XmlResult::ParseError);
}

static void testInvalidCdataOutsideElement(UnitTestContext* ctx)
{
	XmlDocHandle doc;
	UNIT_TEST_EXPECT(ctx, loadXmlString(&doc, "<![CDATA[data]]><root/>") == XmlResult::ParseError);
}

void registerXmlDocTests(UnitTestContext* ctx)
{
	UnitTestSuiteHandle xmlDocSuite;
	createUnitTestSuite(xmlDocSuite, ctx, "XmlDoc", InvalidUnitTestSuiteHandle);

	UnitTestSuiteHandle validSuite;
	createUnitTestSuite(validSuite, ctx, "Valid", xmlDocSuite);

	UnitTestHandle h;
	createUnitTest(h, ctx, "SimpleElement",    testValidSimpleElement,    validSuite);
	createUnitTest(h, ctx, "ElementWithContent", testValidElementWithContent, validSuite);
	createUnitTest(h, ctx, "NestedElements",   testValidNestedElements,   validSuite);
	createUnitTest(h, ctx, "Attributes",       testValidAttributes,       validSuite);
	createUnitTest(h, ctx, "XmlDeclaration",   testValidXmlDeclaration,   validSuite);
	createUnitTest(h, ctx, "Comment",          testValidComment,          validSuite);
	createUnitTest(h, ctx, "Cdata",            testValidCdata,            validSuite);
	createUnitTest(h, ctx, "EntityRef",        testValidEntityRef,        validSuite);

	UnitTestSuiteHandle invalidSuite;
	createUnitTestSuite(invalidSuite, ctx, "Invalid", xmlDocSuite);

	createUnitTest(h, ctx, "Empty",                testInvalidEmpty,                invalidSuite);
	createUnitTest(h, ctx, "UnclosedTag",          testInvalidUnclosedTag,          invalidSuite);
	createUnitTest(h, ctx, "MismatchedTags",       testInvalidMismatchedTags,       invalidSuite);
	createUnitTest(h, ctx, "MultipleRoots",        testInvalidMultipleRoots,        invalidSuite);
	createUnitTest(h, ctx, "TextBeforeRoot",       testInvalidTextBeforeRoot,       invalidSuite);
	createUnitTest(h, ctx, "UnclosedTagBracket",   testInvalidUnclosedTagBracket,   invalidSuite);
	createUnitTest(h, ctx, "UnclosedAttrValue",    testInvalidUnclosedAttrValue,    invalidSuite);
	createUnitTest(h, ctx, "UnclosedComment",      testInvalidUnclosedComment,      invalidSuite);
	createUnitTest(h, ctx, "DoubledDashInComment", testInvalidDoubledDashInComment, invalidSuite);
	createUnitTest(h, ctx, "CdataOutsideElement",  testInvalidCdataOutsideElement,  invalidSuite);
}
