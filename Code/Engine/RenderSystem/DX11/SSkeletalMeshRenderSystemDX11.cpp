/***************************************************************************
* SSkeletalMeshRenderSystemDX11.cpp
*/

#include "RenderSystem/DX11/SSkeletalMeshRenderSystemDX11.h"
#include "RenderSystem/DX11/SConstantBuffersDX11.h"
#include "RenderSystem/SECSComponents.h"
#include "Core/SException.h"


static const char* SkeletalMesh3dShaderName = "SkeletalMesh3d.shader";

SSkeletalMeshRenderSystemDX11::SSkeletalMeshRenderSystemDX11(IRenderSystemDX11& renderSystem)
	: renderSystemDX11(renderSystem)
{
}

SSkeletalMeshRenderSystemDX11::~SSkeletalMeshRenderSystemDX11()
{
	Shutdown();
}

void SSkeletalMeshRenderSystemDX11::Shutdown()
{
	cachedVB = nullptr;
	cachedIB = nullptr;
}

bool SSkeletalMeshRenderSystemDX11::CheckShaderName(const std::string& inShaderName)
{
	if (inShaderName.ends_with(SkeletalMesh3dShaderName))
	{
		shaderName = inShaderName;
		return true;
	}

	return false;
}

void SSkeletalMeshRenderSystemDX11::Setup(IRenderSystem::SShaderData& shaderData)
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
		{ "TANGENT",       0, DXGI_FORMAT_R32G32B32_FLOAT,    0, 24,  D3D11_INPUT_PER_VERTEX_DATA,   0 },
		{ "TEXCOORD",      0, DXGI_FORMAT_R32G32_FLOAT,       0, 36,  D3D11_INPUT_PER_VERTEX_DATA,   0 },
		{ "BLENDINDICES",  0, DXGI_FORMAT_R32G32B32A32_UINT,  0, 44,  D3D11_INPUT_PER_VERTEX_DATA,   0 },
		{ "BLENDWEIGHT",   0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 60,  D3D11_INPUT_PER_VERTEX_DATA,   0 }
	};

	if (FAILED(d3dDevice->CreateInputLayout(layout, std::size(layout),
		shaderDataDX11.vsCode->GetBufferPointer(),
		shaderDataDX11.vsCode->GetBufferSize(),
		shaderDataDX11.layout.GetAddressOf())))
	{
		throw std::exception("Cannot create input layout");
	}

	S_CATCH{ S_THROW("SSkeletalMeshRenderSystemDX11::Setup()") }
}

void SSkeletalMeshRenderSystemDX11::Render(float deltaSeconds)
{
	S_TRY

	auto deviceContext = renderSystemDX11.GetDeviceContext();
	auto shader = renderSystemDX11.FindShader(shaderName);
	auto world = renderSystemDX11.GetWorld();
	if (!deviceContext || !shader || !world)
	{
		if (renderSystemDX11.IsNeedDebugTrace())
		{
			DebugMsg("[%s] SSkeletalMeshRenderSystemDX11::Render(): wrong context\n",
				GetTimeStamp(std::chrono::system_clock::now()).c_str());
		}
		return;
	}

	// prepare cached state
	cachedVB = nullptr;
	cachedIB = nullptr;
	cachedMeshId = 0;
	meshesRendered = 0;

	// render meshes
	const auto& registry = world->GetEntities();
	const auto& meshesView = registry.view<
		const SSkeletalMeshComponent,
		const STransform3DComponent>();
	meshesView.each([this, shader](
		const SSkeletalMeshComponent& meshComponent,
		const STransform3DComponent& transformComponent)
	{
		if (!meshComponent.bVisible) return;
		if (!cachedVB || !cachedIB)
		{
			if (!renderSystemDX11.FindMesh(meshComponent.id, &cachedMaterials, &cachedVB, &cachedIB, &cachedIbFormat))
			{
				DebugMsg("[%s] SSkeletalMeshRenderSystemDX11::Render(): cannot find mesh id=%d\n",
					GetTimeStamp(std::chrono::system_clock::now()).c_str(), meshComponent.id);
				return;
			}
			cachedMeshId = meshComponent.id;
		}

		if (meshComponent.bVisible)
			cachedMaterialFlags = meshComponent.flags;
		else
			return;

		if (cachedMeshId != meshComponent.id)
		{
			if (!renderSystemDX11.FindMesh(meshComponent.id, &cachedMaterials, &cachedVB, &cachedIB, &cachedIbFormat))
			{
				DebugMsg("[%s] SSkeletalMeshRenderSystemDX11::Render(): cannot find mesh id=%d\n",
					GetTimeStamp(std::chrono::system_clock::now()).c_str(), meshComponent.id);
				return;
			}
			cachedMeshId = meshComponent.id;
		}

		RenderSkeletalMesh(shader, meshComponent, transformComponent.transform);

		meshesRendered++;
	});

	if (renderSystemDX11.IsNeedDebugTrace())
	{
		renderSystemDX11.AddDrawCalls(meshesRendered);
		DebugMsg("[%s] SSkeletalMeshRenderSystemDX11::Render(): %d skeletal meshes rendered\n",
			GetTimeStamp(std::chrono::system_clock::now()).c_str(), meshesRendered);
	}

	S_CATCH{ S_THROW("SSkeletalMeshRenderSystemDX11::Render()") }
}

