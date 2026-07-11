#include <NeoWPF/UI/Controls/Button/Button.h>

namespace NeoWPF::UI {

void Button::OnMouseEnter(NeoWPF::Input::Core::MouseEventArgs* args)
{
    ButtonBase::OnMouseEnter(args);
}

void Button::OnMouseLeave(NeoWPF::Input::Core::MouseEventArgs* args)
{
    ButtonBase::OnMouseLeave(args);
}

void Button::OnMouseDown(int x, int y, std::uint32_t buttonState)
{
    ButtonBase::OnMouseDown(x, y, buttonState);
}

void Button::OnMouseUp(int x, int y, std::uint32_t buttonState)
{
    ButtonBase::OnMouseUp(x, y, buttonState);
}

}
