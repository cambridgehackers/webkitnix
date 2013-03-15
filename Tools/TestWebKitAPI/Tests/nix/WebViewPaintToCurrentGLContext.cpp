#include "config.h"

#include "GLUtilities.h"
#include "PageLoader.h"
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

TEST(WebKitNix, WebViewPaintToCurrentGLContext)
{
    const WKSize size = WKSizeMake(100, 100);
    Util::GLOffscreenBuffer offscreenBuffer(size.width, size.height);
    ASSERT_TRUE(offscreenBuffer.makeCurrent());

    WKRetainPtr<WKContextRef> context = adoptWK(WKContextCreate());
    Util::ForceRepaintClient client;
    std::auto_ptr<Nix::WebView> webView(Nix::WebView::create(context.get(), 0, &client));
    client.setView(webView.get());
    client.setClearColor(0, 0, 1, 1);
    webView->initialize();
    WKPageSetUseFixedLayout(webView->pageRef(), true);
    webView->setSize(size);
    Util::PageLoader loader(webView.get());

    glViewport(0, 0, size.width, size.height);
    glClearColor(0, 0, 1, 1);
    glClear(GL_COLOR_BUFFER_BIT);

    unsigned char clearedSample[4];
    glReadPixels(0, 1, 1, 1, GL_RGBA, GL_UNSIGNED_BYTE, &clearedSample);
    EXPECT_EQ(0x00, clearedSample[0]);
    EXPECT_EQ(0x00, clearedSample[1]);
    EXPECT_EQ(0xFF, clearedSample[2]);
    EXPECT_EQ(0xFF, clearedSample[3]);

    glClear(GL_COLOR_BUFFER_BIT);
    loader.waitForLoadURLAndRepaint("../nix/red-background");

    unsigned char redSample[4];
    glReadPixels(0, 1, 1, 1, GL_RGBA, GL_UNSIGNED_BYTE, &redSample);
    EXPECT_EQ(0xFF, redSample[0]);
    EXPECT_EQ(0x00, redSample[1]);
    EXPECT_EQ(0x00, redSample[2]);
    EXPECT_EQ(0xFF, redSample[3]);

    glClear(GL_COLOR_BUFFER_BIT);
    loader.waitForLoadURLAndRepaint("../nix/green-background");

    unsigned char greenSample[4];
    glReadPixels(0, 1, 1, 1, GL_RGBA, GL_UNSIGNED_BYTE, &greenSample);
    EXPECT_EQ(0x00, greenSample[0]);
    EXPECT_EQ(0xFF, greenSample[1]);
    EXPECT_EQ(0x00, greenSample[2]);
    EXPECT_EQ(0xFF, greenSample[3]);
}

} // TestWebKitAPI
