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
		const bool bHovered = SMath::Contains(rect, scaledMousePos);
		if (bHovered)
		{
			onMouseEvent.trigger<SMouseButtonEvent>({ entity, posOnWidget, btn, state });
			const bool bWasPressed = widgetComponent.bPressed;
			// set presed after onMouseEvent delegate to allow check in event
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

				// draggable button
				auto draggableComponent = registry.try_get<SDragComponent>(entity);
				if (draggableComponent)
				{
					draggableComponent->offset = SConvert::ToVector2(posOnWidget);
					draggableComponent->bDragging = (state == SKeyState::Down);
				}
			}

			// checkbox logic
			auto checkboxComponent = registry.try_get<SCheckboxComponent>(entity);
			if (checkboxComponent)
			{
				if (state == SKeyState::Up && bWasPressed)
				{
					checkboxComponent->bChecked = !checkboxComponent->bChecked;
				}

				auto uvComponent = registry.try_get<SSpriteUVComponent>(entity);
				if (uvComponent)
				{
					// update button uv
					if (checkboxComponent->bChecked)
					{
						const SSpriteUV& btnUV = widgetComponent.bPressed ? checkboxComponent->pressedCheckedUV : checkboxComponent->normalCheckedUV;
						memcpy(uvComponent->uvs.uvs, btnUV.uvs, sizeof(SSpriteUV));
					}
					else
					{
						const SSpriteUV& btnUV = widgetComponent.bPressed ? checkboxComponent->pressedUV : checkboxComponent->normalUV;
						memcpy(uvComponent->uvs.uvs, btnUV.uvs, sizeof(SSpriteUV));
					}
				}
			}
		}
		else
		{
			if (state == SKeyState::Up)
			{
				// draggable button
				auto draggableComponent = registry.try_get<SDragComponent>(entity);
				if (draggableComponent &&
					draggableComponent->bDragging)
				{
					draggableComponent->bDragging = false;
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
		auto& registry = world->GetEntities();
		const SRect rect = SConvert::ToRect(spriteComponent.position, spriteComponent.size);
		const SPoint2 posOnWidget = SPoint2{ scaledMousePos.x - rect.left, scaledMousePos.y - rect.top };
		const bool bHovered = SMath::Contains(rect, scaledMousePos);
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

				// checkbox logic
				auto checkboxComponent = registry.try_get<SCheckboxComponent>(entity);
				if (checkboxComponent)
				{
					// go to normal state if mouse leave
					auto uvComponent = registry.try_get<SSpriteUVComponent>(entity);
					if (uvComponent)
					{
						auto& uvs = checkboxComponent->bChecked
							? checkboxComponent->normalCheckedUV
							: checkboxComponent->normalUV;
						memcpy(uvComponent->uvs.uvs, &uvs, sizeof(SSpriteUV));
					}
				}
			}
		}

		// draggable button
		auto draggableComponent = registry.try_get<SDragComponent>(entity);
		if (draggableComponent &&
			draggableComponent->bDragging)
		{
			// apply to sprite position
			auto spriteComponent = registry.try_get<SSpriteComponent>(entity);
			if (spriteComponent)
			{
				const SPoint2F pos = SConvert::ToPoint2(scaledMousePos);
				const float clampedX = std::clamp(
					pos.x - draggableComponent->offset.x + spriteComponent->size.width / 2.0f,
					draggableComponent->area.left, draggableComponent->area.right
				);
				const float clampedY = std::clamp(
					pos.y - draggableComponent->offset.y + spriteComponent->size.height / 2.0f,
					draggableComponent->area.top, draggableComponent->area.bottom
				);

				spriteComponent->position.x = clampedX;
				spriteComponent->position.y = clampedY;

				// apply to slider value
				auto sliderComponent = registry.try_get<SSliderComponent>(entity);
				sliderComponent->sliderValue =
					(clampedX - draggableComponent->area.left) /
					(draggableComponent->area.right - draggableComponent->area.left);
				sliderComponent->value = sliderComponent->minValue
					+ (sliderComponent->maxValue - sliderComponent->minValue)
					* sliderComponent->sliderValue;
			}
		}
	});
}

