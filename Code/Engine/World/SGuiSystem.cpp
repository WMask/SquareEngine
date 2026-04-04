/***************************************************************************
* SGuiSystem.cpp
*/

#include "World/SGuiSystem.h"
#include "World/SWorldModule.h"
#include "RenderSystem/SECSComponents.h"


TGuiSystemPtr CreateGuiSystem(const SAppContext& context)
{
	auto gui = std::make_unique<SGuiSystem>();
	gui->Init(context.world, context.input);
	return gui;
}

void SGuiSystem::Init(IWorld* inWorld, IInputSystem* inInput)
{
	world = inWorld;
	input = inInput;
}

void SGuiSystem::OnKeys(std::int32_t btn, SKeyState state, const SAppContext& context)
{
}

void SGuiSystem::OnMouseButton(SMouseBtn btn, SKeyState state, std::int32_t x, std::int32_t y, const SAppContext& context)
{
	if (!world || !input) return;

	auto& registry = world->GetEntities();
	const SVector2 scale = world->GetScale().GetScale();
	const SPoint2 scaledMousePos = SPoint2 {
		static_cast<std::int32_t>(x / scale.x),
		static_cast<std::int32_t>(y / scale.y)
	};

	const auto& spritesView = registry.view<SSpriteComponent, SWidgetComponent>();
	spritesView.each([this, scaledMousePos, btn, state](
		const entt::entity entity,
		SSpriteComponent& spriteComponent,
		SWidgetComponent& widgetComponent)
	{
		auto& registry = world->GetEntities();
		const SRect rect = SConvert::ToRect(spriteComponent.position, spriteComponent.size);
		const SPoint2 posOnWidget = SPoint2{ scaledMousePos.x - rect.left, scaledMousePos.y - rect.top };
		const bool bHovered = Contains(rect, scaledMousePos);
		if (bHovered)
		{
			onMouseEvent.trigger<SMouseButtonEvent>({ entity, posOnWidget, btn, state });
			// set presed after delegate to allow check in event
			widgetComponent.bPressed = (state == SKeyState::Down);

			// button logic
			auto buttonComponent = registry.try_get<SButtonComponent>(entity);
			if (buttonComponent)
			{
				auto textComponent = registry.try_get<STextComponent>(entity);
				if (textComponent)
				{
					// update text pos
					SPoint2F offset = SConvert::ToPoint2(widgetComponent.bPressed ? buttonComponent->pressedTextOffset : SConst::ZeroSPoint2);
					SVector3 newPos = buttonComponent->initialTextPos + SVector3{ offset.x, offset.y, 0.0f };
					spriteComponent.position = newPos;
				}
				else
				{
					auto uvComponent = registry.try_get<SSpriteUVComponent>(entity);
					if (uvComponent)
					{
						// update button uv
						const SSpriteUV& btnUV = widgetComponent.bPressed ? buttonComponent->pressedUV : buttonComponent->normalUV;
						memcpy(uvComponent->uvs.uvs, btnUV.uvs, sizeof(SSpriteUV));
					}
				}
			}
		}
	});
}

void SGuiSystem::OnMouseMove(std::int32_t x, std::int32_t y, const SAppContext& context)
{
	if (!world || !input) return;

	auto& registry = world->GetEntities();
	const SVector2 scale = world->GetScale().GetScale();
	const SPoint2 scaledMousePos = SPoint2{
		static_cast<std::int32_t>(x / scale.x),
		static_cast<std::int32_t>(y / scale.y)
	};

	const auto& spritesView = registry.view<SSpriteComponent, SWidgetComponent>();
	spritesView.each([this, scaledMousePos](
		const entt::entity entity,
		SSpriteComponent& spriteComponent,
		SWidgetComponent& widgetComponent)
	{
		const SRect rect = SConvert::ToRect(spriteComponent.position, spriteComponent.size);
		const SPoint2 posOnWidget = SPoint2{ scaledMousePos.x - rect.left, scaledMousePos.y - rect.top };
		const bool bHovered = Contains(rect, scaledMousePos);
		if (bHovered)
		{
			if (!widgetComponent.bHovered)
			{
				widgetComponent.bHovered = true;
				onMouseEvent.trigger<SMouseEnterEvent>({ entity });
			}

			onMouseEvent.trigger<SMouseMoveEvent>({ entity, posOnWidget });
		}
		else
		{
			if (widgetComponent.bHovered)
			{
				widgetComponent.bHovered = false;
				widgetComponent.bPressed = false;
				onMouseEvent.trigger<SMouseLeaveEvent>({ entity });

				// button logic
				auto& registry = world->GetEntities();
				auto buttonComponent = registry.try_get<SButtonComponent>(entity);
				auto textComponent = registry.try_get<STextComponent>(entity);
				if (buttonComponent)
				{
					// go to normal state if mouse leave
					if (textComponent)
					{
						spriteComponent.position = buttonComponent->initialTextPos;
					}
					else
					{
						auto uvComponent = registry.try_get<SSpriteUVComponent>(entity);
						if (uvComponent)
						{
							memcpy(uvComponent->uvs.uvs, buttonComponent->normalUV.uvs, sizeof(SSpriteUV));
						}
					}
				}
			}
		}
	});
}

