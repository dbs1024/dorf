// Copyright (c) Darrin Stewart. All rights reserved.
#pragma once

struct Logger;

enum class LogLevel : unsigned
{
	Debug    = 0,
	Info     = 1,
	Warning  = 2,
	Error    = 3,
	Critical = 4,
};

struct LogRecord
{
	LogLevel    level;
	const char* message;
	int         line;
	int         charPosition;
	const char* filename;
};

struct LogHandler
{
	void (*emitRecord)(const LogRecord*);
};

bool createLogger(Logger** outLogger, LogHandler* logHandler = nullptr);
void destroyLogger(Logger* logger);
void aceLog(Logger* logger, LogLevel level, const char* format, ...);
