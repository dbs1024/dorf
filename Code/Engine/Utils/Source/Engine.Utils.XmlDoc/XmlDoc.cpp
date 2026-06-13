// Copyright (c) Darrin Stewart. All rights reserved.
#include "Engine.Utils.XmlDoc/XmlDoc.h"
#include "XmlDocInternal.h"

#include "Core.Base/Assert.h"
#include <cstdlib>
#include <cstring>
#include <new>

XmlResult createXmlDoc(XmlDocument** outDoc)
{
	*outDoc = new (std::nothrow) XmlDocument;
	if (!*outDoc)
		return XmlResult::InternalError;

	memset(*outDoc, 0, sizeof(XmlDocument));

	SlabCacheParams nodePoolParams = { 65536, sizeof(XmlNode), 1 };
	(*outDoc)->nodePool = createSlabCache(nodePoolParams);

	SlabCacheParams attributePoolParams = { 65536, sizeof(XmlAttribute), 1 };
	(*outDoc)->attributePool = createSlabCache(attributePoolParams);

	constexpr int dataBufferSize = 4 * 1024 * 1024;
	(*outDoc)->dataStart = new (std::nothrow) char[dataBufferSize];
	if (!(*outDoc)->dataStart)
	{
		destroySlabCacheUnchecked((*outDoc)->attributePool);
		destroySlabCacheUnchecked((*outDoc)->nodePool);
		delete *outDoc;
		*outDoc = nullptr;
		return XmlResult::InternalError;
	}
	(*outDoc)->dataCurr = (*outDoc)->dataStart;
	(*outDoc)->dataEnd  = (*outDoc)->dataStart + dataBufferSize;

	return XmlResult::Ok;
}

void destroyXmlDoc(XmlDocument* doc)
{
	if (!doc)
		return;
	destroySlabCacheUnchecked(doc->attributePool);
	destroySlabCacheUnchecked(doc->nodePool);
	delete[] doc->dataStart;
	delete doc;
}

XmlElementHandle getRootXmlElement(XmlDocument* doc)
{
	if (!doc || !doc->root)
		return nullptr;
	return doc->root;
}

const char* getXmlElementName(XmlDocument* /*doc*/, XmlElementHandle element)
{
	XmlNode* node = static_cast<XmlNode*>(element);
	ACE_ASSERT(node->type == XmlNodeType::Element);
	return node->tagName;
}

XmlElementHandle getFirstChildXmlElement(XmlDocument* /*doc*/, XmlElementHandle element, const char* nameFilter)
{
	XmlNode* node = static_cast<XmlNode*>(element);
	ACE_ASSERT(node->type == XmlNodeType::Element);

	XmlNode* child = node->firstChild;
	while (child)
	{
		if (child->type == XmlNodeType::Element)
		{
			if (!nameFilter || strcmp(child->tagName, nameFilter) == 0)
				return child;
		}
		child = child->nextSibling;
	}
	return nullptr;
}

XmlElementHandle getNextSiblingXmlElement(XmlDocument* /*doc*/, XmlElementHandle element, const char* nameFilter)
{
	XmlNode* node = static_cast<XmlNode*>(element);
	ACE_ASSERT(node->type == XmlNodeType::Element);

	XmlNode* sibling = node->nextSibling;
	while (sibling)
	{
		if (sibling->type == XmlNodeType::Element)
		{
			if (!nameFilter || strcmp(sibling->tagName, nameFilter) == 0)
				return sibling;
		}
		sibling = sibling->nextSibling;
	}
	return nullptr;
}

const char* getXmlText(XmlDocument* /*doc*/, XmlElementHandle element)
{
	XmlNode* node = static_cast<XmlNode*>(element);
	ACE_ASSERT(node->type == XmlNodeType::Element);

	XmlNode* child = node->firstChild;
	while (child)
	{
		if (child->type == XmlNodeType::Text)
			return child->text;
		child = child->nextSibling;
	}
	return nullptr;
}

XmlAttributeHandle getXmlAttribute(XmlDocument* /*doc*/, XmlElementHandle element, const char* name)
{
	XmlNode* node = static_cast<XmlNode*>(element);
	ACE_ASSERT(node->type == XmlNodeType::Element);

	XmlAttribute* attr = node->firstAttribute;
	while (attr)
	{
		if (strcmp(attr->name, name) == 0)
			return attr;
		attr = attr->nextAttribute;
	}
	return nullptr;
}

XmlAttributeHandle getFirstXmlAttribute(XmlDocument* /*doc*/, XmlElementHandle element)
{
	XmlNode* node = static_cast<XmlNode*>(element);
	ACE_ASSERT(node->type == XmlNodeType::Element);

	return node->firstAttribute;
}

