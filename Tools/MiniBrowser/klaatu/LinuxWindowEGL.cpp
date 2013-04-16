#ifndef KLAATU
#include "LinuxWindow.h"

#include <EGL/egl.h>
#include <GL/gl.h>

struct LinuxWindow::GLContextData {
    EGLDisplay eglDisplay;
    EGLSurface surface;
    EGLContext context;
    EGLConfig config;
};

VisualID LinuxWindow::setupXVisualID()
{
    m_glContextData = new GLContextData;

    m_glContextData->eglDisplay = eglGetDisplay(m_display);
    if (!m_glContextData->eglDisplay)
        fatalError("eglGetDisplay() failed\n");

    if (!eglInitialize(m_glContextData->eglDisplay, 0, 0))
        fatalError("eglInitialize() failed\n");

    static const EGLint attributes[] = {
        EGL_DEPTH_SIZE, 24,
        EGL_RENDERABLE_TYPE, EGL_OPENGL_BIT,
        EGL_NONE
    };

    EGLint configCount;
    if (!eglChooseConfig(m_glContextData->eglDisplay, attributes, &m_glContextData->config, 1, &configCount))
        fatalError("eglChooseConfig() failed\n");

    EGLint visualID;
    if (!eglGetConfigAttrib(m_glContextData->eglDisplay, m_glContextData->config, EGL_NATIVE_VISUAL_ID, &visualID))
        fatalError("couldn't get the native visual ID\n");

    return visualID;
}

void LinuxWindow::createGLContext()
{
    eglBindAPI(EGL_OPENGL_API);
    m_glContextData->context = eglCreateContext(m_glContextData->eglDisplay, m_glContextData->config, EGL_NO_CONTEXT, 0);
    if (!m_glContextData->context)
        fatalError("eglCreateContext failed\n");

    m_glContextData->surface = eglCreateWindowSurface(m_glContextData->eglDisplay, m_glContextData->config, m_window, 0);
}

void LinuxWindow::destroyGLContext()
{
    eglMakeCurrent(m_glContextData->eglDisplay, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
    eglDestroyContext(m_glContextData->eglDisplay, m_glContextData->context);
    eglDestroySurface(m_glContextData->eglDisplay, m_glContextData->surface);
    eglTerminate(m_glContextData->eglDisplay);

    delete m_glContextData;
}

void LinuxWindow::makeCurrent()
{
    eglMakeCurrent(m_glContextData->eglDisplay, m_glContextData->surface, m_glContextData->surface, m_glContextData->context);
}

void LinuxWindow::swapBuffers()
{
    eglSwapBuffers(m_glContextData->eglDisplay, m_glContextData->surface);
}
#endif
