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

	const SVector2 scale = world->GetScale().GetScale();
	const SPoint2 scaledMousePos = SPoint2 {
		static_cast<std::int32_t>(x / scale.x),
		static_cast<std::int32_t>(y / scale.y)
	};

	auto& registry = world->GetEntities();
	const auto& spritesView = registry.view<SColoredSpriteComponent, SWidgetComponent>();
	spritesView.each([this, scaledMousePos, btn, state](
		const entt::entity entity,
		SColoredSpriteComponent& spriteComponent,
		SWidgetComponent& widgetComponent)
	{
		const SRect rect = SConvert::ToRect(spriteComponent.position, spriteComponent.size);
		const SPoint2 posOnWidget = SPoint2{ scaledMousePos.x - rect.left, scaledMousePos.y - rect.top };
		const bool bHovered = Contains(rect, scaledMousePos);
		if (bHovered)
		{
			onMouseEvent.trigger<SMouseButtonEvent>({ entity, posOnWidget, btn, state });
		}
	});
}

void SGuiSystem::OnMouseMove(std::int32_t x, std::int32_t y, const SAppContext& context)
{
	if (!world || !input) return;

	const SVector2 scale = world->GetScale().GetScale();
	const SPoint2 scaledMousePos = SPoint2{
		static_cast<std::int32_t>(x / scale.x),
		static_cast<std::int32_t>(y / scale.y)
	};

	auto& registry = world->GetEntities();
	const auto& spritesView = registry.view<SColoredSpriteComponent, SWidgetComponent>();
	spritesView.each([this, scaledMousePos](
		const entt::entity entity,
		SColoredSpriteComponent& spriteComponent,
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
				onMouseEvent.trigger<SMouseLeaveEvent>({ entity });
			}

		}
	});
}
