// Copyright (c) Darrin Stewart. All rights reserved.
#include "Core.UnitTest/UnitTest.h"
#include "Engine.Utils.XmlDoc/XmlDoc.h"

#include <cstring>

// ---- Valid ----

static void testValidSimpleElement(UnitTestContext* ctx)
{
	XmlDocument* doc;
	UNIT_TEST_EXPECT(ctx, loadXmlString(&doc, "<root/>") == XmlResult::Ok);
	destroyXmlDoc(doc);
}

static void testValidElementWithContent(UnitTestContext* ctx)
{
	XmlDocument* doc;
	UNIT_TEST_EXPECT(ctx, loadXmlString(&doc, "<root>text content</root>") == XmlResult::Ok);
	destroyXmlDoc(doc);
}

static void testValidNestedElements(UnitTestContext* ctx)
{
	XmlDocument* doc;
	UNIT_TEST_EXPECT(ctx, loadXmlString(&doc, "<root><child/><child>text</child></root>") == XmlResult::Ok);
	destroyXmlDoc(doc);
}

static void testValidAttributes(UnitTestContext* ctx)
{
	XmlDocument* doc;
	UNIT_TEST_EXPECT(ctx, loadXmlString(&doc, "<root id=\"1\" name='foo'/>") == XmlResult::Ok);
	destroyXmlDoc(doc);
}

static void testValidXmlDeclaration(UnitTestContext* ctx)
{
	XmlDocument* doc;
	UNIT_TEST_EXPECT(ctx, loadXmlString(&doc, "<?xml version=\"1.0\" encoding=\"UTF-8\"?><root/>") == XmlResult::Ok);
	destroyXmlDoc(doc);
}

static void testValidComment(UnitTestContext* ctx)
{
	XmlDocument* doc;
	UNIT_TEST_EXPECT(ctx, loadXmlString(&doc, "<!-- header --><root><!-- body --></root>") == XmlResult::Ok);
	destroyXmlDoc(doc);
}

static void testValidCdata(UnitTestContext* ctx)
{
	XmlDocument* doc;
	UNIT_TEST_EXPECT(ctx, loadXmlString(&doc, "<root><![CDATA[<not a tag>]]></root>") == XmlResult::Ok);
	destroyXmlDoc(doc);
}

static void testValidEntityRef(UnitTestContext* ctx)
{
	XmlDocument* doc;
	UNIT_TEST_EXPECT(ctx, loadXmlString(&doc, "<root>&amp;&lt;&gt;</root>") == XmlResult::Ok);
	destroyXmlDoc(doc);
}

// ---- Invalid ----

static void testInvalidEmpty(UnitTestContext* ctx)
{
	XmlDocument* doc;
	UNIT_TEST_EXPECT(ctx, loadXmlString(&doc, "") == XmlResult::ParseError);
}

static void testInvalidUnclosedTag(UnitTestContext* ctx)
{
	XmlDocument* doc;
	UNIT_TEST_EXPECT(ctx, loadXmlString(&doc, "<root>") == XmlResult::ParseError);
}

static void testInvalidMismatchedTags(UnitTestContext* ctx)
{
	XmlDocument* doc;
	UNIT_TEST_EXPECT(ctx, loadXmlString(&doc, "<root></other>") == XmlResult::ParseError);
}

static void testInvalidMultipleRoots(UnitTestContext* ctx)
{
	XmlDocument* doc;
	UNIT_TEST_EXPECT(ctx, loadXmlString(&doc, "<a/><b/>") == XmlResult::ParseError);
}

static void testInvalidTextBeforeRoot(UnitTestContext* ctx)
{
	XmlDocument* doc;
	UNIT_TEST_EXPECT(ctx, loadXmlString(&doc, "text<root/>") == XmlResult::ParseError);
}

static void testInvalidUnclosedTagBracket(UnitTestContext* ctx)
{
	XmlDocument* doc;
	UNIT_TEST_EXPECT(ctx, loadXmlString(&doc, "<root") == XmlResult::ParseError);
}