entt::entity SGuiSystem::MakeColoredSprite(entt::registry& registry,
	const SVector3& pos, const SSize2F& size, SColor4F color)
{
	entt::entity spriteEntity = registry.create();
	auto& sprite = registry.emplace<SSpriteComponent>(spriteEntity, true, 0.0f, pos, size);
	auto& colors = registry.emplace<SColoredComponent>(spriteEntity);
	colors.SetColors(color);

	return spriteEntity;
}

entt::entity SGuiSystem::MakeColoredSprite(
	entt::registry& registry, const SVector3& pos, const SSize2F& size,
	const SColor3& lt, const SColor3& rt, const SColor3& rb, const SColor3& lb)
{
	entt::entity spriteEntity = registry.create();
	auto& sprite = registry.emplace<SSpriteComponent>(spriteEntity, true, 0.0f, pos, size);
	auto& colors = registry.emplace<SColoredComponent>(spriteEntity);
	colors.SetColors(lt, rt, rb, lb);

	return spriteEntity;
}

entt::entity SGuiSystem::MakeTexturedSprite(entt::registry& registry,
	STexID texture, const SVector3& pos, const SSize2F& size, SColor4F color)
{
	entt::entity spriteEntity = registry.create();
	auto& sprite = registry.emplace<SSpriteComponent>(spriteEntity, true, 0.0f, pos, size);
	auto& colors = registry.emplace<SColoredComponent>(spriteEntity);
	colors.SetColors(color);
	auto& texUV = registry.emplace<SSpriteUVComponent>(spriteEntity);
	texUV.SetDefaultUV();
	registry.emplace<STexturedComponent>(spriteEntity, texture);

	return spriteEntity;
}

entt::entity SGuiSystem::MakeAnimatedSprite(entt::registry& registry,
	STexID texture, const SVector3& pos, const SSize2F& size, SColor4F color,
	std::int32_t frameOffset, std::int32_t framesCount,
	std::int32_t framesPerSecond, SSize2 frameSize, float startTime)
{
	entt::entity animatedEntity = registry.create();
	auto& sprite = registry.emplace<SSpriteComponent>(
		animatedEntity, true, 0.0f, pos, size
	);
	auto& colors = registry.emplace<SColoredComponent>(animatedEntity);
	colors.SetColors(color);
	registry.emplace<STexturedComponent>(animatedEntity, texture);
	auto& anim = registry.emplace<SSpriteFrameAnimComponent>(animatedEntity);
	anim.SetAnim(frameOffset, framesCount, framesPerSecond, frameSize, startTime);

	return animatedEntity;
}

entt::entity SGuiSystem::MakeText(entt::registry& registry, SWidgetID widgetId, STextID text, SFontID font,
	const SVector3& pos, const SSize2F& size, SColor4F color, STextAlign align)
{
	entt::entity textEntity = registry.create();
	registry.emplace<SSpriteComponent>(textEntity, true, 0.0f, pos, size);
	registry.emplace<STextComponent>(textEntity, color, text, font, align);
	registry.emplace<SWidgetComponent>(textEntity, widgetId);

	return textEntity;
}

std::pair<entt::entity, entt::entity> SGuiSystem::MakeButtonWithText(entt::registry& registry,
	STexID texture, STextID text, SFontID font, SWidgetID btnWidget, SWidgetID textWidget,
	const SVector3& pos, const SSize2F& size, SColor4F color)
{
	SPoint2 textOffset{ 0, 1 };
	SVector3 textPos = pos + SVector3{ 0.0f, 1.0f, 0.05f };
	SSpriteUVComponent top, bottom;
	bottom.SetBottomHalfUV();
	top.SetTopHalfUV();

	entt::entity buttonEntity = registry.create();
	auto& sprite = registry.emplace<SSpriteComponent>(buttonEntity, true, 0.0f, pos, size);
	auto& colors = registry.emplace<SColoredComponent>(buttonEntity);
	colors.SetColors(color);
	auto& texUV = registry.emplace<SSpriteUVComponent>(buttonEntity);
	texUV.SetTopHalfUV();
	registry.emplace<STexturedComponent>(buttonEntity, texture);
	registry.emplace<SWidgetComponent>(buttonEntity, btnWidget);
	registry.emplace<SButtonComponent>(buttonEntity, textOffset, textPos, bottom.uvs, top.uvs);

	entt::entity textEntity = registry.create();
	registry.emplace<SSpriteComponent>(textEntity, true, 0.0f, textPos, size);
	registry.emplace<STextComponent>(textEntity, color, text, font);
	registry.emplace<SWidgetComponent>(textEntity, textWidget);
	registry.emplace<SButtonComponent>(textEntity, textOffset, textPos);

	return { buttonEntity, textEntity };
}
