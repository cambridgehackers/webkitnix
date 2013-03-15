#include "config.h"

#include "GLUtilities.h"
#include "PageLoader.h"
#include "PlatformUtilities.h"
#include "WebView.h"
#include "WebKit2/WKContext.h"
#include "WebKit2/WKRetainPtr.h"
#if USE(OPENGL_ES_2)
#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>
#else
#include <GL/gl.h>
#endif

#include <memory>

namespace TestWebKitAPI {

static bool s_didFindZoomableArea;
const WKPoint touchPoint = {13, 18};

class TestWebViewClient : public Util::ForceRepaintClient
{
    void didFindZoomableArea(WKPoint target, WKRect area)
    {
        EXPECT_EQ(target.x, touchPoint.x);
        EXPECT_EQ(target.y, touchPoint.y);
        EXPECT_EQ(area.origin.x, 0);
        EXPECT_EQ(area.origin.y, 0);
        EXPECT_EQ(area.size.width, 57);
        EXPECT_EQ(area.size.height, 43);
        s_didFindZoomableArea = true;
    }
};

TEST(WebKitNix, WebViewFindZoomableArea)
{
    // This test opens a webpage that contains a white background, no viewport
    // metatag and a red rectangle (57x43)px at (0,0) position. After that we call
    // findZoomableArea and it should return the rectangle's size + coordinates
    // from touch point (13,18).

    const WKSize size = WKSizeMake(100, 100);
    Util::GLOffscreenBuffer offscreenBuffer(size.width, size.height);
    ASSERT_TRUE(offscreenBuffer.makeCurrent());

    WKRetainPtr<WKContextRef> context = adoptWK(WKContextCreate());

    TestWebViewClient client;
    std::auto_ptr<Nix::WebView> webView(Nix::WebView::create(context.get(), 0, &client));

    client.setView(webView.get());
    client.setClearColor(0, 0, 1, 1);
    webView->initialize();
    WKPageSetUseFixedLayout(webView->pageRef(), true);
    webView->setSize(size);

    glViewport(0, 0, size.width, size.height);
    glClearColor(0, 0, 1, 1);
    glClear(GL_COLOR_BUFFER_BIT);

    Util::PageLoader loader(webView.get());

    loader.waitForLoadURLAndRepaint("../nix/red-rectangle");

    // Using same touch radius as MockedTouchPoint::MockedTouchPoint()
    WKPoint contentsPoint = WKPointMake(touchPoint.x, touchPoint.y);
    webView->findZoomableAreaForPoint(contentsPoint, 20, 20);

    Util::run(&s_didFindZoomableArea);
}

} // TestWebKitAPI
