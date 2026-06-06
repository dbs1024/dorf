// Copyright (c) Darrin Stewart. All rights reserved.
#pragma once

using XmlDocHandle = void*;

enum class XmlResult
{
	Ok,
	FileReadError,
	ParseError,
	InternalError,
};

XmlResult loadXmlDoc(XmlDocHandle* outDoc, const char* path);
XmlResult loadXmlString(XmlDocHandle* outDoc, const char* str);
void      destroyXmlDoc(XmlDocHandle doc);
