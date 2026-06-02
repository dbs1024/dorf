// Copyright (c) Darrin Stewart. All rights reserved.
#include "Engine.DemoApp/DemoApp.h"
#include "Engine.Render/Rhi/RhiDevice.h"

#include <cstdio>
#include <new>
#include <windows.h>

struct DemoAppContext
{
	HWND hwnd = nullptr;
	RhiDevice* rhiDevice = nullptr;
};

void createDemoAppContext(DemoAppContext** outCtx)
{
	DemoAppContext* ctx = new (std::nothrow) DemoAppContext{};
	if (!ctx)
	{
		*outCtx = nullptr;
		return;
	}
	*outCtx = ctx;
}

void destroyDemoAppContext(DemoAppContext* ctx)
{
	if (!ctx)
		return;
	destroyRhiDevice(ctx->rhiDevice);
	if (ctx->hwnd)
		DestroyWindow(ctx->hwnd);
	delete ctx;
}

static LRESULT CALLBACK wndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg)
	{
		case WM_DESTROY:
			PostQuitMessage(0);
			return 0;
	}
	return DefWindowProc(hwnd, msg, wParam, lParam);
}

DemoAppResult createDemoAppWindow(DemoAppContext* ctx, const char* title, int clientWidth, int clientHeight)
{
	SetProcessDpiAwarenessContext(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2);

	WNDCLASSEX wc    = {};
	wc.cbSize        = sizeof(WNDCLASSEX);
	wc.style         = 0;
	wc.lpfnWndProc   = wndProc;
	wc.hInstance     = GetModuleHandle(nullptr);
	wc.hIcon         = LoadIcon(nullptr, IDI_APPLICATION);
	wc.hCursor       = LoadCursor(nullptr, IDC_ARROW);
	wc.lpszClassName = title;
	wc.hIconSm       = LoadIcon(nullptr, IDI_APPLICATION);

	RegisterClassEx(&wc);

	RECT rect = { 0, 0, clientWidth, clientHeight };
	AdjustWindowRect(&rect, WS_OVERLAPPEDWINDOW, FALSE);

	ctx->hwnd = CreateWindowEx(
		0,
		title,
		title,
		WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT, CW_USEDEFAULT,
		rect.right - rect.left,
		rect.bottom - rect.top,
		nullptr,
		nullptr,
		GetModuleHandle(nullptr),
		nullptr);

	SetWindowLongPtr(ctx->hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(ctx));

	RhiDeviceCreateParams rhiParams;
	rhiParams.window              = ctx->hwnd;
#ifdef _DEBUG
	rhiParams.enableDebug         = true;
#else
	rhiParams.enableDebug         = false;
#endif
	rhiParams.enableGpuValidation = false;
	rhiParams.maxRenderedFrames   = 2;
	rhiParams.backbufferWidth     = clientWidth;
	rhiParams.backbufferHeight    = clientHeight;

	if (createRhiDevice(&ctx->rhiDevice, rhiParams) != RhiError::Ok)
	{
		printf("DemoApp: failed to create RhiDevice\n");
		DestroyWindow(ctx->hwnd);
		ctx->hwnd = nullptr;
		return DemoAppResult::Failed;
	}

	ShowWindow(ctx->hwnd, SW_SHOWDEFAULT);
	UpdateWindow(ctx->hwnd);
	return DemoAppResult::Ok;
}

DemoAppResult processDemoAppMessages(DemoAppContext* ctx)
{
	MSG msg = {};
	while (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
	{
		if (msg.message == WM_QUIT)
			return DemoAppResult::QuitRequested;
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
	return DemoAppResult::Ok;
}
