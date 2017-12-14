#pragma once

#include "zay_types.h"
#include "zay_scene_instance.h"

namespace ZAY
{
    class SkeletonBoneRenderInstance
    : public SceneInstance
    {
    public:
        SkeletonBoneRenderInstance();
        virtual ~SkeletonBoneRenderInstance();

    public:
        virtual void _preRender() override;
        virtual void _postRender() override;
        
    protected:
        virtual void _notifyWorldTransformUpdated(bool updated) override;

    protected:
        virtual void _notifyAttached() override;
        
    public:
        void createRenderCommand();
        void releaseRenderCommand();
        void updateRenderCommand();
    public:
        RenderCommand* getRenderCommand() const;
    private:
        bool _needToUpdateRenderCommand;
        RenderCommand* _renderCommand;

    protected:
        virtual void _notifyRenderPriorityGroupChanged() override;
        virtual void _notifyRenderPriorityDepthChanged() override;
        

    };
}
