// Copyright (c) Darrin Stewart. All rights reserved.
#pragma once

#include "Engine.Utils.XmlDoc/XmlDoc.h"
#include "Core.Memory/SlabAllocator.h"

struct XmlNode;

struct XmlAttribute
{
	const char*   name;
	const char*   value;
	XmlAttribute* nextAttribute;
};

enum class XmlNodeType
{
	Element,
	Text,
};

struct XmlNode
{
	XmlNodeType   type;
	XmlNode*      parent;
	XmlNode*      nextSibling;

	const char*   tagName;
	XmlNode*      firstChild;
	XmlNode*      lastChild;
	XmlAttribute* firstAttribute;

	const char*   text;
};

struct XmlDocument
{
	SlabCache* nodePool;
	SlabCache* attributePool;
	XmlNode*   root;
	char*      dataStart;
	char*      dataCurr;
	char*      dataEnd;
};

XmlResult createXmlDoc(XmlDocument** outDoc);
