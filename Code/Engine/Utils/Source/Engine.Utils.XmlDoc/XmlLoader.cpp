// Copyright (c) Darrin Stewart. All rights reserved.
#include "Engine.Utils.XmlDoc/XmlDoc.h"
#include "XmlDocInternal.h"

#include <cstdio>
#include <cstring>
#include <new>

namespace
{
	bool isSpace(char c)
	{
		return c == ' ' || c == '\t' || c == '\r' || c == '\n';
	}

	bool isNameStartChar(char c)
	{
		return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || c == '_' || c == ':' || (unsigned char)c > 127;
	}

	bool isNameChar(char c)
	{
		return isNameStartChar(c) || (c >= '0' && c <= '9') || c == '-' || c == '.';
	}

	void skipSpaces(const char*& p)
	{
		while (*p && isSpace(*p))
			++p;
	}

	bool skipComment(const char*& p)
	{
		// Called after consuming "<!--"; "--" must not appear in content except as "-->"
		while (*p)
		{
			if (p[0] == '-' && p[1] == '-')
			{
				if (p[2] != '>') return false;
				p += 3;
				return true;
			}
			++p;
		}
		return false;
	}

	bool skipCdata(const char*& p)
	{
		// Called after consuming "<![CDATA["
		while (*p)
		{
			if (p[0] == ']' && p[1] == ']' && p[2] == '>')
			{
				p += 3;
				return true;
			}
			++p;
		}
		return false;
	}

	bool skipPI(const char*& p)
	{
		// Called after consuming "<?"
		while (*p)
		{
			if (p[0] == '?' && p[1] == '>')
			{
				p += 2;
				return true;
			}
			++p;
		}
		return false;
	}

	bool skipDoctype(const char*& p)
	{
		// Called after consuming "<!DOCTYPE"; handles optional internal subset [...]
		int depth = 0;
		while (*p)
		{
			if (*p == '[')
				{ depth++; ++p; }
			else if (*p == ']' && depth > 0)
				{ depth--; ++p; }
			else if (*p == '>' && depth == 0)
				{ ++p; return true; }
			else if (*p == '"' || *p == '\'')
			{
				char q = *p++;
				while (*p && *p != q) ++p;
				if (!*p) return false;
				++p;
			}
			else
				++p;
		}
		return false;
	}

	bool parseAttrValue(const char*& p, const char*& outValue, int& outLen)
	{
		char q = *p++;
		outValue = p;
		while (*p && *p != q)
		{
			if (*p == '<') return false;
			++p;
		}
		if (!*p) return false;
		outLen = (int)(p - outValue);
		++p;
		return true;
	}

	bool parseName(const char*& p, const char*& outName, int& outLen)
	{
		if (!isNameStartChar(*p)) return false;
		outName = p;
		while (*p && isNameChar(*p)) ++p;
		outLen = (int)(p - outName);
		return true;
	}

	XmlResult parseTagTail(const char*& p, XmlDocument* doc, XmlNode* elem, bool& outSelfClosing)
	{
		outSelfClosing = false;
		XmlAttribute* lastAttr = nullptr;

		while (*p)
		{
			skipSpaces(p);
			if (*p == '>')
				{ ++p; return XmlResult::Ok; }
			if (*p == '/' && p[1] == '>')
				{ p += 2; outSelfClosing = true; return XmlResult::Ok; }

			const char* attrName; int attrNameLen;
			if (!parseName(p, attrName, attrNameLen)) return XmlResult::ParseError;
			skipSpaces(p);
			if (*p != '=') return XmlResult::ParseError;
			++p;
			skipSpaces(p);
			if (*p != '"' && *p != '\'') return XmlResult::ParseError;

			const char* attrValue; int attrValueLen;
			if (!parseAttrValue(p, attrValue, attrValueLen)) return XmlResult::ParseError;

			if (doc->dataEnd - doc->dataCurr < attrNameLen + 1 + attrValueLen + 1)
				return XmlResult::InternalError;

			memcpy(doc->dataCurr, attrName, attrNameLen);
			doc->dataCurr[attrNameLen] = '\0';
			const char* nameCopy = doc->dataCurr;
			doc->dataCurr += attrNameLen + 1;

			memcpy(doc->dataCurr, attrValue, attrValueLen);
			doc->dataCurr[attrValueLen] = '\0';
			const char* valueCopy = doc->dataCurr;
			doc->dataCurr += attrValueLen + 1;

			XmlAttribute* attr = static_cast<XmlAttribute*>(slabCacheAlloc(doc->attributePool));
			if (!attr) return XmlResult::InternalError;
			attr->name          = nameCopy;
			attr->value         = valueCopy;
			attr->nextAttribute = nullptr;

			if (lastAttr)
				lastAttr->nextAttribute = attr;
			else
				elem->firstAttribute = attr;
			lastAttr = attr;
		}
		return XmlResult::ParseError;
	}

