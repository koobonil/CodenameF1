#include "maxrects.h"

#include <cmath>

namespace ctpo
{
    MaxRects::MaxRects()
    {
    }
    
    point MaxRects::insertNode(inputImage *input)
    {
        int i;
        int min = 999999999, mini = -1, m;
        struct_size img = input->sizeCurrent;
//        if(img.width() == w) img.setWidth(img.width() - border->l - border->r);
//        if(img.height() == h) img.setHeight(img.height() - border->t - border->b);
        if(img.width() == 0 || img.height() == 0)
        {
            return point(0, 0);
        }
        bool leftNeighbor = false, rightNeighbor = false;
        bool _leftNeighbor = false, _rightNeighbor = false;
        bool rotated, bestIsRotated = false;
        for(i = 0; i < F.size(); i++)
        {
            if((F.at(i).r.width() >= img.width() && F.at(i).r.height() >= img.height()) ||
               (F.at(i).r.width() >= img.height() && F.at(i).r.height() >= img.width()))
            {
                rotated = false;
                m = 0;
                if((F.at(i).r.width() >= img.height() && F.at(i).r.height() >= img.width()) &&
                   !(F.at(i).r.width() >= img.width() && F.at(i).r.height() >= img.height()))
                {
                    if(rotation == 0)
                    {
                        continue;
                    }
//                    input->rotated = !input->rotated;
//                    input->sizeCurrent.transpose();
                    img.transpose();
                    rotated = true;
                    m += img.height();
                }
                switch(heuristic)
                {
                    case ImagePacker::NONE:
                        mini = i;
                        i = static_cast<int>(F.size());
                        continue;
                    case ImagePacker::TL:
                        m += F.at(i).r.y();
                        _leftNeighbor = _rightNeighbor = false;
                        for(int k = 0; k < R.size(); k++)
                        {
                            if(std::abs(static_cast<float>(R.at(k).y() + R.at(k).height() / 2 - F.at(i).r.y() - F.at(i).r.height() / 2)) <
                               static_cast<float>(std::max(R.at(k).height(), F.at(i).r.height()) / 2))
                            {
                                if(R.at(k).x() + R.at(k).width() == F.at(i).r.x())
                                {
                                    m -= 5;
                                    _leftNeighbor = true;
                                }
                                if(R.at(k).x() == F.at(i).r.x() + F.at(i).r.width())
                                {
                                    m -= 5;
                                    _rightNeighbor = true;
                                }
                            }
                        }
                        if(_leftNeighbor || _rightNeighbor == false)
                        {
                            if(F.at(i).r.x() + F.at(i).r.width() == w)
                            {
                                m -= 1;
                                _rightNeighbor = true;
                            }
                            if(F.at(i).r.x() == 0)
                            {
                                m -= 1;
                                _leftNeighbor = true;
                            }
                        }
                        break;
                    case ImagePacker::BAF:
                        m += F.at(i).r.width() * F.at(i).r.height();
                        break;
                    case ImagePacker::BSSF:
                        m += std::min(F.at(i).r.width() - img.width(), F.at(i).r.height() - img.height());
                        break;
                    case ImagePacker::BLSF:
                        m += std::max(F.at(i).r.width() - img.width(), F.at(i).r.height() - img.height());
                        break;
                    case ImagePacker::MINW:
                        m += F.at(i).r.width();
                        break;
                    case ImagePacker::MINH:
                        m += F.at(i).r.height();
                }
                if(m < min)
                {
                    min = m;
                    mini = i;
                    leftNeighbor = _leftNeighbor;
                    rightNeighbor = _rightNeighbor;
                    bestIsRotated = rotated;
                }
                if(rotated)
                {
                    img.transpose();
                }
            }
        }
        if(bestIsRotated)
        {
            img.transpose();
            input->rotated = !input->rotated;
            input->sizeCurrent.transpose();
        }
        if(mini >= 0)
        {
            i = mini;
            MaxRectsNode n0;
            rect buf(F.at(i).r.x(), F.at(i).r.y(), img.width(), img.height());
            if(heuristic == ImagePacker::TL)
            {
                if(!leftNeighbor && F.at(i).r.x() != 0 &&
                   F.at(i).r.width() + F.at(i).r.x() == w)
                {
                    buf = rect(w - img.width(), F.at(i).r.y(), img.width(), img.height());
                }
                if(!leftNeighbor && rightNeighbor)
                {
                    buf = rect(F.at(i).r.x() + F.at(i).r.width() - img.width(), F.at(i).r.y(),
                               img.width(), img.height());
                }
            }
            n0.r = buf;
            R.push_back(buf);
            if(F.at(i).r.width() > img.width())
            {
                MaxRectsNode n;
                n.r = rect(F.at(i).r.x() + (buf.x() == F.at(i).r.x() ? img.width() : 0),
                           F.at(i).r.y(), F.at(i).r.width() - img.width(), F.at(i).r.height());
//                n.i = NULL;
                F.push_back(n);
            }
            if(F.at(i).r.height() > img.height())
            {
                MaxRectsNode n;
                n.r = rect(F.at(i).r.x(), F.at(i).r.y() + img.height(), F.at(i).r.width(),
                           F.at(i).r.height() - img.height());
//                n.i = NULL;
                F.push_back(n);
            }
            
            auto it = F.begin();
            std::advance(it, i);
            F.erase(it);
            //intersect
            for(i = 0; i < F.size(); i++)
            {
                if(F.at(i).r.intersects(n0.r))
                {
                    if(n0.r.x() + n0.r.width() < F.at(i).r.x() + F.at(i).r.width())
                    {
                        MaxRectsNode n;
                        n.r = rect(n0.r.width() + n0.r.x(), F.at(i).r.y(),
                                   F.at(i).r.width() + F.at(i).r.x() - n0.r.width() - n0.r.x(),
                                   F.at(i).r.height());
//                        n.i = NULL;
                        F.push_back(n);
                    }
                    if(n0.r.y() + n0.r.height() < F.at(i).r.y() + F.at(i).r.height())
                    {
                        MaxRectsNode n;
                        n.r = rect(F.at(i).r.x(), n0.r.height() + n0.r.y(),
                                   F.at(i).r.width(), F.at(i).r.height() + F.at(i).r.y() - n0.r.height() -
                                   n0.r.y());
//                        n.i = NULL;
                        F.push_back(n);
                    }
                    if(n0.r.x() > F.at(i).r.x())
                    {
                        MaxRectsNode n;
                        n.r = rect(F.at(i).r.x(), F.at(i).r.y(), n0.r.x() - F.at(i).r.x(),
                                   F.at(i).r.height());
//                        n.i = NULL;
                        F.push_back(n);
                    }
                    if(n0.r.y() > F.at(i).r.y())
                    {
                        MaxRectsNode n;
                        n.r = rect(F.at(i).r.x(), F.at(i).r.y(), F.at(i).r.width(),
                                   n0.r.y() - F.at(i).r.y());
//                        n.i = NULL;
                        F.push_back(n);
                    }
                    auto it = F.begin();
                    std::advance(it, i);
                    F.erase(it);
                    i--;
                }
            }
            
            for(i = 0; i < F.size(); i++)
            {
                for(int j = i + 1; j < F.size(); j++)
                {
                    if(i != j  && F.at(i).r.contains(F.at(j).r))
                    {
                        auto it = F.begin();
                        std::advance(it, j);
                        F.erase(it);
                        j--;
                    }
                }
            }
            return point(n0.r.x(), n0.r.y());
        }
        return point(999999, 999999);
    }
}
