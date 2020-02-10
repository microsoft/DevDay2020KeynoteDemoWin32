#pragma once
#include <memory>

inline int Width(const RECT& rect) { return rect.right - rect.left; }
inline int Height(const RECT& rect) { return rect.bottom - rect.top; }
inline RECT Shrink(const RECT& rect, int margin) { return { rect.left + margin, rect.top + margin, rect.right - margin, rect.bottom - margin }; }

// Kind of split between different regions.
enum class SplitKind
{
    Unknown,
    None,
    Vertical,
    Horizontal
};

// ScreenInfo is a helper class that provides an abstraction over
// the content rects API.
struct ScreenInfo
{
    ScreenInfo();

    SplitKind GetSplitKind() const;
    RECT GetWindowBounds() const;
    unsigned int GetRectCount() const;
    RECT GetRect(unsigned int index) const;
    int GetIndexForRect(LPRECT rect) const;
    int GetWidestIndex() const;
    int GetTallestIndex() const;
    int GetTallestOrWidestIndex() const;

    void Update(HWND hWnd);

    void EmulateScreens(int screens, ::SplitKind splitKind);
    const bool IsEmulating() const;

    static bool AreMultipleScreensPresent();

private:

    SplitKind m_splitKind{ SplitKind::None };
    RECT m_windowBounds{ 0 };
    unsigned int m_rectCount{ 0 };
    unsigned int m_contentRectsCapacity{ 0 };
    std::unique_ptr<RECT[]> m_contentRects{};

    // To support emulated dual-screen device testing on Desktop
    int m_emulatedScreenCount{ -1 };
    void ComputeEmulatedScreens();
};

