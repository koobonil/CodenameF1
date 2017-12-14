#include "imagepacker.h"

namespace ctpo
{
    bool ImageCompareByHeight(const inputImage &i1, const inputImage &i2)
    {
        if (i1.sizeCurrent.height() == i2.sizeCurrent.height())
        {
            if (i1.sizeCurrent.width() == i2.sizeCurrent.width())
            {
                return true;
            }
            else
            {
                return (i1.sizeCurrent.width() > i2.sizeCurrent.width());
            }
        }
        else
        {
            return (i1.sizeCurrent.height() > i2.sizeCurrent.height());
        }
    }
    bool ImageCompareByWidth(const inputImage &i1, const inputImage &i2)
    {
        if (i1.sizeCurrent.width() == i2.sizeCurrent.width())
        {
            if (i1.sizeCurrent.height() == i2.sizeCurrent.height())
            {
                return true;
            }
            else
            {
                return (i1.sizeCurrent.height() > i2.sizeCurrent.height());
            }
        }
        else
        {
            return (i1.sizeCurrent.width() > i2.sizeCurrent.width());
        }
    }
    bool ImageCompareByArea(const inputImage &i1, const inputImage &i2)
    {
        return (i1.sizeCurrent.height() * i1.sizeCurrent.width()) > (i2.sizeCurrent.height() * i2.sizeCurrent.width());
    }
    
    bool ImageCompareByMax(const inputImage &i1, const inputImage &i2)
    {
        int first = i1.sizeCurrent.height() > i1.sizeCurrent.width() ? i1.sizeCurrent.height() : i1.sizeCurrent.width();
        int second = i2.sizeCurrent.height() > i2.sizeCurrent.width() ? i2.sizeCurrent.height() : i2.sizeCurrent.width();

        if(first == second)
        {
            return ImageCompareByArea(i1, i2);
        }
        else
        {
            return first > second;
        }
    }
    
    void ImagePacker::sort()
    {
        switch(sortOrder)
        {
            case 1:
                std::sort(images.begin(), images.end(), ImageCompareByWidth);
                break;
            case 2:
                std::sort(images.begin(), images.end(), ImageCompareByHeight);
                break;
            case 3:
                std::sort(images.begin(), images.end(), ImageCompareByArea);
                break;
            case 4:
                std::sort(images.begin(), images.end(), ImageCompareByMax);
                break;
        }
    }
}
