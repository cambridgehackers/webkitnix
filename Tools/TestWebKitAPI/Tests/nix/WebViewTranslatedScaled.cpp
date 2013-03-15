#include "config.h"

#include "PageLoader.h"
#include "GLUtilities.h"
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

TEST(WebKitNix, WebViewTranslatedScaled)
{
    // This test opens a webpage that contains a white background, no viewport
    // metatag and a red rectangle (20x20)px at (0,0) position. The viewport is
    // then translated to (10,10) position. After that it's applied a scale=2.0
    // At this point we will have a red rectangle of (40x40)px at (10,10).

    const WKSize size = WKSizeMake(100, 100);
    Util::GLOffscreenBuffer offscreenBuffer(size.width, size.height);
    ASSERT_TRUE(offscreenBuffer.makeCurrent());

    WKRetainPtr<WKContextRef> context = adoptWK(WKContextCreate());

    Util::ForceRepaintClient client;
    std::auto_ptr<Nix::WebView> webView(Nix::WebView::create(context.get(), 0, &client));

    cairo_matrix_t transform;
    const int translationDelta = 10;
    cairo_matrix_init_translate(&transform, translationDelta, translationDelta);
    webView->setUserViewportTransformation(transform);

    client.setView(webView.get());
    client.setClearColor(0, 0, 1, 1);
    webView->initialize();
    WKPageSetUseFixedLayout(webView->pageRef(), true);
    webView->setSize(size);

    glViewport(0, 0, size.width, size.height);
    glClearColor(0, 0, 1, 1);
    glClear(GL_COLOR_BUFFER_BIT);

    Util::PageLoader loader(webView.get());

    loader.waitForLoadURLAndRepaint("../nix/red-square");

    // Note that glReadPixels [0, 0] is at the bottom-left of the buffer.
    unsigned char sample[4 * int(size.width * size.height)];
    for (double scale = 1.0; scale < 3.0; scale++) {
        webView->setScale(scale);
        loader.forceRepaint();
        glReadPixels(0, 0, size.width, size.height, GL_RGBA, GL_UNSIGNED_BYTE, &sample);

        // (Left x Top) must be RED
        int x = translationDelta;
        int y = size.height - translationDelta - 1;
        int index = 4 * (y * size.height + x);
        EXPECT_EQ(0xFF, sample[index]) << "Error when checking RED for pixel (" << x << ", " << y << ")";
        EXPECT_EQ(0x00, sample[index + 1]) << "Error when checking GREEN for pixel (" << x << ", " << y << ")";
        EXPECT_EQ(0x00, sample[index + 2]) << "Error when checking BLUE for pixel (" << x << ", " << y << ")";
        EXPECT_EQ(0xFF, sample[index + 3]) << "Error when checking ALPHA for pixel (" << x << ", " << y << ")";

        // (Right x Bottom) must be RED
        x += (20 * scale - 1);
        y -= (20 * scale - 1);
        index = 4 * (y * size.height + x);
        EXPECT_EQ(0xFF, sample[index]) << "Error when checking RED for pixel (" << x << ", " << y << ")";
        EXPECT_EQ(0x00, sample[index + 1]) << "Error when checking GREEN for pixel (" << x << ", " << y << ")";
        EXPECT_EQ(0x00, sample[index + 2]) << "Error when checking BLUE for pixel (" << x << ", " << y << ")";
        EXPECT_EQ(0xFF, sample[index + 3]) << "Error when checking ALPHA for pixel (" << x << ", " << y << ")";

        // (Right x Bottom) + (1,1) must be WHITE
        x += 1;
        y -= 1;
        index = 4 * (y * size.height + x);
        EXPECT_EQ(0xFF, sample[index]) << "Error when checking RED for pixel (" << x << ", " << y << ")";
        EXPECT_EQ(0xFF, sample[index + 1]) << "Error when checking GREEN for pixel (" << x << ", " << y << ")";
        EXPECT_EQ(0xFF, sample[index + 2]) << "Error when checking BLUE for pixel (" << x << ", " << y << ")";
        EXPECT_EQ(0xFF, sample[index + 3]) << "Error when checking ALPHA for pixel (" << x << ", " << y << ")";
    }

    // FIXME: Leaking memory to avoid bug on WebView destructor or on test
    //        infrastructure destruction that should be fixed ASAP.
    webView.release();
}

} // TestWebKitAPI
