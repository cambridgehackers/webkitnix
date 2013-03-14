#include "GestureRecognizer.h"
#include "LinuxWindow.h"
#include "TouchMocker.h"
//#include "XlibEventUtils.h"
#ifdef WTF_USE_OPENGL_ES_2
#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>
#else
#include <GL/gl.h>
#endif

#include <WebKit2/WKPreferences.h>
#include <WebKit2/WKPreferencesPrivate.h>
#include <WebKit2/WKString.h>
#include <WebKit2/WKURL.h>
#include <WebView.h>
//#include <X11/Xutil.h>
//#include <X11/keysym.h>
#include <cassert>
#include <cmath>
#include <cstdio>
#include <cstring>
#include <glib.h>
#include <string>
#include <fstream>

#include <wtf/Platform.h>
#include <WebKit2/WKRetainPtr.h>


// this is to put up a triangle rather than show the webview render. mostly to make sure our surface is ok.
//#define KLAATU_DRAW_TRIANGLE 1

extern "C" {
static gboolean callUpdateDisplay(gpointer);
extern void glUseProgram(GLuint);
}

class MiniBrowser : public Nix::WebViewClient, public LinuxWindowClient, public GestureRecognizerClient {
public:

    enum Mode {
        MobileMode,
        DesktopMode
    };

    MiniBrowser(GMainLoop* mainLoop, Mode mode, int width, int height, int viewportHorizontalDisplacement, int viewportVerticalDisplacement);
    virtual ~MiniBrowser();

    WKPageRef pageRef() const { return m_webView->pageRef(); }

    // LinuxWindowClient.
    virtual void handleExposeEvent() { scheduleUpdateDisplay(); }
    //    virtual void handleKeyPressEvent(const XKeyPressedEvent&);
    //virtual void handleKeyReleaseEvent(const XKeyReleasedEvent&);
    //virtual void handleButtonPressEvent(const XButtonPressedEvent&);
    //virtual void handleButtonReleaseEvent(const XButtonReleasedEvent&);
    //virtual void handlePointerMoveEvent(const XPointerMovedEvent&);
    virtual void handleSizeChanged(int, int);
    virtual void handleClosed();

    // Nix::WebViewClient.
    virtual void viewNeedsDisplay(WKRect) { scheduleUpdateDisplay(); }
    virtual void webProcessCrashed(WKStringRef url);
    virtual void webProcessRelaunched();
    virtual void pageDidRequestScroll(WKPoint position);
    virtual void didChangeContentsSize(WKSize size);
    virtual void didFindZoomableArea(WKPoint target, WKRect area);
    virtual void doneWithTouchEvent(const Nix::TouchEvent&, bool wasEventHandled);
    virtual void doneWithGestureEvent(const Nix::GestureEvent&, bool wasEventHandled);
    virtual void updateTextInputState(bool isContentEditable, WKRect cursorRect, WKRect editorRect);
    virtual void compositeCustomLayerToCurrentGLContext(uint32_t, WKRect, const float* matrix, float opacity);

    // GestureRecognizerClient.
    virtual void handleSingleTap(double timestamp, const Nix::TouchPoint&);
    virtual void handleDoubleTap(double timestamp, const Nix::TouchPoint&);
    virtual void handlePanning(double timestamp, WKPoint delta);
    virtual void handlePanningFinished(double timestamp);
    virtual void handlePinch(double timestamp, WKPoint delta, double scale, WKPoint contentCenter);
    virtual void handlePinchFinished(double timestamp);

    virtual double scale();

    void setTouchEmulationMode(bool enabled);
    Mode mode() const { return m_mode; }
    void setCustomLayerTestElement(const char* element);
private:

    enum ScaleBehavior {
        AdjustToBoundaries = 1 << 0,
        LowerMinimumScale = 1 << 1
    };
    //void handleWheelEvent(const XButtonPressedEvent&);
    //void updateClickCount(const XButtonPressedEvent&);

    void updateDisplay();
    void scheduleUpdateDisplay();
    WKPoint adjustScrollPositionToBoundaries(WKPoint position);
    void adjustScrollPosition();
    void adjustViewportToTextInputArea();
    double scaleToFitContents();

