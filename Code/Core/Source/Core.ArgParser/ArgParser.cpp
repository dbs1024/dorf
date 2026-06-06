// Copyright (c) Darrin Stewart. All rights reserved.
#include "Core.ArgParser/ArgParser.h"
#include "Core.Base/Assert.h"

#include <algorithm>
#include <cstdlib>
#include <cstring>
#include <string>
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

static bool isOption(const char* token)
{
	if (token[0] != '-' || token[1] == '\0')
		return false;
	if (token[1] == '-')
		return true;
	return token[1] < '0' || token[1] > '9';
}

union ArgValue
{
	const char* strValue;
	int         intValue;
	float       floatValue;
	bool        boolValue;
};

struct Argument
{
	ArgDesc desc;
	bool    isPositional = false;
	int     firstValue   = 0;
	int     valueCount   = 0;
};

static const char* effectiveName(const Argument& arg)
{
	return arg.isPositional ? arg.desc.name : arg.desc.name + 2;
}

static std::string deriveProgName(const char* argv0)
{
	if (!isValidString(argv0))
		return "program";
	const char* name = argv0;
	for (const char* p = argv0; *p != '\0'; ++p)
	{
		if (*p == '/' || *p == '\\')
			name = p + 1;
	}
	std::string result(name);
	if (result.size() > 4)
	{
		const char* ext = result.c_str() + result.size() - 4;
		if (strcmp(ext, ".exe") == 0 || strcmp(ext, ".EXE") == 0)
			result.resize(result.size() - 4);
	}
	return result;
}

static std::string buildMetavar(const Argument& arg)
{
	if (isValidString(arg.desc.metavar))
		return arg.desc.metavar;
	const char* name = effectiveName(arg);
	std::string result;
	for (const char* p = name; *p != '\0'; ++p)
	{
		char c = *p;
		if (c >= 'a' && c <= 'z')  c = static_cast<char>(c - 32);
		else if (c == '-')         c = '_';
		result += c;
	}
	return result;
}

static std::string buildMetavarStr(const Argument& arg)
{
	const unsigned numArgs = arg.desc.numArgs;
	if (numArgs == 0)
		return "";
	const std::string mv = buildMetavar(arg);
	if (numArgs == 1)
		return mv;
	if (numArgs == static_cast<unsigned>(ArgParserNumArgs::ZeroOrOne))
		return "[" + mv + "]";
	if (numArgs == static_cast<unsigned>(ArgParserNumArgs::ZeroOrMore))
		return "[" + mv + " ...]";
	if (numArgs == static_cast<unsigned>(ArgParserNumArgs::OneOrMore))
		return mv + " [" + mv + " ...]";
	std::string result;
	for (unsigned i = 0; i < numArgs; ++i)
	{
		if (i > 0) result += ' ';
		result += mv;
	}
	return result;
}

static std::string buildUsageToken(const Argument& arg)
{
	const std::string mvStr = buildMetavarStr(arg);
	if (arg.isPositional)
	{
		std::string token = mvStr.empty() ? arg.desc.name : mvStr;
		const unsigned numArgs = arg.desc.numArgs;
		const bool optional = !arg.desc.required
			|| numArgs == static_cast<unsigned>(ArgParserNumArgs::ZeroOrOne)
			|| numArgs == static_cast<unsigned>(ArgParserNumArgs::ZeroOrMore);
		return optional ? "[" + token + "]" : token;
	}
	std::string token(arg.desc.name);
	if (!mvStr.empty())
	{
		token += ' ';
		token += mvStr;
	}
	return arg.desc.required ? token : "[" + token + "]";
}

static std::string buildArgLeftColumn(const Argument& arg)
{
	if (arg.isPositional)
		return arg.desc.name;
	const std::string mvStr = buildMetavarStr(arg);
	std::string result;
	if (arg.desc.flag != '\0')
	{
		result += '-';
		result += arg.desc.flag;
		if (!mvStr.empty())
		{
			result += ' ';
			result += mvStr;
		}
		result += ", ";
	}
	result += arg.desc.name;
	if (!mvStr.empty())
	{
		result += ' ';
		result += mvStr;
	}
	return result;
}

