/*
 * Copyright (C) 2007 Apple Computer, Inc.  All rights reserved.
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
 * THIS SOFTWARE IS PROVIDED BY APPLE COMPUTER, INC. ``AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL APPLE COMPUTER, INC. OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
 * OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "config.h"
#include "CSSTimingFunctionValue.h"

#include <wtf/text/WTFString.h>

namespace WebCore {

String CSSLinearTimingFunctionValue::customCssText() const
{
    return "linear";
}


String CSSCubicBezierTimingFunctionValue::customCssText() const
{
    return "cubic-bezier("
        + String::number(m_x1) + ", "
        + String::number(m_y1) + ", "
        + String::number(m_x2) + ", "
        + String::number(m_y2) + ")";
}

bool CSSCubicBezierTimingFunctionValue::equals(const CSSCubicBezierTimingFunctionValue& other) const
{
    return m_x1 == other.m_x1 && m_x2 == other.m_x2 && m_y1 == other.m_y1 && m_y2 == other.m_y2;
}


String CSSStepsTimingFunctionValue::customCssText() const
{
    return "steps(" + String::number(m_steps) + ", " + (m_stepAtStart ? "start" : "end") + ')';
}

bool CSSStepsTimingFunctionValue::equals(const CSSStepsTimingFunctionValue& other) const
{
    return m_steps == other.m_steps && m_stepAtStart == other.m_stepAtStart;
}

} // namespace WebCore
