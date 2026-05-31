// Copyright (c) Darrin Stewart. All rights reserved.
#pragma once

struct DemoAppContext;

enum class DemoAppResult
{
	Ok,
	QuitRequested,
};

void createDemoAppContext(DemoAppContext** outCtx);
void destroyDemoAppContext(DemoAppContext* ctx);

void          createDemoAppWindow(DemoAppContext* ctx, const char* title, int clientWidth, int clientHeight);
DemoAppResult processDemoAppMessages(DemoAppContext* ctx);
