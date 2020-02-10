#include "stdafx.h"
#include "ScreenInfo.h"
#include "contentrects.h"

// The helper defaults to a maximum of 2 content rects. We will dynamically 
// grow the array later if necessary.
ScreenInfo::ScreenInfo() :
    m_contentRectsCapacity{ 2 },
    m_contentRects{ std::make_unique<RECT[]>(m_contentRectsCapacity) },
    m_rectCount{ 0 }
{}

// Call whenever the size or position of the app changes.
void ScreenInfo::Update(HWND hWnd)
{
    unsigned int newRectCount{ m_contentRectsCapacity };
    BOOL result{ FALSE };

    GetClientRect(hWnd, &m_windowBounds);

    // If we're emulating a multi-rect device, then don't try to 
    // get the real info
    if (m_emulatedScreenCount > 0)
    {
        ComputeEmulatedScreens();
        return;
    }

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
    m_splitKind = SplitKind::Unknown;

    if (m_rectCount == 1)
    {
        m_splitKind = SplitKind::None;
    }
    // Detect if this is a horizontal or vertical split. We don't bother trying to
    // guess for more than two screens at this point.
    else if (m_rectCount == 2)
    {
        if (m_contentRects.get()[1].left > 0)
        {
            m_splitKind = SplitKind::Vertical;
        }
        else if (m_contentRects.get()[1].top > 0)
        {
            m_splitKind = SplitKind::Horizontal;
        }
    }
}

// Some of the helper methods...
unsigned int ScreenInfo::GetRectCount() const
{
    return m_rectCount;
}

RECT ScreenInfo::GetRect(unsigned int index) const
{
    _ASSERT_EXPR(index <= m_rectCount, "Invalid index passed");
    return m_contentRects.get()[index];
}

bool ScreenInfo::AreMultipleScreensPresent()
{
    unsigned int count{ 0 };
    GetContentRects(NULL, &count, nullptr);

    return count > 1;
}

#pragma region other less-interesting helper methods

SplitKind ScreenInfo::GetSplitKind() const
{
    return m_splitKind;
}

RECT ScreenInfo::GetWindowBounds() const
{
    return m_windowBounds;
}

// If the window is split vertically, we can choose to put content on the
// screen that has the most pixels
int ScreenInfo::GetWidestIndex() const
{
    int width{ 0 };
    int best{ 0 };
    for (unsigned int i = 0; i < m_rectCount; ++i)
    {
        auto thisWidth{ Width(m_contentRects.get()[i]) };
        if (thisWidth > width)
        {
            width = thisWidth;
            best = i;
        }
    }

    return best;
}

// If the window is split horizontally, we can choose to put content on the
// screen that has the most pixels
int ScreenInfo::GetTallestIndex() const
{
    int height{ 0 };
    int best{ 0 };
    for (unsigned int i = 0; i < m_rectCount; ++i)
    {
        auto thisHeight{ Height(m_contentRects.get()[i]) };
        if (thisHeight > height)
        {
            height = thisHeight;
            best = i;
        }
    }

    return best;
}

int ScreenInfo::GetTallestOrWidestIndex() const
{
    if (m_splitKind == SplitKind::Vertical)
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

void ScreenInfo::EmulateScreens(int screens, ::SplitKind splitKind)
{
    if (screens <= 0)
    {
        screens = -1;
        splitKind = SplitKind::None;
    }

    m_emulatedScreenCount = screens;
    m_splitKind = splitKind;
}

const bool ScreenInfo::IsEmulating() const
{
    return m_emulatedScreenCount > 0;
}

void ScreenInfo::ComputeEmulatedScreens()
{
    int xDelta{ 0 }, yDelta{ 0 }, width{ 0 }, height{ 0 };

    if (m_splitKind == SplitKind::Horizontal)
    {
        width = Width(m_windowBounds);
        height = yDelta = Height(m_windowBounds) / m_emulatedScreenCount;
    }
    else
    {
        height = Height(m_windowBounds);
        width = xDelta = Width(m_windowBounds) / m_emulatedScreenCount;
    }

    m_contentRects = std::make_unique<RECT[]>(m_emulatedScreenCount);
    m_rectCount = m_emulatedScreenCount;
    m_contentRectsCapacity = m_emulatedScreenCount;

    int leftX{ 0 }, topY{ 0 }, rightX{ width }, bottomY{ height };
    for (int i = 0; i < m_emulatedScreenCount; ++i)
    {
        m_contentRects.get()[i] = RECT{ leftX, topY, rightX, bottomY };

        // TODO: deal with emulating non-uniform values. Not a concern for
        // current emulation needs.
        if (m_splitKind == SplitKind::Horizontal)
        {
            topY = bottomY;
            bottomY += yDelta;
        }
        else
        {
            leftX = rightX;
            rightX += xDelta;
        }
    }
}

#pragma endregion