#include "Label.h"

namespace trview
{
    namespace ui
    {
        Label::Label(Point position, Size size, Colour background_colour, std::wstring text, float text_size, TextAlignment text_alignment, ParagraphAlignment paragraph_alignment)
            : Window(position, size, background_colour), _text(text), _text_size(text_size), _text_alignment(text_alignment), _paragraph_alignment(paragraph_alignment)
        {
        }

        std::wstring Label::text() const
        {
            return _text;
        }

        float Label::text_size() const
        {
            return _text_size;
        }

        void Label::set_text(std::wstring text)
        {
            _text = text;
        }

        TextAlignment Label::text_alignment() const
        {
            return _text_alignment;
        }

        ParagraphAlignment Label::paragraph_alignment() const
        {
            return _paragraph_alignment;
        }
    }
}