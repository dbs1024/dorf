// Copyright (c) Darrin Stewart. All rights reserved.
#include "Core.ArgParser/ArgParser.h"
#include "Core.Base/Assert.h"

#include <cstdlib>
#include <cstring>
#include <vector>

static bool tryGetBool(bool& outValue, const char* str)
{
	if (strcmp(str, "true") == 0 || strcmp(str, "True") == 0 || strcmp(str, "TRUE") == 0 ||
		strcmp(str, "t") == 0 || strcmp(str, "T") == 0 || strcmp(str, "1") == 0)
	{
		outValue = true;
		return true;
	}
	if (strcmp(str, "false") == 0 || strcmp(str, "False") == 0 || strcmp(str, "FALSE") == 0 ||
		strcmp(str, "f") == 0 || strcmp(str, "F") == 0 || strcmp(str, "0") == 0)
	{
		outValue = false;
		return true;
	}
	outValue = false;
	return false;
}

static bool tryGetInt(int& outValue, const char* str)
{
	char* end;
	const long value = strtol(str, &end, 10);
	if (end == str || *end != '\0')
	{
		outValue = 0;
		return false;
	}
	outValue = static_cast<int>(value);
	return true;
}

static bool tryGetFloat(float& outValue, const char* str)
{
	char* end;
	const float value = strtof(str, &end);
	if (end == str || *end != '\0')
	{
		outValue = 0.0f;
		return false;
	}
	outValue = value;
	return true;
}

static bool isValidString(const char* str)
{
	return str != nullptr && str[0] != '\0';
}

static bool validateType(const char* str, ArgParserArgType argType)
{
	switch (argType)
	{
		case ArgParserArgType::Bool:
		{
			bool outValue;
			return tryGetBool(outValue, str);
		}
		case ArgParserArgType::Int:
		{
			int outValue;
			return tryGetInt(outValue, str);
		}
		case ArgParserArgType::Float:
		{
			float outValue;
			return tryGetFloat(outValue, str);
		}
		case ArgParserArgType::String:
			return isValidString(str);
		default:
			return false;
	}
}

struct Argument
{
	ArgDesc desc;
	bool    isPositional = false;
};

class ArgParserImpl
{
public:
	ArgParserImpl(const ArgParserDesc& desc);

private:
	std::vector<Argument> m_arguments;
};

ArgParserImpl::ArgParserImpl(const ArgParserDesc& desc)
{
	DORF_ASSERT(desc.argDescCount >= 0);
	DORF_ASSERT(desc.argDescCount == 0 || desc.argDescs != nullptr);

	m_arguments.resize(desc.argDescCount);
	for (int i = 0; i < desc.argDescCount; ++i)
	{
		const ArgDesc& argDesc = desc.argDescs[i];
		Argument&      arg     = m_arguments[i];

		arg.desc = argDesc;

		if (arg.desc.argType == ArgParserArgType::Auto)
		{
			switch (arg.desc.action)
			{
				case ArgParserAction::StoreTrue:
				case ArgParserAction::StoreFalse:
					arg.desc.argType = ArgParserArgType::Bool;
					break;
				case ArgParserAction::Count:
					arg.desc.argType = ArgParserArgType::Int;
					break;
				default:
					arg.desc.argType = ArgParserArgType::String;
					break;
			}
		}

		if (arg.desc.numArgs == static_cast<unsigned>(ArgParserNumArgs::Auto))
		{
			if (arg.desc.action == ArgParserAction::Store || arg.desc.action == ArgParserAction::Append)
				arg.desc.numArgs = 1;
			else
				arg.desc.numArgs = 0;
		}

		DORF_ASSERT(arg.desc.action != ArgParserAction::Store ||
			arg.desc.numArgs == 1 ||
			(arg.desc.numArgs == static_cast<unsigned>(ArgParserNumArgs::ZeroOrOne) && isValidString(arg.desc.defaultValue)));
		DORF_ASSERT(arg.desc.action != ArgParserAction::StoreConst ||
			(arg.desc.numArgs == 0 && isValidString(arg.desc.constant)));
		DORF_ASSERT(arg.desc.action != ArgParserAction::StoreTrue ||
			(arg.desc.argType == ArgParserArgType::Bool && arg.desc.numArgs == 0));
		DORF_ASSERT(arg.desc.action != ArgParserAction::StoreFalse ||
			(arg.desc.argType == ArgParserArgType::Bool && arg.desc.numArgs == 0));
		DORF_ASSERT(arg.desc.action != ArgParserAction::Append || arg.desc.numArgs != 0);
		DORF_ASSERT(arg.desc.action != ArgParserAction::Append ||
			!(arg.desc.numArgs == static_cast<unsigned>(ArgParserNumArgs::ZeroOrOne) ||
			  arg.desc.numArgs == static_cast<unsigned>(ArgParserNumArgs::ZeroOrMore)) ||
			isValidString(arg.desc.defaultValue));
		DORF_ASSERT(arg.desc.action != ArgParserAction::AppendConst ||
			(arg.desc.numArgs == 0 && isValidString(arg.desc.constant)));
		DORF_ASSERT(arg.desc.action != ArgParserAction::Count ||
			(arg.desc.argType == ArgParserArgType::Int && arg.desc.numArgs == 0));
		DORF_ASSERT(!isValidString(arg.desc.constant) || validateType(arg.desc.constant, arg.desc.argType));
		DORF_ASSERT(!isValidString(arg.desc.defaultValue) || validateType(arg.desc.defaultValue, arg.desc.argType));
		DORF_ASSERT(arg.desc.name != nullptr);
		DORF_ASSERT(arg.desc.name[0] != '\0' && arg.desc.name[1] != '\0');
		DORF_ASSERT(!(arg.desc.name[0] == '-' && arg.desc.name[1] != '-'));
		DORF_ASSERT(arg.desc.flag == '\0' || (arg.desc.flag >= 'a' && arg.desc.flag <= 'z'));

		arg.isPositional  = !(arg.desc.name[0] == '-' && arg.desc.name[1] == '-');
		arg.desc.required = arg.desc.required || arg.isPositional;
	}
}

ArgParser::ArgParser(const ArgParserDesc& desc)
{
	m_impl = new ArgParserImpl(desc);
}

ArgParser::~ArgParser()
{
	delete m_impl;
}

bool ArgParser::parseArgs(int argc, const char* argv[])
{
	DORF_ASSERT(false && "TODO");
	return false;
}

const char* ArgParser::getHelpMessage() const
{
	DORF_ASSERT(false && "TODO");
	return nullptr;
}