static void appendArgLine(std::string& out, const Argument& arg)
{
	static const int k_helpColumn = 24;
	static const int k_indent     = 2;

	const std::string left = buildArgLeftColumn(arg);
	out.append(k_indent, ' ');
	out += left;

	if (!isValidString(arg.desc.help))
	{
		out += '\n';
		return;
	}

	const int leftLen = k_indent + static_cast<int>(left.size());
	if (leftLen < k_helpColumn)
		out.append(k_helpColumn - leftLen, ' ');
	else
	{
		out += '\n';
		out.append(k_helpColumn, ' ');
	}
	out += arg.desc.help;
	out += '\n';
}

class ArgParserImpl
{
public:
	ArgParserImpl(const ArgParserDesc& desc);
	bool        parseArgs(int argc, const char* argv[]);
	bool        shouldPrintHelp() const;
	const char* getErrorMessage() const;
	int         getValueCount(const char* name) const;
	bool        getBoolValue(const char* name, int index) const;
	int         getIntValue(const char* name, int index) const;
	float       getFloatValue(const char* name, int index) const;
	const char* getStringValue(const char* name, int index) const;
	const char* getHelpMessage() const;

private:
	int         findSortedIndex(const char* name) const;
	Argument*   findArgByName(const char* name);
	Argument*   findByName(const char* name);
	Argument*   findByFlag(char flag);
	bool        storeTypedValue(Argument& arg, const char* str);
	bool        consumeValues(Argument& arg, int startIdx, int argc, const char* argv[], int& i);
	bool        processNamedArg(Argument& arg, int argc, const char* argv[], int& i);
	std::string buildUsage(const char* progName) const;

	std::vector<Argument> m_arguments;
	std::vector<ArgValue> m_values;
	std::vector<int>      m_sortedArgIndices;
	std::string           m_errorMessage;
	bool                  m_parsed      = false;
	const char*           m_programName = nullptr;
	const char*           m_usage       = nullptr;
	const char*           m_description = nullptr;
	const char*           m_epilog      = nullptr;
	const char*           m_argv0       = nullptr;
	mutable std::string   m_helpMessage;
};

ArgParserImpl::ArgParserImpl(const ArgParserDesc& desc)
{
	m_programName = desc.programName;
	m_usage       = desc.usage;
	m_description = desc.description;
	m_epilog      = desc.epilog;

	ACE_ASSERT(desc.argDescCount >= 0);
	ACE_ASSERT(desc.argDescCount == 0 || desc.argDescs != nullptr);

	m_arguments.resize(desc.argDescCount + 1);

	Argument& helpArg          = m_arguments[0];
	helpArg.desc.name          = "--help";
	helpArg.desc.flag          = 'h';
	helpArg.desc.argType       = ArgParserArgType::Bool;
	helpArg.desc.action        = ArgParserAction::StoreTrue;
	helpArg.desc.numArgs       = 0;
	helpArg.desc.defaultValue  = "false";
	helpArg.desc.help          = "show this help message";

	for (int i = 0; i < desc.argDescCount; ++i)
	{
		const ArgDesc& argDesc = desc.argDescs[i];
		Argument&      arg     = m_arguments[i + 1];

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

		ACE_ASSERT(arg.desc.action != ArgParserAction::Store ||
			arg.desc.numArgs == 1 ||
			(arg.desc.numArgs == static_cast<unsigned>(ArgParserNumArgs::ZeroOrOne) && isValidString(arg.desc.defaultValue)));
		ACE_ASSERT(arg.desc.action != ArgParserAction::StoreConst ||
			(arg.desc.numArgs == 0 && isValidString(arg.desc.constant)));
		ACE_ASSERT(arg.desc.action != ArgParserAction::StoreTrue ||
			(arg.desc.argType == ArgParserArgType::Bool && arg.desc.numArgs == 0));
		ACE_ASSERT(arg.desc.action != ArgParserAction::StoreFalse ||
			(arg.desc.argType == ArgParserArgType::Bool && arg.desc.numArgs == 0));
		ACE_ASSERT(arg.desc.action != ArgParserAction::Append || arg.desc.numArgs != 0);
		ACE_ASSERT(arg.desc.action != ArgParserAction::Append ||
			!(arg.desc.numArgs == static_cast<unsigned>(ArgParserNumArgs::ZeroOrOne) ||
			  arg.desc.numArgs == static_cast<unsigned>(ArgParserNumArgs::ZeroOrMore)) ||
			isValidString(arg.desc.defaultValue));
		ACE_ASSERT(arg.desc.action != ArgParserAction::AppendConst ||
			(arg.desc.numArgs == 0 && isValidString(arg.desc.constant)));
		ACE_ASSERT(arg.desc.action != ArgParserAction::Count ||
			(arg.desc.argType == ArgParserArgType::Int && arg.desc.numArgs == 0));
		ACE_ASSERT(!isValidString(arg.desc.constant) || validateType(arg.desc.constant, arg.desc.argType));
		ACE_ASSERT(!isValidString(arg.desc.defaultValue) || validateType(arg.desc.defaultValue, arg.desc.argType));
		ACE_ASSERT(arg.desc.name != nullptr);
		ACE_ASSERT(arg.desc.name[0] != '\0' && arg.desc.name[1] != '\0');
		ACE_ASSERT(!(arg.desc.name[0] == '-' && arg.desc.name[1] != '-'));
		ACE_ASSERT(arg.desc.flag == '\0' || (arg.desc.flag >= 'a' && arg.desc.flag <= 'z'));

		arg.isPositional  = !(arg.desc.name[0] == '-' && arg.desc.name[1] == '-');
		arg.desc.required = arg.desc.required || arg.isPositional;
	}

	m_sortedArgIndices.resize(m_arguments.size());
	for (int i = 0; i < static_cast<int>(m_arguments.size()); ++i)
		m_sortedArgIndices[i] = i;
	std::sort(m_sortedArgIndices.begin(), m_sortedArgIndices.end(),
		[this](int a, int b)
		{
			return strcmp(effectiveName(m_arguments[a]), effectiveName(m_arguments[b])) < 0;
		});
}

