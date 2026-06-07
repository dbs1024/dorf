// Copyright (c) Darrin Stewart. All rights reserved.
#pragma once

struct XmlDocument;

using XmlElementHandle   = void*;
using XmlAttributeHandle = void*;

enum class XmlResult
{
	Ok,
	FileReadError,
	ParseError,
	InternalError,
};

XmlResult       loadXmlDoc(XmlDocument** outDoc, const char* path);
XmlResult       loadXmlString(XmlDocument** outDoc, const char* str);
void            destroyXmlDoc(XmlDocument* doc);
XmlElementHandle getRootXmlElement(XmlDocument* doc);
const char*      getXmlElementName(XmlDocument* doc, XmlElementHandle element);
XmlElementHandle   getFirstChildXmlElement(XmlDocument* doc, XmlElementHandle element, const char* nameFilter = nullptr);
XmlElementHandle   getNextSiblingXmlElement(XmlDocument* doc, XmlElementHandle element, const char* nameFilter = nullptr);
const char*        getXmlText(XmlDocument* doc, XmlElementHandle element);
XmlAttributeHandle getXmlAttribute(XmlDocument* doc, XmlElementHandle element, const char* name);
XmlAttributeHandle getFirstXmlAttribute(XmlDocument* doc, XmlElementHandle element);
XmlAttributeHandle getNextXmlAttribute(XmlDocument* doc, XmlAttributeHandle attr);
const char*        getXmlAttributeName(XmlDocument* doc, XmlAttributeHandle attr);
const char*        getXmlAttributeValue(XmlDocument* doc, XmlAttributeHandle attr);
const char*        getXmlAttributeAsString(XmlDocument* doc, XmlElementHandle element, const char* name, const char* fallback = nullptr);
int                getXmlAttributeAsInt(XmlDocument* doc, XmlElementHandle element, const char* name, int fallback = 0);
float              getXmlAttributeAsFloat(XmlDocument* doc, XmlElementHandle element, const char* name, float fallback = 0.0f);
bool               getXmlAttributeAsBool(XmlDocument* doc, XmlElementHandle element, const char* name, bool fallback = false);
bool               tryGetAttributeValueAsString(const char*& outValue, XmlDocument* doc, XmlElementHandle element, const char* name);
bool               tryGetAttributeValueAsInt(int& outValue, XmlDocument* doc, XmlElementHandle element, const char* name);
bool               tryGetAttributeValueAsFloat(float& outValue, XmlDocument* doc, XmlElementHandle element, const char* name);
bool               tryGetAttributeValueAsBool(bool& outValue, XmlDocument* doc, XmlElementHandle element, const char* name);
