/***************************************************************************
* SGuiSystem.cpp
*/

#include "World/SGuiSystem.h"
#include "World/SWorldModule.h"
#include "Application/SInputInterface.h"
#include "RenderSystem/SECSComponents.h"



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

	const SVector2 scale = world->GetScale().GetScale();
	const float xf = static_cast<float>(x / scale.x);
	const float yf = static_cast<float>(y / scale.y);

	auto& registry = world->GetEntities();
	const auto& spritesView = registry.view<SColoredSpriteComponent, SWidgetComponent>();
	spritesView.each([this, xf, yf, btn, state](
		const entt::entity entity,
		SColoredSpriteComponent& spriteComponent,
		SWidgetComponent& widgetComponent)
	{
		SRectF rect = SConvert::ToRectF(spriteComponent.position, spriteComponent.size);
		const SPoint2 posOnWidget = SConvert::ToPoint2(SPoint2F{ xf - rect.left, yf - rect.top });
		const bool bIsOver = Contains(rect, SPoint2F{ xf, yf });
		if (bIsOver)
		{
			onMouseEvent.trigger<SMouseButtonEvent>({ entity, posOnWidget, btn, state });
		}
	});
}

void SGuiSystem::OnMouseMove(std::int32_t x, std::int32_t y, const SAppContext& context)
{
	if (!world || !input) return;

	const SVector2 scale = world->GetScale().GetScale();
	const std::int32_t sx = static_cast<float>(x) / scale.x;
	const std::int32_t sy = static_cast<float>(y) / scale.y;

	auto& registry = world->GetEntities();
	const auto& spritesView = registry.view<SColoredSpriteComponent, SWidgetComponent>();
	spritesView.each([this, sx, sy](
		const entt::entity entity,
		SColoredSpriteComponent& spriteComponent,
		SWidgetComponent& widgetComponent)
	{
		SRect rect = SConvert::ToRect(spriteComponent.position, spriteComponent.size);
		const SPoint2 posOnWidget = SPoint2{ sx - rect.left, sy - rect.top };
		const bool bHovered = Contains(rect, SPoint2{ sx, sy });
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
				onMouseEvent.trigger<SMouseLeaveEvent>({ entity });
			}

		}
	});
}
