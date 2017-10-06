#pragma once

#include "imagepacker.h"

namespace ctpo
{
    struct MaxRectsNode
    {
        rect r; //rect
        trbl b; //border
    };
    
    class MaxRects
    {
    public:
        MaxRects();
        
        std::vector<MaxRectsNode> F;
        std::vector<rect> R;
        std::vector<MaxRectsNode> FR;
        point insertNode(inputImage *);
        int heuristic, w, h, rotation;
        bool leftToRight;
        border_t *border;
    };
}
