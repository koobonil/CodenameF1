#include "zay_types.h"
#include "zay_image_data.h"
#include "zay_texture2d.h"

#include <format/boss_bmp.hpp>
#include <format/boss_png.hpp>

namespace ZAY
{
    //////////////////////////////////////////////////////////////////////////
    // Implement Image
    //////////////////////////////////////////////////////////////////////////

    int ImageData::s_tone_R = 255;
    int ImageData::s_tone_G = 255;
    int ImageData::s_tone_B = 255;
    int ImageData::s_tone_Arg = 0;
    int ImageData::s_tone_Method = 0;

    ImageData::ImageData()
    : _data(nullptr)
    , _dataLen(0)
    , _width(0)
    , _height(0)
    , _renderFormat(PixelFormat::NONE)
    , _hasPremultipliedAlpha(true)
    , _hasPreadditivedAlpha(true)
    {
    }
    
    ImageData::~ImageData()
    {
        if (_data)
        {
            Buffer::Free((buffer) _data);
            _data = nullptr;
        }
    }

    bool ImageData::initWithImageData(const unsigned char * data, int32_t dataLen)
    {
        bool ret = false;
        
        do
        {
            if ((data == nullptr) ||
                (dataLen <= 0))
                break;
            ret = initWithPngData(data);
        } while (0);

        return ret;
    }
    
    bool ImageData::initWithTextureData(unsigned char * data, int32_t dataLen, int32_t width, int32_t height, PixelFormat pixelFormat, bool dataOwnership, bool preMulti, bool preAdditive)
    {
        bool bRet = false;
        do
        {
            if ((width == 0) ||
                (height == 0))
                break;

            _width = width;
            _height = height;
            _hasPremultipliedAlpha = preMulti;
            _hasPreadditivedAlpha = preAdditive;
            _renderFormat = pixelFormat;

            if (dataOwnership)
            {
                if (_data)
                {
                    Buffer::Free((buffer) _data);
                }

                _dataLen = dataLen;
                _data = data;
            }
            else
            {
                if (_data)
                {
                    Buffer::Free((buffer) _data);
                }

                _dataLen = dataLen;
                _data = (unsigned char*) Buffer::Alloc(BOSS_DBG _dataLen);
                memcpy(_data, data, _dataLen);
            }

            bRet = true;
        } while (0);
        
        return bRet;
    }
    
    int32_t ImageData::getBitPerPixel()
    {
        return Texture2D::getPixelFormatInfoMap().at(_renderFormat).bpp;
    }
    
    bool ImageData::hasAlpha()
    {
        return Texture2D::getPixelFormatInfoMap().at(_renderFormat).alpha;
    }

