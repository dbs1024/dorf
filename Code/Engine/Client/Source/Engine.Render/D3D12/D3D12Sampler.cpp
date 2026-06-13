// Copyright (c) Darrin Stewart. All rights reserved.
#include "Engine.Render/Rhi/RhiDevice.h"
#include "D3D12Device.h"

#include "Core.Base/Misc.h"

#include <cstring>

static D3D12_FILTER toD3D12Filter(RhiFilter filter)
{
	switch (filter)
	{
		case RhiFilter::MinMagMipPoint: return D3D12_FILTER_MIN_MAG_MIP_POINT;
		case RhiFilter::MinMagPointMipLinear: return D3D12_FILTER_MIN_MAG_POINT_MIP_LINEAR;
		case RhiFilter::MinPointMagLinearMipPoint: return D3D12_FILTER_MIN_POINT_MAG_LINEAR_MIP_POINT;
		case RhiFilter::MinPointMagMipLinear: return D3D12_FILTER_MIN_POINT_MAG_MIP_LINEAR;
		case RhiFilter::MinLinearMagMipPoint: return D3D12_FILTER_MIN_LINEAR_MAG_MIP_POINT;
		case RhiFilter::MinLinearMagPointMipLinear: return D3D12_FILTER_MIN_LINEAR_MAG_POINT_MIP_LINEAR;
		case RhiFilter::MinMagLinearMipPoint: return D3D12_FILTER_MIN_MAG_LINEAR_MIP_POINT;
		case RhiFilter::MinMagMipLinear: return D3D12_FILTER_MIN_MAG_MIP_LINEAR;
		case RhiFilter::Anisotropic: return D3D12_FILTER_ANISOTROPIC;
		case RhiFilter::ComparisonMinMagMipPoint: return D3D12_FILTER_COMPARISON_MIN_MAG_MIP_POINT;
		case RhiFilter::ComparisonMinMagPointMipLinear: return D3D12_FILTER_COMPARISON_MIN_MAG_POINT_MIP_LINEAR;
		case RhiFilter::ComparisonMinPointMagLinearMipPoint: return D3D12_FILTER_COMPARISON_MIN_POINT_MAG_LINEAR_MIP_POINT;
		case RhiFilter::ComparisonMinPointMagMipLinear: return D3D12_FILTER_COMPARISON_MIN_POINT_MAG_MIP_LINEAR;
		case RhiFilter::ComparisonMinLinearMagMipPoint: return D3D12_FILTER_COMPARISON_MIN_LINEAR_MAG_MIP_POINT;
		case RhiFilter::ComparisonMinLinearMagPointMipLinear: return D3D12_FILTER_COMPARISON_MIN_LINEAR_MAG_POINT_MIP_LINEAR;
		case RhiFilter::ComparisonMinMagLinearMipPoint: return D3D12_FILTER_COMPARISON_MIN_MAG_LINEAR_MIP_POINT;
		case RhiFilter::ComparisonMinMagMipLinear: return D3D12_FILTER_COMPARISON_MIN_MAG_MIP_LINEAR;
		case RhiFilter::ComparisonAnisotropic: return D3D12_FILTER_COMPARISON_ANISOTROPIC;
		case RhiFilter::MinimumMinMagMipPoint: return D3D12_FILTER_MINIMUM_MIN_MAG_MIP_POINT;
		case RhiFilter::MinimumMinMagPointMipLinear: return D3D12_FILTER_MINIMUM_MIN_MAG_POINT_MIP_LINEAR;
		case RhiFilter::MinimumMinPointMagLinearMipPoint: return D3D12_FILTER_MINIMUM_MIN_POINT_MAG_LINEAR_MIP_POINT;
		case RhiFilter::MinimumMinPointMagMipLinear: return D3D12_FILTER_MINIMUM_MIN_POINT_MAG_MIP_LINEAR;
		case RhiFilter::MinimumMinLinearMagMipPoint: return D3D12_FILTER_MINIMUM_MIN_LINEAR_MAG_MIP_POINT;
		case RhiFilter::MinimumMinLinearMagPointMipLinear: return D3D12_FILTER_MINIMUM_MIN_LINEAR_MAG_POINT_MIP_LINEAR;
		case RhiFilter::MinimumMinMagLinearMipPoint: return D3D12_FILTER_MINIMUM_MIN_MAG_LINEAR_MIP_POINT;
		case RhiFilter::MinimumMinMagMipLinear: return D3D12_FILTER_MINIMUM_MIN_MAG_MIP_LINEAR;
		case RhiFilter::MinimumAnisotropic: return D3D12_FILTER_MINIMUM_ANISOTROPIC;
		case RhiFilter::MaximumMinMagMipPoint: return D3D12_FILTER_MAXIMUM_MIN_MAG_MIP_POINT;
		case RhiFilter::MaximumMinMagPointMipLinear: return D3D12_FILTER_MAXIMUM_MIN_MAG_POINT_MIP_LINEAR;
		case RhiFilter::MaximumMinPointMagLinearMipPoint: return D3D12_FILTER_MAXIMUM_MIN_POINT_MAG_LINEAR_MIP_POINT;
		case RhiFilter::MaximumMinPointMagMipLinear: return D3D12_FILTER_MAXIMUM_MIN_POINT_MAG_MIP_LINEAR;
		case RhiFilter::MaximumMinLinearMagMipPoint: return D3D12_FILTER_MAXIMUM_MIN_LINEAR_MAG_MIP_POINT;
		case RhiFilter::MaximumMinLinearMagPointMipLinear: return D3D12_FILTER_MAXIMUM_MIN_LINEAR_MAG_POINT_MIP_LINEAR;
		case RhiFilter::MaximumMinMagLinearMipPoint: return D3D12_FILTER_MAXIMUM_MIN_MAG_LINEAR_MIP_POINT;
		case RhiFilter::MaximumMinMagMipLinear: return D3D12_FILTER_MAXIMUM_MIN_MAG_MIP_LINEAR;
	}
	ACE_ASSERT(false);
	return D3D12_FILTER_MIN_MAG_MIP_POINT;
}

