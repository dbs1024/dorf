// Copyright (c) Darrin Stewart. All rights reserved.

#include "Core.ArgParser/ArgParser.h"
#include "Core.Logger/Logger.h"

#include <cstdio>

int main(int argc, char* argv[])
{
	static const ArgDesc argDescs[] =
	{
		{
			.name         = "--dll",
			.flag         = 0,
			.argType      = ArgParserArgType::Auto,
			.action       = ArgParserAction::Append,
			.numArgs      = (unsigned)ArgParserNumArgs::Auto,
			.constant     = nullptr,
			.defaultValue = nullptr,
			.choices      = nullptr,
			.required     = false,
			.help         = nullptr,
			.metavar      = nullptr,
		},
		{
			.name         = "infile",
			.flag         = 0,
			.argType      = ArgParserArgType::Auto,
			.action       = ArgParserAction::Store,
			.numArgs      = (unsigned)ArgParserNumArgs::Auto,
			.constant     = nullptr,
			.defaultValue = nullptr,
			.choices      = nullptr,
			.required     = true,
			.help         = nullptr,
			.metavar      = nullptr,
		},
		{
			.name         = "outpath",
			.flag         = 0,
			.argType      = ArgParserArgType::Auto,
			.action       = ArgParserAction::Store,
			.numArgs      = (unsigned)ArgParserNumArgs::Auto,
			.constant     = nullptr,
			.defaultValue = nullptr,
			.choices      = nullptr,
			.required     = true,
			.help         = nullptr,
			.metavar      = nullptr,
		},
	};

	ArgParserDesc desc;
	desc.description  = "Builds data, preparing it for efficient consumption in a real-time application.";
	desc.argDescs     = argDescs;
	desc.argDescCount = (int)(sizeof(argDescs) / sizeof(argDescs[0]));

	ArgParser parser(desc);
	bool parseOk = parser.parseArgs(argc, (const char**)argv);
	if (parser.shouldPrintHelp())
	{
		printf("%s\n", parser.getHelpMessage());
		return 0;
	}
	if (!parseOk)
	{
		printf("%s\n", parser.getErrorMessage());
		return -1;
	}

	Logger* logger = nullptr;
	if (!createLogger(&logger))
	{
		printf("error: failed to create logger\n");
		return -1;
	}

	destroyLogger(logger);
	return 0;
}
