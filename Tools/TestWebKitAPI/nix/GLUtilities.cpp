#include "config.h"

#include "GLUtilities.h"
#include <iostream>
#if USE(OPENGL_ES_2)
#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>
#else
#include <GL/gl.h>
#endif
#include <png.h>

using namespace std;
using namespace TestWebKitAPI::Util;

void GLOffscreenBuffer::dumpToPng(const char* fileName)
{
    FILE* fp = fopen(fileName, "w+");
    if (!fp) {
        cerr << "Error writing " << fileName << endl;
        return;
    }

    png_structp pngPtr = png_create_write_struct(PNG_LIBPNG_VER_STRING, 0, 0, 0);
    png_infop pngInfo = png_create_info_struct(pngPtr);

    if (!pngPtr || !pngInfo) {
        png_destroy_write_struct(&pngPtr, &pngInfo);
        cerr << "Error creating PNG write strucs\n";
        return;
    }

    png_init_io(pngPtr, fp);
    png_set_IHDR(pngPtr, pngInfo, m_width, m_height, 8, PNG_COLOR_TYPE_RGB_ALPHA, PNG_INTERLACE_NONE, PNG_COMPRESSION_TYPE_DEFAULT, PNG_FILTER_TYPE_DEFAULT);
    png_write_info(pngPtr, pngInfo);

    const size_t imageDataSize = m_width * m_height * 4;
    unsigned char* imageData = new unsigned char[imageDataSize];
    glPixelStorei(GL_PACK_ALIGNMENT, 1);
    glReadPixels(0, 0, m_width, m_height, GL_RGBA, GL_UNSIGNED_BYTE, imageData);

    // write each row separately.
    png_bytep row = 0;
    for (int y = 0; y < m_height; ++y) {
        // glReadPixels returns the image up side down, so we write last rows first.
        png_uint_32 q = imageDataSize - (y + 1) * m_width * 4;
        row = (png_bytep) imageData + q;
        png_write_row(pngPtr, row);
    }
    delete[] imageData;

    png_write_end(pngPtr, 0);
    png_destroy_write_struct(&pngPtr, &pngInfo);

    fclose(fp);
}
