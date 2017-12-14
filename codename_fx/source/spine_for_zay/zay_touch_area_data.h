#pragma once

#include "zay_scene_data.h"

namespace ZAY
{
    class TouchAreaData
    : public SceneData
    {
    public:
        TouchAreaData();
        virtual ~TouchAreaData();

    public:
        virtual SceneInstance* createSceneInstance() override;

    public:
        void setLocalVertices(const char* name, float* vertices, int32_t verticesCount);
    private:
        std::string _name;
        float* _localVerticesXY;
        int32_t _verticesCount;

    public:
        void setRenderPriorityGroup(int32_t renderPriorityGroup);
        int32_t getRenderPriorityGroup() const;
    private:
        int32_t _renderPriorityGroup;
    };
}
