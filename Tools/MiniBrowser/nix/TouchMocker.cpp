#include "TouchMocker.h"

#ifdef WTF_USE_OPENGL_ES_2
#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>
#else
#include <GL/gl.h>
#endif

#include <cassert>
#include "touchTexture.h"

using namespace Nix;

// On single touches, every mouse interaction is used as touch event.
// Holding CTRL key means you're building a multi-touch event and each mouse,
// button represents a touch point. All touch points will be released when
// you release the CTRL key.

MockedTouchPoint::MockedTouchPoint()
{
    verticalRadius = 20;
    horizontalRadius = 20;
    rotationAngle = 0.0;
    pressure = 1.0;
    selected = false;
}

static bool isSingleTouch(const InputEvent& event)
{
    return !event.controlKey();
}

TouchMocker::TouchMocker(WebView* webView)
    : m_webView(webView)
{
    loadTouchPointTexture();
}

TouchMocker::~TouchMocker()
{
    glDeleteTextures(1, &m_touchTextureId);
}

void TouchMocker::paintTouchPoints(const WKSize& size)
{
#if 0
    static float vertexData[12] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
    static const float texCoords[] = { 0, 0, 1, 0, 0, 1, 1, 1};

    if (m_touchPoints.empty())
        return;

    glPushMatrix();
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0, size.width, size.height, 0, -1, 1);

    glEnable(GL_TEXTURE_2D);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glEnableClientState(GL_VERTEX_ARRAY);
    glEnableClientState(GL_TEXTURE_COORD_ARRAY_EXT);
    glTexCoordPointer(2, GL_FLOAT, 0, texCoords);
    glVertexPointer(3, GL_FLOAT, 0, vertexData);

    glBindTexture(GL_TEXTURE_2D, m_touchTextureId);

    TouchMap::const_iterator it;
    for (it = m_touchPoints.begin(); it != m_touchPoints.end(); ++it) {
        const MockedTouchPoint& touch = it->second;
        double x1 = touch.windowX - touch.horizontalRadius;
        double y1 = touch.windowY - touch.verticalRadius;
        double x2 = touch.windowX + touch.horizontalRadius;
        double y2 = touch.windowY + touch.verticalRadius;

        vertexData[0] = x1;
        vertexData[1] = y1;
        vertexData[3] = x2;
        vertexData[4] = y1;
        vertexData[6] = x1;
        vertexData[7] = y2;
        vertexData[9] = x2;
        vertexData[10] = y2;
        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    }
    glPopMatrix();
#endif
}

bool TouchMocker::handleMousePress(const MouseEvent& event, const WKPoint& windowPos)
{
    TouchPoint::TouchState state = m_touchPoints.count(event.button) ? TouchPoint::TouchMoved : TouchPoint::TouchPressed;

    trackTouchPoint(event.button, state, event, windowPos);
    sendCurrentTouchEvent(state, event.timestamp);
    m_touchPoints[event.button].selected = true;
    return true;
}

bool TouchMocker::handleMouseRelease(const MouseEvent& event)
{
    if (isSingleTouch(event))
        releaseTouchPoints(event.timestamp);
    else
        m_touchPoints[event.button].selected = false;
    return true;
}

bool TouchMocker::handleMouseMove(const MouseEvent& event, const WKPoint& windowPos)
{
    if (m_touchPoints.empty())
        return false;

    TouchMap::const_iterator it = m_touchPoints.begin();
    for (; it != m_touchPoints.end(); ++it) {
        if (it->second.selected) {
            trackTouchPoint(it->first, TouchPoint::TouchMoved, event, windowPos);
            sendCurrentTouchEvent(TouchPoint::TouchMoved, event.timestamp);
        }
    }

    return true;
}

bool TouchMocker::handleKeyRelease(const KeyEvent& event)
{
    if (event.key == KeyEvent::Key_Control) {
        releaseTouchPoints(event.timestamp);
        return true;
    }
    return false;
}

void TouchMocker::releaseTouchPoints(double timestamp)
{
    if (m_touchPoints.empty())
        return;

    // FIXME: When we have proper gesture recognition, this code should move away.
    TouchPoint touch;

    if (m_touchPoints.size() == 1)
        // Keep track of the single touch we have so far to emit it as SingleTap afterwards.
        touch = m_touchPoints.begin()->second;

    updateTouchPointsState(TouchPoint::TouchReleased);
    sendCurrentTouchEvent(TouchPoint::TouchReleased, timestamp);

    m_touchPoints.clear();

}

void TouchMocker::updateTouchPointsState(TouchPoint::TouchState state)
{
    TouchMap::iterator it = m_touchPoints.begin();
    for (; it != m_touchPoints.end(); ++it)
        it->second.state = state;
}

void TouchMocker::trackTouchPoint(MouseEvent::Button id, TouchPoint::TouchState state, const Nix::MouseEvent& event, const WKPoint& windowPos)
{
    // While we update some touch's state the others should be on stationary state.
    updateTouchPointsState(TouchPoint::TouchStationary);

    MockedTouchPoint& touch = m_touchPoints[id];
    touch.id = static_cast<unsigned>(id);
    touch.state = state;
    touch.x = event.x;
    touch.y = event.y;
    touch.windowX = windowPos.x;
    touch.windowY = windowPos.y;
    touch.globalX = event.globalX;
    touch.globalY = event.globalY;
}

void TouchMocker::sendCurrentTouchEvent(TouchPoint::TouchState state, double timestamp)
{
    TouchEvent ev;
    ev.timestamp = timestamp;

    switch (state) {
    case TouchPoint::TouchPressed:
        ev.type = InputEvent::TouchStart;
        break;
    case TouchPoint::TouchMoved:
        ev.type = InputEvent::TouchMove;
        break;
    case TouchPoint::TouchReleased:
        ev.type = InputEvent::TouchEnd;
        break;
    }

    TouchMap::iterator it = m_touchPoints.begin();
    for (; it != m_touchPoints.end(); ++it)
        ev.touchPoints.push_back(it->second);

    m_webView->sendEvent(ev);
}

void TouchMocker::loadTouchPointTexture()
{
    // Texture data size, 64x64 RGBA
    assert(sizeof(textureData) == 64 * 64 * 4);
    glGenTextures(1, &m_touchTextureId);
    glBindTexture(GL_TEXTURE_2D, m_touchTextureId);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 64, 64, 0, GL_RGBA, GL_UNSIGNED_BYTE, textureData);
}