bool ArgParserImpl::parseArgs(int argc, const char* argv[])
{
	if (m_parsed)
	{
		m_errorMessage = "parseArgs has already been called";
		return false;
	}
	m_parsed = true;

	if (argc > 0)
		m_argv0 = argv[0];

	int positionalIndex = 0;

	for (int i = 1; i < argc; ++i)
	{
		const char* token = argv[i];

		if (token[0] == '-' && token[1] == '-')
		{
			Argument* arg = findByName(token + 2);
			if (arg == nullptr)
			{
				m_errorMessage  = "unrecognized argument: '";
				m_errorMessage += token;
				m_errorMessage += "'";
				return false;
			}
			if (!processNamedArg(*arg, argc, argv, i))
				return false;
		}
		else if (token[0] == '-' && token[1] != '\0' && (token[1] < '0' || token[1] > '9') && token[2] == '\0')
		{
			Argument* arg = findByFlag(token[1]);
			if (arg == nullptr)
			{
				m_errorMessage  = "unrecognized argument: '";
				m_errorMessage += token;
				m_errorMessage += "'";
				return false;
			}
			if (!processNamedArg(*arg, argc, argv, i))
				return false;
		}
		else
		{
			// Find the positionalIndex-th positional argument
			int seen = 0;
			Argument* arg = nullptr;
			for (Argument& cand : m_arguments)
			{
				if (cand.isPositional)
				{
					if (seen == positionalIndex)
					{
						arg = &cand;
						break;
					}
					++seen;
				}
			}
			if (arg == nullptr)
			{
				m_errorMessage  = "unexpected positional argument: '";
				m_errorMessage += token;
				m_errorMessage += "'";
				return false;
			}
			ACE_ASSERT(arg->desc.action == ArgParserAction::Store || arg->desc.action == ArgParserAction::Append);
			if (!consumeValues(*arg, i, argc, argv, i))
				return false;
			++positionalIndex;
		}
	}

	// Apply defaults and validate required arguments
	for (Argument& arg : m_arguments)
	{
		if (arg.valueCount == 0)
		{
			if (isValidString(arg.desc.defaultValue))
			{
				if (!storeTypedValue(arg, arg.desc.defaultValue))
					return false;
			}
			else if (arg.desc.required)
			{
				m_errorMessage  = "argument '";
				m_errorMessage += arg.desc.name;
				m_errorMessage += "' is required";
				return false;
			}
		}
	}

	return true;
}