static void testInvalidUnclosedAttrValue(UnitTestContext* ctx)
{
	XmlDocument* doc;
	UNIT_TEST_EXPECT(ctx, loadXmlString(&doc, "<root attr=\"unclosed/>") == XmlResult::ParseError);
}

static void testInvalidUnclosedComment(UnitTestContext* ctx)
{
	XmlDocument* doc;
	UNIT_TEST_EXPECT(ctx, loadXmlString(&doc, "<root><!-- unterminated</root>") == XmlResult::ParseError);
}

static void testInvalidDoubledDashInComment(UnitTestContext* ctx)
{
	XmlDocument* doc;
	UNIT_TEST_EXPECT(ctx, loadXmlString(&doc, "<root><!-- bad -- comment --></root>") == XmlResult::ParseError);
}

static void testInvalidCdataOutsideElement(UnitTestContext* ctx)
{
	XmlDocument* doc;
	UNIT_TEST_EXPECT(ctx, loadXmlString(&doc, "<![CDATA[data]]><root/>") == XmlResult::ParseError);
}

// ---- Walk ----

static void testWalkTreeStructure(UnitTestContext* ctx)
{
	XmlDocument* doc;
	UNIT_TEST_EXPECT(ctx, loadXmlString(&doc, "<root><a/><b/><c/></root>") == XmlResult::Ok);

	XmlElementHandle root = getRootXmlElement(doc);
	UNIT_TEST_EXPECT(ctx, root != nullptr);
	UNIT_TEST_EXPECT(ctx, strcmp(getXmlElementName(doc, root), "root") == 0);

	XmlElementHandle a = getFirstChildXmlElement(doc, root);
	UNIT_TEST_EXPECT(ctx, a != nullptr);
	UNIT_TEST_EXPECT(ctx, strcmp(getXmlElementName(doc, a), "a") == 0);

	XmlElementHandle b = getNextSiblingXmlElement(doc, a);
	UNIT_TEST_EXPECT(ctx, b != nullptr);
	UNIT_TEST_EXPECT(ctx, strcmp(getXmlElementName(doc, b), "b") == 0);

	XmlElementHandle c = getNextSiblingXmlElement(doc, b);
	UNIT_TEST_EXPECT(ctx, c != nullptr);
	UNIT_TEST_EXPECT(ctx, strcmp(getXmlElementName(doc, c), "c") == 0);

	UNIT_TEST_EXPECT(ctx, getNextSiblingXmlElement(doc, c) == nullptr);

	destroyXmlDoc(doc);
}

static void testWalkNestedChildren(UnitTestContext* ctx)
{
	XmlDocument* doc;
	UNIT_TEST_EXPECT(ctx, loadXmlString(&doc, "<root><parent><child1/><child2/></parent></root>") == XmlResult::Ok);

	XmlElementHandle root   = getRootXmlElement(doc);
	XmlElementHandle parent = getFirstChildXmlElement(doc, root);
	UNIT_TEST_EXPECT(ctx, parent != nullptr);
	UNIT_TEST_EXPECT(ctx, strcmp(getXmlElementName(doc, parent), "parent") == 0);

	XmlElementHandle child1 = getFirstChildXmlElement(doc, parent);
	UNIT_TEST_EXPECT(ctx, child1 != nullptr);
	UNIT_TEST_EXPECT(ctx, strcmp(getXmlElementName(doc, child1), "child1") == 0);

	XmlElementHandle child2 = getNextSiblingXmlElement(doc, child1);
	UNIT_TEST_EXPECT(ctx, child2 != nullptr);
	UNIT_TEST_EXPECT(ctx, strcmp(getXmlElementName(doc, child2), "child2") == 0);

	UNIT_TEST_EXPECT(ctx, getNextSiblingXmlElement(doc, child2) == nullptr);

	destroyXmlDoc(doc);
}

