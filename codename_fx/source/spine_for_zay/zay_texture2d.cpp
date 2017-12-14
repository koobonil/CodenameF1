#include "zay_types.h"
#include "zay_texture2d.h"
#include "zay_image_data.h"
#include "zay_renderer.h"

#include <boss.hpp>

namespace ZAY
{
    Texture2D::Texture2D()
    : _pixelFormat(PixelFormat::RGBA8888)
    , _width(0)
    , _height(0)
    , _id(0)
    , _maxS(0.0)
    , _maxT(0.0)
    {
    }

    Texture2D::~Texture2D()
    {
        releaseGLTexture();
    }

    void Texture2D::releaseGLTexture()
    {
        if (_id)
        {
            glDeleteTextures(1, &_id);
            Renderer::testGL();
            
            _id = 0;
        }
    }

    bool Texture2D::initWithImageData(ImageData* imageData)
    {
        if (imageData)
        {
            return initWithData(imageData->getData(),
                                imageData->getDataLen(),
                                imageData->getRenderFormat(),
                                imageData->getWidth(),
                                imageData->getHeight());
        }
        
        return false;
    }

    bool Texture2D::initWithData(const void *data, int32_t dataLen, PixelFormat pixelFormat, int32_t width, int32_t height)
    {
        if(_pixelFormatInfoTables.find(pixelFormat) == _pixelFormatInfoTables.end())
        {
            assert(0);
            return false;
        }

        const PixelFormatInfo& info = _pixelFormatInfoTables.at(pixelFormat);

        //Set the row align only when mipmapsNum == 1 and the data is uncompressed
        unsigned int bytesPerRow = width * info.bpp / 8;
        
        GLint alignment;
        glGetIntegerv(GL_UNPACK_ALIGNMENT, &alignment);
        if(bytesPerRow % 8 == 0)
            glPixelStorei(GL_UNPACK_ALIGNMENT, 8);
        else if(bytesPerRow % 4 == 0)
            glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
        else if(bytesPerRow % 2 == 0)
            glPixelStorei(GL_UNPACK_ALIGNMENT, 2);
        else glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
        Renderer::testGL();

        if(_id != 0)
        {
            glDeleteTextures(1, &_id);
            Renderer::testGL();
            _id = 0;
        }

        glGenTextures(1, &_id);
        Renderer::testGL();

        glBindTexture(GL_TEXTURE_2D, _id);
        Renderer::testGL();

        #if BOSS_IPHONE
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
            Renderer::testGL();
            glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
            Renderer::testGL();
            glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE );
            Renderer::testGL();
            glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE );
            Renderer::testGL();
            glTexImage2D(GL_TEXTURE_2D, 0, info.internalFormat, width, height, 0, info.format, info.type, data);
            Renderer::testGL();
        #else
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_NEAREST );
            Renderer::testGL();
            glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
            Renderer::testGL();
            glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE );
            Renderer::testGL();
            glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE );
            Renderer::testGL();
            glTexImage2D(GL_TEXTURE_2D, 0, info.internalFormat, width, height, 0, info.format, info.type, data);
            Renderer::testGL();
            BOSS_GL(GenerateMipmap, GL_TEXTURE_2D);
            Renderer::testGL();
        #endif

        _width = width;
        _height = height;
        _pixelFormat = pixelFormat;
        _maxS = 1.0f;
        _maxT = 1.0f;

        glPixelStorei(GL_UNPACK_ALIGNMENT, alignment);
        Renderer::testGL();
        return true;
    }

    bool Texture2D::updateWithData(const void *data, int32_t offsetX, int32_t offsetY, int32_t width, int32_t height)
    {
        if (_id)
        {
            glBindTexture(GL_TEXTURE_2D, _id);
            Renderer::testGL();

            const PixelFormatInfo& info = _pixelFormatInfoTables.at(_pixelFormat);
            glTexSubImage2D(GL_TEXTURE_2D,
                            0,
                            offsetX, offsetY,
                            width, height,
                            info.format,
                            info.type,
                            data);
            Renderer::testGL();
        }
        
        return false;
    }

    void Texture2D::setTexParameters(const TexParams& texParams)
    {
        if (_id)
        {
            glBindTexture(GL_TEXTURE_2D, _id);
            Renderer::testGL();

            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, texParams.minFilter );
            Renderer::testGL();

            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, texParams.magFilter );
            Renderer::testGL();

            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, texParams.wrapS );
            Renderer::testGL();

            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, texParams.wrapT );
            Renderer::testGL();
        }
    }

    const char* Texture2D::getStringForFormat() const
    {
        switch (_pixelFormat)
        {
            case PixelFormat::RGBA8888:
                return "RGBA8888";
                
            case PixelFormat::RGB888:
                return "RGB888";
                
            case PixelFormat::RGB565:
                return "RGB565";
                
            case PixelFormat::RGBA4444:
                return "RGBA4444";
                
            case PixelFormat::RGB5A1:
                return "RGB5A1";
                
            case PixelFormat::AI88:
                return "AI88";
                
            case PixelFormat::A8:
                return "A8";
                
            case PixelFormat::I8:
                return "I8";
                
            default:
                break;
        }
        
        return  nullptr;
    }

    int32_t Texture2D::getBitsPerPixelForFormat() const
    {
        return getBitsPerPixelForFormat(_pixelFormat);
    }

    int32_t Texture2D::getBitsPerPixelForFormat(PixelFormat format) const
    {
        if (format == PixelFormat::NONE)
        {
            return 0;
        }

        return _pixelFormatInfoTables.at(format).bpp;
    }
    
    PixelFormat Texture2D::getPixelFormat() const
    {
        return _pixelFormat;
    }
        
    int32_t Texture2D::getPixelsWide() const
    {
        return _width;
    }
    
    int32_t Texture2D::getPixelsHigh() const
    {
        return _height;
    }

    GLuint Texture2D::getTextureID() const
    {
        return _id;
    }

    GLfloat Texture2D::getMaxS() const
    {
        return _maxS;
    }

    void Texture2D::setMaxS(GLfloat maxS)
    {
        _maxS = maxS;
    }

    GLfloat Texture2D::getMaxT() const
    {
        return _maxT;
    }

    void Texture2D::setMaxT(GLfloat maxT)
    {
        _maxT = maxT;
    }

    const PixelFormatInfoMap& Texture2D::getPixelFormatInfoMap()
    {
        return _pixelFormatInfoTables;
    }

    PixelFormat Texture2D::convertDataToFormat(const unsigned char* data, int32_t dataLen, PixelFormat originFormat, PixelFormat format, unsigned char** outData, int32_t* outDataLen)
    {
        if (format == originFormat)
        {
            *outData = (unsigned char*)data;
            *outDataLen = dataLen;
            return originFormat;
        }

        switch (originFormat)
        {
            case PixelFormat::I8:
                return convertI8ToFormat(data, dataLen, format, outData, outDataLen);
            case PixelFormat::AI88:
                return convertAI88ToFormat(data, dataLen, format, outData, outDataLen);
            case PixelFormat::RGB888:
                return convertRGB888ToFormat(data, dataLen, format, outData, outDataLen);
            case PixelFormat::RGBA8888:
                return convertRGBA8888ToFormat(data, dataLen, format, outData, outDataLen);
            default:
                assert(0);
                *outData = (unsigned char*)data;
                *outDataLen = dataLen;
                return originFormat;
        }
    }

    PixelFormat Texture2D::convertI8ToFormat(const unsigned char* data, int32_t dataLen, PixelFormat format, unsigned char** outData, int32_t* outDataLen)
    {
        switch (format)
        {
            case PixelFormat::RGBA8888:
                *outDataLen = dataLen*4;
                *outData = (unsigned char*)malloc(sizeof(unsigned char) * (*outDataLen));
                convertI8ToRGBA8888(data, dataLen, *outData);
                break;
            case PixelFormat::RGB888:
                *outDataLen = dataLen*3;
                *outData = (unsigned char*)malloc(sizeof(unsigned char) * (*outDataLen));
                convertI8ToRGB888(data, dataLen, *outData);
                break;
            case PixelFormat::RGB565:
                *outDataLen = dataLen*2;
                *outData = (unsigned char*)malloc(sizeof(unsigned char) * (*outDataLen));
                convertI8ToRGB565(data, dataLen, *outData);
                break;
            case PixelFormat::AI88:
                *outDataLen = dataLen*2;
                *outData = (unsigned char*)malloc(sizeof(unsigned char) * (*outDataLen));
                convertI8ToAI88(data, dataLen, *outData);
                break;
            case PixelFormat::RGBA4444:
                *outDataLen = dataLen*2;
                *outData = (unsigned char*)malloc(sizeof(unsigned char) * (*outDataLen));
                convertI8ToRGBA4444(data, dataLen, *outData);
                break;
            case PixelFormat::RGB5A1:
                *outDataLen = dataLen*2;
                *outData = (unsigned char*)malloc(sizeof(unsigned char) * (*outDataLen));
                convertI8ToRGB5A1(data, dataLen, *outData);
                break;
            default:
                assert(0);
                *outData = (unsigned char*)data;
                *outDataLen = dataLen;
                return PixelFormat::I8;
        }
        
        return format;
    }
    
    PixelFormat Texture2D::convertAI88ToFormat(const unsigned char* data, int32_t dataLen, PixelFormat format, unsigned char** outData, int32_t* outDataLen)
    {
        switch (format)
        {
            case PixelFormat::RGBA8888:
                *outDataLen = dataLen*2;
                *outData = (unsigned char*)malloc(sizeof(unsigned char) * (*outDataLen));
                convertAI88ToRGBA8888(data, dataLen, *outData);
                break;
            case PixelFormat::RGB888:
                *outDataLen = dataLen/2*3;
                *outData = (unsigned char*)malloc(sizeof(unsigned char) * (*outDataLen));
                convertAI88ToRGB888(data, dataLen, *outData);
                break;
            case PixelFormat::RGB565:
                *outDataLen = dataLen;
                *outData = (unsigned char*)malloc(sizeof(unsigned char) * (*outDataLen));
                convertAI88ToRGB565(data, dataLen, *outData);
                break;
            case PixelFormat::A8:
                *outDataLen = dataLen/2;
                *outData = (unsigned char*)malloc(sizeof(unsigned char) * (*outDataLen));
                convertAI88ToA8(data, dataLen, *outData);
                break;
            case PixelFormat::I8:
                *outDataLen = dataLen/2;
                *outData = (unsigned char*)malloc(sizeof(unsigned char) * (*outDataLen));
                convertAI88ToI8(data, dataLen, *outData);
                break;
            case PixelFormat::RGBA4444:
                *outDataLen = dataLen;
                *outData = (unsigned char*)malloc(sizeof(unsigned char) * (*outDataLen));
                convertAI88ToRGBA4444(data, dataLen, *outData);
                break;
            case PixelFormat::RGB5A1:
                *outDataLen = dataLen;
                *outData = (unsigned char*)malloc(sizeof(unsigned char) * (*outDataLen));
                convertAI88ToRGB5A1(data, dataLen, *outData);
                break;
            default:
                assert(0);
                *outData = (unsigned char*)data;
                *outDataLen = dataLen;
                return PixelFormat::AI88;
                break;
        }
        
        return format;
    }
    
    PixelFormat Texture2D::convertRGB888ToFormat(const unsigned char* data, int32_t dataLen, PixelFormat format, unsigned char** outData, int32_t* outDataLen)
    {
        switch (format)
        {
            case PixelFormat::RGBA8888:
                *outDataLen = dataLen/3*4;
                *outData = (unsigned char*)malloc(sizeof(unsigned char) * (*outDataLen));
                convertRGB888ToRGBA8888(data, dataLen, *outData);
                break;
            case PixelFormat::RGB565:
                *outDataLen = dataLen/3*2;
                *outData = (unsigned char*)malloc(sizeof(unsigned char) * (*outDataLen));
                convertRGB888ToRGB565(data, dataLen, *outData);
                break;
            case PixelFormat::I8:
                *outDataLen = dataLen/3;
                *outData = (unsigned char*)malloc(sizeof(unsigned char) * (*outDataLen));
                convertRGB888ToI8(data, dataLen, *outData);
                break;
            case PixelFormat::AI88:
                *outDataLen = dataLen/3*2;
                *outData = (unsigned char*)malloc(sizeof(unsigned char) * (*outDataLen));
                convertRGB888ToAI88(data, dataLen, *outData);
                break;
            case PixelFormat::RGBA4444:
                *outDataLen = dataLen/3*2;
                *outData = (unsigned char*)malloc(sizeof(unsigned char) * (*outDataLen));
                convertRGB888ToRGBA4444(data, dataLen, *outData);
                break;
            case PixelFormat::RGB5A1:
                *outDataLen = dataLen;
                *outData = (unsigned char*)malloc(sizeof(unsigned char) * (*outDataLen));
                convertRGB888ToRGB5A1(data, dataLen, *outData);
                break;
            default:
                assert(0);
                *outData = (unsigned char*)data;
                *outDataLen = dataLen;
                return PixelFormat::RGB888;
        }
        return format;
    }
    
    PixelFormat Texture2D::convertRGBA8888ToFormat(const unsigned char* data, int32_t dataLen, PixelFormat format, unsigned char** outData, int32_t* outDataLen)
    {
        switch (format)
        {
            case PixelFormat::RGB888:
                *outDataLen = dataLen/4*3;
                *outData = (unsigned char*)malloc(sizeof(unsigned char) * (*outDataLen));
                convertRGBA8888ToRGB888(data, dataLen, *outData);
                break;
            case PixelFormat::RGB565:
                *outDataLen = dataLen/2;
                *outData = (unsigned char*)malloc(sizeof(unsigned char) * (*outDataLen));
                convertRGBA8888ToRGB565(data, dataLen, *outData);
                break;
            case PixelFormat::A8:
                *outDataLen = dataLen/4;
                *outData = (unsigned char*)malloc(sizeof(unsigned char) * (*outDataLen));
                convertRGBA8888ToA8(data, dataLen, *outData);
                break;
            case PixelFormat::I8:
                *outDataLen = dataLen/4;
                *outData = (unsigned char*)malloc(sizeof(unsigned char) * (*outDataLen));
                convertRGBA8888ToI8(data, dataLen, *outData);
                break;
            case PixelFormat::AI88:
                *outDataLen = dataLen/2;
                *outData = (unsigned char*)malloc(sizeof(unsigned char) * (*outDataLen));
                convertRGBA8888ToAI88(data, dataLen, *outData);
                break;
            case PixelFormat::RGBA4444:
                *outDataLen = dataLen/2;
                *outData = (unsigned char*)malloc(sizeof(unsigned char) * (*outDataLen));
                convertRGBA8888ToRGBA4444(data, dataLen, *outData);
                break;
            case PixelFormat::RGB5A1:
                *outDataLen = dataLen/2;
                *outData = (unsigned char*)malloc(sizeof(unsigned char) * (*outDataLen));
                convertRGBA8888ToRGB5A1(data, dataLen, *outData);
                break;
            default:
                assert(0);
                *outData = (unsigned char*)data;
                *outDataLen = dataLen;
                return PixelFormat::RGBA8888;
        }
        
        return format;
    }

    //////////////////////////////////////////////////////////////////////////
    //conventer function
    
    // IIIIIIII -> RRRRRRRRGGGGGGGGGBBBBBBBB
    void Texture2D::convertI8ToRGB888(const unsigned char* data, int32_t dataLen, unsigned char* outData)
    {
        for (int32_t i=0; i < dataLen; ++i)
        {
            *outData++ = data[i];     //R
            *outData++ = data[i];     //G
            *outData++ = data[i];     //B
        }
    }
    
    // IIIIIIIIAAAAAAAA -> RRRRRRRRGGGGGGGGBBBBBBBB
    void Texture2D::convertAI88ToRGB888(const unsigned char* data, int32_t dataLen, unsigned char* outData)
    {
        for (int32_t i = 0, l = dataLen - 1; i < l; i += 2)
        {
            *outData++ = data[i];     //R
            *outData++ = data[i];     //G
            *outData++ = data[i];     //B
        }
    }
    
    // IIIIIIII -> RRRRRRRRGGGGGGGGGBBBBBBBBAAAAAAAA
    void Texture2D::convertI8ToRGBA8888(const unsigned char* data, int32_t dataLen, unsigned char* outData)
    {
        for (int32_t i = 0; i < dataLen; ++i)
        {
            *outData++ = data[i];     //R
            *outData++ = data[i];     //G
            *outData++ = data[i];     //B
            *outData++ = 0xFF;        //A
        }
    }
    
    // IIIIIIIIAAAAAAAA -> RRRRRRRRGGGGGGGGBBBBBBBBAAAAAAAA
    void Texture2D::convertAI88ToRGBA8888(const unsigned char* data, int32_t dataLen, unsigned char* outData)
    {
        for (int32_t i = 0, l = dataLen - 1; i < l; i += 2)
        {
            *outData++ = data[i];     //R
            *outData++ = data[i];     //G
            *outData++ = data[i];     //B
            *outData++ = data[i + 1]; //A
        }
    }
    
    // IIIIIIII -> RRRRRGGGGGGBBBBB
    void Texture2D::convertI8ToRGB565(const unsigned char* data, int32_t dataLen, unsigned char* outData)
    {
        unsigned short* out16 = (unsigned short*)outData;
        for (int32_t i = 0; i < dataLen; ++i)
        {
            *out16++ = (data[i] & 0x00F8) << 8    //R
            | (data[i] & 0x00FC) << 3         //G
            | (data[i] & 0x00F8) >> 3;        //B
        }
    }
    
    // IIIIIIIIAAAAAAAA -> RRRRRGGGGGGBBBBB
    void Texture2D::convertAI88ToRGB565(const unsigned char* data, int32_t dataLen, unsigned char* outData)
    {
        unsigned short* out16 = (unsigned short*)outData;
        for (int32_t i = 0, l = dataLen - 1; i < l; i += 2)
        {
            *out16++ = (data[i] & 0x00F8) << 8    //R
            | (data[i] & 0x00FC) << 3         //G
            | (data[i] & 0x00F8) >> 3;        //B
        }
    }
    
    // IIIIIIII -> RRRRGGGGBBBBAAAA
    void Texture2D::convertI8ToRGBA4444(const unsigned char* data, int32_t dataLen, unsigned char* outData)
    {
        unsigned short* out16 = (unsigned short*)outData;
        for (int32_t i = 0; i < dataLen; ++i)
        {
            *out16++ = (data[i] & 0x00F0) << 8    //R
            | (data[i] & 0x00F0) << 4             //G
            | (data[i] & 0x00F0)                  //B
            | 0x000F;                             //A
        }
    }
    
    // IIIIIIIIAAAAAAAA -> RRRRGGGGBBBBAAAA
    void Texture2D::convertAI88ToRGBA4444(const unsigned char* data, int32_t dataLen, unsigned char* outData)
    {
        unsigned short* out16 = (unsigned short*)outData;
        for (int32_t i = 0, l = dataLen - 1; i < l; i += 2)
        {
            *out16++ = (data[i] & 0x00F0) << 8    //R
            | (data[i] & 0x00F0) << 4             //G
            | (data[i] & 0x00F0)                  //B
            | (data[i+1] & 0x00F0) >> 4;          //A
        }
    }
    
    // IIIIIIII -> RRRRRGGGGGBBBBBA
    void Texture2D::convertI8ToRGB5A1(const unsigned char* data, int32_t dataLen, unsigned char* outData)
    {
        unsigned short* out16 = (unsigned short*)outData;
        for (int32_t i = 0; i < dataLen; ++i)
        {
            *out16++ = (data[i] & 0x00F8) << 8    //R
            | (data[i] & 0x00F8) << 3         //G
            | (data[i] & 0x00F8) >> 2         //B
            | 0x0001;                         //A
        }
    }
    
    // IIIIIIIIAAAAAAAA -> RRRRRGGGGGBBBBBA
    void Texture2D::convertAI88ToRGB5A1(const unsigned char* data, int32_t dataLen, unsigned char* outData)
    {
        unsigned short* out16 = (unsigned short*)outData;
        for (int32_t i = 0, l = dataLen - 1; i < l; i += 2)
        {
            *out16++ = (data[i] & 0x00F8) << 8    //R
            | (data[i] & 0x00F8) << 3         //G
            | (data[i] & 0x00F8) >> 2         //B
            | (data[i + 1] & 0x0080) >> 7;    //A
        }
    }
    
    // IIIIIIII -> IIIIIIIIAAAAAAAA
    void Texture2D::convertI8ToAI88(const unsigned char* data, int32_t dataLen, unsigned char* outData)
    {
        unsigned short* out16 = (unsigned short*)outData;
        for (int32_t i = 0; i < dataLen; ++i)
        {
            *out16++ = 0xFF00     //A
            | data[i];            //I
        }
    }
    
    // IIIIIIIIAAAAAAAA -> AAAAAAAA
    void Texture2D::convertAI88ToA8(const unsigned char* data, int32_t dataLen, unsigned char* outData)
    {
        for (int32_t i = 1; i < dataLen; i += 2)
        {
            *outData++ = data[i]; //A
        }
    }
    
    // IIIIIIIIAAAAAAAA -> IIIIIIII
    void Texture2D::convertAI88ToI8(const unsigned char* data, int32_t dataLen, unsigned char* outData)
    {
        for (int32_t i = 0, l = dataLen - 1; i < l; i += 2)
        {
            *outData++ = data[i]; //R
        }
    }
    
    // RRRRRRRRGGGGGGGGBBBBBBBB -> RRRRRRRRGGGGGGGGBBBBBBBBAAAAAAAA
    void Texture2D::convertRGB888ToRGBA8888(const unsigned char* data, int32_t dataLen, unsigned char* outData)
    {
        for (int32_t i = 0, l = dataLen - 2; i < l; i += 3)
        {
            *outData++ = data[i];         //R
            *outData++ = data[i + 1];     //G
            *outData++ = data[i + 2];     //B
            *outData++ = 0xFF;            //A
        }
    }
    
    // RRRRRRRRGGGGGGGGBBBBBBBBAAAAAAAA -> RRRRRRRRGGGGGGGGBBBBBBBB
    void Texture2D::convertRGBA8888ToRGB888(const unsigned char* data, int32_t dataLen, unsigned char* outData)
    {
        for (int32_t i = 0, l = dataLen - 3; i < l; i += 4)
        {
            *outData++ = data[i];         //R
            *outData++ = data[i + 1];     //G
            *outData++ = data[i + 2];     //B
        }
    }
    
    // RRRRRRRRGGGGGGGGBBBBBBBB -> RRRRRGGGGGGBBBBB
    void Texture2D::convertRGB888ToRGB565(const unsigned char* data, int32_t dataLen, unsigned char* outData)
    {
        unsigned short* out16 = (unsigned short*)outData;
        for (int32_t i = 0, l = dataLen - 2; i < l; i += 3)
        {
            *out16++ = (data[i] & 0x00F8) << 8    //R
            | (data[i + 1] & 0x00FC) << 3     //G
            | (data[i + 2] & 0x00F8) >> 3;    //B
        }
    }
    
    // RRRRRRRRGGGGGGGGBBBBBBBBAAAAAAAA -> RRRRRGGGGGGBBBBB
    void Texture2D::convertRGBA8888ToRGB565(const unsigned char* data, int32_t dataLen, unsigned char* outData)
    {
        unsigned short* out16 = (unsigned short*)outData;
        for (int32_t i = 0, l = dataLen - 3; i < l; i += 4)
        {
            *out16++ = (data[i] & 0x00F8) << 8    //R
            | (data[i + 1] & 0x00FC) << 3     //G
            | (data[i + 2] & 0x00F8) >> 3;    //B
        }
    }
    
    // RRRRRRRRGGGGGGGGBBBBBBBB -> IIIIIIII
    void Texture2D::convertRGB888ToI8(const unsigned char* data, int32_t dataLen, unsigned char* outData)
    {
        for (int32_t i = 0, l = dataLen - 2; i < l; i += 3)
        {
            *outData++ = (data[i] * 299 + data[i + 1] * 587 + data[i + 2] * 114 + 500) / 1000;  //I =  (R*299 + G*587 + B*114 + 500) / 1000
        }
    }
    
    // RRRRRRRRGGGGGGGGBBBBBBBBAAAAAAAA -> IIIIIIII
    void Texture2D::convertRGBA8888ToI8(const unsigned char* data, int32_t dataLen, unsigned char* outData)
    {
        for (int32_t i = 0, l = dataLen - 3; i < l; i += 4)
        {
            *outData++ = (data[i] * 299 + data[i + 1] * 587 + data[i + 2] * 114 + 500) / 1000;  //I =  (R*299 + G*587 + B*114 + 500) / 1000
        }
    }
    
    // RRRRRRRRGGGGGGGGBBBBBBBBAAAAAAAA -> AAAAAAAA
    void Texture2D::convertRGBA8888ToA8(const unsigned char* data, int32_t dataLen, unsigned char* outData)
    {
        for (int32_t i = 0, l = dataLen -3; i < l; i += 4)
        {
            *outData++ = data[i + 3]; //A
        }
    }
    
    // RRRRRRRRGGGGGGGGBBBBBBBB -> IIIIIIIIAAAAAAAA
    void Texture2D::convertRGB888ToAI88(const unsigned char* data, int32_t dataLen, unsigned char* outData)
    {
        for (int32_t i = 0, l = dataLen - 2; i < l; i += 3)
        {
            *outData++ = (data[i] * 299 + data[i + 1] * 587 + data[i + 2] * 114 + 500) / 1000;  //I =  (R*299 + G*587 + B*114 + 500) / 1000
            *outData++ = 0xFF;
        }
    }
    
    
    // RRRRRRRRGGGGGGGGBBBBBBBBAAAAAAAA -> IIIIIIIIAAAAAAAA
    void Texture2D::convertRGBA8888ToAI88(const unsigned char* data, int32_t dataLen, unsigned char* outData)
    {
        for (int32_t i = 0, l = dataLen - 3; i < l; i += 4)
        {
            *outData++ = (data[i] * 299 + data[i + 1] * 587 + data[i + 2] * 114 + 500) / 1000;  //I =  (R*299 + G*587 + B*114 + 500) / 1000
            *outData++ = data[i + 3];
        }
    }
    
    // RRRRRRRRGGGGGGGGBBBBBBBB -> RRRRGGGGBBBBAAAA
    void Texture2D::convertRGB888ToRGBA4444(const unsigned char* data, int32_t dataLen, unsigned char* outData)
    {
        unsigned short* out16 = (unsigned short*)outData;
        for (int32_t i = 0, l = dataLen - 2; i < l; i += 3)
        {
            *out16++ = ((data[i] & 0x00F0) << 8           //R
                        | (data[i + 1] & 0x00F0) << 4     //G
                        | (data[i + 2] & 0xF0)            //B
                        |  0x0F);                         //A
        }
    }
    
    // RRRRRRRRGGGGGGGGBBBBBBBBAAAAAAAA -> RRRRGGGGBBBBAAAA
    void Texture2D::convertRGBA8888ToRGBA4444(const unsigned char* data, int32_t dataLen, unsigned char* outData)
    {
        unsigned short* out16 = (unsigned short*)outData;
        for (int32_t i = 0, l = dataLen - 3; i < l; i += 4)
        {
            *out16++ = (data[i] & 0x00F0) << 8    //R
            | (data[i + 1] & 0x00F0) << 4         //G
            | (data[i + 2] & 0xF0)                //B
            |  (data[i + 3] & 0xF0) >> 4;         //A
        }
    }
    
    // RRRRRRRRGGGGGGGGBBBBBBBB -> RRRRRGGGGGBBBBBA
    void Texture2D::convertRGB888ToRGB5A1(const unsigned char* data, int32_t dataLen, unsigned char* outData)
    {
        unsigned short* out16 = (unsigned short*)outData;
        for (int32_t i = 0, l = dataLen - 2; i < l; i += 3)
        {
            *out16++ = (data[i] & 0x00F8) << 8    //R
            | (data[i + 1] & 0x00F8) << 3     //G
            | (data[i + 2] & 0x00F8) >> 2     //B
            |  0x01;                          //A
        }
    }
    
    // RRRRRRRRGGGGGGGGBBBBBBBB -> RRRRRGGGGGBBBBBA
    void Texture2D::convertRGBA8888ToRGB5A1(const unsigned char* data, int32_t dataLen, unsigned char* outData)
    {
        unsigned short* out16 = (unsigned short*)outData;
        for (int32_t i = 0, l = dataLen - 2; i < l; i += 4)
        {
            *out16++ = (data[i] & 0x00F8) << 8    //R
            | (data[i + 1] & 0x00F8) << 3     //G
            | (data[i + 2] & 0x00F8) >> 2     //B
            |  (data[i + 3] & 0x0080) >> 7;   //A
        }
    }

    const PixelFormatInfoMap Texture2D::_pixelFormatInfoTables =
    {
        {PixelFormat::RGBA8888,     PixelFormatInfo(GL_RGBA, GL_RGBA, GL_UNSIGNED_BYTE, 32, false, true)},
        {PixelFormat::RGBA4444,     PixelFormatInfo(GL_RGBA, GL_RGBA, GL_UNSIGNED_SHORT_4_4_4_4, 16, false, true)},
        {PixelFormat::RGB5A1,       PixelFormatInfo(GL_RGBA, GL_RGBA, GL_UNSIGNED_SHORT_5_5_5_1, 16, false, true)},
        {PixelFormat::RGB565,       PixelFormatInfo(GL_RGB, GL_RGB, GL_UNSIGNED_SHORT_5_6_5, 16, false, false)},
        {PixelFormat::RGB888,       PixelFormatInfo(GL_RGB, GL_RGB, GL_UNSIGNED_BYTE, 24, false, false)},
        {PixelFormat::A8,           PixelFormatInfo(GL_ALPHA, GL_ALPHA, GL_UNSIGNED_BYTE, 8, false, false)},
        {PixelFormat::I8,           PixelFormatInfo(GL_RED, GL_RED, GL_UNSIGNED_BYTE, 8, false, false)},
        {PixelFormat::AI88,         PixelFormatInfo(GL_RG, GL_RG, GL_UNSIGNED_BYTE, 16, false, true)},
    };
}