bool ArgParserImpl::shouldPrintHelp() const
{
	ACE_ASSERT(m_parsed);
	const Argument& helpArg = m_arguments[0];
	return helpArg.valueCount > 0 && m_values[helpArg.firstValue].boolValue;
}

const char* ArgParserImpl::getErrorMessage() const
{
	return m_errorMessage.empty() ? nullptr : m_errorMessage.c_str();
}

int ArgParserImpl::getValueCount(const char* name) const
{
	ACE_ASSERT(m_parsed && m_errorMessage.empty());
	const int idx = findSortedIndex(name);
	ACE_ASSERT(idx >= 0);
	return m_arguments[idx].valueCount;
}

bool ArgParserImpl::getBoolValue(const char* name, int index) const
{
	ACE_ASSERT(m_parsed && m_errorMessage.empty());
	const int idx = findSortedIndex(name);
	ACE_ASSERT(idx >= 0);
	const Argument& arg = m_arguments[idx];
	ACE_ASSERT(arg.desc.argType == ArgParserArgType::Bool);
	ACE_ASSERT(index >= 0 && index < arg.valueCount);
	return m_values[arg.firstValue + index].boolValue;
}

int ArgParserImpl::getIntValue(const char* name, int index) const
{
	ACE_ASSERT(m_parsed && m_errorMessage.empty());
	const int idx = findSortedIndex(name);
	ACE_ASSERT(idx >= 0);
	const Argument& arg = m_arguments[idx];
	ACE_ASSERT(arg.desc.argType == ArgParserArgType::Int);
	ACE_ASSERT(index >= 0 && index < arg.valueCount);
	return m_values[arg.firstValue + index].intValue;
}

float ArgParserImpl::getFloatValue(const char* name, int index) const
{
	ACE_ASSERT(m_parsed && m_errorMessage.empty());
	const int idx = findSortedIndex(name);
	ACE_ASSERT(idx >= 0);
	const Argument& arg = m_arguments[idx];
	ACE_ASSERT(arg.desc.argType == ArgParserArgType::Float);
	ACE_ASSERT(index >= 0 && index < arg.valueCount);
	return m_values[arg.firstValue + index].floatValue;
}

const char* ArgParserImpl::getStringValue(const char* name, int index) const
{
	ACE_ASSERT(m_parsed && m_errorMessage.empty());
	const int idx = findSortedIndex(name);
	ACE_ASSERT(idx >= 0);
	const Argument& arg = m_arguments[idx];
	ACE_ASSERT(arg.desc.argType == ArgParserArgType::String);
	ACE_ASSERT(index >= 0 && index < arg.valueCount);
	return m_values[arg.firstValue + index].strValue;
}

const char* ArgParserImpl::getHelpMessage() const
{
	std::string progName;
	if (isValidString(m_programName))
		progName = m_programName;
	else
		progName = deriveProgName(m_argv0);

	m_helpMessage.clear();

	if (isValidString(m_usage))
	{
		m_helpMessage += "usage: ";
		m_helpMessage += m_usage;
	}
	else
	{
		m_helpMessage += buildUsage(progName.c_str());
	}
	m_helpMessage += '\n';

	if (isValidString(m_description))
	{
		m_helpMessage += '\n';
		m_helpMessage += m_description;
		m_helpMessage += '\n';
	}

	bool hasPositionals = false;
	for (const Argument& arg : m_arguments)
	{
		if (arg.isPositional) { hasPositionals = true; break; }
	}
	if (hasPositionals)
	{
		m_helpMessage += "\npositional arguments:\n";
		for (const Argument& arg : m_arguments)
		{
			if (arg.isPositional)
				appendArgLine(m_helpMessage, arg);
		}
	}

	m_helpMessage += "\noptions:\n";
	for (const Argument& arg : m_arguments)
	{
		if (!arg.isPositional)
			appendArgLine(m_helpMessage, arg);
	}

	if (isValidString(m_epilog))
	{
		m_helpMessage += '\n';
		m_helpMessage += m_epilog;
		m_helpMessage += '\n';
	}

	return m_helpMessage.c_str();
}

