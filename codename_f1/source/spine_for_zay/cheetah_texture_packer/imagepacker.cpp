#include "imagepacker.h"
#include "maxrects.h"

namespace ctpo
{
    ImagePacker::ImagePacker()
    {
        prevSortOrder = -1;
        extrude = 1;
        cropThreshold = 10;
        minTextureSizeX = 32;
        minTextureSizeY = 32;
    }
    
    void ImagePacker::pack(int heur, int w, int h)
    {
        SortImages(w, h);
        
        missingImages = 1;
        mergedImages = 0;
        area = 0;
        bins.clear();
        
        unsigned areaBuf = AddImgesToBins(heur, w, h);
        
        if(areaBuf && !missingImages)
        {
            CropLastImage(heur, w, h, false);
        }
        
        if(merge)
            for(int i = 0; i < images.size(); i++)
                if(images.at(i).duplicateId != NULL)
                {
                    images.operator [](i).pos = find(images.at(i).duplicateId)->pos;
                    images.operator [](i).textureId = find(images.at(i).duplicateId)->textureId;
                    mergedImages++;
                }
    }
    
    void ImagePacker::UpdateCrop()
    {
    }
    
    void ImagePacker::addItem(const image &img)
    {
        inputImage i;
        if(img.width() == 0 || img.height() == 0)
        {
            return;
        }

        i.crop = crop(img);
        i.size = img.size();
        i.id = img.img;
        images.push_back(i);
    }
    
    void ImagePacker::clear()
    {
        images.clear();
    }
    
    void ImagePacker::realculateDuplicates()
    {
        for(int i = 0; i < images.size(); i++)
        {
            images.operator [](i).duplicateId = NULL;
        }
        for(int i = 0; i < images.size(); i++)
        {
            for(int k = i + 1; k < images.size(); k++)
            {
                if(images.at(k).duplicateId == NULL &&
//                   images.at(i).hash == images.at(k).hash &&
                   images.at(i).size == images.at(k).size &&
                   images.at(i).crop == images.at(k).crop)
                {
                    images.operator [](k).duplicateId = images.at(i).id;
                }
            }
        }
    }
    
    void ImagePacker::removeId(void *data)
    {
        for(int k = 0; k < images.size(); k++)
        {
            if(images.at(k).id == data)
            {
                auto it = images.begin();
                std::advance(it, k);
                images.erase(it);
                break;
            }
        }
    }
    const inputImage *ImagePacker::find(void *data)
    {
        for(int i = 0; i < images.size(); i++)
        {
            if(data == images.at(i).id)
            {
                return &images.at(i);
            }
        }
        return NULL;
    }
    
    void ImagePacker::SortImages(int w, int h)
    {
        realculateDuplicates();
        neededArea = 0;
        struct_size ssize;
        for(int i = 0; i < images.size(); i++)
        {
            images.operator [](i).pos = point(999999, 999999);
            if(cropThreshold)
            {
                ssize = images.at(i).crop.size();
            }
            else
            {
                ssize = images.at(i).size;
            }
            if(ssize.width() == w)
            {
                ssize.setWidth(ssize.width() - border.l - border.r - 2 * extrude);
            }
            if(ssize.height() == h)
            {
                ssize.setHeight(ssize.height() - border.t - border.b - 2 * extrude);
            }
            ssize += struct_size(border.l + border.r + 2 * extrude,
                                 border.t + border.b + 2 * extrude);
            
            images.operator [](i).rotated = false;
            if((rotate == WIDTH_GREATHER_HEIGHT && ssize.width() > ssize.height()) ||
               (rotate == WIDTH_GREATHER_2HEIGHT && ssize.width() > 2 * ssize.height()) ||
               (rotate == HEIGHT_GREATHER_WIDTH && ssize.height() > ssize.width()) ||
               (rotate == H2_WIDTH_H && ssize.height() > ssize.width() &&
                ssize.width() * 2 > ssize.height()) ||
               (rotate == W2_HEIGHT_W && ssize.width() > ssize.height() &&
                ssize.height() * 2 > ssize.width()) ||
               (rotate == HEIGHT_GREATHER_2WIDTH && ssize.height() > 2 * ssize.width()))
            {
                ssize.transpose();
                images.operator [](i).rotated = true;
            }
            images.operator [](i).sizeCurrent = ssize;
            if(images.at(i).duplicateId == NULL || !merge)
            {
                neededArea += ssize.width() * ssize.height();
            }
        }
        sort();
    }
    