    void scaleAtPoint(const WKPoint& point, double scale, ScaleBehavior scaleBehavior = AdjustToBoundaries);

    Nix::WebView* webViewAtX11Position(const WKPoint& poisition);

    WKRetainPtr<WKContextRef> m_context;
    WKRetainPtr<WKPageGroupRef> m_pageGroup;
    LinuxWindow* m_window;
    Nix::WebView* m_webView;
    WKRect m_webViewRect;
    GMainLoop* m_mainLoop;
    double m_lastClickTime;
    int m_lastClickX;
    int m_lastClickY;
    Nix::MouseEvent::Button m_lastClickButton;
    unsigned m_clickCount;
    TouchMocker* m_touchMocker;
    Mode m_mode;
    bool m_displayUpdateScheduled;
    WKSize m_contentsSize;
    GestureRecognizer m_gestureRecognizer;
    bool m_postponeTextInputUpdates;
    bool m_shouldFocusEditableArea;
    WKRect m_cursorRect;
    WKRect m_editorRect;
    bool m_shouldRestoreViewportWhenLosingFocus;
    double m_scaleBeforeFocus;
    WKPoint m_scrollPositionBeforeFocus;
    uint32_t m_customRendererID;

    friend gboolean callUpdateDisplay(gpointer);
};

MiniBrowser::MiniBrowser(GMainLoop* mainLoop, Mode mode, int width, int height, int viewportHorizontalDisplacement, int viewportVerticalDisplacement)
    : m_context(AdoptWK, WKContextCreate())
    , m_pageGroup(AdoptWK, (WKPageGroupCreateWithIdentifier(WKStringCreateWithUTF8CString("MiniBrowser"))))
    , m_window(new LinuxWindow(this, width, height))
    , m_webView(0)
    , m_mainLoop(mainLoop)
    , m_lastClickTime(0)
    , m_lastClickX(0)
    , m_lastClickY(0)
    , m_lastClickButton(Nix::MouseEvent::NoButton)
    , m_clickCount(0)
    , m_touchMocker(0)
    , m_mode(mode)
    , m_displayUpdateScheduled(false)
    , m_gestureRecognizer(GestureRecognizer(this))
    , m_postponeTextInputUpdates(true)
    , m_shouldFocusEditableArea(false)
    , m_shouldRestoreViewportWhenLosingFocus(false)
{
    g_main_loop_ref(m_mainLoop);

    WKPreferencesRef preferences = WKPageGroupGetPreferences(m_pageGroup.get());
    WKPreferencesSetAcceleratedCompositingEnabled(preferences, true);
    WKPreferencesSetFrameFlatteningEnabled(preferences, true);
    WKPreferencesSetDeveloperExtrasEnabled(preferences, true);

    m_webView = Nix::WebView::create(m_context.get(), m_pageGroup.get(), this);
    m_webView->initialize();

    if (m_mode == MobileMode)
        WKPageSetUseFixedLayout(pageRef(), true);

    WKSize size = m_window->size();
    m_webViewRect = WKRectMake(viewportHorizontalDisplacement, viewportVerticalDisplacement, size.width - viewportHorizontalDisplacement, size.height - viewportVerticalDisplacement);
    m_webView->setSize(m_webViewRect.size);

    if (viewportHorizontalDisplacement || viewportVerticalDisplacement) {
        cairo_matrix_t userTransform;
        cairo_matrix_init_translate(&userTransform, viewportHorizontalDisplacement, viewportVerticalDisplacement);
        m_webView->setUserViewportTransformation(userTransform);
    }

    m_webView->setFocused(true);
    m_webView->setVisible(true);
    m_webView->setActive(true);
}

MiniBrowser::~MiniBrowser()
{
    g_main_loop_unref(m_mainLoop);

    delete m_webView;
    delete m_window;
    delete m_touchMocker;
}

void MiniBrowser::setCustomLayerTestElement(const char* element)
{
    WKRetainPtr<WKStringRef> str = adoptWK(WKStringCreateWithUTF8CString(element));
    m_customRendererID = m_webView->addCustomLayer(str.get());
}