void SGuiSystem::OnMouseLeave()
{
	if (!world) return;

	auto& registry = world->GetEntities();
	const auto& buttonsView = registry.view<SWidgetComponent, SDragComponent>();
	buttonsView.each([this](
		const entt::entity entity,
		SWidgetComponent& widgetComponent,
		SDragComponent& dragComponent)
	{
		// draggable button
		if (dragComponent.bDragging)
		{
			dragComponent.bDragging = false;
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
	texUV.uvs.SetDefaultUV();
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
	STexID texture, STextID text, SFontID font, SWidgetID btnWidget,
	const SVector3& pos, const SSize2F& size, SColor4F color, SColor4F textColor)
{
	SPoint2 textOffset{ 0, 1 };
	SVector3 textPos = pos + SVector3{ 0.0f, 1.0f, 0.05f };
	SSpriteUV normal, pressed;
	pressed.SetBottomHalfUV();
	normal.SetTopHalfUV();

	entt::entity buttonEntity = registry.create();
	auto& sprite = registry.emplace<SSpriteComponent>(buttonEntity, true, 0.0f, pos, size);
	auto& colors = registry.emplace<SColoredComponent>(buttonEntity);
	colors.SetColors(color);
	auto& texUV = registry.emplace<SSpriteUVComponent>(buttonEntity);
	texUV.uvs = normal;
	registry.emplace<STexturedComponent>(buttonEntity, texture);
	registry.emplace<SWidgetComponent>(buttonEntity, btnWidget);
	registry.emplace<SButtonComponent>(buttonEntity, textOffset, textPos, normal, pressed);

	entt::entity textEntity = registry.create();
	registry.emplace<SSpriteComponent>(textEntity, true, 0.0f, textPos, size);
	registry.emplace<STextComponent>(textEntity, textColor, text, font);
	registry.emplace<SWidgetComponent>(textEntity, btnWidget + 1);
	registry.emplace<SButtonComponent>(textEntity, textOffset, textPos);

	return { buttonEntity, textEntity };
}

entt::entity SGuiSystem::MakeCheckbox(entt::registry& registry, STexID texture, SWidgetID widgetId,
	bool bChecked, const SVector3& pos, const SSize2F& size, SColor4F color)
{
	SSpriteUV pressed, normal, pressedChecked, normalChecked;
	pressedChecked.SetRightBottomUV();
	normalChecked.SetRightTopUV();
	pressed.SetLeftBottomUV();
	normal.SetLeftTopUV();

	entt::entity buttonEntity = registry.create();
	auto& sprite = registry.emplace<SSpriteComponent>(buttonEntity, true, 0.0f, pos, size);
	auto& colors = registry.emplace<SColoredComponent>(buttonEntity);
	colors.SetColors(color);
	auto& texUV = registry.emplace<SSpriteUVComponent>(buttonEntity);
	texUV.uvs = bChecked ? normalChecked : normal;
	registry.emplace<STexturedComponent>(buttonEntity, texture);
	registry.emplace<SWidgetComponent>(buttonEntity, widgetId);
	registry.emplace<SCheckboxComponent>(buttonEntity, normal, pressed, normalChecked, pressedChecked, bChecked);

	return buttonEntity;
}

std::pair<entt::entity, entt::entity> SGuiSystem::MakeSlider(entt::registry& registry, STexID texture, SWidgetID widgetId,
	float value, float minValue, float maxValue, const SVector3& pos, const SSize2F& size, const SSize2F& texSize, SColor4F color)
{
	SRectF area {
		pos.x - size.width / 2.0f, pos.y,
		pos.x + size.width / 2.0f, pos.y
	};
	const float sliderValue = (value - minValue) / (maxValue - minValue);
	const float btnUVWidth = (texSize.height / 2.0f) / texSize.width;
	const SSize2F buttonSize{ size.height, size.height };
	const SSize2F sliderSize{ size.width, texSize.height / 2.0f };
	const SVector3 buttonPos = SVector3{ area.left + size.width * sliderValue, pos.y, pos.z + 0.05f };
	SSpriteUV normal, pressed, slider;
	slider.SetBottomHalfUV();
	pressed.SetTopHalfUV();
	normal.SetTopHalfUV();
	normal.uvs[0].x = normal.uvs[2].x = btnUVWidth;
	pressed.uvs[1].x = pressed.uvs[3].x = btnUVWidth;
	pressed.uvs[0].x = pressed.uvs[2].x = btnUVWidth * 2.0f;

	entt::entity buttonEntity = registry.create();
	auto& sprite = registry.emplace<SSpriteComponent>(buttonEntity, true, 0.0f, buttonPos, buttonSize);
	auto& colors = registry.emplace<SColoredComponent>(buttonEntity);
	colors.SetColors(color);
	auto& texUV = registry.emplace<SSpriteUVComponent>(buttonEntity);
	texUV.uvs = normal;
	registry.emplace<STexturedComponent>(buttonEntity, texture);
	registry.emplace<SWidgetComponent>(buttonEntity, widgetId);
	registry.emplace<SButtonComponent>(buttonEntity, SConst::ZeroSPoint2, SConst::OneSVector3, normal, pressed);
	registry.emplace<SSliderComponent>(buttonEntity, sliderValue, minValue, maxValue, value);
	registry.emplace<SDragComponent>(buttonEntity, SConst::ZeroSVector2, area);

	entt::entity sliderEntity = registry.create();
	registry.emplace<SSpriteComponent>(sliderEntity, true, 0.0f, pos, sliderSize);
	auto& sliderColors = registry.emplace<SColoredComponent>(sliderEntity);
	sliderColors.SetColors(color);
	registry.emplace<STexturedComponent>(sliderEntity, texture);
	registry.emplace<SWidgetComponent>(sliderEntity, widgetId + 1);
	auto& sliderUV = registry.emplace<SSpriteUVComponent>(sliderEntity);
	sliderUV.uvs = slider;

	return { buttonEntity, sliderEntity };
}
