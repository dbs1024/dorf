// Copyright (c) Darrin Stewart. All rights reserved.

#include "Core.ArgParser/ArgParser.h"
#include "Core.UnitTest/UnitTest.h"

#include <Windows.h>
#include <DbgHelp.h>
#include <crtdbg.h>
#include <cstdio>
#include <string>
#include <vector>

using RegisterUnitTestsFn = void (*)(UnitTestContext*);

namespace
{
	UnitTestContext* g_ctx    = nullptr;
	int              g_depth  = 0;
	int              g_total  = 0;
	int              g_passed = 0;
	int              g_failed = 0;
	bool             g_testOk = true;

	constexpr const char* ColorGreen = "\033[92m";
	constexpr const char* ColorRed   = "\033[91m";
	constexpr const char* ColorReset = "\033[0m";

	struct AssertFailure
	{
		const char* expr;
		const char* file;
		int         line;
	};

	std::vector<AssertFailure>   g_failures;
	std::string                  g_exceptionMsg;
	DWORD                        g_exceptionCode = 0;
	std::vector<std::string>     g_callstack;

	void printIndent(int n)
	{
		for (int i = 0; i < n; ++i)
			printf("  ");
	}

	void onSuiteBegin(UnitTestSuiteHandle suite)
	{
		printIndent(g_depth);
		printf("%s\n", getUnitTestSuiteName(g_ctx, suite));
		++g_depth;
	}

	void onSuiteEnd(UnitTestSuiteHandle)
	{
		--g_depth;
		if (g_depth == 0)
			printf("\n");
	}

	void captureCallstackFrames(const CONTEXT* ctx, std::vector<std::string>& outFrames)
	{
		CONTEXT      ctxCopy = *ctx;
		HANDLE       process = GetCurrentProcess();
		HANDLE       thread  = GetCurrentThread();

		STACKFRAME64 frame        = {};
		frame.AddrPC.Offset       = ctxCopy.Rip;
		frame.AddrPC.Mode         = AddrModeFlat;
		frame.AddrFrame.Offset    = ctxCopy.Rbp;
		frame.AddrFrame.Mode      = AddrModeFlat;
		frame.AddrStack.Offset    = ctxCopy.Rsp;
		frame.AddrStack.Mode      = AddrModeFlat;

		alignas(SYMBOL_INFO) char symBuf[sizeof(SYMBOL_INFO) + MAX_SYM_NAME];
		SYMBOL_INFO* sym     = reinterpret_cast<SYMBOL_INFO*>(symBuf);
		IMAGEHLP_LINE64 line = {};
		line.SizeOfStruct    = sizeof(IMAGEHLP_LINE64);

		for (int i = 0; i < 24; ++i)
		{
			if (!StackWalk64(IMAGE_FILE_MACHINE_AMD64, process, thread, &frame, &ctxCopy,
			                 nullptr, SymFunctionTableAccess64, SymGetModuleBase64, nullptr))
				break;
			DWORD64 pc = frame.AddrPC.Offset;
			if (pc == 0 || pc < 0x10000 || pc == 0xcccccccccccccccc)
				break;

			sym->SizeOfStruct = sizeof(SYMBOL_INFO);
			sym->MaxNameLen   = MAX_SYM_NAME;

			char buf[512];
			DWORD64 disp64 = 0;
			if (SymFromAddr(process, frame.AddrPC.Offset, &disp64, sym))
			{
				DWORD disp32 = 0;
				if (SymGetLineFromAddr64(process, frame.AddrPC.Offset, &disp32, &line))
					snprintf(buf, sizeof(buf), "%s (%s:%lu)", sym->Name, line.FileName, line.LineNumber);
				else
					snprintf(buf, sizeof(buf), "%s", sym->Name);
			}
			else
			{
				snprintf(buf, sizeof(buf), "0x%016llx", static_cast<unsigned long long>(frame.AddrPC.Offset));
			}
			outFrames.push_back(buf);
		}
	}

	void onTestBegin(UnitTestSuiteHandle, UnitTestHandle test)
	{
		g_testOk = true;
		g_failures.clear();
		g_exceptionMsg.clear();
		g_exceptionCode = 0;
		g_callstack.clear();
		printIndent(g_depth);
		printf("[RUN ] %s", getUnitTestName(g_ctx, test));
		fflush(stdout);
	}

	void onTestEnd(UnitTestSuiteHandle, UnitTestHandle test)
	{
		++g_total;
		printf("\r");
		printIndent(g_depth);
		if (g_testOk)
		{
			++g_passed;
			printf("%s[PASS]%s %s\n", ColorGreen, ColorReset, getUnitTestName(g_ctx, test));
		}
		else
		{
			++g_failed;
			printf("%s[FAIL]%s %s\n", ColorRed, ColorReset, getUnitTestName(g_ctx, test));
			for (const AssertFailure& f : g_failures)
			{
				printIndent(g_depth + 1);
				printf("[----] %s:%d: (%s)\n", f.file, f.line, f.expr);
			}
			if (!g_exceptionMsg.empty())
			{
				printIndent(g_depth + 1);
				printf("[!!!!] %s\n", g_exceptionMsg.c_str());
			}
			if (g_exceptionCode != 0)
			{
				printIndent(g_depth + 1);
				printf("[!!!!] fatal exception (code 0x%08lX)\n", g_exceptionCode);
				for (const auto& f : g_callstack)
				{
					printIndent(g_depth + 2);
					printf("%s\n", f.c_str());
				}
			}
		}
	}