static void testWalkNameFilterFirstChild(UnitTestContext* ctx)
{
	XmlDocument* doc;
	UNIT_TEST_EXPECT(ctx, loadXmlString(&doc, "<root><a/><b/><b/><c/></root>") == XmlResult::Ok);

	XmlElementHandle root = getRootXmlElement(doc);

	XmlElementHandle b = getFirstChildXmlElement(doc, root, "b");
	UNIT_TEST_EXPECT(ctx, b != nullptr);
	UNIT_TEST_EXPECT(ctx, strcmp(getXmlElementName(doc, b), "b") == 0);

	UNIT_TEST_EXPECT(ctx, getFirstChildXmlElement(doc, root, "z") == nullptr);

	destroyXmlDoc(doc);
}

static void testWalkNameFilterNextSibling(UnitTestContext* ctx)
{
	XmlDocument* doc;
	UNIT_TEST_EXPECT(ctx, loadXmlString(&doc, "<root><a/><b/><c/><b/></root>") == XmlResult::Ok);

	XmlElementHandle root = getRootXmlElement(doc);
	XmlElementHandle a    = getFirstChildXmlElement(doc, root);

	XmlElementHandle b = getNextSiblingXmlElement(doc, a, "b");
	UNIT_TEST_EXPECT(ctx, b != nullptr);
	UNIT_TEST_EXPECT(ctx, strcmp(getXmlElementName(doc, b), "b") == 0);

	XmlElementHandle c = getNextSiblingXmlElement(doc, a, "c");
	UNIT_TEST_EXPECT(ctx, c != nullptr);
	UNIT_TEST_EXPECT(ctx, strcmp(getXmlElementName(doc, c), "c") == 0);

	UNIT_TEST_EXPECT(ctx, getNextSiblingXmlElement(doc, a, "z") == nullptr);

	destroyXmlDoc(doc);
}

static void testWalkSkipsTextNodes(UnitTestContext* ctx)
{
	XmlDocument* doc;
	UNIT_TEST_EXPECT(ctx, loadXmlString(&doc, "<root>text<a/>more<b/></root>") == XmlResult::Ok);

	XmlElementHandle root = getRootXmlElement(doc);

	XmlElementHandle a = getFirstChildXmlElement(doc, root);
	UNIT_TEST_EXPECT(ctx, a != nullptr);
	UNIT_TEST_EXPECT(ctx, strcmp(getXmlElementName(doc, a), "a") == 0);

	XmlElementHandle b = getNextSiblingXmlElement(doc, a);
	UNIT_TEST_EXPECT(ctx, b != nullptr);
	UNIT_TEST_EXPECT(ctx, strcmp(getXmlElementName(doc, b), "b") == 0);

	destroyXmlDoc(doc);
}

// ---- Attribute ----

static void testAttributeFound(UnitTestContext* ctx)
{
	XmlDocument* doc;
	UNIT_TEST_EXPECT(ctx, loadXmlString(&doc, "<root id=\"42\"/>") == XmlResult::Ok);

	XmlElementHandle   root = getRootXmlElement(doc);
	XmlAttributeHandle attr = getXmlAttribute(doc, root, "id");
	UNIT_TEST_EXPECT(ctx, attr != nullptr);
	UNIT_TEST_EXPECT(ctx, strcmp(getXmlAttributeName(doc, attr), "id") == 0);
	UNIT_TEST_EXPECT(ctx, strcmp(getXmlAttributeValue(doc, attr), "42") == 0);

	destroyXmlDoc(doc);
}

static void testAttributeNotFound(UnitTestContext* ctx)
{
	XmlDocument* doc;
	UNIT_TEST_EXPECT(ctx, loadXmlString(&doc, "<root id=\"42\"/>") == XmlResult::Ok);

	XmlElementHandle root = getRootXmlElement(doc);
	UNIT_TEST_EXPECT(ctx, getXmlAttribute(doc, root, "missing") == nullptr);

	destroyXmlDoc(doc);
}