void SSkeletalMeshRenderSystemDX11::RenderSkeletalMesh(const SShaderDataDX11* shader,
	const SSkeletalMeshComponent& meshComponent, const STransform& transform)
{
	auto& constantBuffers = renderSystemDX11.GetConstantBuffers();
	auto deviceContext = renderSystemDX11.GetDeviceContext();
	if (deviceContext && shader && cachedVB && cachedIB)
	{
		// setup context
		ID3D11Buffer* buffers[] = { cachedVB };
		static UINT stride = sizeof(SBlendVertex);
		static UINT offset = 0u;
		deviceContext->IASetVertexBuffers(0, 1, buffers, &stride, &offset);
		deviceContext->IASetIndexBuffer(cachedIB, cachedIbFormat, 0);
		deviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		deviceContext->IASetInputLayout(shader->layout.Get());
		deviceContext->VSSetShader(shader->vs.Get(), NULL, 0);
		deviceContext->PSSetShader(shader->ps.Get(), NULL, 0);

		auto defaultTexture = constantBuffers.GetDefaultTexture();
		std::uint32_t indexOffset = 0;
		std::uint32_t vertexOffset = 0;
		for (auto& material : cachedMaterials)
		{
			// set textures
			auto [baseView, size1] = renderSystemDX11.FindTexture(material.baseTexId);
			if (baseView) deviceContext->PSSetShaderResources(0, 1, &baseView);

			auto [normView, size2] = renderSystemDX11.FindTexture(material.normTexId);
			if (normView) deviceContext->PSSetShaderResources(1, 1, &normView);

			auto [rmaView, size3] = renderSystemDX11.FindTexture(material.rmaTexId);
			if (rmaView) deviceContext->PSSetShaderResources(2, 1, &rmaView);

			auto [emiView, size4] = renderSystemDX11.FindTexture(material.emiTexId);
			if (emiView) deviceContext->PSSetShaderResources(3, 1, &emiView);

			// setup material flags
			SMaterialFlagsBuffer matFlags {
				(cachedMaterialFlags.bHasBaseTexture && baseView) ? 1u : 0u,
				(cachedMaterialFlags.bHasNormTexture && normView) ? 1u : 0u,
				(cachedMaterialFlags.bHasRMATexture && rmaView) ? 1u : 0u,
				(cachedMaterialFlags.bHasEmiTexture && emiView) ? 1u : 0u
			};
			constantBuffers.UpdateMaterialFlags(renderSystemDX11, matFlags);
			constantBuffers.UpdateTransform(renderSystemDX11, transform);

			// render meshes
			deviceContext->DrawIndexed(material.numIndices, indexOffset, vertexOffset);

			vertexOffset += material.numVertices;
			indexOffset += material.numIndices;
		}
	}
}
