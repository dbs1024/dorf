// Copyright (c) Darrin Stewart. All rights reserved.
#include "Core.Color/Color.h"
#include "Engine.DemoApp/DemoApp.h"
#include "Engine.Input/InputManager.h"
#include "Engine.Render/Rhi/RhiDevice.h"

int main()
{
	DemoAppContext* ctx = nullptr;
	createDemoAppContext(&ctx);
	createDemoAppWindow(ctx, "BasicDraw", 1920, 1080);

	RhiDevice* rhiDevice = getDemoAppRhiDevice(ctx);

	while (updateDemoApp(ctx) == DemoAppResult::Ok)
	{
		if (wasKeyboardButtonPressed(getDemoAppInputManager(ctx), KeyboardButton::KB_Escape))
			break;

		rhiBeginFrame(rhiDevice);

		RhiCommandList* cmdList   = rhiOpenCommandList(rhiDevice, RhiCommandListType::Graphics);
		RhiResource*    backBuffer = rhiGetBackBuffer(rhiDevice);

		rhiTransitionState(cmdList, backBuffer, RhiResourceState::RenderTarget);
		rhiClearRenderTarget(cmdList, backBuffer, colorToVector4f({ 64, 10, 10, 255 }));
		rhiTransitionState(cmdList, backBuffer, RhiResourceState::Present);

		rhiCloseCommandList(cmdList);
		rhiExecuteCommandList(cmdList);

		rhiEndFrame(rhiDevice);
	}

	destroyDemoAppContext(ctx);
	return 0;
}
