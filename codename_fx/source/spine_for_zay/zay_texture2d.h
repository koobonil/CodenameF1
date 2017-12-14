#pragma once

#include "zay_base.h"
#include "zay_opengl.h"

namespace ZAY
{
    class Texture2D
    : public Base
    {
    public:
        Texture2D();
        virtual ~Texture2D();

        void releaseGLTexture();

        bool initWithImageData(ImageData* imageData);
        bool initWithData(const void *data, int32_t dataLen, PixelFormat pixelFormat, int32_t width, int32_t height);
        bool updateWithData(const void *data, int32_t offsetX, int32_t offsetY, int32_t width, int32_t height);

        void setTexParameters(const TexParams& texParams);
        void setTexParameters(const TexParams* texParams)
        {
            return setTexParameters(*texParams);
        }

        void setAntiAliasTexParameters();

        void setAliasTexParameters();

        const char* getStringForFormat() const;

        int32_t getBitsPerPixelForFormat() const;
        
        int32_t getBitsPerPixelForFormat(PixelFormat format) const;

        bool hasPremultipliedAlpha() const;
        bool hasMipmaps() const;

        PixelFormat getPixelFormat() const;
        
        int32_t getPixelsWide() const;
        int32_t getPixelsHigh() const;
        
        GLuint getTextureID() const;
        
        GLfloat getMaxS() const;
        void setMaxS(GLfloat maxS);
        
        GLfloat getMaxT() const;
        void setMaxT(GLfloat maxT);
        
    public:
        static const PixelFormatInfoMap& getPixelFormatInfoMap();
        
    public:
        
        /**convert functions*/
        
        /**
         Convert the format to the format param you specified, if the format is PixelFormat::Automatic, it will detect it automatically and convert to the closest format for you.
         It will return the converted format to you. if the outData != data, you must delete it manually.
         */
        static PixelFormat convertDataToFormat(const unsigned char* data, int32_t dataLen, PixelFormat originFormat, PixelFormat format, unsigned char** outData, int32_t* outDataLen);
        
        static PixelFormat convertI8ToFormat(const unsigned char* data, int32_t dataLen, PixelFormat format, unsigned char** outData, int32_t* outDataLen);
        static PixelFormat convertAI88ToFormat(const unsigned char* data, int32_t dataLen, PixelFormat format, unsigned char** outData, int32_t* outDataLen);
        static PixelFormat convertRGB888ToFormat(const unsigned char* data, int32_t dataLen, PixelFormat format, unsigned char** outData, int32_t* outDataLen);
        static PixelFormat convertRGBA8888ToFormat(const unsigned char* data, int32_t dataLen, PixelFormat format, unsigned char** outData, int32_t* outDataLen);
        
        //I8 to XXX
        static void convertI8ToRGB888(const unsigned char* data, int32_t dataLen, unsigned char* outData);
        static void convertI8ToRGBA8888(const unsigned char* data, int32_t dataLen, unsigned char* outData);
        static void convertI8ToRGB565(const unsigned char* data, int32_t dataLen, unsigned char* outData);
        static void convertI8ToRGBA4444(const unsigned char* data, int32_t dataLen, unsigned char* outData);
        static void convertI8ToRGB5A1(const unsigned char* data, int32_t dataLen, unsigned char* outData);
        static void convertI8ToAI88(const unsigned char* data, int32_t dataLen, unsigned char* outData);
        
        //AI88 to XXX
        static void convertAI88ToRGB888(const unsigned char* data, int32_t dataLen, unsigned char* outData);
        static void convertAI88ToRGBA8888(const unsigned char* data, int32_t dataLen, unsigned char* outData);
        static void convertAI88ToRGB565(const unsigned char* data, int32_t dataLen, unsigned char* outData);
        static void convertAI88ToRGBA4444(const unsigned char* data, int32_t dataLen, unsigned char* outData);
        static void convertAI88ToRGB5A1(const unsigned char* data, int32_t dataLen, unsigned char* outData);
        static void convertAI88ToA8(const unsigned char* data, int32_t dataLen, unsigned char* outData);
        static void convertAI88ToI8(const unsigned char* data, int32_t dataLen, unsigned char* outData);
        
        //RGB888 to XXX
        static void convertRGB888ToRGBA8888(const unsigned char* data, int32_t dataLen, unsigned char* outData);
        static void convertRGB888ToRGB565(const unsigned char* data, int32_t dataLen, unsigned char* outData);
        static void convertRGB888ToI8(const unsigned char* data, int32_t dataLen, unsigned char* outData);
        static void convertRGB888ToAI88(const unsigned char* data, int32_t dataLen, unsigned char* outData);
        static void convertRGB888ToRGBA4444(const unsigned char* data, int32_t dataLen, unsigned char* outData);
        static void convertRGB888ToRGB5A1(const unsigned char* data, int32_t dataLen, unsigned char* outData);
        
        //RGBA8888 to XXX
        static void convertRGBA8888ToRGB888(const unsigned char* data, int32_t dataLen, unsigned char* outData);
        static void convertRGBA8888ToRGB565(const unsigned char* data, int32_t dataLen, unsigned char* outData);
        static void convertRGBA8888ToI8(const unsigned char* data, int32_t dataLen, unsigned char* outData);
        static void convertRGBA8888ToA8(const unsigned char* data, int32_t dataLen, unsigned char* outData);
        static void convertRGBA8888ToAI88(const unsigned char* data, int32_t dataLen, unsigned char* outData);
        static void convertRGBA8888ToRGBA4444(const unsigned char* data, int32_t dataLen, unsigned char* outData);
        static void convertRGBA8888ToRGB5A1(const unsigned char* data, int32_t dataLen, unsigned char* outData);
        
    protected:
        PixelFormat _pixelFormat;

        int32_t _width;
        int32_t _height;

        GLuint _id;

        GLfloat _maxS;
        GLfloat _maxT;

        static const PixelFormatInfoMap _pixelFormatInfoTables;
    };
}
