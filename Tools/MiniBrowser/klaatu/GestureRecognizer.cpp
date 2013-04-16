#include "GestureRecognizer.h"
#include <cassert>
#include <cmath>
#include <cstdio>

static WKPoint computeCenter(WKPoint a, WKPoint b)
{
    return WKPointMake((a.x + b.x) / 2, (a.y + b.y) / 2);
}

static double computeDistance(WKPoint a, WKPoint b)
{
    return sqrt(pow(a.x - b.x, 2) + pow(a.y - b.y, 2));
}

static bool hasTouchPointPressed(const Nix::TouchEvent& event)
{
    std::vector<Nix::TouchPoint>::const_iterator it = event.touchPoints.begin();

    for (; it < event.touchPoints.end(); ++it)
        if ((*it).state == Nix::TouchPoint::TouchPressed)
            return true;

    return false;
}

GestureRecognizer::GestureRecognizer(GestureRecognizerClient *client)
    : m_client(client)
    , m_state(&GestureRecognizer::noGesture)
    , m_timestamp(0)
    , m_doubleTapTimerId(0)
    , m_initialPinchDistance(0)
    , m_initialPinchScale(0)
{
}

void GestureRecognizer::reset()
{
    m_state = &GestureRecognizer::noGesture;
}

static const int PanDistanceThreshold = 10;
static const int MaxDoubleTapInterval = 500;

bool exceedsPanThreshold(const Nix::TouchPoint& first, const Nix::TouchPoint& last)
{
    return std::abs(first.globalX - last.globalX) > PanDistanceThreshold || std::abs(first.globalY - last.globalY);
}

void GestureRecognizer::doubleTapTimerTriggered()
{
    if (m_state == &GestureRecognizer::waitForDoubleTap) {
        m_client->handleSingleTap(m_timestamp, m_firstTouchPoint);
        m_state = &GestureRecognizer::noGesture;
    }
}

void GestureRecognizer::fail(const char* reason)
{
    fprintf(stderr, "Gesture recognizer failed: %s\n", reason);
    reset();
}

void GestureRecognizer::noGesture(const Nix::TouchEvent& event)
{
    Nix::TouchPoint touch = event.touchPoints[0];
    switch (event.type) {
    case Nix::InputEvent::TouchStart:
        m_state = &GestureRecognizer::singleTapPressed;
        m_firstTouchPoint = touch;
        m_previousTouchPoint = touch;
        break;
    case Nix::InputEvent::TouchMove:
    case Nix::InputEvent::TouchEnd:
        fail("received TouchMove or TouchEnd when in NoGesture state.");
        break;
    }
}

void GestureRecognizer::setupPinchData(const std::vector<Nix::TouchPoint>& points)
{
    Nix::TouchPoint first = points[0];
    Nix::TouchPoint second = points[1];

    m_initialPinchDistance = computeDistance(WKPointMake(first.globalX, first.globalY), WKPointMake(second.globalX, second.globalY));
    m_initialPinchScale = m_client->scale();
    m_initialPinchContentCenter = computeCenter(WKPointMake(first.x, first.y), WKPointMake(second.x, second.y));
    m_previousPinchGlobalCenter = computeCenter(WKPointMake(first.globalX, first.globalY), WKPointMake(second.globalX, second.globalY));
}

