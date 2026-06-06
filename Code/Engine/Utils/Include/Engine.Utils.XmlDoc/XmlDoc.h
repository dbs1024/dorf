// Copyright (c) Darrin Stewart. All rights reserved.
#pragma once

struct XmlDocument;

enum class XmlResult
{
	Ok,
	FileReadError,
	ParseError,
	InternalError,
};

XmlResult loadXmlDoc(XmlDocument** outDoc, const char* path);
XmlResult loadXmlString(XmlDocument** outDoc, const char* str);
void      destroyXmlDoc(XmlDocument* doc);
