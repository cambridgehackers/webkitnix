#ifndef GestureRecognizer_h
#define GestureRecognizer_h

#include <NixEvents.h>
#include <WebKit2/WKGeometry.h>
#include <glib.h>

extern "C" {
gboolean doubleTapTimer(gpointer);
}

class GestureRecognizerClient {
public:
    virtual void handleSingleTap(double timestamp, const Nix::TouchPoint&) { }
    virtual void handleDoubleTap(double timestamp, const Nix::TouchPoint&) { }
    virtual void handleTapAndHold(double timestamp, const Nix::TouchPoint&) { }

    virtual void handlePanning(double timestamp, WKPoint delta) { }
    virtual void handlePanningFinished(double timestamp) { }

    virtual double scale() { return 1; }

    virtual void handlePinchStarted(double timestamp) { }
    virtual void handlePinch(double timestamp, WKPoint delta, double distanceRatio, WKPoint contentCenter) { }
    virtual void handlePinchFinished(double timestamp) { }
};

class GestureRecognizer {
public:
    GestureRecognizer(GestureRecognizerClient*);

    void handleTouchEvent(const Nix::TouchEvent&);
    void reset();


private:
    // State functions.
    void noGesture(const Nix::TouchEvent&);
    void singleTapPressed(const Nix::TouchEvent&);
    void waitForDoubleTap(const Nix::TouchEvent&);
    void doubleTapPressed(const Nix::TouchEvent&);
    void panningInProgress(const Nix::TouchEvent&);
    void pinchInProgress(const Nix::TouchEvent&);

    void updatePanningData(double, const Nix::TouchPoint&);
    void setupPinchData(const std::vector<Nix::TouchPoint>& points);
    void updatePinchData(double, const std::vector<Nix::TouchPoint>& points);

    typedef void (GestureRecognizer::*StateFunction)(const Nix::TouchEvent&);
    StateFunction m_state;

    double m_timestamp;
    Nix::TouchPoint m_firstTouchPoint;
    guint m_doubleTapTimerId;
    Nix::TouchPoint m_previousTouchPoint;
    GestureRecognizerClient* m_client;

    double m_initialPinchDistance;
    double m_initialPinchScale;
    WKPoint m_previousPinchGlobalCenter;
    WKPoint m_initialPinchContentCenter;

    void fail(const char*);
    void doubleTapTimerTriggered();
    friend gboolean doubleTapTimer(gpointer);
};

#endif
