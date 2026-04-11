/***************************************************************************
* SMeshRenderSystemDX11.cpp
*/

#include "RenderSystem/DX11/SMeshRenderSystemDX11.h"
#include "RenderSystem/DX11/SConstantBuffersDX11.h"
#include "RenderSystem/SECSComponents.h"
#include "Core/SException.h"


static const char* StaticMesh3dShaderName = "StaticMesh3d.shader";

SMeshRenderSystemDX11::SMeshRenderSystemDX11(IRenderSystemDX11& renderSystem) : renderSystemDX11(renderSystem)
{
}

SMeshRenderSystemDX11::~SMeshRenderSystemDX11()
{
	Shutdown();
}

void SMeshRenderSystemDX11::Shutdown()
{
	if (instanceBuffer)
	{
		instanceBuffer.Reset();
	}

	cachedVB = nullptr;
	cachedIB = nullptr;
}

bool SMeshRenderSystemDX11::CheckShaderName(const std::string& inShaderName)
{
	if (inShaderName.ends_with(StaticMesh3dShaderName))
	{
		shaderName = inShaderName;
		return true;
	}

	return false;
}

void SMeshRenderSystemDX11::Setup(IRenderSystem::SShaderData& shaderData)
{
	S_TRY

	auto& shaderDataDX11 = static_cast<SShaderDataDX11&>(shaderData);
	auto d3dDevice = renderSystemDX11.GetDevice();
	if (!d3dDevice)
	{
		throw std::exception("Invalid render device");
	}

	// create input layouts
	D3D11_INPUT_ELEMENT_DESC layout[] =
	{
		{ "SV_Position",   0, DXGI_FORMAT_R32G32B32_FLOAT,    0, 0,   D3D11_INPUT_PER_VERTEX_DATA,   0 },
		{ "NORMAL",        0, DXGI_FORMAT_R32G32B32_FLOAT,    0, 12,  D3D11_INPUT_PER_VERTEX_DATA,   0 },
		{ "TEXCOORD",      0, DXGI_FORMAT_R32G32_FLOAT,       0, 24,  D3D11_INPUT_PER_VERTEX_DATA,   0 },
		{ "INSTANCEPOS",   0, DXGI_FORMAT_R32G32B32_FLOAT,    1, 0,   D3D11_INPUT_PER_INSTANCE_DATA, 1 },
		{ "INSTANCEROT",   0, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, 12,  D3D11_INPUT_PER_INSTANCE_DATA, 1 },
		{ "INSTANCESCALE", 0, DXGI_FORMAT_R32G32B32_FLOAT,    1, 28,  D3D11_INPUT_PER_INSTANCE_DATA, 1 },
		{ "INSTANCECOLOR", 0, DXGI_FORMAT_R32G32B32_FLOAT,    1, 40,  D3D11_INPUT_PER_INSTANCE_DATA, 1 }
	};

	if (FAILED(d3dDevice->CreateInputLayout(layout, std::size(layout),
		shaderDataDX11.vsCode->GetBufferPointer(),
		shaderDataDX11.vsCode->GetBufferSize(),
		shaderDataDX11.layout.GetAddressOf())))
	{
		throw std::exception("Cannot create input layout");
	}

	// create instance buffer
	D3D11_BUFFER_DESC bufferDesc{};
	bufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	bufferDesc.ByteWidth = sizeof(DX11MESHINSTANCE) * SConst::MaxInstancedMeshesCount;
	bufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	if (FAILED(d3dDevice->CreateBuffer(&bufferDesc, NULL, instanceBuffer.GetAddressOf())))
	{
		throw std::exception("Cannot create vertex buffer");
	}

	S_CATCH{ S_THROW("SMeshRenderSystemDX11::Setup()") }
}