    bool ImageData::initWithPngData(const unsigned char * data)
    {
        id_bitmap NewBitmap = Png().ToBmp(data, true);
        if(!NewBitmap) return false;

        _width = Bmp::GetWidth(NewBitmap);
        _height = Bmp::GetHeight(NewBitmap);
        _renderFormat = PixelFormat::RGBA8888;

        _dataLen = 4 * _width * _height;
        _data = (unsigned char*) Buffer::Alloc(BOSS_DBG _dataLen);

        uint32* DstBits = (uint32*) _data;
        const uint32* SrcBits = (const uint32*) Bmp::GetBits(NewBitmap);
        for(sint32 y = 0; y < _height; ++y)
        for(sint32 x = 0; x < _width; ++x)
        {
            uint32* DstBit = &DstBits[x + _width * y];
            const uint32 SrcBit = SrcBits[x + _width * (_height - y - 1)];
            const uint32 A = SrcBit & 0xFF000000; // Alpha
            const int R = (SrcBit & 0x00FF0000) >> 16; // Red
            const int G = (SrcBit & 0x0000FF00) >> 8; // Green
            const int B = (SrcBit & 0x000000FF) >> 0; // Blue
            if(s_tone_Method == 0)
            {
                const uint32 TR = (((R * (255 - s_tone_Arg) + (R * s_tone_R / 255) * s_tone_Arg) / 255) << 0) & 0x000000FF; // Multifly Red
                const uint32 TG = (((G * (255 - s_tone_Arg) + (G * s_tone_G / 255) * s_tone_Arg) / 255) << 8) & 0x0000FF00; // Multifly Green
                const uint32 TB = (((B * (255 - s_tone_Arg) + (B * s_tone_B / 255) * s_tone_Arg) / 255) << 16) & 0x00FF0000; // Multifly Blue
                *DstBit = A | TR | TG | TB;
            }
            else
            {
                const uint32 TR = (((R * (255 - s_tone_Arg) + s_tone_R * s_tone_Arg) / 255) << 0) & 0x000000FF; // Coloring Red
                const uint32 TG = (((G * (255 - s_tone_Arg) + s_tone_G * s_tone_Arg) / 255) << 8) & 0x0000FF00; // Coloring Green
                const uint32 TB = (((B * (255 - s_tone_Arg) + s_tone_B * s_tone_Arg) / 255) << 16) & 0x00FF0000; // Coloring Blue
                *DstBit = A | TR | TG | TB;
            }
        }

        _hasPremultipliedAlpha = false;
        _hasPreadditivedAlpha = false;

        Bmp::Remove(NewBitmap);
        return true;
    }

    // premultiply alpha, or the effect will wrong when want to use other pixel format in Texture2D,
    // such as RGB888, RGB5A1
#define ZAY_RGB_PREMULTIPLY_ALPHA(vr, vg, vb, va) \
(unsigned)(((unsigned)((unsigned char)(vr) * ((unsigned char)(va) + 1)) >> 8) | \
((unsigned)((unsigned char)(vg) * ((unsigned char)(va) + 1) >> 8) << 8) | \
((unsigned)((unsigned char)(vb) * ((unsigned char)(va) + 1) >> 8) << 16) | \
((unsigned)(unsigned char)(va) << 24))
    
#define ZAY_RGB_PREADDITIVE_ALPHA(vr, vg, vb, va) \
(unsigned)(((unsigned)((unsigned char)(vr) * ((unsigned char)(va) + 1)) >> 8) | \
((unsigned)((unsigned char)(vg) * ((unsigned char)(va) + 1) >> 8) << 8) | \
((unsigned)((unsigned char)(vb) * ((unsigned char)(va) + 1) >> 8) << 16) | \
((unsigned)(unsigned char)(0) << 24))
    
    void ImageData::premultiplyAlpha()
    {
        assert(_renderFormat == PixelFormat::RGBA8888);

        if (_hasPremultipliedAlpha == false &&
            _hasPreadditivedAlpha == false &&
            _data)
        {
            unsigned int* fourBytes = (unsigned int*)_data;

            for(int i = 0; i < _width * _height; i++)
            {
                unsigned char* p = _data + i * 4;
                fourBytes[i] = ZAY_RGB_PREMULTIPLY_ALPHA(p[0], p[1], p[2], p[3]);
            }

            _hasPremultipliedAlpha = true;
        }
    }
    
    void ImageData::preadditiveAlpha()
    {
        assert(_renderFormat == PixelFormat::RGBA8888);
        
        if (_hasPremultipliedAlpha == false &&
            _hasPreadditivedAlpha == false &&
            _data)
        {
            unsigned int* fourBytes = (unsigned int*)_data;
        
            for(int i = 0; i < _width * _height; i++)
            {
                unsigned char* p = _data + i * 4;
                fourBytes[i] = ZAY_RGB_PREADDITIVE_ALPHA(p[0], p[1], p[2], p[3]);
            }
            
            _hasPreadditivedAlpha = true;
        }
    }

    ImageData::ImageData(const ImageData&)
    {
        assert(0);
    }

    ImageData & ImageData::operator=(const ImageData&)
    {
        assert(0);
        return (*this);
    }
}
