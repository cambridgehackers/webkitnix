#include "config.h"
#include "TextureMapperPNG.h"

#include "TextureMapperGL.h"
#include <GL/gl.h>

namespace WebCore {

TextureMapperPNG::TextureMapperPNG(FloatSize size)
    : TextureMapper(OpenGLMode)
{
    m_surface = cairo_image_surface_create(CAIRO_FORMAT_RGB24, size.width(), size.height());
    m_cairo = cairo_create(m_surface);
}

TextureMapperPNG::~TextureMapperPNG()
{
    String path("/tmp/layer_");
    path.append(String::number(long(this)));
    path.append(".png");
    cairo_surface_write_to_png(m_surface, path.utf8().data());

    cairo_destroy(m_cairo);
    cairo_surface_destroy(m_surface);
}

void TextureMapperPNG::drawTexture(const BitmapTexture& texture, const FloatRect& target, const TransformationMatrix& modelViewMatrix, float opacity, unsigned exposedEdges)
{
    if (!texture.isValid())
        return;

    const BitmapTextureGL& textureGL = static_cast<const BitmapTextureGL&>(texture);
    GLint textureWidth = textureGL.textureSize().width();
    GLint textureHeight = textureGL.textureSize().height();

    GLubyte* buffer = (GLubyte*)malloc(textureWidth * textureHeight * 4);
    memset(buffer, 0, textureWidth * textureHeight * 4);
    glBindTexture(GL_TEXTURE_2D, textureGL.id());
    glGetTexImage(GL_TEXTURE_2D, 0, GL_RGBA, GL_UNSIGNED_BYTE, buffer);

    cairo_surface_t* surface = cairo_image_surface_create_for_data(buffer, CAIRO_FORMAT_RGB24, textureWidth, textureHeight, 4*textureWidth);

    cairo_set_source_surface(m_cairo, surface, target.x(), target.y());
    cairo_paint(m_cairo);
    cairo_surface_destroy(surface);
    free(buffer);
}

PassRefPtr<BitmapTexture> TextureMapperPNG::createTexture()
{
    ASSERT_NOT_REACHED();
    return PassRefPtr<BitmapTexture>();
}

IntSize TextureMapperPNG::maxTextureSize() const
{
    ASSERT_NOT_REACHED();
    return IntSize(2000, 2000);
}


}
