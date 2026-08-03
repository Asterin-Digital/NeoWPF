#pragma once

// pls dont add implementations in this header file
// keep it only as an umbrella header file without a source implementation
// DO NOT USE THIS IN FRAMEWORK INTERNALS, ONLY FOR NeoWPF.h FILE!! bcs its for public API

#include <NeoWPF/UI/Controls/Templates/TemplateVisual.h>
#include <NeoWPF/UI/Core/Element/Visual.h>
#include <NeoWPF/UI/Core/Element/UIElement.h>
#include <NeoWPF/UI/Core/Element/FrameworkElement.h>
#include <NeoWPF/UI/Core/Element/Control.h>
#include <NeoWPF/UI/Core/Media/Color.h>
#include <NeoWPF/UI/Core/Media/CornerRadius.h>
#include <NeoWPF/UI/Data/Binding.h>
#include <NeoWPF/UI/Data/BindingOperations.h>
#include <NeoWPF/UI/Data/BindingExpression.h>
#include <NeoWPF/UI/Animation/AnimationController.h>
#include <NeoWPF/UI/Animation/Core/Timeline.h>
#include <NeoWPF/UI/Animation/Storyboards/Storyboard.h>
#include <NeoWPF/UI/Core/Transform/Transform.h>
#include <NeoWPF/UI/Layout/Panel/Panel.h>
#include <NeoWPF/UI/Layout/Panel/StackPanel.h>
#include <NeoWPF/UI/Layout/Grid/Grid.h>
#include <NeoWPF/UI/Layout/Border/Border.h>
#include <NeoWPF/UI/Layout/Placement/Orientation.h>
#include <NeoWPF/UI/Controls/Window.h>
#include <NeoWPF/UI/Data/INotifyPropertyChanged.h>
#include <NeoWPF/UI/Controls/Text/TextBlock.h>
#include <NeoWPF/UI/Controls/TextBox/TextBox.h>
#include <NeoWPF/UI/Controls/Base/ButtonBase/ButtonBase.h>
#include <NeoWPF/UI/Controls/Base/ToggleBase/ToggleBase.h>
#include <NeoWPF/UI/Controls/Button/Button.h>
#include <NeoWPF/UI/Controls/Checkbox/Checkbox.h>
#include <NeoWPF/UI/Controls/ItemsControl/ItemsControl.h>
#include <NeoWPF/UI/Controls/ScrollBar/ScrollBar.h>
#include <NeoWPF/UI/Styling/State/VisualStateManager.h>
#include <NeoWPF/UI/Controls/Templates/ControlTemplate.h>
#include <NeoWPF/UI/Controls/Templates/TemplateParts.h>
#include <NeoWPF/UI/Content/ContentControl.h>
#include <NeoWPF/UI/Content/ContentPresenter.h>
#include <NeoWPF/Core/Application/ApplicationResources.h>