void MiniBrowser::setTouchEmulationMode(bool enabled)
{
    if (enabled && !m_touchMocker) {
        m_touchMocker = new TouchMocker(m_webView);
    } else if (!enabled && m_touchMocker) {
        delete m_touchMocker;
        m_touchMocker = 0;
    }
}


enum NavigationCommand {
    NoNavigation,
    BackNavigation,
    ForwardNavigation,
    ReloadNavigation
};

static const double doubleClickInterval = 300;
static const double horizontalMarginForViewportAdjustment = 8.0;
static const double scaleFactorForTextInputFocus = 2.0;

void MiniBrowser::handleSizeChanged(int width, int height)
{
    if (!m_webView)
        return;

    m_webViewRect.size.width = width - m_webViewRect.origin.x;
    m_webViewRect.size.height = height - m_webViewRect.origin.y;
    m_webView->setSize(m_webViewRect.size);

    if (m_mode == MobileMode)
        m_webView->setScale(scaleToFitContents());
}

void MiniBrowser::handleClosed()
{
    g_main_loop_quit(m_mainLoop);
}

#define FLOAT_TO_FIXED(X)   ((X) * 65535.0)



static GLfloat view_rotx = 0.0, view_roty = 0.0;

static GLint u_matrix = -1;
static GLint attr_pos = 0, attr_color = 1;

static void
make_z_rot_matrix(GLfloat angle, GLfloat *m)
{
   float c = cos(angle * M_PI / 180.0);
   float s = sin(angle * M_PI / 180.0);
   int i;
   for (i = 0; i < 16; i++)
      m[i] = 0.0;
   m[0] = m[5] = m[10] = m[15] = 1.0;

   m[0] = c;
   m[1] = s;
   m[4] = -s;
   m[5] = c;
}

static void
make_scale_matrix(GLfloat xs, GLfloat ys, GLfloat zs, GLfloat *m)
{
   int i;
   for (i = 0; i < 16; i++)
      m[i] = 0.0;
   m[0] = xs;
   m[5] = ys;
   m[10] = zs;
   m[15] = 1.0;
}


static void
mul_matrix(GLfloat *prod, const GLfloat *a, const GLfloat *b)
{
#define A(row,col)  a[(col<<2)+row]
#define B(row,col)  b[(col<<2)+row]
#define P(row,col)  p[(col<<2)+row]
   GLfloat p[16];
   GLint i;
   for (i = 0; i < 4; i++) {
      const GLfloat ai0=A(i,0),  ai1=A(i,1),  ai2=A(i,2),  ai3=A(i,3);
      P(i,0) = ai0 * B(0,0) + ai1 * B(1,0) + ai2 * B(2,0) + ai3 * B(3,0);
      P(i,1) = ai0 * B(0,1) + ai1 * B(1,1) + ai2 * B(2,1) + ai3 * B(3,1);
      P(i,2) = ai0 * B(0,2) + ai1 * B(1,2) + ai2 * B(2,2) + ai3 * B(3,2);
      P(i,3) = ai0 * B(0,3) + ai1 * B(1,3) + ai2 * B(2,3) + ai3 * B(3,3);
   }
   memcpy(prod, p, sizeof(p));
#undef A
#undef B
#undef PROD
}


