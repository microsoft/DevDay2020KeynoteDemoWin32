#include "stdafx.h"
#include "ScreenInfo.h"
#include "polyfill.h"

// Default to a maximum of 2 content rects. We will dynamically 
// grow the array if necessary.
ScreenInfo::ScreenInfo() :
    m_contentRectsCapacity{ 2 },
    m_contentRects{ std::make_unique<RECT[]>(m_contentRectsCapacity) },
    m_rectCount{ 0 }
{}

// Called when we receive a WM_SIZE or WM_MOVE message.
void ScreenInfo::Update(HWND hWnd)
{
    // Copy the list of content rects into our array.
    unsigned int newRectCount{ m_contentRectsCapacity };
    BOOL result = GetContentRects(hWnd, &newRectCount, m_contentRects.get());

    // If necessary, grow the array. For example a Desktop PC with 
    // 3 monitors attached could have 3 content rects.
    if (result == FALSE)
    {
        _ASSERTE(GetLastError() == ERROR_MORE_DATA);
        m_contentRects = std::make_unique<RECT[]>(newRectCount);
        GetContentRects(hWnd, &newRectCount, m_contentRects.get());
        m_contentRectsCapacity = newRectCount;
    }

    m_rectCount = newRectCount;
}












void ScreenInfo::UpdateDemo(HWND hWnd)
{
    unsigned int newRectCount{ m_contentRectsCapacity };
    BOOL result{ FALSE };

    GetClientRect(hWnd, &WindowBounds);

    do
    {
        result = GetContentRects(hWnd, &newRectCount, m_contentRects.get());
        if (result == FALSE)
        {
            _ASSERT_EXPR(GetLastError() == ERROR_MORE_DATA, "Unknown failure for GetContentRects");
            m_contentRects = std::make_unique<RECT[]>(newRectCount);
            m_contentRectsCapacity = newRectCount;
        }
    } while (result == FALSE);

    m_rectCount = newRectCount;
    SplitKind = SplitKind::None;

    // Detect if this is a horizontal or vertical split
    if (m_rectCount > 0)
    {
        if (m_contentRects.get()[1].left > 0)
        {
            SplitKind = SplitKind::Vertical;
        }
        else if (m_contentRects.get()[1].top > 0)
        {
            SplitKind = SplitKind::Horizontal;
        }
    }
}

// If the window is split vertically, we can choose to put content on the
// screen that has the most pixels
int ScreenInfo::GetWidestIndex() const
{
    if (SplitKind == SplitKind::None || (Width(m_contentRects.get())) >= Width(m_contentRects.get() + 1))
    {
        return 0;
    }

    return 1;
}

// If the window is split horizontally, we can choose to put content on the
// screen that has the most pixels
int ScreenInfo::GetTallestIndex() const
{
    if (SplitKind == SplitKind::None || (Height(m_contentRects.get())) >= Height(m_contentRects.get() + 1))
    {
        return 0;
    }

    return 1;
}

int ScreenInfo::GetTallestOrWidestIndex() const
{
    if (SplitKind == SplitKind::Vertical)
    {
        return GetWidestIndex();
    }

    return GetTallestIndex();
}


int ScreenInfo::GetIndexForRect(LPRECT rect) const
{
    RECT dummy{};
    for (unsigned int i = 0; i < m_rectCount; ++i)
    {
        if (IntersectRect(&dummy, &m_contentRects.get()[i], rect))
        {
            return i;
        }
    }

    return -1;
}

unsigned int ScreenInfo::GetRectCount() const
{
    return m_rectCount;
}

RECT ScreenInfo::GetRect(unsigned int index) const
{
    _ASSERT_EXPR(index <= m_rectCount, "Invalid index passed");
    return m_contentRects.get()[index];
}