static void testAttributeMultiple(UnitTestContext* ctx)
{
	XmlDocument* doc;
	UNIT_TEST_EXPECT(ctx, loadXmlString(&doc, "<root id=\"1\" name=\"foo\"/>") == XmlResult::Ok);

	XmlElementHandle root = getRootXmlElement(doc);

	XmlAttributeHandle id = getXmlAttribute(doc, root, "id");
	UNIT_TEST_EXPECT(ctx, id != nullptr);
	UNIT_TEST_EXPECT(ctx, strcmp(getXmlAttributeName(doc, id), "id") == 0);
	UNIT_TEST_EXPECT(ctx, strcmp(getXmlAttributeValue(doc, id), "1") == 0);

	XmlAttributeHandle name = getXmlAttribute(doc, root, "name");
	UNIT_TEST_EXPECT(ctx, name != nullptr);
	UNIT_TEST_EXPECT(ctx, strcmp(getXmlAttributeName(doc, name), "name") == 0);
	UNIT_TEST_EXPECT(ctx, strcmp(getXmlAttributeValue(doc, name), "foo") == 0);

	destroyXmlDoc(doc);
}

// ---- AttributeAs ----

static void testAttributeAsStringFound(UnitTestContext* ctx)
{
	XmlDocument* doc;
	UNIT_TEST_EXPECT(ctx, loadXmlString(&doc, "<root v=\"hello\"/>") == XmlResult::Ok);
	XmlElementHandle root = getRootXmlElement(doc);
	UNIT_TEST_EXPECT(ctx, strcmp(getXmlAttributeAsString(doc, root, "v"), "hello") == 0);
	destroyXmlDoc(doc);
}

static void testAttributeAsStringMissing(UnitTestContext* ctx)
{
	XmlDocument* doc;
	UNIT_TEST_EXPECT(ctx, loadXmlString(&doc, "<root/>") == XmlResult::Ok);
	XmlElementHandle root = getRootXmlElement(doc);
	UNIT_TEST_EXPECT(ctx, getXmlAttributeAsString(doc, root, "v") == nullptr);
	UNIT_TEST_EXPECT(ctx, strcmp(getXmlAttributeAsString(doc, root, "v", "default"), "default") == 0);
	destroyXmlDoc(doc);
}

static void testAttributeAsIntFound(UnitTestContext* ctx)
{
	XmlDocument* doc;
	UNIT_TEST_EXPECT(ctx, loadXmlString(&doc, "<root v=\"42\"/>") == XmlResult::Ok);
	XmlElementHandle root = getRootXmlElement(doc);
	UNIT_TEST_EXPECT(ctx, getXmlAttributeAsInt(doc, root, "v") == 42);
	destroyXmlDoc(doc);
}

static void testAttributeAsIntMissing(UnitTestContext* ctx)
{
	XmlDocument* doc;
	UNIT_TEST_EXPECT(ctx, loadXmlString(&doc, "<root/>") == XmlResult::Ok);
	XmlElementHandle root = getRootXmlElement(doc);
	UNIT_TEST_EXPECT(ctx, getXmlAttributeAsInt(doc, root, "v") == 0);
	UNIT_TEST_EXPECT(ctx, getXmlAttributeAsInt(doc, root, "v", -1) == -1);
	destroyXmlDoc(doc);
}

static void testAttributeAsIntInvalid(UnitTestContext* ctx)
{
	XmlDocument* doc;
	UNIT_TEST_EXPECT(ctx, loadXmlString(&doc, "<root v=\"abc\"/>") == XmlResult::Ok);
	XmlElementHandle root = getRootXmlElement(doc);
	UNIT_TEST_EXPECT(ctx, getXmlAttributeAsInt(doc, root, "v", -1) == -1);
	destroyXmlDoc(doc);
}

static void testAttributeAsFloatFound(UnitTestContext* ctx)
{
	XmlDocument* doc;
	UNIT_TEST_EXPECT(ctx, loadXmlString(&doc, "<root v=\"1.5\"/>") == XmlResult::Ok);
	XmlElementHandle root = getRootXmlElement(doc);
	UNIT_TEST_EXPECT(ctx, getXmlAttributeAsFloat(doc, root, "v") == 1.5f);
	destroyXmlDoc(doc);
}

