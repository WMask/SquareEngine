/***************************************************************************
* SFrameAnimSpriteRenderSystemDX11.cpp
*/

#include "RenderSystem/DX11/SFrameAnimSpriteRenderSystemDX11.h"
#include "RenderSystem/DX11/SRenderSystemTypesDX11.h"
#include "RenderSystem/SECSComponents.h"
#include "Core/SException.h"


void SFrameAnimSpriteRenderSystemDX11::Render(float deltaSeconds, float gameTime)
{
	S_TRY

	auto deviceContext = renderSystemDX11.GetDeviceContext();
	auto shader = renderSystemDX11.FindShader(shaderName);
	auto world = renderSystemDX11.GetWorld();
	if (!deviceContext || !shader || !world || !spriteVertexBuffer || !spriteIndexBuffer || !instanceBuffer)
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
	deviceContext->IASetVertexBuffers(0, 2, buffers, strides, offsets);
	deviceContext->IASetIndexBuffer(spriteIndexBuffer, DXGI_FORMAT_R16_UINT, 0);
	deviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
	deviceContext->IASetInputLayout(shader->layout.Get());
	deviceContext->VSSetShader(shader->vs.Get(), NULL, 0);
	deviceContext->PSSetShader(shader->ps.Get(), NULL, 0);

	// prepare cached state
	if (batchData.capacity() < SConst::MaxInstancedSpritesCount) batchData.reserve(SConst::MaxInstancedSpritesCount);
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
			auto [view, texSize] = renderSystemDX11.FindTexture(texturedComponent.id);
			cachedTexId = texturedComponent.id;
			cachedTexView = view;
			cachedTexSize = texSize;
		}

		if (cachedTexId != texturedComponent.id)
		{
			if (!batchData.empty())
			{
				// render if texture changed
				RenderBatch();
			}

			auto [view, texSize] = renderSystemDX11.FindTexture(texturedComponent.id);
			cachedTexId = texturedComponent.id;
			cachedTexView = view;
			cachedTexSize = texSize;
		}

		// generate frame uv
		SSpriteUV tmpUV;
		animComponent.GenerateFrameUV(gameTime, cachedTexSize, tmpUV);

		// store instance data
		DX11TEXTUREDSPRITEINSTANCE instance{};
		instance.pos = spriteComponent.position;
		instance.rotation = spriteComponent.rotation;
		instance.scale = SConvert::ToVector2(spriteComponent.size);
		memcpy(instance.colors, coloredComponent.colors, sizeof(SColor4F) * 4);
		memcpy(instance.uvs, tmpUV.uvs, sizeof(SVector2) * 4);
		batchData.push_back(instance);

		if (batchData.size() == SConst::MaxInstancedSpritesCount)
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
	auto deviceContext = renderSystemDX11.GetDeviceContext();
	if (deviceContext && cachedTexView)
	{
		// fill instanced vertex buffer
		const std::uint32_t numInstances = batchData.size();
		D3D11_MAPPED_SUBRESOURCE mappedResource{};
		if (FAILED(deviceContext->Map(instanceBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource)))
		{
			throw std::exception("Cannot update vertex buffer");
		}
		memcpy(mappedResource.pData, batchData.data(), sizeof(DX11TEXTUREDSPRITEINSTANCE) * numInstances);
		deviceContext->Unmap(instanceBuffer.Get(), 0);

		// render sprites
		deviceContext->PSSetShaderResources(0, 1, &cachedTexView);
		deviceContext->DrawIndexedInstanced(4, numInstances, 0, 0, 0);
		batchesRendered++;
	}

	batchData.clear();
}
