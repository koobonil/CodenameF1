#include "zay_types.h"
#include "zay_colour_value.h"
#include "zay_math.h"

namespace ZAY
{
    const ColourValue ColourValue::ZERO = ColourValue(0.0,0.0,0.0,0.0);
    const ColourValue ColourValue::Black = ColourValue(0.0,0.0,0.0);
    const ColourValue ColourValue::White = ColourValue(1.0,1.0,1.0);
    const ColourValue ColourValue::Red = ColourValue(1.0,0.0,0.0);
    const ColourValue ColourValue::Green = ColourValue(0.0,1.0,0.0);
    const ColourValue ColourValue::Blue = ColourValue(0.0,0.0,1.0);
    
    ABGR ColourValue::getAsPremultipliedABGR() const
    {
#if     (defined ZAY_BIG_ENDIAN)
        uint8_t val8;
        uint32_t val32 = 0;
        
        // Convert to 32bit pattern
        // (RGBA = 8888)
        
        // Red
        val8 = static_cast<uint8_t>(a * r * 255);
        val32 = val8 << 24;
        
        // Green
        val8 = static_cast<uint8_t>(a * g * 255);
        val32 += val8 << 16;
        
        // Blue
        val8 = static_cast<uint8_t>(a * b * 255);
        val32 += val8 << 8;
        
        // Alpha
        val8 = static_cast<uint8_t>(a * 255);
        val32 += val8;
        
        return val32;
#elif   (defined ZAY_LITTLE_ENDIAN)
        uint8_t val8;
        uint32_t val32 = 0;

        // Convert to 32bit pattern
        // (ABRG = 8888)

        // Alpha
        val8 = static_cast<uint8_t>(a * 255);
        val32 = val8 << 24;

        // Blue
        val8 = static_cast<uint8_t>(a * b * 255);
        val32 += val8 << 16;

        // Green
        val8 = static_cast<uint8_t>(a * g * 255);
        val32 += val8 << 8;

        // Red
        val8 = static_cast<uint8_t>(a * r * 255);
        val32 += val8;

        return val32;
#else
# error The file zay_colour_value.h needs ZAY_XXX_ENDIAN.
#endif
    }

#if     (defined ZAY_BIG_ENDIAN)
    ABGR ColourValue::getAsABGR(void) const
#elif   (defined ZAY_LITTLE_ENDIAN)
    RGBA ColourValue::getAsRGBA(void) const
#else
# error The file zay_colour_value.h needs ZAY_XXX_ENDIAN.
#endif
    {
        uint8_t val8;
        uint32_t val32 = 0;

        // Convert to 32bit pattern
        // (RGBA = 8888)

        // Red
        val8 = static_cast<uint8_t>(r * 255);
        val32 = val8 << 24;

        // Green
        val8 = static_cast<uint8_t>(g * 255);
        val32 += val8 << 16;

        // Blue
        val8 = static_cast<uint8_t>(b * 255);
        val32 += val8 << 8;

        // Alpha
        val8 = static_cast<uint8_t>(a * 255);
        val32 += val8;

        return val32;
    }

#if     (defined ZAY_BIG_ENDIAN)
    BGRA ColourValue::getAsBGRA(void) const
#elif   (defined ZAY_LITTLE_ENDIAN)
    ARGB ColourValue::getAsARGB(void) const
#else
# error The file zay_colour_value.h needs ZAY_XXX_ENDIAN.
#endif
    {
        uint8_t val8;
        uint32_t val32 = 0;

        // Convert to 32bit pattern
        // (ARGB = 8888)

        // Alpha
        val8 = static_cast<uint8_t>(a * 255);
        val32 = val8 << 24;

        // Red
        val8 = static_cast<uint8_t>(r * 255);
        val32 += val8 << 16;

        // Green
        val8 = static_cast<uint8_t>(g * 255);
        val32 += val8 << 8;

        // Blue
        val8 = static_cast<uint8_t>(b * 255);
        val32 += val8;


        return val32;
    }

#if     (defined ZAY_BIG_ENDIAN)
    ARGB ColourValue::getAsARGB(void) const
#elif   (defined ZAY_LITTLE_ENDIAN)
    BGRA ColourValue::getAsBGRA(void) const
#else
# error The file zay_colour_value.h needs ZAY_XXX_ENDIAN.
#endif
    {
        uint8_t val8;
        uint32_t val32 = 0;

        // Convert to 32bit pattern
        // (ARGB = 8888)

        // Blue
        val8 = static_cast<uint8_t>(b * 255);
        val32 = val8 << 24;

        // Green
        val8 = static_cast<uint8_t>(g * 255);
        val32 += val8 << 16;

        // Red
        val8 = static_cast<uint8_t>(r * 255);
        val32 += val8 << 8;

        // Alpha
        val8 = static_cast<uint8_t>(a * 255);
        val32 += val8;


        return val32;
    }

#if     (defined ZAY_BIG_ENDIAN)
    RGBA ColourValue::getAsRGBA(void) const
#elif   (defined ZAY_LITTLE_ENDIAN)
    ABGR ColourValue::getAsABGR(void) const
#else
# error The file zay_colour_value.h needs ZAY_XXX_ENDIAN.
#endif
    {
        uint8_t val8;
        uint32_t val32 = 0;

        // Convert to 32bit pattern
        // (ABRG = 8888)

        // Alpha
        val8 = static_cast<uint8_t>(a * 255);
        val32 = val8 << 24;

        // Blue
        val8 = static_cast<uint8_t>(b * 255);
        val32 += val8 << 16;

        // Green
        val8 = static_cast<uint8_t>(g * 255);
        val32 += val8 << 8;

        // Red
        val8 = static_cast<uint8_t>(r * 255);
        val32 += val8;


        return val32;
    }

#if     (defined ZAY_BIG_ENDIAN)
    void ColourValue::setAsABGR(const ABGR val)
#elif   (defined ZAY_LITTLE_ENDIAN)
    void ColourValue::setAsRGBA(const RGBA val)
#else
# error The file zay_colour_value.h needs ZAY_XXX_ENDIAN.
#endif
    {
        uint32_t val32 = val;

        // Convert from 32bit pattern
        // (RGBA = 8888)

        // Red
        r = ((val32 >> 24) & 0xFF) / 255.0f;

        // Green
        g = ((val32 >> 16) & 0xFF) / 255.0f;

        // Blue
        b = ((val32 >> 8) & 0xFF) / 255.0f;

        // Alpha
        a = (val32 & 0xFF) / 255.0f;
    }

#if     (defined ZAY_BIG_ENDIAN)
    void ColourValue::setAsBGRA(const BGRA val)
#elif   (defined ZAY_LITTLE_ENDIAN)
    void ColourValue::setAsARGB(const ARGB val)
#else
# error The file zay_colour_value.h needs ZAY_XXX_ENDIAN.
#endif
    {
        uint32_t val32 = val;

        // Convert from 32bit pattern
        // (ARGB = 8888)

        // Alpha
        a = ((val32 >> 24) & 0xFF) / 255.0f;

        // Red
        r = ((val32 >> 16) & 0xFF) / 255.0f;

        // Green
        g = ((val32 >> 8) & 0xFF) / 255.0f;

        // Blue
        b = (val32 & 0xFF) / 255.0f;
    }

#if     (defined ZAY_BIG_ENDIAN)
    void ColourValue::setAsARGB(const ARGB val)
#elif   (defined ZAY_LITTLE_ENDIAN)
    void ColourValue::setAsBGRA(const BGRA val)
#else
# error The file zay_colour_value.h needs ZAY_XXX_ENDIAN.
#endif
    {
        uint32_t val32 = val;

        // Convert from 32bit pattern
        // (ARGB = 8888)

        // Blue
        b = ((val32 >> 24) & 0xFF) / 255.0f;

        // Green
        g = ((val32 >> 16) & 0xFF) / 255.0f;

        // Red
        r = ((val32 >> 8) & 0xFF) / 255.0f;

        // Alpha
        a = (val32 & 0xFF) / 255.0f;
    }

#if     (defined ZAY_BIG_ENDIAN)
    void ColourValue::setAsRGBA(const RGBA val)
#elif   (defined ZAY_LITTLE_ENDIAN)
    void ColourValue::setAsABGR(const ABGR val)
#else
# error The file zay_colour_value.h needs ZAY_XXX_ENDIAN.
#endif
    {
        uint32_t val32 = val;

        // Convert from 32bit pattern
        // (ABGR = 8888)

        // Alpha
        a = ((val32 >> 24) & 0xFF) / 255.0f;

        // Blue
        b = ((val32 >> 16) & 0xFF) / 255.0f;

        // Green
        g = ((val32 >> 8) & 0xFF) / 255.0f;

        // Red
        r = (val32 & 0xFF) / 255.0f;
    }

