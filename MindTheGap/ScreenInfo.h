#pragma once
#include <memory>

inline int Width(const RECT* const rect) { return rect->right - rect->left; }
inline int Height(const RECT* const rect) { return rect->bottom - rect->top; }
inline RECT Shrink(const RECT* rect, int margin) { return { rect->left + margin, rect->top + margin, rect->right - margin, rect->bottom - margin }; }

enum class SplitKind
{
    None,
    Vertical,
    Horizontal
};

struct ScreenInfo
{
    ScreenInfo();

    SplitKind SplitKind{ SplitKind::None };
    RECT WindowBounds{ 0 };

    unsigned int GetRectCount() const;
    RECT GetRect(unsigned int index) const;
    int GetIndexForRect(LPRECT rect) const;
    int GetWidestIndex() const;
    int GetTallestIndex() const;
    int GetTallestOrWidestIndex() const;

    void Update(HWND hWnd);
    void UpdateDemo(HWND hWnd);

private:
    unsigned int m_rectCount;
    unsigned int m_contentRectsCapacity;
    std::unique_ptr<RECT[]> m_contentRects;
};

