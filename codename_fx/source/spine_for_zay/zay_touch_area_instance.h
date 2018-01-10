#pragma once

#include "zay_scene_instance.h"
#include "zay_render_priority.h"

namespace ZAY
{
    class TouchAreaInstance
    : public SceneInstance
    {
    public:
        TouchAreaInstance();
        virtual ~TouchAreaInstance();

    public:
        virtual void _preRender() override; // bx
        virtual void _postRender() override; // bx

    public:
        void setLocalVertices(std::string name, float* vertices, int32_t verticesCount);
    private:
        std::string _name;
        float* _localVertciesXY;
        float* _worldVertciesXY;
        float* _bossVertciesXY; //bx1116
        int32_t _vertciesCount;
        bool _worldVerticesValid;
        bool _bossVerticesValid; //bx1116

    public:
        void updateBoundBox(); //bx
        void renderBoundBox(int r, int g, int b, int a) const; //bx

    public:
        void setEnabled(bool flag);
        bool getEnabled() const;
    private:
        bool _enabled;

    public:
        void setEnabledForController(bool flag);
        bool getEnabledForController() const;
    private:
        bool _enabledForController;

    public:
        virtual void _updateOneTick() override;
        virtual void _updateSubTicks(int32_t subTicks) override;

    public:
        bool containsPoint(const Vector2& point) const;

    protected:
        virtual void _notifyRenderPriorityGroupChanged() override;
        virtual void _notifyRenderPriorityDepthChanged() override;
    public:
        void setRenderPrioritySceneData(const SceneData* sceneData);
        const SceneData* getRenderPrioritySceneData() const;
        void setRenderPrioritySceneDataDepth(float depth);
        float getRenderPrioritySceneDataDepth() const;
    public:
        const RenderPriority& getRenderPriority() const;
    private:
        RenderPriority _renderPriority;

    public:
        std::function<bool(TouchEvent*)> onTouchBegan;
        std::function<void(TouchEvent*)> onTouchMoved;
        std::function<void(TouchEvent*)> onTouchEnded;
        std::function<void(TouchEvent*)> onTouchCancelled;
    };
}
