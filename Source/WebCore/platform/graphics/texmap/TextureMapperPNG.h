#ifndef TextureMapperPNG_h
#define TextureMapperPNG_h

#include "TextureMapper.h"

#include <cairo/cairo.h>

namespace WebCore {

class TextureMapperPNG : public TextureMapper
{
public:
    TextureMapperPNG(FloatSize);
    ~TextureMapperPNG();

    virtual void drawTexture(const BitmapTexture&, const FloatRect& target, const TransformationMatrix& modelViewMatrix = TransformationMatrix(), float opacity = 1.0f, unsigned exposedEdges = AllEdges);
    virtual PassRefPtr<BitmapTexture> createTexture();
    virtual IntSize maxTextureSize() const;

    virtual void drawBorder(const Color&, float, const FloatRect&, const TransformationMatrix&) {}
    virtual void drawNumber(int, const Color&, const FloatPoint&, const TransformationMatrix&) {}
    virtual void drawSolidColor(const FloatRect&, const TransformationMatrix&, const Color&) {}
    virtual void bindSurface(BitmapTexture* surface) {}
    virtual void beginClip(const TransformationMatrix&, const FloatRect&) {}
    virtual void endClip() {}

private:
    cairo_surface_t* m_surface;
    cairo_t* m_cairo;
};

}

#endif