    bool ColourValue::operator==(const ColourValue& rhs) const
    {
        return (r == rhs.r &&
            g == rhs.g &&
            b == rhs.b &&
            a == rhs.a);
    }

    bool ColourValue::operator!=(const ColourValue& rhs) const
    {
        return !(*this == rhs);
    }

    void ColourValue::setHSB(float hue, float saturation, float brightness)
    {
        // wrap hue
        if (hue > 1.0f)
        {
            hue -= (int)hue;
        }
        else if (hue < 0.0f)
        {
            hue += (int)hue + 1;
        }
        // clamp saturation / brightness
        saturation = std::min(saturation, 1.0f);
        saturation = std::max(saturation, 0.0f);
        brightness = std::min(brightness, 1.0f);
        brightness = std::max(brightness, 0.0f);

        if (brightness == 0.0f)
        {   
            // early exit, this has to be black
            r = g = b = 0.0f;
            return;
        }

        if (saturation == 0.0f)
        {   
            // early exit, this has to be grey

            r = g = b = brightness;
            return;
        }


        float hueDomain  = hue * 6.0f;
        if (hueDomain >= 6.0f)
        {
            // wrap around, and allow mathematical errors
            hueDomain = 0.0f;
        }
        unsigned short domain = (unsigned short)hueDomain;
        float f1 = brightness * (1.0f - saturation);
        float f2 = brightness * (1.0f - saturation * (hueDomain - domain));
        float f3 = brightness * (1.0f - saturation * (1.0f - (hueDomain - domain)));

        switch (domain)
        {
        case 0:
            // red domain; green ascends
            r = brightness;
            g = f3;
            b = f1;
            break;
        case 1:
            // yellow domain; red descends
            r = f2;
            g = brightness;
            b = f1;
            break;
        case 2:
            // green domain; blue ascends
            r = f1;
            g = brightness;
            b = f3;
            break;
        case 3:
            // cyan domain; green descends
            r = f1;
            g = f2;
            b = brightness;
            break;
        case 4:
            // blue domain; red ascends
            r = f3;
            g = f1;
            b = brightness;
            break;
        case 5:
            // magenta domain; blue descends
            r = brightness;
            g = f1;
            b = f2;
            break;
        }


    }

    void ColourValue::getHSB(float* hue, float* saturation, float* brightness) const
    {

        float vMin = std::min(r, std::min(g, b));
        float vMax = std::max(r, std::max(g, b));
        float delta = vMax - vMin;

        *brightness = vMax;

        if (Math::RealEqual(delta, 0.0f, 1e-6f))
        {
            // grey
            *hue = 0;
            *saturation = 0;
        }
        else                                    
        {
            // a colour
            *saturation = delta / vMax;

            float deltaR = (((vMax - r) / 6.0f) + (delta / 2.0f)) / delta;
            float deltaG = (((vMax - g) / 6.0f) + (delta / 2.0f)) / delta;
            float deltaB = (((vMax - b) / 6.0f) + (delta / 2.0f)) / delta;

            if (Math::RealEqual(r, vMax))
                *hue = deltaB - deltaG;
            else if (Math::RealEqual(g, vMax))
                *hue = 0.3333333f + deltaR - deltaB;
            else if (Math::RealEqual(b, vMax)) 
                *hue = 0.6666667f + deltaG - deltaR;

            if (*hue < 0.0f) 
                *hue += 1.0f;
            if (*hue > 1.0f)
                *hue -= 1.0f;
        }

        
    }

}

