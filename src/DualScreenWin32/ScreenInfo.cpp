#include "stdafx.h"
#include "ScreenInfo.h"
#include "contentrects.h"
#include <algorithm>

using namespace dual_screen;

// ScreenInfo is a helper class that provides an abstraction over
// the content rects API.

// The helper defaults to a maximum of 2 content rects. We will dynamically 
// grow the array later if necessary.
ScreenInfo::ScreenInfo()
{
    m_contentRects.reserve(2);
}

// Call whenever the size or position of the app changes.
bool ScreenInfo::Update(HWND hWnd) noexcept // if we OOM on a RECT alloc, we're in bad shape...
{
    auto snapshot{ GetSnapshot() };

    ::GetClientRect(hWnd, &m_clientRect);
    ::GetWindowRect(hWnd, &m_windowRect);

    if (m_emulatedScreenCount > 0)
    {
        return ComputeEmulatedScreens(snapshot);
    }

    std::vector<RECT> updatedRects{ 2 };
    auto newRectCount{ static_cast<unsigned>(updatedRects.size()) };

    while (GetContentRects(hWnd, &newRectCount, updatedRects.data()) == FALSE)
    {
        // Only expected error is "you need a bigger array" - otherwise
        // we will revert to GetClientRect.
        if (GetLastError() != ERROR_MORE_DATA)
        {
            newRectCount = 1;
            updatedRects[0] = m_clientRect;
            break;
        }

        // Re-allocate, and try again.
        updatedRects.resize(newRectCount);
    }

    // Delete any no-longer-needed rects.
    if (newRectCount < updatedRects.size())
    {
        updatedRects.resize(newRectCount);
    }

    // Make sure they're always in logical order
    if (newRectCount > 1)
    {
        std::sort(begin(updatedRects), end(updatedRects), [](const auto& r1, const auto& r2) { return IsBefore(r1, r2); });
    }

    m_contentRects = updatedRects;

    m_splitKind = SplitKind::Unknown;

    // Detect if this is a horizontal or vertical split - currently only useful for dual-screen
    // apps with identical screens (e.g. won't handle a Desktop window spanning 3 monitors in random 
    // placements).
    if (GetRectCount() == 1)
    {
        m_splitKind = SplitKind::None;
    }
    else if (GetRectCount() == 2)
    {
        if (GetRect(0).top == GetRect(1).top)
        {
            m_splitKind = SplitKind::Vertical;
        }
        else if (GetRect(0).left == GetRect(1).left)
        {
            m_splitKind = SplitKind::Horizontal;
        }
    }

    // No redraw needed if zero rects (minimized) or nothing has materially changed.
    return newRectCount > 0 && !snapshot.IsSameAs(m_contentRects, m_clientRect);
}

unsigned int ScreenInfo::GetRectCount() const
{
    return static_cast<unsigned int>(m_contentRects.size());
}

RECT ScreenInfo::GetRect(unsigned int index) const
{
    return m_contentRects[index];
}

bool ScreenInfo::AreMultipleScreensPresent()
{
    unsigned int count{ 0 };
    GetContentRects(NULL, &count, nullptr);

    return count > 1;
}

SplitKind ScreenInfo::GetSplitKind() const
{
    return m_splitKind;
}

RECT ScreenInfo::GetClientRect() const
{
    return m_clientRect;
}

RECT ScreenInfo::GetWindowRect() const
{
    return m_windowRect;
}


// If the window is split vertically, we can choose to put content on the
// screen that has the most pixels
int ScreenInfo::GetWidestIndex() const
{
    int width{ 0 };
    int best{ -1 };
    for (unsigned int i = 0; i < GetRectCount(); ++i)
    {
        auto thisWidth{ RectWidth(m_contentRects[i]) };
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
    int best{ -1 };
    for (unsigned int i = 0; i < GetRectCount(); ++i)
    {
        auto thisHeight{ RectHeight(m_contentRects[i]) };
        if (thisHeight > height)
        {
            height = thisHeight;
            best = i;
        }
    }

    return best;
}

int ScreenInfo::GetBestIndexForHorizontalContent() const
{
    if (m_splitKind == SplitKind::Vertical)
    {
        return GetWidestIndex();
    }

    // If horizontal split (or no split), prefer the top-most rect
    return 0;
}

int ScreenInfo::GetIndexForRect(LPRECT rect) const
{
    RECT dummy{};
    for (unsigned int i = 0; i < GetRectCount(); ++i)
    {
        if (IntersectRect(&dummy, &m_contentRects[i], rect))
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

ScreenInfo::Snapshot ScreenInfo::GetSnapshot() const
{
    return { m_contentRects, m_clientRect };
}

bool ScreenInfo::HasConfigurationChanged(const Snapshot& other) const
{
    return !other.IsSameAs(GetSnapshot());
}

const bool ScreenInfo::IsEmulating() const
{
    return m_emulatedScreenCount > 0;
}

bool ScreenInfo::ComputeEmulatedScreens(const ScreenInfo::Snapshot& snapshot)
{
    int xDelta{ 0 }, yDelta{ 0 }, width{ 0 }, height{ 0 };

    if (m_splitKind == SplitKind::Horizontal)
    {
        width = RectWidth(m_clientRect);
        height = yDelta = RectHeight(m_clientRect) / m_emulatedScreenCount;
    }
    else
    {
        height = RectHeight(m_clientRect);
        width = xDelta = RectWidth(m_clientRect) / m_emulatedScreenCount;
    }

    m_contentRects = std::vector<RECT>(m_emulatedScreenCount);

    int leftX{ 0 }, topY{ 0 }, rightX{ width }, bottomY{ height };
    for (int i = 0; i < m_emulatedScreenCount; ++i)
    {
        m_contentRects[i] = RECT{ leftX, topY, rightX, bottomY };

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

    return !snapshot.IsSameAs(GetSnapshot());
}

ScreenInfo::Snapshot::Snapshot(const std::vector<RECT>& rects, const RECT& clientRect) :
    m_contentRects{ rects },
    m_clientRect{ clientRect }
{
}

bool ScreenInfo::Snapshot::IsSameAs(const Snapshot& other) const
{
    return IsSameAs(other.m_contentRects, other.m_clientRect);
}

bool ScreenInfo::Snapshot::IsSameAs(const std::vector<RECT>& other_rects, const RECT& other_clientRect) const
{
    if (!EqualRect(&m_clientRect, &other_clientRect))
    {
        return false;
    }

    if (m_contentRects.size() != other_rects.size())
    {
        return false;
    }

    for (int i = 0; i < m_contentRects.size(); ++i)
    {
        if (!EqualRect(&m_contentRects[i], &other_rects[i]))
        {
            return false;
        }
    }

    return true;
}
