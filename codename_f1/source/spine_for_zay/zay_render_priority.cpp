#include "zay_types.h"
#include "zay_render_priority.h"

namespace ZAY
{
    RenderPriority::RenderPriority()
    {
        _group = 0;
        _depth = 0.0f;
        _sceneData = nullptr;
        _sceneDataDepth = 0.0f;
    }

    RenderPriority::RenderPriority(const RenderPriority& other)
    {
        _group = other._group;
        _depth = other._depth;
        _sceneData = other._sceneData;
        _sceneDataDepth = other._sceneDataDepth;
    }

    void RenderPriority::setGroup(int32_t group)
    {
        _group = group;
    }
    
    int32_t RenderPriority::getGroup() const
    {
        return _group;
    }
    
    void RenderPriority::setDepth(float depth)
    {
        _depth = depth;
    }
    
    float RenderPriority::getDepth() const
    {
        return _depth;
    }
    
    void RenderPriority::setSceneData(const SceneData* sceneData)
    {
        _sceneData = sceneData;
    }
    
    const SceneData* RenderPriority::getSceneData() const
    {
        return _sceneData;
    }
    
    void RenderPriority::setSceneDataDepth(float depth)
    {
        _sceneDataDepth = depth;
    }
    
    float RenderPriority::getSceneDataDepth() const
    {
        return _sceneDataDepth;
    }

    const RenderPriority& RenderPriority::operator = (const RenderPriority& other)
    {
        _group = other._group;
        _depth = other._depth;
        _sceneData = other._sceneData;
        _sceneDataDepth = other._sceneDataDepth;
        
        return *this;
    }

    bool RenderPriority::operator < (const RenderPriority& other) const
    {
        if (_group < other._group)
        {
            return true;
        }
        else if (_group == other._group)
        {
            if (_depth < other._depth)
            {
                return true;
            }
            else if (_depth == other._depth)
            {
                if (_sceneDataDepth < other._sceneDataDepth)
                {
                    return true;
                }
            }
        }

        return false;
    }
    
    bool RenderPriority::operator > (const RenderPriority& other) const
    {
        if (_group > other._group)
        {
            return true;
        }
        else if (_group == other._group)
        {
            if (_depth > other._depth)
            {
                return true;
            }
            else if (_depth == other._depth)
            {
                if (_sceneDataDepth > other._sceneDataDepth)
                {
                    return true;
                }
            }
        }

        return false;
    }
    
    bool RenderPriority::operator <= (const RenderPriority& other) const
    {
        if (*this == other)
        {
            return true;
        }
        
        return (*this < other);
    }
    
    bool RenderPriority::operator >= (const RenderPriority& other) const
    {
        if (*this == other)
        {
            return true;
        }
        
        return (*this > other);
    }
    
    bool RenderPriority::operator != (const RenderPriority& other) const
    {
        if (_group != other._group ||
            _depth != other._depth ||
            _sceneData != other._sceneData ||
            _sceneDataDepth != other._sceneDataDepth)
        {
            return true;
        }

        return false;
    }
    
    bool RenderPriority::operator == (const RenderPriority& other) const
    {
        if (_group == other._group ||
            _depth == other._depth ||
            _sceneData == other._sceneData ||
            _sceneDataDepth == other._sceneDataDepth)
        {
            return true;
        }

        return false;
    }
}

