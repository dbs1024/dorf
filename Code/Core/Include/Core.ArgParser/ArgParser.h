// Copyright (c) Darrin Stewart. All rights reserved.
#pragma once

class ArgParserImpl;

enum class ArgParserArgType : unsigned
{
	Auto,	// Tries to infer the the arg type based on the action (usually a string)
	Bool,	// Converts arg values from string to bools (accepts variations - e.g. [t, T, TRUE, true] are all true)
	Int,	// Converts arg values to integers.
	Float,	// Converts arg values to floats.
	String,	// Leaves arg values in their string form.
};

enum class ArgParserNumArgs : unsigned
{
	Auto		= 0x7fffffff,	// Tries to infer the number of arguments based on the action
	ZeroOrOne	= 0x7ffffffe,	// Exactly zero or one additional argument will be consumed.
	ZeroOrMore	= 0x7ffffffd,	// Zero or more arguments will be consumed.
	OneOrMore	= 0x7ffffffc,	// One or more arguments will be consumed.
};

enum class ArgParserAction : unsigned
{
	Store,			// Stores the argument's value (default)
	StoreConst,		// Stores the value specified by the 'constant' member. Can be used to store a value based on a flag.
	StoreTrue,		// Stores the bool true (special case of StoreConst). Requires ParseArgType_Bool
	StoreFalse,		// Stores the bool false (special case of StoreConst). Requires ParseArgType_Bool
	Append,			// Appends values to a list. Requires one of the ParseArgTypes that supports lists.
	AppendConst,	// Appends constant value to a list. Requires one of the ParseArgTypes that supports lists.
	Count,			// Counts the number of types the arg is encountered.  Requires ParseArgType_Int.
};

struct ArgDesc
{
	const char* name;			// The argument name. E.g. --foo
	char flag;					// Shorter version instead of a flag. E.g. -f
	ArgParserArgType argType;
	ArgParserAction action;
	unsigned numArgs;
	const char* constant;		// value to store with StoreConst or AppendConst
	const char* defaultValue;	// value to use if the arg wasn't set on the command-line
	const char** choices;		// restrict options to specific choices
	bool required;
	const char* help;			// optional help string
	const char* metavar;		// displayed name for the arg in help text. E.g. --foo FOO_VAR
};

struct ArgParserDesc
{
	const char*      programName  = nullptr;	// override the program name displayed by the generated help
	const char*      usage        = nullptr;	// override the usage string. Default means usage will be auto-generated.
	const char*      description  = nullptr;	// Optional text to display before the detailed list of individual arguments
	const char*      epilog       = nullptr;	// Optional text to display at the end of the help string
	const ArgDesc*   argDescs     = nullptr;
	int              argDescCount = 0;
};

class ArgParser
{
public:
	ArgParser(const ArgParserDesc& desc);
	~ArgParser();

	bool        parseArgs(int argc, const char* argv[]);
	bool        shouldPrintHelp() const;
	const char* getErrorMessage() const;
	// name must be the bare argument name without leading "--" (e.g. "dll", not "--dll").
	int         getValueCount(const char* name) const;
	bool        getBoolValue(const char* name, int index = 0) const;
	int         getIntValue(const char* name, int index = 0) const;
	float       getFloatValue(const char* name, int index = 0) const;
	const char* getStringValue(const char* name, int index = 0) const;
	const char* getHelpMessage() const;

private:
	ArgParserImpl* m_impl = nullptr;
};
