#include "LinuxWindow.h"

#ifdef WTF_USE_OPENGL_ES_2
#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>
#else
#include <GL/gl.h>
#endif


#include <cstdio>
#include <cstdlib>
#include <glib.h>

#include <ui/DisplayInfo.h>
#include <ui/FramebufferNativeWindow.h>
#include <gui/SurfaceComposerClient.h>
#include "klaatu_events.h"

using namespace android;
#define ASSERT_EQ(A, B) {if ((A) != (B)) {printf ("ERROR: %d -- (%d:%d)\n", __LINE__,A,B); exit(9); }}
#define ASSERT_NE(A, B) {if ((A) == (B)) {printf ("ERROR: %d -- (%d:%d)\n", __LINE__,A,B); exit(9); }}
#define EXPECT_TRUE(A) {if ((A) == 0) {printf ("ERROR: %d -- (%d)\n", __LINE__,A); exit(9); }}



void fatalError(const char* message)
{
    fprintf(stderr, "Error: %s\n", message);
    exit(1);
}



static EGLDisplay mEglDisplay = EGL_NO_DISPLAY;
static EGLSurface mEglSurface = EGL_NO_SURFACE;
static EGLContext mEglContext = EGL_NO_CONTEXT;
static sp<android::SurfaceComposerClient> mSession;
static sp<android::SurfaceControl>        mControl;
static sp<android::Surface>               mAndroidSurface;


void LinuxWindow::klaatu_init_graphics()
{
  
  DisplayInfo display_info;

  
// initial part shamelessly stolen from klaatu-api
  static EGLint sDefaultContextAttribs[] = {
    EGL_CONTEXT_CLIENT_VERSION, 2, EGL_NONE };

#if 1
  // these were from jca's sio2 tests
  static EGLint sDefaultConfigAttribs[] = {
    EGL_SURFACE_TYPE, EGL_PBUFFER_BIT, EGL_RENDERABLE_TYPE, EGL_OPENGL_ES2_BIT,
    EGL_RED_SIZE, 8, EGL_GREEN_SIZE, 8, EGL_BLUE_SIZE, 8, EGL_ALPHA_SIZE, 8,
    EGL_DEPTH_SIZE, 16, EGL_STENCIL_SIZE, 8, EGL_NONE };

#else

  //these are what the X version is using
  /*  static const EGLint sDefaultConfigAttribs[] = {
      EGL_DEPTH_SIZE, 24,
      EGL_RED_SIZE, 8, 
      EGL_ALPHA_SIZE, 0,
      EGL_NONE
  };
  */
  /*  static EGLint sDefaultConfigAttribs[] = {
      // EGL_SURFACE_TYPE, EGL_WINDOW_BIT, 
      //EGL_RENDERABLE_TYPE, EGL_OPENGL_ES2_BIT,
    EGL_RED_SIZE, 8, 
    //EGL_GREEN_SIZE, 8, 
    //EGL_BLUE_SIZE, 8, 
    EGL_ALPHA_SIZE, 0,
    // EGL_DEPTH_SIZE, 16, 
    //EGL_STENCIL_SIZE, 8, 
    EGL_NONE };
  */

  static const EGLint sDefaultConfigAttribs[] = {
      EGL_CONFIG_ID, 4,
      EGL_NONE
  };
#endif

  mSession = new SurfaceComposerClient();
  int status = mSession->getDisplayInfo(0, &display_info);
  //*width = display_info.w;
  //*height = display_info.h;
  int width,height;
  width = m_width;
  height = m_height;

  fprintf(stderr,"KIG_> width = %d height = %d\n",width,height);
  fprintf(stderr,"KIG_> DISPLAY: width = %d height = %d\n",display_info.w,display_info.h);
  mControl = mSession->createSurface(
				     0, width, height, PIXEL_FORMAT_RGB_888);
  SurfaceComposerClient::openGlobalTransaction();
  mControl->setLayer(0x40000000);
  SurfaceComposerClient::closeGlobalTransaction();
  mAndroidSurface = mControl->getSurface();
  EGLNativeWindowType eglWindow = mAndroidSurface.get();
  mEglDisplay = eglGetDisplay(EGL_DEFAULT_DISPLAY);
  ASSERT_EQ(EGL_SUCCESS, eglGetError());
  ASSERT_NE(EGL_NO_DISPLAY, mEglDisplay);
  EGLint majorVersion, minorVersion;
  EXPECT_TRUE(eglInitialize(mEglDisplay, &majorVersion, &minorVersion));
  ASSERT_EQ(EGL_SUCCESS, eglGetError());
  printf("EglVersion %d:%d\n", majorVersion, minorVersion);

  EGLint numConfigs = 0;
  EGLConfig  mGlConfig;
  EXPECT_TRUE(eglChooseConfig(mEglDisplay, sDefaultConfigAttribs, &mGlConfig, 1, &numConfigs));
  printf("numConfigs %d\n", numConfigs);
  // debugging
  EGLint bavery_value;
  eglGetConfigAttrib(mEglDisplay,mGlConfig,EGL_CONFIG_ID,&bavery_value);
  printf("bavery: we wackily chose config id = %d\n",bavery_value);
  eglGetConfigAttrib(mEglDisplay,mGlConfig,EGL_DEPTH_SIZE,&bavery_value);
  printf("bavery: we wackily chose depth = %d\n",bavery_value);
  eglGetConfigAttrib(mEglDisplay,mGlConfig,EGL_BUFFER_SIZE,&bavery_value);
  printf("bavery: we wackily chose buffer = %d\n",bavery_value);
  eglGetConfigAttrib(mEglDisplay,mGlConfig,EGL_RED_SIZE,&bavery_value);
  printf("bavery: we wackily chose red = %d\n",bavery_value);
  eglGetConfigAttrib(mEglDisplay,mGlConfig,EGL_ALPHA_SIZE,&bavery_value);
  printf("bavery: we wackily chose ALPHA = %d\n",bavery_value);
      

  //end debugging
  mEglSurface = eglCreateWindowSurface(mEglDisplay, mGlConfig, eglWindow, NULL);
  printf("bavery eglsurface  eglgeterror = %d\n",eglGetError());
  ASSERT_EQ(EGL_SUCCESS, eglGetError());
  ASSERT_NE(EGL_NO_SURFACE, mEglSurface);
  mEglContext = eglCreateContext(mEglDisplay, mGlConfig, EGL_NO_CONTEXT, sDefaultContextAttribs);
  printf("bavery context eglgeterror = %d\n",eglGetError());
  ASSERT_EQ(EGL_SUCCESS, eglGetError());
  ASSERT_NE(EGL_NO_CONTEXT, mEglContext);
  EXPECT_TRUE(eglMakeCurrent(mEglDisplay, mEglSurface, mEglSurface, mEglContext));
  ASSERT_EQ(EGL_SUCCESS, eglGetError());
  
  printf("done with the klaatu_init_graphics\n");


}

