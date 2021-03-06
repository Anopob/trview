#include "Window.h"

namespace trview
{
    namespace ui
    {
        Window::Window(const Size& size, const Colour& background_colour)
            : Window(Point(), size, background_colour)
        {
        }

        Window::Window(const Point& point, const Size& size, const Colour& background_colour)
            : Control(point, size), _background_colour(background_colour)
        {
        }

        Colour Window::background_colour() const
        {
            return _background_colour;
        }

        void Window::set_background_colour(Colour colour)
        {
            _background_colour = colour;
            on_invalidate();
        }
    }
}