static void testAttributeAsFloatMissing(UnitTestContext* ctx)
{
	XmlDocument* doc;
	UNIT_TEST_EXPECT(ctx, loadXmlString(&doc, "<root/>") == XmlResult::Ok);
	XmlElementHandle root = getRootXmlElement(doc);
	UNIT_TEST_EXPECT(ctx, getXmlAttributeAsFloat(doc, root, "v") == 0.0f);
	UNIT_TEST_EXPECT(ctx, getXmlAttributeAsFloat(doc, root, "v", 2.0f) == 2.0f);
	destroyXmlDoc(doc);
}

static void testAttributeAsFloatInvalid(UnitTestContext* ctx)
{
	XmlDocument* doc;
	UNIT_TEST_EXPECT(ctx, loadXmlString(&doc, "<root v=\"xyz\"/>") == XmlResult::Ok);
	XmlElementHandle root = getRootXmlElement(doc);
	UNIT_TEST_EXPECT(ctx, getXmlAttributeAsFloat(doc, root, "v", 2.0f) == 2.0f);
	destroyXmlDoc(doc);
}

static void testAttributeAsBoolTrue(UnitTestContext* ctx)
{
	XmlDocument* doc;
	UNIT_TEST_EXPECT(ctx, loadXmlString(&doc, "<root a=\"true\" b=\"True\" c=\"T\" d=\"t\" e=\"1\"/>") == XmlResult::Ok);
	XmlElementHandle root = getRootXmlElement(doc);
	UNIT_TEST_EXPECT(ctx, getXmlAttributeAsBool(doc, root, "a") == true);
	UNIT_TEST_EXPECT(ctx, getXmlAttributeAsBool(doc, root, "b") == true);
	UNIT_TEST_EXPECT(ctx, getXmlAttributeAsBool(doc, root, "c") == true);
	UNIT_TEST_EXPECT(ctx, getXmlAttributeAsBool(doc, root, "d") == true);
	UNIT_TEST_EXPECT(ctx, getXmlAttributeAsBool(doc, root, "e") == true);
	destroyXmlDoc(doc);
}

static void testAttributeAsBoolFalse(UnitTestContext* ctx)
{
	XmlDocument* doc;
	UNIT_TEST_EXPECT(ctx, loadXmlString(&doc, "<root a=\"false\" b=\"False\" c=\"F\" d=\"f\" e=\"0\"/>") == XmlResult::Ok);
	XmlElementHandle root = getRootXmlElement(doc);
	UNIT_TEST_EXPECT(ctx, getXmlAttributeAsBool(doc, root, "a") == false);
	UNIT_TEST_EXPECT(ctx, getXmlAttributeAsBool(doc, root, "b") == false);
	UNIT_TEST_EXPECT(ctx, getXmlAttributeAsBool(doc, root, "c") == false);
	UNIT_TEST_EXPECT(ctx, getXmlAttributeAsBool(doc, root, "d") == false);
	UNIT_TEST_EXPECT(ctx, getXmlAttributeAsBool(doc, root, "e") == false);
	destroyXmlDoc(doc);
}

static void testAttributeAsBoolMissing(UnitTestContext* ctx)
{
	XmlDocument* doc;
	UNIT_TEST_EXPECT(ctx, loadXmlString(&doc, "<root/>") == XmlResult::Ok);
	XmlElementHandle root = getRootXmlElement(doc);
	UNIT_TEST_EXPECT(ctx, getXmlAttributeAsBool(doc, root, "v") == false);
	UNIT_TEST_EXPECT(ctx, getXmlAttributeAsBool(doc, root, "v", true) == true);
	destroyXmlDoc(doc);
}

static void testAttributeAsBoolInvalid(UnitTestContext* ctx)
{
	XmlDocument* doc;
	UNIT_TEST_EXPECT(ctx, loadXmlString(&doc, "<root v=\"yes\"/>") == XmlResult::Ok);
	XmlElementHandle root = getRootXmlElement(doc);
	UNIT_TEST_EXPECT(ctx, getXmlAttributeAsBool(doc, root, "v", true) == true);
	destroyXmlDoc(doc);
}

