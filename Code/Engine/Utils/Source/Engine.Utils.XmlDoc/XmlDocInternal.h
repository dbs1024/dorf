// Copyright (c) Darrin Stewart. All rights reserved.
#pragma once

#include "Engine.Utils.XmlDoc/XmlDoc.h"
#include "Core.Memory/FixedItemPool.h"

struct XmlAttribute
{
	const char*     name;
	const char*     value;
	FixedItemHandle nextAttribute;
};

enum class XmlNodeType
{
	Element,
	Text,
};

struct XmlNode
{
	XmlNodeType     type;
	FixedItemHandle parent;
	FixedItemHandle nextSibling;

	const char*     tagName;
	FixedItemHandle firstChild;
	FixedItemHandle lastChild;
	FixedItemHandle firstAttribute;

	const char*     text;
};

struct XmlDocument
{
	FixedItemPoolHandle nodePool;
	FixedItemPoolHandle attributePool;
	FixedItemHandle     root;
	char*               dataStart;
	char*               dataCurr;
	char*               dataEnd;
};

XmlResult createXmlDoc(XmlDocument** outDoc);