static int klaatu_shaders_created=0;
static void klaatu_init_shaders(void){
{
    klaatu_shaders_created=1;
#ifndef KLAATU
   static const char *fragShaderText =
      "varying vec4 v_color;\n"
      "void main() {\n"
      "   gl_FragColor = v_color;\n"
      "}\n";
#else
   static const char *fragShaderText =
      "precision mediump float;\n"
      "void main() {\n"
      "   gl_FragColor = vec4(1.0,0.0,1.0,1.0);\n"
      "}\n";
#endif //KLAATU
   static const char *vertShaderText =
      "uniform mat4 modelviewProjection;\n"
      "attribute vec4 pos;\n"
      "attribute vec4 color;\n"
      "varying vec4 v_color;\n"
      "void main() {\n"
      "   gl_Position = modelviewProjection * pos;\n"
      "   v_color = color;\n"
      "}\n";

   GLuint fragShader, vertShader, program;
   GLint stat;


   fragShader = glCreateShader(GL_FRAGMENT_SHADER);
   glShaderSource(fragShader, 1, (const char **) &fragShaderText, NULL);
   glCompileShader(fragShader);
   glGetShaderiv(fragShader, GL_COMPILE_STATUS, &stat);
   if (!stat) {
      printf("Error: fragment shader did not compile!\n");
      exit(1);
   }

   vertShader = glCreateShader(GL_VERTEX_SHADER);
   glShaderSource(vertShader, 1, (const char **) &vertShaderText, NULL);
   glCompileShader(vertShader);
   glGetShaderiv(vertShader, GL_COMPILE_STATUS, &stat);
   if (!stat) {
      printf("Error: vertex shader did not compile!\n");
      exit(1);
   }

   program = glCreateProgram();
   glAttachShader(program, fragShader);
   glAttachShader(program, vertShader);
   glLinkProgram(program);

   glGetProgramiv(program, GL_LINK_STATUS, &stat);
   if (!stat) {
      char log[1000];
      GLsizei len;
      glGetProgramInfoLog(program, 1000, &len, log);
      printf("Error: linking:\n%s\n", log);
      exit(1);
   }

   glUseProgram(program);

   if (1) {
      /* test setting attrib locations */
      glBindAttribLocation(program, attr_pos, "pos");
      glBindAttribLocation(program, attr_color, "color");
      glLinkProgram(program);  /* needed to put attribs into effect */
   }
   else {
      /* test automatic attrib locations */
      attr_pos = glGetAttribLocation(program, "pos");
      attr_color = glGetAttribLocation(program, "color");
   }

   u_matrix = glGetUniformLocation(program, "modelviewProjection");
   printf("Uniform modelviewProjection at %d\n", u_matrix);
   printf("Attrib pos at %d\n", attr_pos);
   printf("Attrib color at %d\n", attr_color);
}


}
static void klaatu_draw_triangle(void)
{
   static const GLfloat verts[3][2] = {
      { -1, -1 },
      {  1, -1 },
      {  0,  1 }
   };
   static const GLfloat colors[3][3] = {
      { 1, 0, 0 },
      { 0, 1, 0 },
      { 0, 0, 1 }
   };
   GLfloat mat[16], rot[16], scale[16];

   if (!klaatu_shaders_created)
       klaatu_init_shaders();

   /* Set modelview/projection matrix */
   make_z_rot_matrix(view_rotx, rot);
   make_scale_matrix(0.5, 0.5, 0.5, scale);
   mul_matrix(mat, rot, scale);
   glUniformMatrix4fv(u_matrix, 1, GL_FALSE, mat);

   glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

   {
      glVertexAttribPointer(attr_pos, 2, GL_FLOAT, GL_FALSE, 0, verts);
      glVertexAttribPointer(attr_color, 3, GL_FLOAT, GL_FALSE, 0, colors);
      glEnableVertexAttribArray(attr_pos);
      glEnableVertexAttribArray(attr_color);

      glDrawArrays(GL_TRIANGLES, 0, 3);

      glDisableVertexAttribArray(attr_pos);
      glDisableVertexAttribArray(attr_color);
   }
}