XmlAttributeHandle getNextXmlAttribute(XmlDocument* /*doc*/, XmlAttributeHandle attr)
{
	return static_cast<XmlAttribute*>(attr)->nextAttribute;
}

const char* getXmlAttributeName(XmlDocument* /*doc*/, XmlAttributeHandle attr)
{
	return static_cast<XmlAttribute*>(attr)->name;
}

const char* getXmlAttributeValue(XmlDocument* /*doc*/, XmlAttributeHandle attr)
{
	return static_cast<XmlAttribute*>(attr)->value;
}

const char* getXmlAttributeAsString(XmlDocument* doc, XmlElementHandle element, const char* name, const char* fallback)
{
	XmlAttributeHandle attr = getXmlAttribute(doc, element, name);
	if (!attr)
		return fallback;
	return static_cast<XmlAttribute*>(attr)->value;
}

int getXmlAttributeAsInt(XmlDocument* doc, XmlElementHandle element, const char* name, int fallback)
{
	XmlAttributeHandle attr = getXmlAttribute(doc, element, name);
	if (!attr)
		return fallback;

	const char* value  = static_cast<XmlAttribute*>(attr)->value;
	char*       end;
	long        result = strtol(value, &end, 10);
	if (end == value || *end != '\0')
		return fallback;
	return static_cast<int>(result);
}

float getXmlAttributeAsFloat(XmlDocument* doc, XmlElementHandle element, const char* name, float fallback)
{
	XmlAttributeHandle attr = getXmlAttribute(doc, element, name);
	if (!attr)
		return fallback;

	const char* value  = static_cast<XmlAttribute*>(attr)->value;
	char*       end;
	float       result = strtof(value, &end);
	if (end == value || *end != '\0')
		return fallback;
	return result;
}

bool getXmlAttributeAsBool(XmlDocument* doc, XmlElementHandle element, const char* name, bool fallback)
{
	XmlAttributeHandle attr = getXmlAttribute(doc, element, name);
	if (!attr)
		return fallback;

	const char* value = static_cast<XmlAttribute*>(attr)->value;
	if (strcmp(value, "true") == 0 || strcmp(value, "True") == 0 || strcmp(value, "T") == 0 || strcmp(value, "t") == 0 || strcmp(value, "1") == 0)
		return true;
	if (strcmp(value, "false") == 0 || strcmp(value, "False") == 0 || strcmp(value, "F") == 0 || strcmp(value, "f") == 0 || strcmp(value, "0") == 0)
		return false;
	return fallback;
}

bool tryGetAttributeValueAsString(const char*& outValue, XmlDocument* doc, XmlElementHandle element, const char* name)
{
	XmlAttributeHandle attr = getXmlAttribute(doc, element, name);
	if (!attr)
		return false;
	outValue = static_cast<XmlAttribute*>(attr)->value;
	return true;
}

bool tryGetAttributeValueAsInt(int& outValue, XmlDocument* doc, XmlElementHandle element, const char* name)
{
	XmlAttributeHandle attr = getXmlAttribute(doc, element, name);
	if (!attr)
		return false;

	const char* value  = static_cast<XmlAttribute*>(attr)->value;
	char*       end;
	long        result = strtol(value, &end, 10);
	if (end == value || *end != '\0')
		return false;
	outValue = static_cast<int>(result);
	return true;
}

bool tryGetAttributeValueAsFloat(float& outValue, XmlDocument* doc, XmlElementHandle element, const char* name)
{
	XmlAttributeHandle attr = getXmlAttribute(doc, element, name);
	if (!attr)
		return false;

	const char* value  = static_cast<XmlAttribute*>(attr)->value;
	char*       end;
	float       result = strtof(value, &end);
	if (end == value || *end != '\0')
		return false;
	outValue = result;
	return true;
}

bool tryGetAttributeValueAsBool(bool& outValue, XmlDocument* doc, XmlElementHandle element, const char* name)
{
	XmlAttributeHandle attr = getXmlAttribute(doc, element, name);
	if (!attr)
		return false;

	const char* value = static_cast<XmlAttribute*>(attr)->value;
	if (strcmp(value, "true") == 0 || strcmp(value, "True") == 0 || strcmp(value, "T") == 0 || strcmp(value, "t") == 0 || strcmp(value, "1") == 0)
	{
		outValue = true;
		return true;
	}
	if (strcmp(value, "false") == 0 || strcmp(value, "False") == 0 || strcmp(value, "F") == 0 || strcmp(value, "f") == 0 || strcmp(value, "0") == 0)
	{
		outValue = false;
		return true;
	}
	return false;
}
