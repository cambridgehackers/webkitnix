/*
 * Copyright (C) 2012-2013 Nokia Corporation and/or its subsidiary(-ies).
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS ``AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT HOLDERS OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
 * OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "WebView.h"

#include <GL/glx.h>

struct WebView::GLContextData {
    XVisualInfo* visualInfo;
    GLXContext context;
};

VisualID WebView::setupXVisualID()
{
    m_glContextData = new GLContextData;

    GLint attributes[] = {
        GLX_RGBA,
        GLX_DEPTH_SIZE, 24,
        GLX_DOUBLEBUFFER,
        None
    };
    m_glContextData->visualInfo = glXChooseVisual(m_display, 0, attributes);

    if (!m_glContextData->visualInfo)
        fatalError("glXChooseVisual() failed.");

    return m_glContextData->visualInfo->visualid;
}

void WebView::createGLContext()
{

    m_glContextData->context = glXCreateContext(m_display, m_glContextData->visualInfo, 0, GL_TRUE);
    if (!m_glContextData->context)
        fatalError("glXCreateContext() failed.");
}

void WebView::destroyGLContext()
{
    glXMakeCurrent(m_display, None, 0);
    glXDestroyContext(m_display, m_glContextData->context);
    XFree(m_glContextData->visualInfo);
    delete m_glContextData;
}

void WebView::makeCurrent()
{
    glXMakeCurrent(m_display, m_window, m_glContextData->context);
}

void WebView::swapBuffers()
{
    glXSwapBuffers(m_display, m_window);
}
