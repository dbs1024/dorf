// Copyright (c) Darrin Stewart. All rights reserved.
#pragma once

struct DemoAppContext;
struct InputManager;
struct RhiDevice;

enum class DemoAppResult
{
	Ok,
	QuitRequested,
	Failed,
};

void createDemoAppContext(DemoAppContext** outCtx);
void destroyDemoAppContext(DemoAppContext* ctx);

DemoAppResult  createDemoAppWindow(DemoAppContext* ctx, const char* title, int clientWidth, int clientHeight);
DemoAppResult  updateDemoApp(DemoAppContext* ctx);
InputManager*  getDemoAppInputManager(DemoAppContext* ctx);
RhiDevice*     getDemoAppRhiDevice(DemoAppContext* ctx);
