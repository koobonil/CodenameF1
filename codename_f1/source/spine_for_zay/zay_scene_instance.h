#pragma once

#include "zay_object_base.h"
#include "zay_colour_value.h"

namespace ZAY
{
    class SceneInstance
    : public ObjectBase
    {
    public:
        SceneInstance();
        virtual ~SceneInstance();

    public:
        void detachFromAttachedNode();
        Node* getAttachedNode() const;
    protected:
        virtual void _notifyAttached();
    public:
        void _setAttachedNode(Node* attachedNode);
    private:
        Node* _attachedNode;

    public:
        virtual void _preRender();
        virtual void _postRender();

    public:
        virtual void setVisible(bool visible);
        bool getVisible() const;
    private:
        bool _visible;

    public:
        void setRenderPriorityGroup(int32_t group);
        int32_t getRenderPriorityGroup() const;
    protected:
        virtual void _notifyRenderPriorityGroupChanged();
    private:
        int32_t _renderPriorityGroup;

    public:
        void setRenderPriorityDepth(float depth);
        float getRenderPriorityDepth() const;
    protected:
        virtual void _notifyRenderPriorityDepthChanged();
    private:
        float _renderPriorityDepth;

    public:
        virtual void _updateOneTick();
        virtual void _updateSubTicks(int32_t subTicks);

    public:
        Renderer* getAttachedRenderer() const;
    public:
        void _setRenderer(Renderer* renderer);
    protected:
        virtual void _notifyAttachedToRenderer();
        virtual void _notifyDetachedFromRenderer();
    private:
        Renderer* _renderer;

    public:
        TouchEventDispatcher* getAttachedTouchEventDispatcher() const;
    public:
        void _setTouchEventDispatcher(TouchEventDispatcher* touchEventDispatcher);
    protected:
        virtual void _notifyAttachedToTouchEventDispatcher();
        virtual void _notifyDetachedFromTouchEventDispatcher();
    private:
        TouchEventDispatcher* _touchEventDispatcher;

    protected:
        void addRenderCommand(RenderCommand* renderCommand, bool usingSceneInstanceDepth = true);
        void removeRenderCommand(RenderCommand* renderCommand);
        void removeRenderCommandsAll();
    public:
        const std::set<RenderCommand*>& getRenderCommands() const;
    private:
        std::set<RenderCommand*> _renderCommands;
        std::set<RenderCommand*> _renderCommandsUsingSceneInstanceDepth;

    public:
        virtual void setTextureAtlasWithTextureAtlasSet(TextureAtlasSet* textureAtlasSet);

    public:
        void _setNeedToUpdateWorldTransform();
    public:
        void _updateWorldTransformAscending();
        void _updateWorldTransformDescending();
    private:
        void _updateWorldTransform();
    protected:
        virtual void _updateWorldTransformToChildren();
    protected:
        virtual void _notifyWorldTransformUpdated(bool updated);
    public:
        const Matrix4& getWorldTransform() const;
        const Matrix4& getWorldTransformInverse() const;
        float getWorldRotation() const;
        const Vector3& getWorldPosition() const;
        const Vector3& getWorldScale() const;
        bool getWorldFlipX() const;
        bool getWorldFlipY() const;
    private:
        bool _needToUpdateWorldTransform;

    public:
        void setLocalColor(const ColourValue& color);
        void setLocalColorMultiply(const ColourValue& color); //bx
        const ColourValue& getLocalColor() const;
    private:
        ColourValue _localColor;

    public:
        void _setNeedToUpdateWorldColor();
    public:
        void _updateWorldColorAscending();
        void _updateWorldColorDescending();
    public:
        void _updateWorldColor();
    protected:
        virtual void _updateWorldColorToChildren();
    protected:
        virtual void _notifyWorldColorUpdated(bool updated);
    public:
        const ColourValue& getWorldColor() const;
    private:
        bool _hasFirstColor;
        bool _needToUpdateWorldColor;
        ColourValue _worldColor;
    };
}
