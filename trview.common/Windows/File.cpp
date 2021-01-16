#include "File.h"
#include <commdlg.h>

namespace trview
{
    bool open_file(const std::wstring& dialog_title, const wchar_t* const file_filter, std::wstring& selected_file)
    {
        wchar_t cd[MAX_PATH];
        GetCurrentDirectoryW(MAX_PATH, cd);

        OPENFILENAME ofn;
        memset(&ofn, 0, sizeof(ofn));

        wchar_t path[MAX_PATH];
        memset(&path, 0, sizeof(path));

        ofn.lStructSize = sizeof(ofn);
        ofn.lpstrFile = path;
        ofn.nMaxFile = MAX_PATH;
        ofn.lpstrTitle = dialog_title.c_str();
        ofn.lpstrFilter = file_filter;
        ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;

        if (GetOpenFileName(&ofn))
        {
            SetCurrentDirectory(cd);
            selected_file = ofn.lpstrFile;
            return true;
        }
        return false;
    }
}