static D3D12_TEXTURE_ADDRESS_MODE toD3D12TextureAddressMode(RhiTextureAddressMode mode)
{
	switch (mode)
	{
		case RhiTextureAddressMode::Wrap:       return D3D12_TEXTURE_ADDRESS_MODE_WRAP;
		case RhiTextureAddressMode::Mirror:     return D3D12_TEXTURE_ADDRESS_MODE_MIRROR;
		case RhiTextureAddressMode::Clamp:      return D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
		case RhiTextureAddressMode::Border:     return D3D12_TEXTURE_ADDRESS_MODE_BORDER;
		case RhiTextureAddressMode::MirrorOnce: return D3D12_TEXTURE_ADDRESS_MODE_MIRROR_ONCE;
	}
	ACE_ASSERT(false);
	return D3D12_TEXTURE_ADDRESS_MODE_WRAP;
}

static D3D12_COMPARISON_FUNC toD3D12ComparisonFunc(RhiComparisonFunc func)
{
	switch (func)
	{
		case RhiComparisonFunc::None:         return D3D12_COMPARISON_FUNC_NONE;
		case RhiComparisonFunc::Never:        return D3D12_COMPARISON_FUNC_NEVER;
		case RhiComparisonFunc::Less:         return D3D12_COMPARISON_FUNC_LESS;
		case RhiComparisonFunc::Equal:        return D3D12_COMPARISON_FUNC_EQUAL;
		case RhiComparisonFunc::LessEqual:    return D3D12_COMPARISON_FUNC_LESS_EQUAL;
		case RhiComparisonFunc::Greater:      return D3D12_COMPARISON_FUNC_GREATER;
		case RhiComparisonFunc::NotEqual:     return D3D12_COMPARISON_FUNC_NOT_EQUAL;
		case RhiComparisonFunc::GreaterEqual: return D3D12_COMPARISON_FUNC_GREATER_EQUAL;
		case RhiComparisonFunc::Always:       return D3D12_COMPARISON_FUNC_ALWAYS;
	}
	ACE_ASSERT(false);
	return D3D12_COMPARISON_FUNC_NONE;
}

static D3D12_SAMPLER_DESC toD3D12SamplerDesc(const RhiSamplerDesc& desc)
{
	D3D12_SAMPLER_DESC d3dDesc = {};
	d3dDesc.Filter         = toD3D12Filter(desc.filter);
	d3dDesc.AddressU       = toD3D12TextureAddressMode(desc.addressU);
	d3dDesc.AddressV       = toD3D12TextureAddressMode(desc.addressV);
	d3dDesc.AddressW       = toD3D12TextureAddressMode(desc.addressW);
	d3dDesc.MipLODBias     = desc.mipLodBias;
	d3dDesc.MaxAnisotropy  = clamp(desc.maxAnisotropy, 1u, 16u);
	d3dDesc.ComparisonFunc = toD3D12ComparisonFunc(desc.comparisonFunc);
	d3dDesc.BorderColor[0] = desc.borderColor.x;
	d3dDesc.BorderColor[1] = desc.borderColor.y;
	d3dDesc.BorderColor[2] = desc.borderColor.z;
	d3dDesc.BorderColor[3] = desc.borderColor.w;
	d3dDesc.MinLOD         = desc.minLod;
	d3dDesc.MaxLOD         = desc.maxLod;
	return d3dDesc;
}

RhiSampler* rhiCreateSampler(RhiDevice* device, const RhiSamplerDesc& desc)
{
	RhiSampler* sampler = new RhiSampler;
	memset(sampler, 0, sizeof(*sampler));
	sampler->samplerHandle = allocPersistentDescriptor(&device->gpuSamplerHeap);

	D3D12_SAMPLER_DESC d3dDesc = toD3D12SamplerDesc(desc);
	D3D12_CPU_DESCRIPTOR_HANDLE cpuHandle = getD3D12CpuDescriptorHandle(&device->gpuSamplerHeap, sampler->samplerHandle);
	device->d3dDevice->CreateSampler(&d3dDesc, cpuHandle);

	return sampler;
}

void rhiDestroySampler(RhiDevice* device, RhiSampler* sampler)
{
	if (!sampler)
		return;

	freePersistentDescriptor(&device->gpuSamplerHeap, sampler->samplerHandle);
	delete sampler;
}
