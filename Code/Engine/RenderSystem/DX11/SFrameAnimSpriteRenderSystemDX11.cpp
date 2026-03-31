/***************************************************************************
* SFrameAnimSpriteRenderSystemDX11.cpp
*/

#include "RenderSystem/DX11/SFrameAnimSpriteRenderSystemDX11.h"
#include "RenderSystem/DX11/SRenderSystemDX11.h"
#include "RenderSystem/SECSComponents.h"
#include "Core/SException.h"


void SFrameAnimSpriteRenderSystemDX11::Render(float deltaSeconds, float gameTime)
{
	S_TRY

	auto shader = renderSystemDX11.FindShader(shaderName);
	auto world = renderSystemDX11.GetWorld();
	if (!shader || !world || !d3dDeviceContext || !spriteVertexBuffer || !spriteIndexBuffer || !instanceBuffer)
	{
		if (renderSystemDX11.IsNeedDebugTrace())
		{
			DebugMsg("[%s] SFrameAnimSpriteRenderSystemDX11::Render(): wrong context\n",
				GetTimeStamp(std::chrono::system_clock::now()).c_str());
		}
		return;
	}

	// setup context
	ID3D11Buffer* buffers[2] = { spriteVertexBuffer, instanceBuffer.Get() };
	static UINT strides[2] = { sizeof(DX11SPRITEVERTEX), sizeof(DX11TEXTUREDSPRITEINSTANCE) };
	static UINT offsets[2] = { 0, 0 };
	d3dDeviceContext->IASetVertexBuffers(0, 2, buffers, strides, offsets);
	d3dDeviceContext->IASetIndexBuffer(spriteIndexBuffer, DXGI_FORMAT_R16_UINT, 0);
	d3dDeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
	d3dDeviceContext->IASetInputLayout(shader->layout.Get());
	d3dDeviceContext->VSSetShader(shader->vs.Get(), NULL, 0);
	d3dDeviceContext->PSSetShader(shader->ps.Get(), NULL, 0);

	// prepare cached state
	if (batchData.capacity() < MaxInstancedSpritesCount) batchData.reserve(MaxInstancedSpritesCount);
	batchesRendered = 0;
	numSprites = 0;
	cachedTexView = nullptr;
	cachedTexId = 0;

	// render sprites
	const auto& registry = world->GetEntities();
	const auto& spritesView = registry.view<
		const STexturedComponent,
		const SColoredComponent,
		const SSpriteComponent,
		const SSpriteFrameAnimComponent>();
	spritesView.each([this, gameTime](
		const STexturedComponent& texturedComponent,
		const SColoredComponent& coloredComponent,
		const SSpriteComponent& spriteComponent,
		const SSpriteFrameAnimComponent& animComponent)
	{
		if (!spriteComponent.bVisible) return;
		if (!cachedTexView)
		{
			auto [view, texSize] = renderSystemDX11.FindTexture(texturedComponent.texId);
			cachedTexId = texturedComponent.texId;
			cachedTexView = view;
			cachedTexSize = texSize;
		}

		if (cachedTexId != texturedComponent.texId)
		{
			if (!batchData.empty())
			{
				// render if texture changed
				RenderBatch();
			}

			auto [view, texSize] = renderSystemDX11.FindTexture(texturedComponent.texId);
			cachedTexId = texturedComponent.texId;
			cachedTexView = view;
			cachedTexSize = texSize;
		}

		// generate frame uv
		SSpriteUVComponent tmpUV;
		animComponent.GenerateFrameUV(gameTime, cachedTexSize, tmpUV);

		// store instance data
		DX11TEXTUREDSPRITEINSTANCE instance{};
		instance.pos = spriteComponent.position;
		instance.rotation = spriteComponent.rotation;
		instance.scale = SConvert::ToVector2(spriteComponent.size);
		memcpy(instance.colors, coloredComponent.colors, sizeof(SColor4F) * 4);
		memcpy(instance.uvs, tmpUV.uvs, sizeof(SVector2) * 4);
		batchData.push_back(instance);

		if (batchData.size() == MaxInstancedSpritesCount)
		{
			// render if max number reached
			RenderBatch();
		}

		numSprites++;
	});

	if (!batchData.empty())
	{
		// render last
		RenderBatch();
	}

	if (renderSystemDX11.IsNeedDebugTrace())
	{
		renderSystemDX11.AddDrawCalls(batchesRendered);
		DebugMsg("[%s] SFrameAnimSpriteRenderSystemDX11::Render(): %d batches, %d sprite instances\n",
			GetTimeStamp(std::chrono::system_clock::now()).c_str(), batchesRendered, numSprites);
	}

	S_CATCH{ S_THROW("SFrameAnimSpriteRenderSystemDX11::Render()") }
}

void SFrameAnimSpriteRenderSystemDX11::RenderBatch()
{
	if (!cachedTexView)
	{
		// skip rendering if texture not loaded yet
		return;
	}

	// fill instanced vertex buffer
	const std::uint32_t numInstances = batchData.size();
	D3D11_MAPPED_SUBRESOURCE mappedResource{};
	if (FAILED(d3dDeviceContext->Map(instanceBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource)))
	{
		throw std::exception("Cannot update vertex buffer");
	}
	memcpy(mappedResource.pData, batchData.data(), sizeof(DX11TEXTUREDSPRITEINSTANCE) * numInstances);
	d3dDeviceContext->Unmap(instanceBuffer.Get(), 0);

	// render sprites
	d3dDeviceContext->PSSetShaderResources(0, 1, &cachedTexView);
	d3dDeviceContext->DrawIndexedInstanced(4, numInstances, 0, 0, 0);

	// cleanup batch data
	batchData.clear();
	batchesRendered++;
}