// ---- TryGetAttribute ----

static void testTryGetStringFound(UnitTestContext* ctx)
{
	XmlDocument* doc;
	UNIT_TEST_EXPECT(ctx, loadXmlString(&doc, "<root v=\"hello\"/>") == XmlResult::Ok);
	XmlElementHandle root    = getRootXmlElement(doc);
	const char*      out     = nullptr;
	UNIT_TEST_EXPECT(ctx, tryGetAttributeValueAsString(out, doc, root, "v") == true);
	UNIT_TEST_EXPECT(ctx, strcmp(out, "hello") == 0);
	destroyXmlDoc(doc);
}

static void testTryGetStringMissing(UnitTestContext* ctx)
{
	XmlDocument* doc;
	UNIT_TEST_EXPECT(ctx, loadXmlString(&doc, "<root/>") == XmlResult::Ok);
	XmlElementHandle root = getRootXmlElement(doc);
	const char*      out  = nullptr;
	UNIT_TEST_EXPECT(ctx, tryGetAttributeValueAsString(out, doc, root, "v") == false);
	destroyXmlDoc(doc);
}

static void testTryGetIntFound(UnitTestContext* ctx)
{
	XmlDocument* doc;
	UNIT_TEST_EXPECT(ctx, loadXmlString(&doc, "<root v=\"99\"/>") == XmlResult::Ok);
	XmlElementHandle root = getRootXmlElement(doc);
	int              out  = 0;
	UNIT_TEST_EXPECT(ctx, tryGetAttributeValueAsInt(out, doc, root, "v") == true);
	UNIT_TEST_EXPECT(ctx, out == 99);
	destroyXmlDoc(doc);
}

static void testTryGetIntInvalid(UnitTestContext* ctx)
{
	XmlDocument* doc;
	UNIT_TEST_EXPECT(ctx, loadXmlString(&doc, "<root v=\"abc\"/>") == XmlResult::Ok);
	XmlElementHandle root = getRootXmlElement(doc);
	int              out  = 0;
	UNIT_TEST_EXPECT(ctx, tryGetAttributeValueAsInt(out, doc, root, "v") == false);
	destroyXmlDoc(doc);
}

static void testTryGetIntMissing(UnitTestContext* ctx)
{
	XmlDocument* doc;
	UNIT_TEST_EXPECT(ctx, loadXmlString(&doc, "<root/>") == XmlResult::Ok);
	XmlElementHandle root = getRootXmlElement(doc);
	int              out  = 0;
	UNIT_TEST_EXPECT(ctx, tryGetAttributeValueAsInt(out, doc, root, "v") == false);
	destroyXmlDoc(doc);
}

static void testTryGetFloatFound(UnitTestContext* ctx)
{
	XmlDocument* doc;
	UNIT_TEST_EXPECT(ctx, loadXmlString(&doc, "<root v=\"1.5\"/>") == XmlResult::Ok);
	XmlElementHandle root = getRootXmlElement(doc);
	float            out  = 0.0f;
	UNIT_TEST_EXPECT(ctx, tryGetAttributeValueAsFloat(out, doc, root, "v") == true);
	UNIT_TEST_EXPECT(ctx, out == 1.5f);
	destroyXmlDoc(doc);
}

static void testTryGetFloatInvalid(UnitTestContext* ctx)
{
	XmlDocument* doc;
	UNIT_TEST_EXPECT(ctx, loadXmlString(&doc, "<root v=\"xyz\"/>") == XmlResult::Ok);
	XmlElementHandle root = getRootXmlElement(doc);
	float            out  = 0.0f;
	UNIT_TEST_EXPECT(ctx, tryGetAttributeValueAsFloat(out, doc, root, "v") == false);
	destroyXmlDoc(doc);
}

