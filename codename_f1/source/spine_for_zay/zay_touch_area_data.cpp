#include "zay_types.h"
#include "zay_touch_area_data.h"
#include "zay_touch_area_instance.h"

namespace ZAY
{
    TouchAreaData::TouchAreaData()
    {
        _localVerticesXY = nullptr;
        _verticesCount = 0;
        _renderPriorityGroup = 0;
    }

    TouchAreaData::~TouchAreaData()
    {
        if (_localVerticesXY)
        {
            delete [] _localVerticesXY;
            _localVerticesXY = nullptr;
        }
    }

    SceneInstance* TouchAreaData::createSceneInstance()
    {
        auto touchAreaObject = new TouchAreaInstance;
        
        touchAreaObject->setLocalVertices(_name, _localVerticesXY, _verticesCount);
        touchAreaObject->setRenderPriorityGroup(_renderPriorityGroup);
        
        return touchAreaObject;
    }

    void TouchAreaData::setLocalVertices(const char* name, float* vertices, int32_t verticesCount)
    {
        if (_verticesCount != verticesCount)
        {
            _name = name;
            if (_localVerticesXY)
            {
                delete [] _localVerticesXY;
                _localVerticesXY = nullptr;
            }

            _verticesCount = verticesCount;

            if (_verticesCount > 0)
            {
                _localVerticesXY = new float[_verticesCount];

                if (vertices)
                {
                    memcpy(_localVerticesXY, vertices, sizeof(float) * _verticesCount);
                }
            }
        }
    }

    void TouchAreaData::setRenderPriorityGroup(int32_t renderPriorityGroup)
    {
        _renderPriorityGroup = renderPriorityGroup;
    }

    int32_t TouchAreaData::getRenderPriorityGroup() const
    {
        return _renderPriorityGroup;
    }
}
