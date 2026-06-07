// Copyright (c) by Darrin Stewart

#ifdef __INTELLISENSE__
#define __SHADER_STAGE_VERTEX 1
#define __SHADER_STAGE_PIXEL 1
#endif

struct DebugDrawConstants
{
	// Keep in sync with cpp code
	matrix localToScreen;
	uint enableClip;
	uint3 pad;
};

cbuffer cb0 : register(b0)
{
	DebugDrawConstants g_constants;
}

//#ifdef __SHADER_STAGE_PIXEL
//Texture2D<float4> g_texture : register(t0);
//SamplerState g_sampler : register(s0);
//#endif

struct VsInput
{
	float3 position : POSITION;
	float2 uv : TEXCOORD0;
	float3 normal : NORMAL;
	float4 color : COLOR;
};

struct PsInput
{
	float4 position : SV_Position;
	float2 uv : TEXCOORD0;
	float4 color : COLOR;
};

PsInput vsMain(VsInput input)
{
	PsInput output;
	output.position = mul(g_constants.localToScreen, float4(input.position, 1.0f));
	output.uv = input.uv;
	output.color = input.color;
	return output;
}

float4 psMain(PsInput input) : SV_Target0
{
	//float4 texColor = g_texture.Sample(g_sampler, input.uv);
	//float4 result = input.color * texColor;

	//if (g_constants.enableClip)
	//	clip(result.a - 0.5f);
		
	return float4(input.color);
}