#pragma once

namespace trview
{
    bool open_file(const std::wstring& dialog_title, const wchar_t* const file_filter, std::wstring& selected_file);
}