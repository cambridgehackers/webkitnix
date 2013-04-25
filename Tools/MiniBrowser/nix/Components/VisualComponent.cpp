/*
 * Copyright (C) 2012-2013 Nokia Corporation and/or its subsidiary(-ies).
 * Copyright (C) 2013 University of Szeged
 * Copyright (C) 2013 Roland Takacs <rtakacs@inf.u-szeged.hu>
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

#include "VisualComponent.h"

#include <cstdio>
#include <cstdlib>

VisualComponent::VisualComponent(Display* display, BrowserControl* control, WKRect size)
    : m_display(display)
    , m_size(size)
    , m_control(control)
{
}

VisualComponent::~VisualComponent()
{
    XDestroyWindow(m_display, m_window);
}

WKSize VisualComponent::size() const
{
    return WKSizeMake(m_size.size.width, m_size.size.height);
}

void VisualComponent::resize(int width, int height)
{
    if (width == m_size.size.width && height == m_size.size.height)
        return;

    m_size.size.width = width;
    m_size.size.height = height;

    XResizeWindow(m_display, m_window, width, height);
}

int VisualComponent::createXColor(const char* color)
{
    XColor c;
    Colormap colormap = DefaultColormap(m_display, 0);

    XParseColor(m_display, colormap, color, &c);
    XAllocColor(m_display, colormap, &c);

    return c.pixel;
}

void fatalError(const char* message)
{
    fprintf(stderr, "Error: %s\n", message);
    exit(1);
}