void MiniBrowser::updateDisplay()
{
    if (!m_webView || !m_window)
        return;

    m_window->makeCurrent();

#ifndef KLAATU_DRAW_TRIANGLE
    WKSize size = m_window->size();
    glViewport(0, 0, size.width, size.height);
    glClearColor(0.4, 0.4, 0.4, 1.0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    m_webView->paintToCurrentGLContext();
    if (m_touchMocker)
        m_touchMocker->paintTouchPoints(size);
#else
    klaatu_draw_triangle();
#endif
    m_window->swapBuffers();
}

static gboolean callUpdateDisplay(gpointer data)
{
    MiniBrowser* browser = reinterpret_cast<MiniBrowser*>(data);

    assert(browser->m_displayUpdateScheduled);
    browser->m_displayUpdateScheduled = false;
    browser->updateDisplay();
    return 0;
}

void MiniBrowser::scheduleUpdateDisplay()
{
    if (m_displayUpdateScheduled)
        return;

    m_displayUpdateScheduled = true;
    g_timeout_add(0, callUpdateDisplay, this);
}

WKPoint MiniBrowser::adjustScrollPositionToBoundaries(WKPoint position)
{
    WKSize visibleContentsSize = m_webView->visibleContentsSize();
    double rightBoundary = m_contentsSize.width - visibleContentsSize.width;
    // Contents height may be shorter than the scaled viewport height.
    double bottomBoundary = m_contentsSize.height < visibleContentsSize.height ? 0 : m_contentsSize.height - visibleContentsSize.height;

    if (position.x < 0)
        position.x = 0;
    else if (position.x > rightBoundary)
        position.x = rightBoundary;
    if (position.y < 0)
        position.y = 0;
    else if (position.y > bottomBoundary)
        position.y = bottomBoundary;

    return position;
}

double MiniBrowser::scaleToFitContents()
{
    return m_webView->size().width / m_contentsSize.width;
}

void MiniBrowser::adjustScrollPosition()
{
    WKPoint position = adjustScrollPositionToBoundaries(m_webView->scrollPosition());
    if (position == m_webView->scrollPosition())
        return;
    m_webView->setScrollPosition(position);
}

void MiniBrowser::webProcessCrashed(WKStringRef url)
{
    size_t urlStringSize =  WKStringGetMaximumUTF8CStringSize(url);
    char* urlString = new char[urlStringSize];
    WKStringGetUTF8CString(url, urlString, urlStringSize);
    fprintf(stderr, "The web process has crashed on '%s'.\n", urlString);
    delete urlString;
}

void MiniBrowser::webProcessRelaunched()
{
    fprintf(stdout, "The web process has been restarted.\n");
}

void MiniBrowser::pageDidRequestScroll(WKPoint position)
{
    if (!m_webView->isSuspended())
        m_webView->setScrollPosition(adjustScrollPositionToBoundaries(position));
}

void MiniBrowser::didChangeContentsSize(WKSize size)
{
    m_contentsSize = size;

    if (m_mode == MobileMode) {
        m_webView->setScale(scaleToFitContents());
        adjustScrollPosition();
    }
}

void MiniBrowser::didFindZoomableArea(WKPoint target, WKRect area)
{
    // Zoomable area width is the same as web page width, and this is fully visible.
    if (m_contentsSize.width == area.size.width && m_contentsSize.width == m_webView->visibleContentsSize().width)
        return;

    // The zoomed area will look nicer with a horizontal margin.
    double scale = m_webView->size().width / (area.size.width + horizontalMarginForViewportAdjustment * 2.0);

    // Trying to zoom to an area with the same scale factor causes a zoom out.
    if (scale == m_webView->scale())
        scale = scaleToFitContents();
    else {
        // We want the zoomed content area to fit horizontally in the WebView,
        // so let's give the scaleAtPoint method a suitable value.
        target.x = area.origin.x - horizontalMarginForViewportAdjustment;
        m_webView->setScrollPosition(WKPointMake(target.x, m_webView->scrollPosition().y));
    }

    scaleAtPoint(target, scale);
}

void MiniBrowser::doneWithTouchEvent(const Nix::TouchEvent& touchEvent, bool wasEventHandled)
{
    if (wasEventHandled)
        return;
    m_gestureRecognizer.handleTouchEvent(touchEvent);
}

double MiniBrowser::scale()
{
    return m_webView->scale();
}

void MiniBrowser::handleSingleTap(double timestamp, const Nix::TouchPoint& touchPoint)
{
    Nix::GestureEvent gestureEvent;
    gestureEvent.type = Nix::InputEvent::GestureSingleTap;
    gestureEvent.timestamp = timestamp;
    gestureEvent.modifiers = 0;
    gestureEvent.x = touchPoint.x;
    gestureEvent.y = touchPoint.y;
    gestureEvent.globalX = touchPoint.globalX;
    gestureEvent.globalY = touchPoint.globalY;
    gestureEvent.width = 20;
    gestureEvent.height = 20;
    gestureEvent.deltaX = 0.0;
    gestureEvent.deltaY = 0.0;

    m_postponeTextInputUpdates = false;
    m_webView->sendEvent(gestureEvent);
}

void MiniBrowser::handleDoubleTap(double timestamp, const Nix::TouchPoint& touchPoint)
{
    WKPoint contentsPoint = WKPointMake(touchPoint.x, touchPoint.y);
    m_webView->findZoomableAreaForPoint(contentsPoint, touchPoint.verticalRadius, touchPoint.horizontalRadius);
}

void MiniBrowser::handlePanning(double timestamp, WKPoint delta)
{
    // When the user is panning around the contents we don't force the page scroll position
    // to respect any boundaries other than the physical constraints of the device from where
    // the user input came. This will be adjusted after the user interaction ends.
    m_webView->suspendActiveDOMObjectsAndAnimations();
    WKPoint position = m_webView->scrollPosition();
    position.x -= delta.x;
    position.y -= delta.y;
    m_webView->setScrollPosition(position);
}

void MiniBrowser::handlePanningFinished(double timestamp)
{
    adjustScrollPosition();
    m_webView->resumeActiveDOMObjectsAndAnimations();
}

void MiniBrowser::handlePinch(double timestamp, WKPoint delta, double scale, WKPoint contentCenter)
{
    // Scaling: The page should be scaled proportionally to the distance of the pinch.
    // Scrolling: If the center of the pinch initially was position (120,120) in content
    //            coordinates, them during the page must be scrolled to keep the pinch center
    //            at the same coordinates.
    m_webView->suspendActiveDOMObjectsAndAnimations();
    WKPoint position = WKPointMake(m_webView->scrollPosition().x - delta.x, m_webView->scrollPosition().y - delta.y);

    m_webView->setScrollPosition(position);
    scaleAtPoint(contentCenter, scale, LowerMinimumScale);
}

void MiniBrowser::handlePinchFinished(double timestamp)
{
    adjustScrollPosition();
    m_webView->resumeActiveDOMObjectsAndAnimations();
}

void MiniBrowser::scaleAtPoint(const WKPoint& point, double scale, ScaleBehavior scaleBehavior)
{
    double minimumScale = double(m_webView->size().width) / m_contentsSize.width;
    if(scaleBehavior & LowerMinimumScale)
        minimumScale *= 0.5;
    if (scale < minimumScale)
        scale = minimumScale;

    // Calculate new scroll points that will keep the content
    // approximately at the same visual point.
    double scaleRatio = m_webView->scale() / scale;
    WKPoint position = WKPointMake(point.x - (point.x - m_webView->scrollPosition().x) * scaleRatio,
                                   point.y - (point.y - m_webView->scrollPosition().y) * scaleRatio);

    m_webView->setScale(scale);
    if (scaleBehavior & AdjustToBoundaries)
        position = adjustScrollPositionToBoundaries(position);
    m_webView->setScrollPosition(position);
}

static inline bool areaContainsPoint(const WKRect& area, const WKPoint& point)
{
    return !(point.x < area.origin.x || point.y < area.origin.y || point.x >= (area.origin.x + area.size.width) || point.y >= (area.origin.y + area.size.height));
}

#if 0
//used to handle mouse button down, up, move etc.
Nix::WebView* MiniBrowser::webViewAtX11Position(const WKPoint& position)
{
    if (areaContainsPoint(m_webViewRect, position))
        return m_webView;
    return 0;
}
#endif 
void MiniBrowser::adjustViewportToTextInputArea()
{
    m_shouldRestoreViewportWhenLosingFocus = true;
    m_scaleBeforeFocus = m_webView->scale();
    m_scrollPositionBeforeFocus = m_webView->scrollPosition();

    m_webView->setScale(scaleFactorForTextInputFocus);

    // After scaling to fit editor rect width, we align vertically based on cursor rect.
    WKPoint scrollPosition;
    scrollPosition.x = m_editorRect.origin.x - scaleFactorForTextInputFocus * horizontalMarginForViewportAdjustment;
    double verticalOffset = (m_webView->visibleContentsSize().height - m_cursorRect.size.height) / 2.0;
    scrollPosition.y = m_cursorRect.origin.y - verticalOffset;
    scrollPosition = adjustScrollPositionToBoundaries(scrollPosition);
    m_webView->setScrollPosition(scrollPosition);
}

void MiniBrowser::doneWithGestureEvent(const Nix::GestureEvent& event, bool wasEventHandled)
{
    if (!wasEventHandled)
        return;

    if (event.type == Nix::InputEvent::GestureSingleTap && m_shouldFocusEditableArea) {
        m_shouldFocusEditableArea = false;
        adjustViewportToTextInputArea();
    }

    m_postponeTextInputUpdates = true;
}

static inline bool WKRectIsEqual(const WKRect& a, const WKRect& b)
{
    return a.origin == b.origin && a.size.width == b.size.width && a.size.height == b.size.height;
}

void MiniBrowser::updateTextInputState(bool isContentEditable, WKRect cursorRect, WKRect editorRect)
{
    if (m_postponeTextInputUpdates)
        return;

    if (isContentEditable) {
        // If we're only moving cursor inside the current editor, then we should not focus it again.
        if (WKRectIsEqual(editorRect, m_editorRect))
            return;

        m_shouldFocusEditableArea = true;
        m_cursorRect = cursorRect;
        m_editorRect = editorRect;
    } else {
        if (m_shouldRestoreViewportWhenLosingFocus) {
            m_shouldRestoreViewportWhenLosingFocus = false;
            m_webView->setScale(m_scaleBeforeFocus);
            m_webView->setScrollPosition(m_scrollPositionBeforeFocus);
        }
        m_cursorRect = WKRectMake(0, 0, 0, 0);
        m_editorRect = WKRectMake(0, 0, 0, 0);
    }
}


void MiniBrowser::compositeCustomLayerToCurrentGLContext(uint32_t id, WKRect rect, const float* matrix, float opacity)
{
    fprintf(stderr," MiniBrowser::compositeCustomLayerToCurrentGLContext called id=%d, m_customRendererID=%d, opacity = %f\n",id,m_customRendererID,opacity);
    return;
    
#if 0
    if (id != m_customRendererID)
        return;

    glUseProgram(0);

    const float p[] = { rect.origin.x, rect.origin.y, rect.origin.x + rect.size.width, rect.origin.y + rect.size.width };
    const GLfloat vertexData[] = { p[0], p[1], p[2], p[1], p[2], p[3], p[0], p[3] };

    WKSize size = m_window->size();
    glPushMatrix();
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0, size.width, size.height, 0, -1, 1);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glLoadMatrixf(matrix);
    glEnable(GL_BLEND);
    glEnable(GL_COLOR);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnableClientState(GL_VERTEX_ARRAY);
    glColor4f(1, 0, 0, 1);
    glVertexPointer(2, GL_FLOAT, 0, vertexData);
    glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
    glPopMatrix();
#endif
}