int ArgParserImpl::findSortedIndex(const char* name) const
{
	int lo = 0, hi = static_cast<int>(m_sortedArgIndices.size()) - 1;
	while (lo <= hi)
	{
		const int mid = lo + (hi - lo) / 2;
		const int cmp = strcmp(effectiveName(m_arguments[m_sortedArgIndices[mid]]), name);
		if (cmp == 0)
			return m_sortedArgIndices[mid];
		if (cmp < 0)
			lo = mid + 1;
		else
			hi = mid - 1;
	}
	return -1;
}

Argument* ArgParserImpl::findArgByName(const char* name)
{
	const int idx = findSortedIndex(name);
	return idx >= 0 ? &m_arguments[idx] : nullptr;
}

Argument* ArgParserImpl::findByName(const char* name)
{
	Argument* arg = findArgByName(name);
	return (arg != nullptr && !arg->isPositional) ? arg : nullptr;
}

Argument* ArgParserImpl::findByFlag(char flag)
{
	for (Argument& arg : m_arguments)
	{
		if (!arg.isPositional && arg.desc.flag == flag)
			return &arg;
	}
	return nullptr;
}

bool ArgParserImpl::storeTypedValue(Argument& arg, const char* str)
{
	if (arg.desc.choices != nullptr)
	{
		bool found = false;
		for (const char** c = arg.desc.choices; *c != nullptr; ++c)
		{
			if (strcmp(*c, str) == 0)
			{
				found = true;
				break;
			}
		}
		if (!found)
		{
			m_errorMessage  = "argument '";
			m_errorMessage += arg.desc.name;
			m_errorMessage += "': invalid choice '";
			m_errorMessage += str;
			m_errorMessage += "'";
			return false;
		}
	}

	ArgValue v = {};
	switch (arg.desc.argType)
	{
		case ArgParserArgType::Bool:
			if (!tryGetBool(v.boolValue, str))
			{
				m_errorMessage  = "argument '";
				m_errorMessage += arg.desc.name;
				m_errorMessage += "': invalid bool value '";
				m_errorMessage += str;
				m_errorMessage += "'";
				return false;
			}
			break;
		case ArgParserArgType::Int:
			if (!tryGetInt(v.intValue, str))
			{
				m_errorMessage  = "argument '";
				m_errorMessage += arg.desc.name;
				m_errorMessage += "': invalid int value '";
				m_errorMessage += str;
				m_errorMessage += "'";
				return false;
			}
			break;
		case ArgParserArgType::Float:
			if (!tryGetFloat(v.floatValue, str))
			{
				m_errorMessage  = "argument '";
				m_errorMessage += arg.desc.name;
				m_errorMessage += "': invalid float value '";
				m_errorMessage += str;
				m_errorMessage += "'";
				return false;
			}
			break;
		default: // String
			v.strValue = str;
			break;
	}

	if (arg.valueCount == 0)
		arg.firstValue = static_cast<int>(m_values.size());
	m_values.push_back(v);
	++arg.valueCount;
	return true;
}