	void onTestCrash(UnitTestSuiteHandle, UnitTestHandle, void* exceptionInfo)
	{
		g_testOk = false;
		auto* exInfo    = static_cast<EXCEPTION_POINTERS*>(exceptionInfo);
		g_exceptionCode = exInfo->ExceptionRecord->ExceptionCode;
		captureCallstackFrames(exInfo->ContextRecord, g_callstack);
	}

	void onTestException(UnitTestSuiteHandle, UnitTestHandle, const char* message)
	{
		g_testOk = false;
		g_exceptionMsg = message ? message : "unknown exception";
	}

	void onTestAssert(UnitTestSuiteHandle, UnitTestHandle, const char* expr, const char* file, int line, int passed)
	{
		if (!passed)
		{
			g_testOk = false;
			g_failures.push_back({ expr, file, line });
		}
	}
}

int main(int argc, char* argv[])
{
	static const ArgDesc argDescs[] =
	{
		{
			.name         = "--dll",
			.flag         = 0,
			.argType      = ArgParserArgType::String,
			.action       = ArgParserAction::Append,
			.numArgs      = (unsigned)ArgParserNumArgs::OneOrMore,
			.constant     = nullptr,
			.defaultValue = nullptr,
			.choices      = nullptr,
			.required     = true,
			.help         = "Path to a unit test DLL to load and run.",
			.metavar      = nullptr,
		},
	};

	ArgParserDesc desc;
	desc.description  = "Loads one or more unit test DLLs and runs all registered tests.";
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

	// Redirect CRT assert/error/warn dialogs to stderr so they don't pop up during testing.
	// _set_abort_behavior suppresses the separate abort message box.
#if defined(_DEBUG)
	_CrtSetReportMode(_CRT_ASSERT, _CRTDBG_MODE_FILE);
	_CrtSetReportFile(_CRT_ASSERT, _CRTDBG_FILE_STDERR);
	_CrtSetReportMode(_CRT_ERROR,  _CRTDBG_MODE_FILE);
	_CrtSetReportFile(_CRT_ERROR,  _CRTDBG_FILE_STDERR);
	_CrtSetReportMode(_CRT_WARN,   _CRTDBG_MODE_FILE);
	_CrtSetReportFile(_CRT_WARN,   _CRTDBG_FILE_STDERR);
	_set_abort_behavior(0, _WRITE_ABORT_MSG | _CALL_REPORTFAULT);
#endif

	HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
	DWORD dwMode = 0;
	GetConsoleMode(hOut, &dwMode);
	SetConsoleMode(hOut, dwMode | ENABLE_VIRTUAL_TERMINAL_PROCESSING);

	std::vector<const char*> dllPaths;
	int dllCount = parser.getValueCount("dll");
	for (int i = 0; i < dllCount; ++i)
		dllPaths.push_back(parser.getStringValue("dll", i));

	UnitTestContext* ctx = nullptr;
	if (createUnitTestContext(&ctx) != UnitTestResult::Success)
	{
		printf("error: failed to create unit test context\n");
		return -1;
	}
	g_ctx = ctx;

	const UnitTestListener listener =
	{
		.onSuiteBegin = onSuiteBegin,
		.onSuiteEnd   = onSuiteEnd,
		.onTestBegin  = onTestBegin,
		.onTestEnd    = onTestEnd,
		.onTestAssert     = onTestAssert,
		.onTestException  = onTestException,
		.onTestCrash      = onTestCrash,
	};
	setUnitTestListener(ctx, &listener);

	std::vector<HMODULE> loadedDlls;
	for (const char* path : dllPaths)
	{
		HMODULE dll = LoadLibraryA(path);
		if (!dll)
		{
			printf("error: failed to load '%s' (error %lu)\n", path, GetLastError());
			destroyUnitTestContext(ctx);
			for (HMODULE h : loadedDlls)
				FreeLibrary(h);
			return -1;
		}

		auto registerFn = (RegisterUnitTestsFn)GetProcAddress(dll, "registerUnitTests");
		if (!registerFn)
		{
			printf("error: '%s' does not export registerUnitTests\n", path);
			FreeLibrary(dll);
			destroyUnitTestContext(ctx);
			for (HMODULE h : loadedDlls)
				FreeLibrary(h);
			return -1;
		}

		loadedDlls.push_back(dll);
		registerFn(ctx);
	}

	SymSetOptions(SYMOPT_UNDNAME | SYMOPT_DEFERRED_LOADS | SYMOPT_LOAD_LINES);
	SymInitialize(GetCurrentProcess(), nullptr, TRUE);

	runUnitTests(ctx);

	printf("[====] Synthesis: Tested: %d | Passing: %d | Failing: %d\n", g_total, g_passed, g_failed);

	SymCleanup(GetCurrentProcess());
	destroyUnitTestContext(ctx);
	for (HMODULE h : loadedDlls)
		FreeLibrary(h);

	return g_failed > 0 ? 1 : 0;
}