static void testTryGetBoolTrue(UnitTestContext* ctx)
{
	XmlDocument* doc;
	UNIT_TEST_EXPECT(ctx, loadXmlString(&doc, "<root a=\"true\" b=\"T\" c=\"1\"/>") == XmlResult::Ok);
	XmlElementHandle root = getRootXmlElement(doc);
	bool             out  = false;
	UNIT_TEST_EXPECT(ctx, tryGetAttributeValueAsBool(out, doc, root, "a") == true);
	UNIT_TEST_EXPECT(ctx, out == true);
	UNIT_TEST_EXPECT(ctx, tryGetAttributeValueAsBool(out, doc, root, "b") == true);
	UNIT_TEST_EXPECT(ctx, out == true);
	UNIT_TEST_EXPECT(ctx, tryGetAttributeValueAsBool(out, doc, root, "c") == true);
	UNIT_TEST_EXPECT(ctx, out == true);
	destroyXmlDoc(doc);
}

static void testTryGetBoolFalse(UnitTestContext* ctx)
{
	XmlDocument* doc;
	UNIT_TEST_EXPECT(ctx, loadXmlString(&doc, "<root a=\"false\" b=\"F\" c=\"0\"/>") == XmlResult::Ok);
	XmlElementHandle root = getRootXmlElement(doc);
	bool             out  = true;
	UNIT_TEST_EXPECT(ctx, tryGetAttributeValueAsBool(out, doc, root, "a") == true);
	UNIT_TEST_EXPECT(ctx, out == false);
	UNIT_TEST_EXPECT(ctx, tryGetAttributeValueAsBool(out, doc, root, "b") == true);
	UNIT_TEST_EXPECT(ctx, out == false);
	UNIT_TEST_EXPECT(ctx, tryGetAttributeValueAsBool(out, doc, root, "c") == true);
	UNIT_TEST_EXPECT(ctx, out == false);
	destroyXmlDoc(doc);
}

static void testTryGetBoolInvalid(UnitTestContext* ctx)
{
	XmlDocument* doc;
	UNIT_TEST_EXPECT(ctx, loadXmlString(&doc, "<root v=\"yes\"/>") == XmlResult::Ok);
	XmlElementHandle root = getRootXmlElement(doc);
	bool             out  = false;
	UNIT_TEST_EXPECT(ctx, tryGetAttributeValueAsBool(out, doc, root, "v") == false);
	destroyXmlDoc(doc);
}

// ---- Text ----

static void testGetXmlTextFound(UnitTestContext* ctx)
{
	XmlDocument* doc;
	UNIT_TEST_EXPECT(ctx, loadXmlString(&doc, "<root>hello</root>") == XmlResult::Ok);
	XmlElementHandle root = getRootXmlElement(doc);
	UNIT_TEST_EXPECT(ctx, strcmp(getXmlText(doc, root), "hello") == 0);
	destroyXmlDoc(doc);
}

static void testGetXmlTextMissing(UnitTestContext* ctx)
{
	XmlDocument* doc;
	UNIT_TEST_EXPECT(ctx, loadXmlString(&doc, "<root><child/></root>") == XmlResult::Ok);
	XmlElementHandle root = getRootXmlElement(doc);
	UNIT_TEST_EXPECT(ctx, getXmlText(doc, root) == nullptr);
	destroyXmlDoc(doc);
}

