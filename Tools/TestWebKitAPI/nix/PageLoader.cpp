#include "config.h"
#include "PageLoader.h"
#include "PlatformUtilities.h"
#include "WebView.h"
#include "WebKit2/WKRetainPtr.h"
#include <cstring>
#if USE(OPENGL_ES_2)
#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>
#else
#include <GL/gl.h>
#endif

using namespace TestWebKitAPI::Util;

PageLoader::PageLoader(Nix::WebView* webView)
    : m_webView(webView)
    , m_didFinishLoadAndRepaint(false)
{
    std::memset(&m_loaderClient, 0, sizeof(m_loaderClient));

    m_loaderClient.didFinishLoadForFrame = didFinishLoadForFrame;
    m_loaderClient.clientInfo = this;
    WKPageSetPageLoaderClient(m_webView->pageRef(), &m_loaderClient);
}

void PageLoader::didForceRepaint(WKErrorRef, void* context)
{
    reinterpret_cast<PageLoader*>(context)->m_didFinishLoadAndRepaint = true;
}

void PageLoader::didFinishLoadForFrame(WKPageRef page, WKFrameRef, WKTypeRef, const void* context)
{
    WKPageForceRepaint(page, const_cast<void*>(context), &PageLoader::didForceRepaint);
}

void PageLoader::waitForLoadURLAndRepaint(const char* resource)
{
    WKRetainPtr<WKURLRef> urlRef = adoptWK(createURLForResource(resource, "html"));
    WKPageLoadURL(m_webView->pageRef(), urlRef.get());
    Util::run(&m_didFinishLoadAndRepaint);
    m_didFinishLoadAndRepaint = false;
}

void PageLoader::forceRepaint()
{
    WKPageForceRepaint(m_webView->pageRef(), this, &PageLoader::didForceRepaint);
    Util::run(&m_didFinishLoadAndRepaint);
    m_didFinishLoadAndRepaint = false;
}

ForceRepaintClient::ForceRepaintClient()
    : m_clearR(0)
    , m_clearG(0)
    , m_clearB(0)
    , m_clearA(0)
{
}

void ForceRepaintClient::setClearColor(int r, int g, int b, int a)
{
    m_clearR = r;
    m_clearG = g;
    m_clearB = b;
    m_clearA = a;
}

void ForceRepaintClient::viewNeedsDisplay(WKRect)
{
    assert(m_webView);
    glClearColor(m_clearR, m_clearG, m_clearB, m_clearA);

    glClear(GL_COLOR_BUFFER_BIT);
    m_webView->paintToCurrentGLContext();
}