bool ArgParserImpl::consumeValues(Argument& arg, int startIdx, int argc, const char* argv[], int& i)
{
	if (arg.desc.action == ArgParserAction::Store)
		arg.valueCount = 0;

	const unsigned numArgs = arg.desc.numArgs;

	if (numArgs == static_cast<unsigned>(ArgParserNumArgs::ZeroOrOne))
	{
		if (startIdx < argc && !isOption(argv[startIdx]))
		{
			if (!storeTypedValue(arg, argv[startIdx]))
				return false;
			i = startIdx;
		}
		else if (isValidString(arg.desc.defaultValue))
		{
			if (!storeTypedValue(arg, arg.desc.defaultValue))
				return false;
		}
		return true;
	}

	if (numArgs == static_cast<unsigned>(ArgParserNumArgs::ZeroOrMore) ||
		numArgs == static_cast<unsigned>(ArgParserNumArgs::OneOrMore))
	{
		const bool needsAtLeastOne = numArgs == static_cast<unsigned>(ArgParserNumArgs::OneOrMore);
		int consumed = 0;
		while (startIdx + consumed < argc && !isOption(argv[startIdx + consumed]))
		{
			if (!storeTypedValue(arg, argv[startIdx + consumed]))
				return false;
			++consumed;
		}
		if (consumed > 0)
			i = startIdx + consumed - 1;
		if (needsAtLeastOne && consumed == 0)
		{
			m_errorMessage  = "argument '";
			m_errorMessage += arg.desc.name;
			m_errorMessage += "': expected at least one value";
			return false;
		}
		if (consumed == 0 && isValidString(arg.desc.defaultValue))
		{
			if (!storeTypedValue(arg, arg.desc.defaultValue))
				return false;
		}
		return true;
	}

	// Exact count
	for (unsigned j = 0; j < numArgs; ++j)
	{
		const int idx = startIdx + static_cast<int>(j);
		if (idx >= argc || isOption(argv[idx]))
		{
			m_errorMessage  = "argument '";
			m_errorMessage += arg.desc.name;
			m_errorMessage += "': expected ";
			m_errorMessage += std::to_string(numArgs);
			m_errorMessage += " value(s)";
			return false;
		}
		if (!storeTypedValue(arg, argv[idx]))
			return false;
	}
	if (numArgs > 0)
		i = startIdx + static_cast<int>(numArgs) - 1;
	return true;
}

bool ArgParserImpl::processNamedArg(Argument& arg, int argc, const char* argv[], int& i)
{
	switch (arg.desc.action)
	{
		case ArgParserAction::StoreTrue:
		{
			ArgValue v = {};
			v.boolValue = true;
			if (arg.valueCount == 0)
			{
				arg.firstValue = static_cast<int>(m_values.size());
				m_values.push_back(v);
				arg.valueCount = 1;
			}
			else
			{
				m_values[arg.firstValue] = v;
			}
			return true;
		}
		case ArgParserAction::StoreFalse:
		{
			ArgValue v = {};
			v.boolValue = false;
			if (arg.valueCount == 0)
			{
				arg.firstValue = static_cast<int>(m_values.size());
				m_values.push_back(v);
				arg.valueCount = 1;
			}
			else
			{
				m_values[arg.firstValue] = v;
			}
			return true;
		}
		case ArgParserAction::StoreConst:
			arg.valueCount = 0;
			return storeTypedValue(arg, arg.desc.constant);
		case ArgParserAction::AppendConst:
			return storeTypedValue(arg, arg.desc.constant);
		case ArgParserAction::Count:
		{
			if (arg.valueCount == 0)
			{
				arg.firstValue = static_cast<int>(m_values.size());
				ArgValue v = {};
				v.intValue = 1;
				m_values.push_back(v);
				arg.valueCount = 1;
			}
			else
			{
				++m_values[arg.firstValue].intValue;
			}
			return true;
		}
		case ArgParserAction::Store:
		case ArgParserAction::Append:
			return consumeValues(arg, i + 1, argc, argv, i);
		default:
			ACE_ASSERT(false);
			return false;
	}
}

std::string ArgParserImpl::buildUsage(const char* progName) const
{
	std::string usage = "usage: ";
	usage += progName;
	for (const Argument& arg : m_arguments)
	{
		usage += ' ';
		usage += buildUsageToken(arg);
	}
	return usage;
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
	return m_impl->parseArgs(argc, argv);
}

bool ArgParser::shouldPrintHelp() const
{
	return m_impl->shouldPrintHelp();
}

const char* ArgParser::getErrorMessage() const
{
	return m_impl->getErrorMessage();
}

int ArgParser::getValueCount(const char* name) const
{
	return m_impl->getValueCount(name);
}

bool ArgParser::getBoolValue(const char* name, int index) const
{
	return m_impl->getBoolValue(name, index);
}

int ArgParser::getIntValue(const char* name, int index) const
{
	return m_impl->getIntValue(name, index);
}

float ArgParser::getFloatValue(const char* name, int index) const
{
	return m_impl->getFloatValue(name, index);
}

const char* ArgParser::getStringValue(const char* name, int index) const
{
	return m_impl->getStringValue(name, index);
}

const char* ArgParser::getHelpMessage() const
{
	return m_impl->getHelpMessage();
}