void GestureRecognizer::singleTapPressed(const Nix::TouchEvent& event)
{
    Nix::TouchPoint touch = event.touchPoints[0];
    switch (event.type) {
    case Nix::InputEvent::TouchMove:
        if (exceedsPanThreshold(touch, m_firstTouchPoint)) {
            updatePanningData(event.timestamp, touch);
        }
        break;
    case Nix::InputEvent::TouchEnd:
        m_state = &GestureRecognizer::waitForDoubleTap;
        m_timestamp = event.timestamp;
        m_doubleTapTimerId = g_timeout_add(MaxDoubleTapInterval, doubleTapTimer, this);
        break;
    case Nix::InputEvent::TouchStart:
        m_state = &GestureRecognizer::pinchInProgress;
        setupPinchData(event.touchPoints);
        m_client->handlePinchStarted(event.timestamp);
        break;
    }
}
void GestureRecognizer::waitForDoubleTap(const Nix::TouchEvent& event)
{
    g_source_remove(m_doubleTapTimerId);
    switch (event.type) {
    case Nix::InputEvent::TouchStart:
        m_state = &GestureRecognizer::doubleTapPressed;
        break;
    case Nix::InputEvent::TouchMove:
    case Nix::InputEvent::TouchEnd:
        fail("received TouchMove or TouchEnd when in WaitForDoubleTap state.");
        break;
    }
}
void GestureRecognizer::doubleTapPressed(const Nix::TouchEvent& event)
{
    Nix::TouchPoint touch = event.touchPoints[0];
    switch (event.type) {
    case Nix::InputEvent::TouchMove:
        if (exceedsPanThreshold(touch, m_firstTouchPoint)) {
            updatePanningData(event.timestamp, touch);
        }
        break;
    case Nix::InputEvent::TouchEnd:
        m_state = &GestureRecognizer::noGesture;
        m_client->handleDoubleTap(event.timestamp, touch);
        break;
    case Nix::InputEvent::TouchStart:
        m_state = &GestureRecognizer::pinchInProgress;
        setupPinchData(event.touchPoints);
        m_client->handlePinchStarted(event.timestamp);
        break;
    }
}

void GestureRecognizer::updatePanningData(double timestamp, const Nix::TouchPoint& current)
{
    WKPoint delta = WKPointMake((current.globalX - m_previousTouchPoint.globalX) / m_client->scale(),
                                (current.globalY - m_previousTouchPoint.globalY) / m_client->scale());

    m_state = &GestureRecognizer::panningInProgress;
    m_client->handlePanning(timestamp, delta);
}

void GestureRecognizer::panningInProgress(const Nix::TouchEvent& event)
{
    Nix::TouchPoint touch = event.touchPoints[0];
    switch (event.type) {
    case Nix::InputEvent::TouchMove:
        updatePanningData(event.timestamp, touch);
        break;
    case Nix::InputEvent::TouchEnd:
        m_state = &GestureRecognizer::noGesture;
        m_client->handlePanningFinished(event.timestamp);
        break;
    case Nix::InputEvent::TouchStart:
        m_state = &GestureRecognizer::pinchInProgress;
        setupPinchData(event.touchPoints);
        m_client->handlePinchStarted(event.timestamp);
        break;
    }
}

void GestureRecognizer::pinchInProgress(const Nix::TouchEvent& event)
{
    if (event.touchPoints.size() < 2) {
        fail("Received only one touch point while in PinchInProgressState.");
        return;
    }

    switch (event.type) {
    case Nix::InputEvent::TouchMove:
        updatePinchData(event.timestamp, event.touchPoints);
        break;
    case Nix::InputEvent::TouchEnd:
        if (hasTouchPointPressed(event))
            m_state = &GestureRecognizer::panningInProgress;
        else {
            m_state == &GestureRecognizer::noGesture;
            reset();
        }
        m_client->handlePinchFinished(event.timestamp);
        break;
    case Nix::InputEvent::TouchStart:
        // Ignore extra touch points.
        break;
    }
}

void GestureRecognizer::handleTouchEvent(const Nix::TouchEvent& event)
{
    (this->*m_state)(event);

    Nix::TouchPoint touch = event.touchPoints[0];
    m_previousTouchPoint = touch;
}

gboolean doubleTapTimer(gpointer data)
{
    reinterpret_cast<GestureRecognizer*>(data)->doubleTapTimerTriggered();
    return FALSE;
}

void GestureRecognizer::updatePinchData(double timestamp, const std::vector<Nix::TouchPoint>& points)
{
    assert(m_initialPinchScale > 0);

    Nix::TouchPoint first = points[0];
    Nix::TouchPoint second = points[1];

    WKPoint currentCenter = computeCenter(WKPointMake(first.globalX, first.globalY), WKPointMake(second.globalX, second.globalY));
    WKPoint delta = WKPointMake((currentCenter.x - m_previousPinchGlobalCenter.x) / m_client->scale(),
                                (currentCenter.y - m_previousPinchGlobalCenter.y) / m_client->scale());
    double currentDistance = computeDistance(WKPointMake(first.globalX, first.globalY), WKPointMake(second.globalX, second.globalY));
    double newScale = m_initialPinchScale * (currentDistance / m_initialPinchDistance);

    m_client->handlePinch(timestamp, delta, newScale, m_initialPinchContentCenter);

    m_previousPinchGlobalCenter = currentCenter;
}
