#pragma once

#include <algorithm>
#include <cstdint>
#include <string>
#include <vector>

namespace ctpo
{
    class MaxRects;
    
    struct struct_size
    {
        struct_size()
        {
            _w = 0;
            _h = 0;
        }

        struct_size(int w, int h)
        {
            _w = w;
            _h = h;
        }

        int _w;
        int _h;
        
        void setWidth(int width)
        {
            _w = width;
        }

        void setHeight(int height)
        {
            _h = height;
        }

        int width() const
        {
            return _w;
        }
        
        int height() const
        {
            return _h;
        }
        
        void transpose()
        {
            std::swap(_w, _h);
        }
        
        bool operator==(const struct_size& other) const
        {
            return (_w == other._w) && (_h == other._h);
        }
        
        struct_size& operator+=(const struct_size& other)
        {
            _w += other._w;
            _h += other._h;

            return *this;
        }
    };
    
    struct rect
    {
        rect()
        {
            _x = 0;
            _y = 0;
            _size._w = 0;
            _size._h = 0;
        }

        rect(int x, int y, int width, int height)
        {
            _x = x;
            _y = y;
            _size._w = width;
            _size._h = height;
        }

        int _x;
        int _y;
        struct_size _size;
        
        const struct_size& size() const
        {
            return _size;
        }
        
        int width() const
        {
            return _size._w;
        }
        
        int height() const
        {
            return _size._h;
        }
        
        int x() const
        {
            return _x;
        }
        
        int y() const
        {
            return _y;
        }
        
        int getMinX() const
        {
            return _x;
        }

        int getMaxX() const
        {
            return _x + width();
        }

        int getMinY() const
        {
            return _y;
        }
        
        int getMaxY() const
        {
            return _y + height();
        }

        bool intersects(const rect& rect) const
        {
            return !(getMaxX() <= rect.getMinX() ||
                     rect.getMaxX() <= getMinX() ||
                     getMaxY() <= rect.getMinY() ||
                     rect.getMaxY() <= getMinY());
        }
        
        bool contains(const rect& rect) const
        {
            return (getMinX() <= rect.getMinX() &&
                    getMaxX() >= rect.getMaxX() &&
                    getMinY() <= rect.getMinY() &&
                    getMaxY() >= rect.getMaxY());
        }
        
        bool operator == (const rect& other) const
        {
            return (_x == other._x) && (_y == other._y) && (width() == other.width()) && (height() == other.height());
        }
    };
    
    struct point
    {
        point()
        {
            _x = 0;
            _y = 0;
        }
        
        point(int x, int y)
        {
            _x = x;
            _y = y;
        }
        
        int _x;
        int _y;
        
        int x() const
        {
            return _x;
        }

        int y() const
        {
            return _y;
        }

        bool operator == (const point& other) const
        {
            return (_x == other._x) && (_y == other._y);
        }
        
        bool operator != (const point& other) const
        {
            return (_x != other._x) || (_y != other._y);
        }
    };
    
    struct trbl
    {
        point t, r, b, l;
    };
    
    struct image
    {
        void* img;
        
        struct_size _size;
        rect _crop;
        point _pos;
        
        int width() const
        {
            return _size.width();
        }
        int height() const
        {
            return _size.width();
        }
        const struct_size& size() const
        {
            return _size;
        }
        const rect& crop() const
        {
            return _crop;
        }
        const point& pos() const
        {
            return _pos;
        }
    };
    
    struct packedImage
    {
        void* img;
        rect rc;
        rect crop;
        bool border, rotate;
        int textureId;
        int id;
    };
    
    struct inputImage
    {
        uint32_t hash;
        int textureId;
        void *id;
        void *duplicateId;
        point pos;
        struct_size size, sizeCurrent;
        rect crop;
        std::string path;
        
        bool cropped, rotated;
    };
    
    struct border_t
    {
        int t, b, l, r;
    };
    
    class ImagePacker
    {
    private:
        int prevSortOrder;
        void internalPack(int heur, int w, int h);
        
        void SortImages(int w, int h);
        
    public:
        std::vector<inputImage> images;
        std::vector<struct_size> bins;
        ImagePacker();
        bool compareImages(image *img1, image *img2, int *i, int *j);
        void pack(int heur, int w, int h);
        
        unsigned AddImgesToBins(int heur, int w, int h);
        
        void CropLastImage(int heur, int w, int h, bool wh);
        void DivideLastImage(int heur, int w, int h, bool wh);
        
        void UpdateCrop();
        
        float GetFillRate();
        
        void ClearBin(int binIndex);
        
        int FillBin(int heur, int w, int h, int binIndex);
        
        rect crop(const image &img);
        void sort();
        void addItem(const image &img);
        const inputImage *find(void *data);
        void removeId(void *);
        void realculateDuplicates();
        void clear();
        int compare;
        uint64_t area, neededArea;
        int missingImages;
        int mergedImages;
        bool ltr, merge, square, autosize, mergeBF;
        int cropThreshold;
        border_t border;
        int extrude;
        int rotate;
        int sortOrder;
        int minFillRate;
        int minTextureSizeX;
        int minTextureSizeY;
        enum {GUILLOTINE, MAXRECTS}; //method
        enum {NONE, TL, BAF, BSSF, BLSF, MINW, MINH, HEURISTIC_NUM}; //heuristic
        enum {SORT_NONE, WIDTH, HEIGHT, SORT_AREA, SORT_MAX, SORT_NUM}; //sort
        enum {NEVER, ONLY_WHEN_NEEDED, H2_WIDTH_H, WIDTH_GREATHER_HEIGHT, WIDTH_GREATHER_2HEIGHT, W2_HEIGHT_W, HEIGHT_GREATHER_WIDTH, HEIGHT_GREATHER_2WIDTH, ROTATION_NUM}; //rotation
    };
}