	XmlResult parseXmlDoc(XmlDocument* doc, const char* str)
	{
		const char* p        = str;
		XmlNode*    stackTop = nullptr;
		bool        hasRoot  = false;

		while (*p)
		{
			if (*p != '<')
			{
				if (!stackTop)
				{
					if (isSpace(*p)) { ++p; continue; }
					return XmlResult::ParseError;
				}

				const char* textStart = p;
				while (*p && *p != '<')
				{
					if (*p == '&')
					{
						++p;
						while (*p && *p != ';') ++p;
						if (!*p) return XmlResult::ParseError;
						++p;
					}
					else
						++p;
				}
				int textLen = (int)(p - textStart);

				if (doc->dataEnd - doc->dataCurr < textLen + 1)
					return XmlResult::InternalError;
				memcpy(doc->dataCurr, textStart, textLen);
				doc->dataCurr[textLen] = '\0';

				XmlNode* textNode = static_cast<XmlNode*>(slabCacheAlloc(doc->nodePool));
				if (!textNode) return XmlResult::InternalError;
				textNode->type           = XmlNodeType::Text;
				textNode->text           = doc->dataCurr;
				doc->dataCurr           += textLen + 1;
				textNode->parent         = stackTop;
				textNode->nextSibling    = nullptr;
				textNode->tagName        = nullptr;
				textNode->firstChild     = nullptr;
				textNode->lastChild      = nullptr;
				textNode->firstAttribute = nullptr;

				if (stackTop->lastChild)
					stackTop->lastChild->nextSibling = textNode;
				else
					stackTop->firstChild = textNode;
				stackTop->lastChild = textNode;
				continue;
			}

			++p; // consume '<'

			if (*p == '/')
			{
				++p;
				const char* name; int len;
				if (!parseName(p, name, len)) return XmlResult::ParseError;
				skipSpaces(p);
				if (*p != '>') return XmlResult::ParseError;
				++p;

				if (!stackTop) return XmlResult::ParseError;

				if (memcmp(stackTop->tagName, name, len) != 0 || stackTop->tagName[len] != '\0')
					return XmlResult::ParseError;

				stackTop = stackTop->parent;
			}
			else if (*p == '!')
			{
				++p;
				if (p[0] == '-' && p[1] == '-')
				{
					p += 2;
					if (!skipComment(p)) return XmlResult::ParseError;
				}
				else if (strncmp(p, "[CDATA[", 7) == 0)
				{
					if (!stackTop) return XmlResult::ParseError;
					p += 7;
					if (!skipCdata(p)) return XmlResult::ParseError;
				}
				else if (strncmp(p, "DOCTYPE", 7) == 0)
				{
					if (hasRoot || stackTop) return XmlResult::ParseError;
					p += 7;
					if (!skipDoctype(p)) return XmlResult::ParseError;
				}
				else return XmlResult::ParseError;
			}
			else if (*p == '?')
			{
				++p;
				if (!skipPI(p)) return XmlResult::ParseError;
			}
			else
			{
				if (!stackTop && hasRoot) return XmlResult::ParseError;

				const char* name; int len;
				if (!parseName(p, name, len)) return XmlResult::ParseError;

				if (doc->dataEnd - doc->dataCurr < len + 1)
					return XmlResult::InternalError;
				memcpy(doc->dataCurr, name, len);
				doc->dataCurr[len] = '\0';

				XmlNode* elem = static_cast<XmlNode*>(slabCacheAlloc(doc->nodePool));
				if (!elem) return XmlResult::InternalError;
				elem->type           = XmlNodeType::Element;
				elem->tagName        = doc->dataCurr;
				doc->dataCurr       += len + 1;
				elem->text           = nullptr;
				elem->parent         = stackTop;
				elem->firstChild     = nullptr;
				elem->lastChild      = nullptr;
				elem->nextSibling    = nullptr;
				elem->firstAttribute = nullptr;

				if (stackTop)
				{
					if (stackTop->lastChild)
						stackTop->lastChild->nextSibling = elem;
					else
						stackTop->firstChild = elem;
					stackTop->lastChild = elem;
				}
				else
				{
					doc->root = elem;
					hasRoot   = true;
				}

				bool selfClosing = false;
				XmlResult tagResult = parseTagTail(p, doc, elem, selfClosing);
				if (tagResult != XmlResult::Ok) return tagResult;

				if (!selfClosing)
					stackTop = elem;
			}
		}

		return (!stackTop && hasRoot) ? XmlResult::Ok : XmlResult::ParseError;
	}
}

XmlResult loadXmlDoc(XmlDocument** outDoc, const char* path)
{
	*outDoc = nullptr;

	FILE* file = fopen(path, "rb");
	if (!file)
		return XmlResult::FileReadError;

	fseek(file, 0, SEEK_END);
	long fileSize = ftell(file);
	fseek(file, 0, SEEK_SET);

	if (fileSize < 0)
	{
		fclose(file);
		return XmlResult::FileReadError;
	}

	char* buf = new (std::nothrow) char[fileSize + 1];
	if (!buf)
	{
		fclose(file);
		return XmlResult::InternalError;
	}

	size_t bytesRead = fread(buf, 1, static_cast<size_t>(fileSize), file);
	fclose(file);

	if (static_cast<long>(bytesRead) != fileSize)
	{
		delete[] buf;
		return XmlResult::FileReadError;
	}

	buf[fileSize] = 0;

	XmlResult result = loadXmlString(outDoc, buf);

	delete[] buf;

	return result;
}

XmlResult loadXmlString(XmlDocument** outDoc, const char* str)
{
	*outDoc = nullptr;
	if (!str) return XmlResult::InternalError;

	XmlDocument* doc = nullptr;
	XmlResult result = createXmlDoc(&doc);
	if (result != XmlResult::Ok)
		return result;

	result = parseXmlDoc(doc, str);
	if (result != XmlResult::Ok)
	{
		destroyXmlDoc(doc);
		return result;
	}

	*outDoc = doc;
	return XmlResult::Ok;
}