    int ImagePacker::FillBin(int heur, int w, int h, int binIndex)
    {
        int areaBuf = 0;
        MaxRects rects;
        MaxRectsNode mrn;
        mrn.r = rect(0, 0, w, h);
        rects.F.push_back(mrn);
        rects.heuristic = heur;
        rects.leftToRight = ltr;
        rects.w = w;
        rects.h = h;
        rects.rotation = rotate;
        rects.border = &border;
        for(int i = 0; i < images.size(); i++)
        {
            if(point(999999, 999999) != images.at(i).pos)
            {
                continue;
            }
            if(images.at(i).duplicateId == NULL || !merge)
            {
                images.operator [](i).pos = rects.insertNode(&images.operator [](i));
                images.operator [](i).textureId = binIndex;
                if(point(999999, 999999) == images.at(i).pos)
                {
                    missingImages++;
                }
                else
                {
                    areaBuf += images.at(i).sizeCurrent.width() * images.at(i).sizeCurrent.height();
                    area += images.at(i).sizeCurrent.width() * images.at(i).sizeCurrent.height();
                }
            }
        }
        return areaBuf;
    }
    
    void ImagePacker::ClearBin(int binIndex)
    {
        for(int i = 0; i < images.size(); i++)
        {
            if(images.at(i).textureId == binIndex)
            {
                area -= images.at(i).sizeCurrent.width() * images.at(i).sizeCurrent.height();
                images.operator [](i).pos = point(999999, 999999);
            }
        }
    }
    
    unsigned ImagePacker::AddImgesToBins(int heur, int w, int h)
    {
        int binIndex = bins.size() - 1;
        unsigned areaBuf = 0;
        unsigned lastAreaBuf = 0;
        do
        {
            missingImages = 0;
            bins.push_back(struct_size(w, h));
            lastAreaBuf = FillBin(heur, w , h , ++binIndex);
            if(!lastAreaBuf)
            {
                bins.erase(--bins.end());
            }
            areaBuf += lastAreaBuf;
        }
        while(missingImages && lastAreaBuf);
        return areaBuf;
    }
    
    void ImagePacker::CropLastImage(int heur, int w, int h, bool wh)
    {
        missingImages = 0;
        std::vector<inputImage> last_images = images;
        std::vector<struct_size> last_bins = bins;
        uint64_t last_area = area;
        
        bins.erase(--bins.end());
        ClearBin(bins.size());
        
        if(square)
        {
            w /= 2;
            h /= 2;
        }
        else
        {
            if(wh)
            {
                w /= 2;
            }
            else
            {
                h /= 2;
            }
            wh = !wh;
        }
        
        int binIndex = bins.size();
        missingImages = 0;
        bins.push_back(struct_size(w, h));
        FillBin(heur, w , h , binIndex);
        if(missingImages)
        {
            images = last_images;
            bins = last_bins;
            area = last_area;
            missingImages = 0;
            if(square)
            {
                w *= 2;
                h *= 2;
            }
            else
            {
                if(!wh)
                {
                    w *= 2;
                }
                else
                {
                    h *= 2;
                }
                wh = !wh;
            }
            if(autosize)
            {
                float rate = GetFillRate();
                if((rate < (static_cast<float>(minFillRate) / 100.f)) &&
                   ((w > minTextureSizeX) && (h > minTextureSizeY)))
                {
                    DivideLastImage(heur, w, h, wh);
                    if(GetFillRate() <= rate)
                    {
                        images = last_images;
                        bins = last_bins;
                        area = last_area;
                    }
                }
            }
        }
        else
        {
            CropLastImage(heur, w, h, wh);
        }
    }
    
    void ImagePacker::DivideLastImage(int heur, int w, int h, bool wh)
    {
        missingImages = 0;
        std::vector<inputImage> last_images = images;
        std::vector<struct_size> last_bins = bins;
        uint64_t last_area = area;
        
        bins.erase(--bins.end());
        ClearBin(bins.size());
        
        if(square)
        {
            w /= 2;
            h /= 2;
        }
        else
        {
            if(wh)
            {
                w /= 2;
            }
            else
            {
                h /= 2;
            }
            wh = !wh;
        }
        AddImgesToBins(heur, w, h);
        if(missingImages)
        {
            images = last_images;
            bins = last_bins;
            area = last_area;
            missingImages = 0;
        }
        else
        {
            CropLastImage(heur, w, h, wh);
        }
    }
    
    float ImagePacker::GetFillRate()
    {
        uint64_t binArea = 0;
        for(int i = 0; i < bins.size(); i++)
        {
            binArea += bins.at(i).width() * bins.at(i).height();
        }
        return (float)((double)area / (double)binArea);
    }
}