void SMeshRenderSystemDX11::Render(float deltaSeconds)
{
	S_TRY

	auto deviceContext = renderSystemDX11.GetDeviceContext();
	auto shader = renderSystemDX11.FindShader(shaderName);
	auto world = renderSystemDX11.GetWorld();
	if (!deviceContext || !shader || !world || !instanceBuffer)
	{
		if (renderSystemDX11.IsNeedDebugTrace())
		{
			DebugMsg("[%s] SMeshRenderSystemDX11::Render(): wrong context\n",
				GetTimeStamp(std::chrono::system_clock::now()).c_str());
		}
		return;
	}

	// prepare cached state
	batchData.clear();
	batchData.reserve(SConst::MaxInstancedMeshesCount);
	cachedVB = nullptr;
	cachedIB = nullptr;
	cachedMeshId = 0;
	numMeshInstances = 0;
	batchesRendered = 0;

	// render meshes
	const auto& registry = world->GetEntities();
	const auto& meshesView = registry.view<
		const SStaticMeshComponent,
		const STransform3DComponent>();
	meshesView.each([this, shader](
		const SStaticMeshComponent& meshComponent,
		const STransform3DComponent& transformComponent)
	{
		if (!meshComponent.bVisible) return;
		if (!cachedVB || !cachedIB)
		{
			if (!renderSystemDX11.FindMesh(meshComponent.id, &cachedMaterials, &cachedVB, &cachedIB))
			{
				DebugMsg("[%s] SMeshRenderSystemDX11::Render(): cannot find mesh id=%d\n",
					GetTimeStamp(std::chrono::system_clock::now()).c_str(), meshComponent.id);
				return;
			}
			cachedMeshId = meshComponent.id;
		}

		if (meshComponent.bVisible) cachedMaterialFlags = meshComponent.flags;

		if (cachedMeshId != meshComponent.id)
		{
			if (!batchData.empty())
			{
				// render if mesh changed
				RenderBatch(shader);
			}

			if (!renderSystemDX11.FindMesh(meshComponent.id, &cachedMaterials, &cachedVB, &cachedIB))
			{
				DebugMsg("[%s] SMeshRenderSystemDX11::Render(): cannot find mesh id=%d\n",
					GetTimeStamp(std::chrono::system_clock::now()).c_str(), meshComponent.id);
				return;
			}
			cachedMeshId = meshComponent.id;
		}

		if (!meshComponent.bVisible) return;

		// store instance data
		DX11MESHINSTANCE instance{};/*
		instance.pos = transformComponent.transform.pos;
		instance.rotation = transformComponent.transform.rotation;
		instance.scale = transformComponent.transform.scale;
		instance.tint = SConvert::ToColor3(meshComponent.tint);*/
		instance.pos = SVector3{ 0.0f, 5.0f, 0.0f };
		instance.scale = SConst::OneSVector3;
		instance.rotation = SConvert::ToQuat(0.0f, 0.0f, 0.0f);
		instance.tint = SConst::White3F;
		batchData.push_back(instance);

		if (batchData.size() == SConst::MaxInstancedMeshesCount)
		{
			// render if max number reached
			RenderBatch(shader);
		}

		numMeshInstances++;
	});

	if (!batchData.empty())
	{
		// render last
		RenderBatch(shader);
	}

	if (renderSystemDX11.IsNeedDebugTrace())
	{
		renderSystemDX11.AddDrawCalls(batchesRendered);
		DebugMsg("[%s] SMeshRenderSystemDX11::Render(): %d batches, %d mesh instances\n",
			GetTimeStamp(std::chrono::system_clock::now()).c_str(), batchesRendered, numMeshInstances);
	}

	S_CATCH{ S_THROW("SMeshRenderSystemDX11::Render()") }
}

void SMeshRenderSystemDX11::RenderBatch(const SShaderDataDX11* shader)
{
	auto deviceContext = renderSystemDX11.GetDeviceContext();
	if (deviceContext && shader && cachedVB && cachedIB)
	{
		const std::uint32_t numInstances = batchData.size();

		// fill instanced vertex buffer
		D3D11_MAPPED_SUBRESOURCE mappedResource{};
		if (FAILED(deviceContext->Map(instanceBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource)))
		{
			throw std::exception("Cannot update vertex buffer");
		}
		memcpy(mappedResource.pData, batchData.data(), sizeof(DX11MESHINSTANCE) * numInstances);
		deviceContext->Unmap(instanceBuffer.Get(), 0);

		// setup context
		ID3D11Buffer* buffers[2] = { cachedVB, instanceBuffer.Get() };
		static UINT strides[2] = { sizeof(SVertex), sizeof(DX11MESHINSTANCE) };
		static UINT offsets[2] = { 0, 0 };
		deviceContext->IASetVertexBuffers(0, 2, buffers, strides, offsets);
		deviceContext->IASetIndexBuffer(cachedIB, DXGI_FORMAT_R16_UINT, 0);
		deviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		deviceContext->IASetInputLayout(shader->layout.Get());
		deviceContext->VSSetShader(shader->vs.Get(), NULL, 0);
		deviceContext->PSSetShader(shader->ps.Get(), NULL, 0);

		auto defaultTexture = renderSystemDX11.GetConstantBuffers().defaultTextureView.Get();
		std::uint32_t indexOffset = 0;
		std::uint32_t vertexOffset = 0;
		for (auto& material : cachedMaterials)
		{
			// set textures
			auto baseTexId = ResourceID<STexID>(material.baseTexture.string());
			auto normTexId = ResourceID<STexID>(material.normTexture.string());
			auto rmaTexId = ResourceID<STexID>(material.rmaTexture.string());
			auto emiTexId = ResourceID<STexID>(material.emiTexture.string());

			auto [baseView, size1] = renderSystemDX11.FindTexture(baseTexId);
			if (baseView) deviceContext->PSSetShaderResources(0, 1, &baseView);

			auto [normView, size2] = renderSystemDX11.FindTexture(normTexId);
			if (normView) deviceContext->PSSetShaderResources(1, 1, &normView);

			auto [rmaView, size3] = renderSystemDX11.FindTexture(rmaTexId);
			if (rmaView) deviceContext->PSSetShaderResources(2, 1, &rmaView);

			auto [emiView, size4] = renderSystemDX11.FindTexture(emiTexId);
			if (emiView) deviceContext->PSSetShaderResources(3, 1, &emiView);

			// setup material flags
			SMaterialBuffer matFlags {
				(cachedMaterialFlags.bHasBaseTexture && baseView) ? 1 : 0,
				(cachedMaterialFlags.bHasNormTexture && normView) ? 1 : 0,
				(cachedMaterialFlags.bHasRMATexture && rmaView) ? 1 : 0,
				(cachedMaterialFlags.bHasEmiTexture && emiView) ? 1 : 0
			};
			deviceContext->UpdateSubresource(renderSystemDX11.GetConstantBuffers().materialBuffer.Get(), 0, NULL, &matFlags, 0, 0);

			// render meshes
			deviceContext->DrawIndexedInstanced(material.numIndices, numInstances, indexOffset, vertexOffset, 0);

			vertexOffset += material.numVertices;
			indexOffset += material.numIndices;
			batchesRendered++;
		}
	}

	batchData.clear();
}
