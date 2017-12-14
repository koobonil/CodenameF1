#pragma once

#include "zay_base.h"
#include "zay_render_priority.h"

namespace ZAY
{
	class RenderCommand
    : public Base
    {
        friend class SceneInstance;
        
    public:
        RenderCommand();
        virtual ~RenderCommand();

    protected:
        void _setRenderer(Renderer* renderer);
    public:
        Renderer* getRenderer() const;
    private:
        Renderer* _renderer;

    public:
        void setVisible(bool visible);
        bool getVisible() const;
    private:
        bool _visible;

    public:
        void setRenderPriorityGroup(int32_t priority);
        int32_t getRenderPriorityGroup() const;
        void setRenderPriorityDepth(float depth);
        float getRenderPriorityDepth() const;
        void setRenderPrioritySceneData(const SceneData* sceneData);
        const SceneData* getRenderPrioritySceneData() const;
        void setRenderPrioritySceneDataDepth(float depth);
        float getRenderPrioritySceneDataDepth() const;
        const RenderPriority& getRenderPriority() const;
    private:
        RenderPriority _renderPriority;
    };
}
