// Copyright (c) Darrin Stewart. All rights reserved.
#include "Engine.DemoApp/DemoApp.h"
#include "Engine.Input/InputManager.h"

int main()
{
	DemoAppContext* ctx = nullptr;
	createDemoAppContext(&ctx);
	createDemoAppWindow(ctx, "BasicDraw", 1920, 1080);

	while (updateDemoApp(ctx) == DemoAppResult::Ok)
	{
		if (wasKeyboardButtonPressed(getDemoAppInputManager(ctx), KeyboardButton::KB_Escape))
			break;

		// TODO: rendering
	}

	destroyDemoAppContext(ctx);
	return 0;
}