static void testGetXmlTextAfterElement(UnitTestContext* ctx)
{
	XmlDocument* doc;
	UNIT_TEST_EXPECT(ctx, loadXmlString(&doc, "<root><child/>hello</root>") == XmlResult::Ok);
	XmlElementHandle root = getRootXmlElement(doc);
	UNIT_TEST_EXPECT(ctx, strcmp(getXmlText(doc, root), "hello") == 0);
	destroyXmlDoc(doc);
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

	UnitTestSuiteHandle walkSuite;
	createUnitTestSuite(walkSuite, ctx, "Walk", xmlDocSuite);

	UnitTestSuiteHandle attributeSuite;
	createUnitTestSuite(attributeSuite, ctx, "Attribute", xmlDocSuite);

	createUnitTest(h, ctx, "Found",    testAttributeFound,    attributeSuite);
	createUnitTest(h, ctx, "NotFound", testAttributeNotFound, attributeSuite);
	createUnitTest(h, ctx, "Multiple", testAttributeMultiple, attributeSuite);

	UnitTestSuiteHandle attributeAsSuite;
	createUnitTestSuite(attributeAsSuite, ctx, "AttributeAs", xmlDocSuite);

	createUnitTest(h, ctx, "StringFound",    testAttributeAsStringFound,   attributeAsSuite);
	createUnitTest(h, ctx, "StringMissing",  testAttributeAsStringMissing, attributeAsSuite);
	createUnitTest(h, ctx, "IntFound",       testAttributeAsIntFound,      attributeAsSuite);
	createUnitTest(h, ctx, "IntMissing",     testAttributeAsIntMissing,    attributeAsSuite);
	createUnitTest(h, ctx, "IntInvalid",     testAttributeAsIntInvalid,    attributeAsSuite);
	createUnitTest(h, ctx, "FloatFound",     testAttributeAsFloatFound,    attributeAsSuite);
	createUnitTest(h, ctx, "FloatMissing",   testAttributeAsFloatMissing,  attributeAsSuite);
	createUnitTest(h, ctx, "FloatInvalid",   testAttributeAsFloatInvalid,  attributeAsSuite);
	createUnitTest(h, ctx, "BoolTrue",       testAttributeAsBoolTrue,      attributeAsSuite);
	createUnitTest(h, ctx, "BoolFalse",      testAttributeAsBoolFalse,     attributeAsSuite);
	createUnitTest(h, ctx, "BoolMissing",    testAttributeAsBoolMissing,   attributeAsSuite);
	createUnitTest(h, ctx, "BoolInvalid",    testAttributeAsBoolInvalid,   attributeAsSuite);

	UnitTestSuiteHandle tryGetSuite;
	createUnitTestSuite(tryGetSuite, ctx, "TryGetAttribute", xmlDocSuite);

	createUnitTest(h, ctx, "StringFound",   testTryGetStringFound,   tryGetSuite);
	createUnitTest(h, ctx, "StringMissing", testTryGetStringMissing, tryGetSuite);
	createUnitTest(h, ctx, "IntFound",      testTryGetIntFound,      tryGetSuite);
	createUnitTest(h, ctx, "IntInvalid",    testTryGetIntInvalid,    tryGetSuite);
	createUnitTest(h, ctx, "IntMissing",    testTryGetIntMissing,    tryGetSuite);
	createUnitTest(h, ctx, "FloatFound",    testTryGetFloatFound,    tryGetSuite);
	createUnitTest(h, ctx, "FloatInvalid",  testTryGetFloatInvalid,  tryGetSuite);
	createUnitTest(h, ctx, "BoolTrue",      testTryGetBoolTrue,      tryGetSuite);
	createUnitTest(h, ctx, "BoolFalse",     testTryGetBoolFalse,     tryGetSuite);
	createUnitTest(h, ctx, "BoolInvalid",   testTryGetBoolInvalid,   tryGetSuite);

	UnitTestSuiteHandle textSuite;
	createUnitTestSuite(textSuite, ctx, "Text", xmlDocSuite);

	createUnitTest(h, ctx, "Found",        testGetXmlTextFound,        textSuite);
	createUnitTest(h, ctx, "Missing",      testGetXmlTextMissing,      textSuite);
	createUnitTest(h, ctx, "AfterElement", testGetXmlTextAfterElement, textSuite);

	createUnitTest(h, ctx, "TreeStructure",         testWalkTreeStructure,         walkSuite);
	createUnitTest(h, ctx, "NestedChildren",        testWalkNestedChildren,        walkSuite);
	createUnitTest(h, ctx, "NameFilterFirstChild",  testWalkNameFilterFirstChild,  walkSuite);
	createUnitTest(h, ctx, "NameFilterNextSibling", testWalkNameFilterNextSibling, walkSuite);
	createUnitTest(h, ctx, "SkipsTextNodes",        testWalkSkipsTextNodes,        walkSuite);
}
