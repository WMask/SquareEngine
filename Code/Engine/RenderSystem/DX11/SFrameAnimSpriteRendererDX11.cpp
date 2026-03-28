/***************************************************************************
* SFrameAnimSpriteRendererDX11.cpp
*/

#include "RenderSystem/DX11/SFrameAnimSpriteRendererDX11.h"
#include "RenderSystem/DX11/SRenderSystemDX11.h"
#include "RenderSystem/SECSComponents.h"
#include "Core/SException.h"


void SFrameAnimSpriteRendererDX11::Render(float deltaSeconds, float gameTime)
{
	S_TRY

	auto shader = renderSystemDX11.FindShader(shaderName);
	auto world = renderSystemDX11.GetWorld();
	if (!shader || !world || !d3dDeviceContext || !spriteVertexBuffer || !spriteIndexBuffer || !instanceBuffer)
	{
		if (renderSystemDX11.IsNeedDebugTrace())
		{
			DebugMsg("[%s] SFrameAnimSpriteRendererDX11::Render(): wrong context\n",
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

	numSprites = 0;
	batchesRendered = 0;
	batchData.reserve(MaxInstancedSpritesCount);
	ID3D11ShaderResourceView* cachedView = nullptr;
	SSize2 cachedTexSize{};
	STexID cachedId = 0;

	const auto& registry = world->GetEntities();
	const auto& spritesView = registry.view<
		const STexturedComponent,
		const SColoredSpriteComponent,
		const SSpriteFrameAnimComponent>();
	auto firstEntity = spritesView.front();
	if (firstEntity != entt::null)
	{
		// cache first entity
		auto [texturedComponent, spr, uv] = registry.get<
			const STexturedComponent,
			const SColoredSpriteComponent,
			const SSpriteFrameAnimComponent>(firstEntity);
		auto [view, texSize] = renderSystemDX11.FindTexture(texturedComponent.texId);
		cachedId = texturedComponent.texId;
		cachedView = view;
		cachedTexSize = texSize;
	}

	// render sprites
	spritesView.each([this, gameTime, &cachedView, &cachedTexSize, &cachedId](
		const STexturedComponent& texturedComponent,
		const SColoredSpriteComponent& spriteComponent,
		const SSpriteFrameAnimComponent& animComponent)
	{
		if (cachedId != texturedComponent.texId)
		{
			if (!batchData.empty())
			{
				// render if texture changed
				RenderBatch(cachedView);
			}

			auto [view, texSize] = renderSystemDX11.FindTexture(texturedComponent.texId);
			cachedId = texturedComponent.texId;
			cachedView = view;
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
		memcpy(instance.colors, spriteComponent.colors, sizeof(SColor4F) * 4);
		memcpy(instance.uvs, tmpUV.uvs, sizeof(SVector2) * 4);
		batchData.push_back(instance);

		if (batchData.size() == MaxInstancedSpritesCount)
		{
			// render if max number reached
			RenderBatch(cachedView);
		}

		numSprites++;
	});

	if (!batchData.empty())
	{
		// render last
		RenderBatch(cachedView);
	}

	if (renderSystemDX11.IsNeedDebugTrace())
	{
		renderSystemDX11.AddDrawCalls(batchesRendered);
		DebugMsg("[%s] SFrameAnimSpriteRendererDX11::Render(): %d batches, %d sprite instances\n",
			GetTimeStamp(std::chrono::system_clock::now()).c_str(), batchesRendered, numSprites);
	}

	S_CATCH{ S_THROW("SFrameAnimSpriteRendererDX11::Render()") }
}

void SFrameAnimSpriteRendererDX11::RenderBatch(ID3D11ShaderResourceView* view)
{
	// fill instanced vertex buffer
	const std::uint32_t numInstances = batchData.size();
	D3D11_MAPPED_SUBRESOURCE mappedResource{};
	if (FAILED(d3dDeviceContext->Map(instanceBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource)))
	{
		throw std::exception("Cannot update vertex buffer");
	}
	memcpy(mappedResource.pData, batchData.data(), sizeof(DX11TEXTUREDSPRITEINSTANCE) * numInstances);
	d3dDeviceContext->Unmap(instanceBuffer.Get(), 0);

	if (view)
	{
		d3dDeviceContext->PSSetShaderResources(0, 1, &view);
	}

	// render sprites
	d3dDeviceContext->DrawIndexedInstanced(4, numInstances, 0, 0, 0);

	// cleanup batch data
	batchData.clear();
	batchesRendered++;
}
