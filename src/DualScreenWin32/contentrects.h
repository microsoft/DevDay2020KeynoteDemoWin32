#include <windows.h>
#include <vector>
#include <algorithm> // for std::min

// Version of the API to use when the OS-provided API isn't available
namespace polyfill
{
    typedef BOOL WINAPI GetContentRects_t(HWND hwnd, UINT* count, RECT* pContentRects);

    namespace details
    {
        BOOL CALLBACK CountWindowsCallback(const HMONITOR monitor, const HDC dc, const LPRECT rect, LPARAM param)
        {
            try
            {
                auto rects = (std::vector<RECT>*)param;
                rects->push_back(*rect);
                return TRUE;
            }
            catch (const std::bad_alloc&)
            {
                return FALSE;
            }
        }
    }

    // Poly-filled GetContentRects relies on EnumDisplayMonitors to get the
    // different regions the app can use. Note that any content in a window that
    // is off-screen will NOT be included in these regions.
    BOOL WINAPI GetContentRects(HWND hwnd, UINT* count, RECT* pContentRects)
    {
        // Must have valid count ptr, and cannot pass null array ptr unless count is 0.
        if ((count == nullptr) || (pContentRects == nullptr && *count != 0))
        {
            SetLastError(ERROR_INVALID_PARAMETER);
            return FALSE;
        }

        BOOL result{ FALSE };
        std::vector<RECT> rects;
        rects.reserve(2); // Assume 2 screens will cover most cases. It's OK if it resizes later.

        auto hDc = GetDC(hwnd);
        result = EnumDisplayMonitors(hDc, nullptr, details::CountWindowsCallback, (LPARAM)&rects);
        ReleaseDC(hwnd, hDc);

        // Enum failed, maybe due to OOM
        if (!result)
        {
            return result;
        }

        // Copy as many rects as we have room for; if there are more than
        // will fit we return FALSE with ERROR_MORE_DATA.
        for (unsigned int i = 0; i < std::min<UINT>(*count, static_cast<UINT>(rects.size())); ++i)
        {
            // Warning C6011: Dereferencing NULL pointer 'pContentRects'
            // If pContentRects is null, then *count must be zero (see checks at
            // start of function) so this loop never runs. 
            pContentRects[i] = rects[i];
        }

        if (*count < rects.size())
        {
            SetLastError(ERROR_MORE_DATA);
            result = FALSE;
        }
        else
        {
            SetLastError(ERROR_SUCCESS);
            result = TRUE;
        }

        *count = static_cast<UINT>(rects.size());
        return result;
    }
}

BOOL WINAPI GetContentRects(HWND hwnd, UINT* count, RECT* pContentRects)
{
    static polyfill::GetContentRects_t* impl{ nullptr };

    // No need for synchronization since worst-case two threads write the exact
    // same value into the pointer. The answer can't change at runtime.
    if (impl == nullptr)
    {
        // FYI only, eventually there will be an actual platform API to call, and we
        // can invoke it directly. But since the name & shape of the API are subject
        // to change, we won't actually try to use it yet (in case the name stays the
        // same but the ABI changes).

        // auto module = LoadLibraryA("user32.dll");
        // if (module != 0)
        // {
        //     impl = (polyfill::GetContentRects_t*)GetProcAddress(module, "GetContentRects");
        // }

        if (impl == nullptr)
        {
            impl = polyfill::GetContentRects;
        }
    }

    return impl(hwnd, count, pContentRects);
}
