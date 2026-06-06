// Copyright (c) Darrin Stewart. All rights reserved.
#include "Core.Logger/Logger.h"

#include <cstdarg>
#include <cstdio>
#include <cstring>
#include <new>

static const char* getLevelString(LogLevel level)
{
	switch (level)
	{
		case LogLevel::Debug:    return "DEBUG";
		case LogLevel::Info:     return "INFO";
		case LogLevel::Warning:  return "WARNING";
		case LogLevel::Error:    return "ERROR";
		case LogLevel::Critical: return "CRITICAL";
		default:                 return "UNKNOWN";
	}
}

static void defaultEmitRecord(const LogRecord* record)
{
	FILE* out = (record->level >= LogLevel::Error) ? stderr : stdout;
	const char* level = getLevelString(record->level);
	bool hasFilename = (record->filename != nullptr && record->filename[0] != '\0');

	if (!hasFilename)
	{
		fprintf(out, "%s: %s\n", level, record->message);
	}
	else if (record->line > 0 && record->charPosition > 0)
	{
		fprintf(out, "%s: %s(%d,%d): %s\n", level, record->filename, record->line, record->charPosition, record->message);
	}
	else
	{
		fprintf(out, "%s: %s: %s\n", level, record->filename, record->message);
	}
}

static LogHandler s_defaultHandler = { defaultEmitRecord };

struct Logger
{
	LogHandler handler;
};

bool createLogger(Logger** outLogger, LogHandler* logHandler)
{
	if (outLogger == nullptr)
		return false;

	Logger* logger = new (std::nothrow) Logger;
	if (logger == nullptr)
		return false;

	memset(logger, 0, sizeof(Logger));

	logger->handler = (logHandler != nullptr) ? *logHandler : s_defaultHandler;

	*outLogger = logger;
	return true;
}

void destroyLogger(Logger* logger)
{
	delete logger;
}

void aceLog(Logger* logger, LogLevel level, const char* format, ...)
{
	static char s_messageBuffer[2048];

	va_list args;
	va_start(args, format);
	vsnprintf(s_messageBuffer, sizeof(s_messageBuffer), format, args);
	va_end(args);

	LogRecord record;
	record.level        = level;
	record.message      = s_messageBuffer;
	record.line         = 0;
	record.charPosition = 0;
	record.filename     = nullptr;

	if (logger->handler.emitRecord != nullptr)
		logger->handler.emitRecord(&record);
}
