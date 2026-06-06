// Copyright (c) Darrin Stewart. All rights reserved.
#include "Engine.Utils.XmlDoc/XmlDoc.h"
#include "XmlDocInternal.h"

#include <cstdio>
#include <cstring>
#include <new>

namespace
{
	struct NameNode
	{
		const char* data;
		int         len;
		NameNode*   prev;
	};

	void cleanupStack(NameNode*& top)
	{
		while (top)
		{
			NameNode* prev = top->prev;
			delete top;
			top = prev;
		}
	}

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

	bool skipAttrValue(const char*& p)
	{
		char q = *p++;
		while (*p && *p != q)
		{
			if (*p == '<') return false;
			++p;
		}
		if (!*p) return false;
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

	// Returns 1 for '>', 2 for '/>', 0 on error
	int parseTagTail(const char*& p)
	{
		while (*p)
		{
			skipSpaces(p);
			if (*p == '>')
				{ ++p; return 1; }
			if (*p == '/' && p[1] == '>')
				{ p += 2; return 2; }

			const char* attrName; int attrLen;
			if (!parseName(p, attrName, attrLen)) return 0;
			skipSpaces(p);
			if (*p != '=') return 0;
			++p;
			skipSpaces(p);
			if (*p != '"' && *p != '\'') return 0;
			if (!skipAttrValue(p)) return 0;
		}
		return 0;
	}

	bool isWellFormed(const char* str)
	{
		const char* p        = str;
		NameNode*   stackTop = nullptr;
		bool        hasRoot  = false;

		while (*p)
		{
			skipSpaces(p);
			if (!*p) break;

			if (*p != '<')
			{
				if (!stackTop) { cleanupStack(stackTop); return false; }
				while (*p && *p != '<')
				{
					if (*p == '&')
					{
						++p;
						while (*p && *p != ';') ++p;
						if (!*p) { cleanupStack(stackTop); return false; }
						++p;
					}
					else
						++p;
				}
				continue;
			}

			++p; // consume '<'

			if (*p == '/')
			{
				++p;
				const char* name; int len;
				if (!parseName(p, name, len)) { cleanupStack(stackTop); return false; }
				skipSpaces(p);
				if (*p != '>') { cleanupStack(stackTop); return false; }
				++p;

				if (!stackTop) { cleanupStack(stackTop); return false; }
				if (stackTop->len != len || memcmp(stackTop->data, name, len) != 0)
					{ cleanupStack(stackTop); return false; }

				NameNode* prev = stackTop->prev;
				delete stackTop;
				stackTop = prev;
			}
			else if (*p == '!')
			{
				++p;
				if (p[0] == '-' && p[1] == '-')
				{
					p += 2;
					if (!skipComment(p)) { cleanupStack(stackTop); return false; }
				}
				else if (strncmp(p, "[CDATA[", 7) == 0)
				{
					if (!stackTop) { cleanupStack(stackTop); return false; }
					p += 7;
					if (!skipCdata(p)) { cleanupStack(stackTop); return false; }
				}
				else if (strncmp(p, "DOCTYPE", 7) == 0)
				{
					if (hasRoot || stackTop) { cleanupStack(stackTop); return false; }
					p += 7;
					if (!skipDoctype(p)) { cleanupStack(stackTop); return false; }
				}
				else { cleanupStack(stackTop); return false; }
			}
			else if (*p == '?')
			{
				++p;
				if (!skipPI(p)) { cleanupStack(stackTop); return false; }
			}
			else
			{
				if (!stackTop && hasRoot) { cleanupStack(stackTop); return false; }

				const char* name; int len;
				if (!parseName(p, name, len)) { cleanupStack(stackTop); return false; }

				int close = parseTagTail(p);
				if (close == 0) { cleanupStack(stackTop); return false; }

				if (!stackTop) hasRoot = true;

				if (close == 1)
				{
					NameNode* node = new (std::nothrow) NameNode;
					if (!node) { cleanupStack(stackTop); return false; }
					node->data = name;
					node->len  = len;
					node->prev = stackTop;
					stackTop   = node;
				}
			}
		}

		bool ok = (stackTop == nullptr && hasRoot);
		cleanupStack(stackTop);
		return ok;
	}
}

XmlResult loadXmlDoc(XmlDocHandle* outDoc, const char* path)
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

XmlResult loadXmlString(XmlDocHandle* outDoc, const char* str)
{
	*outDoc = nullptr;
	if (!str) return XmlResult::InternalError;
	return isWellFormed(str) ? XmlResult::Ok : XmlResult::ParseError;
}
