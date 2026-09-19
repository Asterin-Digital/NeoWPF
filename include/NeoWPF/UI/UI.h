/**
 * MIT License — Copyright (c) 2026 NeoWPF
 * See LICENSE file in the project root for full license text.
 *
 * Notes:
 * Keep src implementations out of this file.
 * Don't create a src file for UI.h (this) I prefer to keep it header only.
 * Do not use this header file in internal code, doing so will make compile time explode
 * when this header file changes, tyy!! :3
 */

#pragma once

// Markup
#include <NeoWPF/UI/Markup/Builder/Builder.h>

// Localization
#include <NeoWPF/UI/Localization/Culture.h>
#include <NeoWPF/UI/Localization/LocaleExpression.h>
#include <NeoWPF/UI/Localization/LocaleOperations.h>
#include <NeoWPF/UI/Localization/LocaleString.h>
#include <NeoWPF/UI/Localization/LocaleTable.h>
#include <NeoWPF/UI/Localization/LocalizationManager.h>

// Core
#include <NeoWPF/UI/Core/Element/Visual.h>
#include <NeoWPF/UI/Core/Element/UIElement.h>
#include <NeoWPF/UI/Core/Element/FrameworkElement.h>
#include <NeoWPF/UI/Core/Element/Control.h>
#include <NeoWPF/UI/Core/Media/Color.h>
#include <NeoWPF/UI/Core/Media/BitmapSource.h>
#include <NeoWPF/UI/Core/Media/BitmapImage.h>
#include <NeoWPF/UI/Core/Media/CornerRadius.h>
#include <NeoWPF/UI/Core/Media/Spacing.h>
#include <NeoWPF/UI/Core/Transform/Transform.h>

// Data
#include <NeoWPF/UI/Data/Binding.h>
#include <NeoWPF/UI/Data/BindingOperations.h>
#include <NeoWPF/UI/Data/BindingExpression.h>
#include <NeoWPF/UI/Data/INotifyPropertyChanged.h>

// Resources
#include <NeoWPF/Core/Application/Resources/ResourceKey.h>
#include <NeoWPF/Core/Application/Resources/IResourceProvider.h>
#include <NeoWPF/Core/Application/Resources/EmbeddedResourceProvider.h>
#include <NeoWPF/Core/Application/Resources/ExternalFileResourceProvider.h>
#include <NeoWPF/Core/Application/Resources/ResourceManager.h>
#include <NeoWPF/Core/Application/Resources/ApplicationResources.h>

// Animation
#include <NeoWPF/UI/Animation/AnimationController.h>
#include <NeoWPF/UI/Animation/Definitions/AnimationTimeline.h>
#include <NeoWPF/UI/Animation/Definitions/DoubleAnimation.h>
#include <NeoWPF/UI/Animation/Definitions/ColorAnimation.h>
#include <NeoWPF/UI/Animation/Easing/EasingFunctionBase.h>
#include <NeoWPF/UI/Animation/Easing/EasingFunctions.h>
#include <NeoWPF/UI/Animation/Core/AnimationClock.h>
#include <NeoWPF/UI/Animation/Storyboards/Storyboard.h>

// Layout
#include <NeoWPF/UI/Layout/Panel/Panel.h>
#include <NeoWPF/UI/Layout/Panel/StackPanel.h>
#include <NeoWPF/UI/Layout/Grid/Grid.h>
#include <NeoWPF/UI/Layout/Border/Border.h>
#include <NeoWPF/UI/Layout/Placement/Orientation.h>

// Styling
#include <NeoWPF/UI/Styling/State/VisualStateManager.h>
#include <NeoWPF/UI/Styling/Triggers/Setter.h>
#include <NeoWPF/UI/Styling/Triggers/TriggerBase.h>
#include <NeoWPF/UI/Styling/Triggers/Trigger.h>
#include <NeoWPF/UI/Styling/Triggers/DataTrigger.h>
#include <NeoWPF/UI/Styling/Triggers/EventTrigger.h>
#include <NeoWPF/UI/Styling/Triggers/TriggerCollection.h>
#include <NeoWPF/UI/Styling/Triggers/Actions/TriggerAction.h>
#include <NeoWPF/UI/Styling/Triggers/Actions/BeginStoryboardAction.h>
#include <NeoWPF/UI/Styling/Triggers/Actions/StopStoryboardAction.h>

// Controls
#include <NeoWPF/UI/Controls/Window.h>
#include <NeoWPF/UI/Controls/DefaultTitleBar.h>
#include <NeoWPF/UI/Controls/Templates/TemplateVisual.h>
#include <NeoWPF/UI/Controls/Templates/ControlTemplate.h>
#include <NeoWPF/UI/Controls/Templates/TemplateParts.h>
#include <NeoWPF/UI/Controls/Text/TextBlock.h>
#include <NeoWPF/UI/Controls/TextBox/TextBox.h>
#include <NeoWPF/UI/Controls/Image/Image.h>
#include <NeoWPF/UI/Controls/Base/ButtonBase/ButtonBase.h>
#include <NeoWPF/UI/Controls/Base/ToggleBase/ToggleBase.h>
#include <NeoWPF/UI/Controls/Button/Button.h>
#include <NeoWPF/UI/Controls/FilePicker/FilePicker.h>
#include <NeoWPF/UI/Controls/Checkbox/Checkbox.h>
#include <NeoWPF/UI/Controls/ItemsControl/ItemsControl.h>
#include <NeoWPF/UI/Controls/ListBox/ListBox.h>
#include <NeoWPF/UI/Controls/ScrollBar/ScrollBar.h>
#include <NeoWPF/UI/Controls/Slider/Slider.h>
#include <NeoWPF/UI/Controls/ProgressBar/ProgressBar.h>
#include <NeoWPF/UI/Controls/ScrollViewer/ScrollViewer.h>
#include <NeoWPF/UI/Controls/Selector/Selector.h>
#include <NeoWPF/UI/Controls/ComboBox/ComboBox.h>
#include <NeoWPF/UI/Controls/Popup/Popup.h>
#include <NeoWPF/UI/Controls/Thumb/Thumb.h>

// Content
#include <NeoWPF/UI/Content/ContentControl.h>
#include <NeoWPF/UI/Content/ContentPresenter.h>

// Text
#include <NeoWPF/UI/Text/FontStyle.h>