EventSingleton* eventSingleton;
class EVDispatcher : public EventSingleton {
public:
    bool down;
    //    Local<Function> cb;
    EVDispatcher() {
        down = false;
        //cb = CB;
    }
    virtual void touchStart(float rx, float ry, unsigned int tap_count=0) { 
        if(down) {
            fprintf(stderr,"touch moving\n");
            //Local<Object> event = Object::New();
            //event->Set(String::NewSymbol("x"), Number::New(rx));
            //event->Set(String::NewSymbol("y"), Number::New(ry));
            //event->Set(String::NewSymbol("type"), String::New("drag"));
            //Handle<Value> argv[] = {event};
            //cb->Call(Context::GetCurrent()->Global(), 1, argv);
            
        } else {
            down = true;
            fprintf(stderr,"touch starting\n");
            //Local<Object> event = Object::New();
            //event->Set(String::NewSymbol("x"), Number::New(rx));
            //event->Set(String::NewSymbol("y"), Number::New(ry));
            //event->Set(String::NewSymbol("type"), String::New("press"));
            //Handle<Value> argv[] = {event};
            //cb->Call(Context::GetCurrent()->Global(), 1, argv);
        }
    }
    virtual void touchMove(float rx, float ry, unsigned int tap_count=0) { 
        fprintf(stderr,"touch moving\n");
    }
    virtual void touchEnd(float rx, float ry, unsigned int tap_count=0) { 
        fprintf(stderr,"touch ending\n");
        //Local<Object> event = Object::New();
        //event->Set(String::NewSymbol("x"), Number::New(rx));
        //event->Set(String::NewSymbol("y"), Number::New(ry));
        //event->Set(String::NewSymbol("type"), String::New("release"));
        //Handle<Value> argv[] = {event};
        //cb->Call(Context::GetCurrent()->Global(), 1, argv);
        down = false;
    }
};


LinuxWindow::LinuxWindow(LinuxWindowClient* client, int width, int height)
    : m_client(client)
      //    , m_eventSource(0)
      //    , m_display(XOpenDisplay(0))
      //, m_glContextData(0)
    , m_width(width)
    , m_height(height)
{
#if 0
    if (!m_display)
        fatalError("couldn't connect to X server\n");

    VisualID visualID = setupXVisualID();
    m_eventSource = new XlibEventSource(m_display, this);
    m_window = createXWindow(visualID);

    createGLContext();
    makeCurrent();
    glEnable(GL_DEPTH_TEST);
#endif

    klaatu_init_graphics();
    eventSingleton = new EVDispatcher();
    enable_touch(width,height);

}

LinuxWindow::~LinuxWindow()
{
 #if 0
   delete m_eventSource;
    destroyGLContext();
    XDestroyWindow(m_display, m_window);
    XCloseDisplay(m_display);
#endif
}

WKSize LinuxWindow::size() const
{
    return WKSizeMake(m_width, m_height);
}


void LinuxWindow::makeCurrent(){
    //fprintf(stderr,"LinuxWindow::makeCurrent\n");
    EXPECT_TRUE(eglMakeCurrent(mEglDisplay, mEglSurface, mEglSurface, mEglContext));
    ASSERT_EQ(EGL_SUCCESS, eglGetError());
  
}
void LinuxWindow::swapBuffers(){
    eglSwapBuffers(mEglDisplay, mEglSurface);
}

void LinuxWindow::updateSizeIfNeeded(int width, int height)
{
    if (width == m_width && height == m_height)
        return;

    fprintf(stderr,"old w,h=(%d,%d) NEW w,h=(%d,%d)\n",
            m_width,
            m_height,
            width,
            height);
    m_width = width;
    m_height = height;

    if (!m_client)
        return;

    m_client->handleSizeChanged(width, height);
}
