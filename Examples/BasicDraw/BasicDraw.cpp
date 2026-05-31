// Copyright (c) Darrin Stewart. All rights reserved.
#include "Engine.DemoApp/DemoApp.h"

int main()
{
	DemoAppContext* ctx = nullptr;
	createDemoAppContext(&ctx);
	createDemoAppWindow(ctx, "BasicDraw", 1920, 1080);

	while (processDemoAppMessages(ctx) != DemoAppResult::QuitRequested)
	{
		// TODO: rendering
	}

	destroyDemoAppContext(ctx);
	return 0;
}
