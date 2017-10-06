#pragma once

#include "zay_types.h"

namespace ZAY
{
    class ImageData
    {
    public:
        /**
         * @js ctor
         */
        ImageData();
        /**
         * @js NA
         * @lua NA
         */
        virtual ~ImageData();

        /**
         @brief Load image from stream buffer.
         @param data  stream buffer which holds the image data.
         @param dataLen  data length expressed in (number of) bytes.
         @return true if loaded correctly.
         * @js NA
         * @lua NA
         */
        
        // data must be alloced by malloc
        bool initWithImageData(const unsigned char * data, int32_t dataLen);
        
        // data must be alloced by malloc
        bool initWithTextureData(unsigned char * data, int32_t dataLen, int32_t width, int32_t height, PixelFormat pixelFormat, bool dataOwnership = false, bool preMulti = false, bool preAdditive = false);
        
        // Getters
        inline unsigned char *              getData()               { return _data; }
        inline int32_t                      getDataLen()            { return _dataLen; }
        inline int32_t                      getWidth()              { return _width; }
        inline int32_t                      getHeight()             { return _height; }
        inline PixelFormat                  getRenderFormat()       { return _renderFormat; }
        inline bool                         hasPremultipliedAlpha() { return _hasPremultipliedAlpha; }
        inline bool                         hasPreadditivedAlpha()  { return _hasPreadditivedAlpha; }

        int32_t                 getBitPerPixel();
        bool                    hasAlpha();

    protected:
        bool initWithPngData(const unsigned char * data);

    public:
        void premultiplyAlpha();
        void preadditiveAlpha();

    protected:
        // _data must be alloced by malloc
        unsigned char *_data;
        int32_t _dataLen;
        int32_t _width;
        int32_t _height;
        PixelFormat _renderFormat;
        bool _hasPremultipliedAlpha;
        bool _hasPreadditivedAlpha;

    protected:
        ImageData(const ImageData&);
        ImageData & operator=(const ImageData&);

    public:
        static int s_tone_R;
        static int s_tone_G;
        static int s_tone_B;
        static int s_tone_Arg;
        static int s_tone_Method;
    };
}
