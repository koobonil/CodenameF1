#pragma once

#include "zay_base.h"
#include "zay_vector3.h"
#include "zay_matrix4.h"
#include "zay_colour_value.h"
#include "zay_scene_instance.h"

namespace ZAY
{
    class Node
    : public Base
    {
    public:
        Node();
        virtual ~Node();
        
    public:
        Node* getParentNode() const;
    protected:
        void _notifyParentNode(Node* parent);
    private:
        Node* _parent;

    public:
        Node* createChild();
        virtual void addChild(Node* child);
        Node* getChild(int32_t index) const;
        int32_t getChildrenCount() const;
        void removeChild(int32_t index);
        void removeChild(Node* child);
        void removeChildrenAll();
        void removeFromParent();
    protected:
        virtual Node* createChildImpl() = 0;
    private:
        std::vector<Node*> _children;

    public:
        void attachObject(SceneInstance* object);
        SceneInstance* getAttachedObject(int32_t index) const;
        int32_t getAttachedObjectsCount() const;
        void detachObject(SceneInstance* object);
        void detachObjectsAll();
    private:
        std::vector<SceneInstance*> _attachedObjects;
        
    public:
        virtual void _updateOneTick();
        virtual void _updateSubTicks(int32_t subTicks);

    protected:
        virtual void _updateRootWorldTransform() = 0;
    public:
        void _setNeedToUpdateWorldTransform();
        void _updateWorldTransformAscending();
        void _updateWorldTransformDescending();
    private:
        void _updateWorldTransform();
    public:
        Vector3 _convertWorldToLocalPosition(const Vector3& worldPosition) const;
        Vector3 _convertLocalToWorldPosition(const Vector3& localPosition) const;
        Vector3 _convertWorldToLocalScale(const Vector3& worldScale) const;
        Vector3 _convertLocalToWorldScale(const Vector3& localScale) const;
        float _convertWorldToLocalRotation(float worldRotation) const;
        float _convertLocalToWorldRotation(float localRotation) const;
    public:
        static Vector3 convertWorldToLocalPosition(float nodeRotation,
                                                   const Vector3& nodeScale,
                                                   const Vector3& nodePosition,
                                                   bool nodeFlipX,
                                                   bool nodeFlipY,
                                                   const Vector3& worldPosition);
        static Vector3 convertLocalToWorldPosition(float nodeRotation,
                                                   const Vector3& nodeScale,
                                                   const Vector3& nodePosition,
                                                   bool nodeFlipX,
                                                   bool nodeFlipY,
                                                   const Vector3& localPosition);
        static Vector3 convertWorldToLocalScale(const Vector3& nodeScale,
                                                const Vector3& worldScale);
        static Vector3 convertLocalToWorldScale(const Vector3& nodeScale,
                                                const Vector3& localScale);
        static float convertWorldToLocalRotation(float nodeRotation,
                                                 float worldRotation);
        static float convertLocalToWorldRotation(float nodeRotation,
                                                 float localRotation);
    public:
        virtual const Matrix4& getWorldTransform() const;
        virtual const Matrix4& getWorldTransformInverse() const;
        float getWorldRotation() const;
        const Vector3& getWorldPosition() const;
        const Vector3& getWorldScale() const;
        bool getWorldFlipX() const;
        bool getWorldFlipY() const;
    private:
        bool _needToUpdateWorldTransform;
        Matrix4 _worldTransform;
        Matrix4 _worldTransformInverse;
        float _worldRotation;
        Vector3 _worldPosition;
        Vector3 _worldScale;
        bool _worldFlipX;
        bool _worldFlipY;

    public:
        virtual void setTextureAtlasWithTextureAtlasSet(TextureAtlasSet* textureAtlasSet);

    public:
        void setPosition(float x, float y, float z = 0.0f);
        void setPosition(const Vector3& position);
        void setScale(float x, float y, float z = 1.0f);
        void setScale(const Vector3& scale);
        void setRotation(float angle);
        void setFlipX(bool flipX);
        void setFlipY(bool flipY);
        void setInheritScale(bool inherit);
        void setInheritRotation(bool inherit);
    public:
        const Vector3& getPosition() const;
        const Vector3& getScale() const;
        float getRotation() const;
        bool getFlipX() const;
        bool getFlipY() const;
        bool getInheritScale() const;
        bool getInheritRotation() const;
    private:
        Vector3 _position;
        Vector3 _scale;
        float _rotation;
        bool _flipX;
        bool _flipY;
        
        bool _inheritScale;
        bool _inheritRotation;

    protected:
        virtual void _updateRootWorldColor() = 0;
    public:
        void _setNeedToUpdateWorldColor();
        void _updateWorldColorAscending();
        void _updateWorldColorDescending();
    protected:
        void _updateWorldColor();
    public:
        const ColourValue& getWorldColor() const;
    private:
        bool _needToUpdateWorldColor;
        ColourValue _worldColor;
        
    public:
        void setLocalColor(const ColourValue& color);
        void setLocalColor(float r, float g, float b, float a);
        void setLocalColorByBound(const ColourValue& color);
        void restoreLocalColorByBound();
        const ColourValue& getLocalColor() const;
    private:
        ColourValue _localColor;
        ColourValue* _savedLocalColor;

    public:
        void setRenderer(Renderer* renderer);
        Renderer* getRenderer() const;
    private:
        Renderer* _renderer;

    public:
        void setTouchEventDispatcher(TouchEventDispatcher* touchEventDispatcher);
        TouchEventDispatcher* getTouchEventDispatcher() const;
    private:
        TouchEventDispatcher* _touchEventDispatcher;

    public:
        void _preRender();
        void _postRender();

    public:
        void doFunctionSelfAndChildren(const std::function<void(Node*)>& function);
    };
}