struct Device {
    enum Type {
        Default,
        N9,
        IPad,
        IPhone,
        Android
    };

    int width;
    int height;
    const char* userAgent;
};

Device deviceList[] = {
    //{ 1024, 768, "" },
    { 720, 1280, "" },
    { 854, 480, "Mozilla/5.0 (MeeGo; NokiaN9) AppleWebKit/534.13 (KHTML, like Gecko) NokiaBrowser/8.5.0 Mobile Safari/534.13" },
    { 1024, 768, "Mozilla/5.0 (iPad; CPU OS 5_0 like Mac OS X) AppleWebKit/534.46 (KHTML, like Gecko) Version/5.1 Mobile/9A334 Safari/7534.48.3" },
    { 960, 640, "Mozilla/5.0 (iPhone; CPU iPhone OS 5_0 like Mac OS X) AppleWebKit/534.46 (KHTML, like Gecko) Version/5.1 Mobile/9A334 Safari/7534.48.3" },
    { 800, 480, "Mozilla/5.0 (Linux; U; Android 4.0.2; en-us; Galaxy Nexus Build/ICL53F) AppleWebKit/534.30 (KHTML, like Gecko) Version/4.0 Mobile Safari/534.30" }
};

int main(int argc, char* argv[])
{
    printf("MiniBrowser: Use Alt + Left and Alt + Right to navigate back and forward. Use F5 to reload.\n");

    int width = 0;
    int height = 0;
    int viewportHorizontalDisplacement = 0;
    int viewportVerticalDisplacement = 0;
    std::string url;
    Device::Type device = Device::Default;
    MiniBrowser::Mode browserMode = MiniBrowser::MobileMode;
    bool touchEmulationEnabled = false;
    const char* customLayerTestElement = 0;

    for (int i = 1; i < argc; ++i) {
        if (!strcmp(argv[i], "--desktop"))
            browserMode = MiniBrowser::DesktopMode;
        else if (!strcmp(argv[i], "-t") || !strcmp(argv[i], "--touch-emulation"))
            touchEmulationEnabled = true;
        else if (!strcmp(argv[i], "--window-size")) {
            if (i + 1 == argc) {
                fprintf(stderr, "--window-size requires an argument.\n");
                return 1;
            }
            if (sscanf(argv[++i], "%dx%d", &width, &height) != 2) {
                fprintf(stderr, "--window-size format is WIDTHxHEIGHT.\n");
                return 1;
            }
        } else if (!strcmp(argv[i], "--viewport-displacement")) {
            if (i + 1 == argc) {
                fprintf(stderr, "--viewport-displacement requires an argument.\n");
                return 1;
            }
            if (sscanf(argv[++i], "%dx%d", &viewportHorizontalDisplacement, &viewportVerticalDisplacement) != 2) {
                fprintf(stderr, "--viewport-displacement format is HORIZDISPLACEMENTxVERTDISPLACEMENT.\n");
                return 1;
            }
        } else if (!strcmp(argv[i], "--n9"))
            device = Device::N9;
        else if (!strcmp(argv[i], "--ipad"))
            device = Device::IPad;
        else if (!strcmp(argv[i], "--iphone"))
            device = Device::IPhone;
        else if (!strcmp(argv[i], "--android"))
            device = Device::Android;
        else if (!strcmp(argv[i], "--custom-layer-id"))
            customLayerTestElement = argv[++i];
        else
            url = argv[i];
    }

    if (width == 0 && height == 0) {
        width = deviceList[device].width;
        height = deviceList[device].height;
    }

    if (url.empty())
        url = "http://www.google.com";
    else if (url.find("http") != 0 && url.find("file://") != 0) {
        std::ifstream localFile(url.c_str());
        url.insert(0, localFile ? "file://" : "http://");
    }

    GMainLoop* mainLoop = g_main_loop_new(0, false);
    MiniBrowser browser(mainLoop, browserMode, width, height, viewportHorizontalDisplacement, viewportVerticalDisplacement);

    if (browser.mode() == MiniBrowser::MobileMode || touchEmulationEnabled) {
        printf("Touch Emulation Mode toggled. Hold Control key to build and emit a multi-touch event: each mouse button should be a different touch point. Release Control Key to clear all tracking pressed touches.\n");
        browser.setTouchEmulationMode(true);
    }

    if (deviceList[device].userAgent)
        WKPageSetCustomUserAgent(browser.pageRef(), WKStringCreateWithUTF8CString(deviceList[device].userAgent));

    if (browser.mode() == MiniBrowser::MobileMode)
        printf("Use Control + mouse wheel to zoom in and out.\n");

    WKPageLoadURL(browser.pageRef(), WKURLCreateWithUTF8CString(url.c_str()));
    if (customLayerTestElement)
        browser.setCustomLayerTestElement(customLayerTestElement);

    g_main_loop_run(mainLoop);
    g_main_loop_unref(mainLoop);
}
