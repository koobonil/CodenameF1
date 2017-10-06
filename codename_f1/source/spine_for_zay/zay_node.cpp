#include "zay_types.h"
#include "zay_node.h"
#include "zay_scene_instance.h"
#include "zay_renderer.h"
#include "zay_touch_event_dispatcher.h"
#include "zay_scene_slot_instance.h"
#include "zay_touch_area_instance.h"

namespace ZAY
{
    int USE_EXCLUDED_RED = 0;

    Node::Node()
    {
        _parent = nullptr;

        _needToUpdateWorldTransform = false;
        _worldTransform = Matrix4::IDENTITY;
        _worldTransformInverse = Matrix4::IDENTITY;
        _worldRotation = 0.0f;
        _worldPosition = Vector3::ZERO;
        _worldScale = Vector3::UNIT_SCALE;
        _worldFlipX = false;
        _worldFlipY = false;

        _rotation = 0.0f;
        _position = Vector3::ZERO;
        _scale = Vector3::UNIT_SCALE;
        _flipX = false;
        _flipY = false;
        _inheritScale = true;
        _inheritRotation = true;

        _needToUpdateWorldColor = false;
        _worldColor = ColourValue::White;

        _localColor = ColourValue::White;
        _savedLocalColor = nullptr;

        _renderer = nullptr;
        _touchEventDispatcher = nullptr;
    }

    Node::~Node()
    {
        removeFromParent();
        detachObjectsAll();
        removeChildrenAll();

        delete _savedLocalColor;
        _savedLocalColor = nullptr;

        if (_renderer)
        {
            _renderer->release();
            _renderer = nullptr;
        }

        if (_touchEventDispatcher)
        {
            _touchEventDispatcher->release();
            _touchEventDispatcher = nullptr;
        }
    }

    Node* Node::getParentNode() const
    {
        return _parent;
    }

    void Node::_notifyParentNode(Node* parent)
    {
        if (_parent != parent)
        {
            _parent = parent;

            _needToUpdateWorldTransform = true;
            _needToUpdateWorldColor = true;
        }
    }

    Node* Node::createChild()
    {
        Node* node = createChildImpl();

        if (node)
        {
            addChild(node);
        }

        return node;
    }

    void Node::addChild(Node* child)
    {
        child->_notifyParentNode(this);

        _children.push_back(child);

        child->setRenderer(getRenderer());
    }

    Node* Node::getChild(int32_t index) const
    {
        if (index >= 0 &&
            index < static_cast<int32_t>(_children.size()))
        {
            return _children[index];
        }

        return nullptr;
    }

    int32_t Node::getChildrenCount() const
    {
        return static_cast<int32_t>(_children.size());
    }

    void Node::removeChild(int32_t index)
    {
        if (index >= 0 &&
            index < static_cast<int32_t>(_children.size()))
        {
            auto it = _children.begin();

            std::advance(it, index);

            (*it)->_notifyParentNode(nullptr);
            (*it)->setRenderer(nullptr);

            _children.erase(it);
        }
    }

    void Node::removeChild(Node* child)
    {
        for (auto it = _children.begin() ;
             it != _children.end() ;
             ++it)
        {
            if (*it == child)
            {
                child->_notifyParentNode(nullptr);
                child->setRenderer(nullptr);

                _children.erase(it);
                return;
            }
        }
    }

    void Node::removeChildrenAll()
    {
        for (auto child : _children)
        {
            child->_notifyParentNode(nullptr);
            child->setRenderer(nullptr);
        }
        _children.clear();
    }

    void Node::removeFromParent()
    {
        if (_parent)
        {
            _parent->removeChild(this);
        }
    }

    void Node::attachObject(SceneInstance* object)
    {
        object->_setAttachedNode(this);
        object->_setRenderer(getRenderer());
        object->_setTouchEventDispatcher(getTouchEventDispatcher());
        object->_setNeedToUpdateWorldTransform();
        object->_setNeedToUpdateWorldColor();
        object->retain();

        _attachedObjects.push_back(object);
    }

    SceneInstance* Node::getAttachedObject(int32_t index) const
    {
        if (index >= 0 &&
            index < static_cast<int32_t>(_attachedObjects.size()))
        {
            return _attachedObjects[index];
        }

        return nullptr;
    }

    int32_t Node::getAttachedObjectsCount() const
    {
        return static_cast<int32_t>(_attachedObjects.size());
    }

    void Node::detachObject(SceneInstance* object)
    {
        for (auto it = _attachedObjects.begin() ;
             it != _attachedObjects.end() ;
             ++it)
        {
            if ((*it) == object)
            {
                object->_setAttachedNode(nullptr);
                object->_setRenderer(nullptr);
                object->_setTouchEventDispatcher(nullptr);
                object->release();

                _attachedObjects.erase(it);

                return;
            }
        }
    }

    void Node::detachObjectsAll()
    {
        for (auto attachedObject : _attachedObjects)
        {
            attachedObject->_setAttachedNode(nullptr);
            attachedObject->_setRenderer(nullptr);
            attachedObject->_setTouchEventDispatcher(nullptr);
            attachedObject->release();
        }
        _attachedObjects.clear();
    }

    void Node::_updateOneTick()
    {
        for (auto attachedObject : _attachedObjects)
        {
            attachedObject->_updateOneTick();
        }

        for (auto child : _children)
        {
            child->_updateOneTick();
        }
    }

    void Node::_updateSubTicks(int32_t subTicks)
    {
        for (auto attachedObject : _attachedObjects)
        {
            attachedObject->_updateSubTicks(subTicks);
        }

        for (auto child : _children)
        {
            child->_updateSubTicks(subTicks);
        }
    }

    void Node::_setNeedToUpdateWorldTransform()
    {
        _needToUpdateWorldTransform = true;
    }

    void Node::_updateWorldTransformAscending()
    {
        if (_parent)
        {
            _parent->_updateWorldTransformAscending();
        }
        else
        {
            _updateRootWorldTransform();
        }

        _updateWorldTransform();
    }

    void Node::_updateWorldTransformDescending()
    {
        _updateWorldTransform();

        for (auto child : _children)
        {
            child->_updateWorldTransformDescending();
        }

        for (auto attachedObject : _attachedObjects)
        {
            attachedObject->_updateWorldTransformDescending();
        }
    }

    void Node::_updateWorldTransform()
    {
        if (_needToUpdateWorldTransform)
        {
            if (_parent)
            {
                _worldFlipX = _parent->getWorldFlipX() ^ _flipX;
                _worldFlipY = _parent->getWorldFlipY() ^ _flipY;

                if (_inheritRotation)
                {
                    _worldRotation = _parent->_worldRotation + _rotation;
                }
                else
                {
                    _worldRotation = _rotation;
                }

                if (_inheritScale)
                {
                    _worldScale = _parent->_worldScale * _scale;
                }
                else
                {
                    _worldScale = _scale;
                }

                _worldPosition = _parent->getWorldTransform() * _position;
            }
            else
            {
                _worldFlipX = _flipX;
                _worldFlipY = _flipY;

                _worldRotation = _rotation;

                _worldScale = _scale;

                _worldPosition = _position;
            }

            _worldTransform.makeTransform(_worldPosition, _worldScale, _worldRotation, _worldFlipX, _worldFlipY);
            _worldTransformInverse.makeInverseTransform(_worldPosition, _worldScale, _worldRotation, _worldFlipX, _worldFlipY);





            for (auto child : _children)
            {
                child->_setNeedToUpdateWorldTransform();
            }

            for (auto attachedObject : _attachedObjects)
            {
                attachedObject->_setNeedToUpdateWorldTransform();
            }
        }

        _needToUpdateWorldTransform = false;
    }

    Vector3 Node::_convertWorldToLocalPosition(const Vector3& worldPosition) const
    {
        return convertWorldToLocalPosition(_worldRotation,
                                           _worldScale,
                                           _worldPosition,
                                           _worldFlipX,
                                           _worldFlipY,
                                           worldPosition);
    }

    Vector3 Node::_convertLocalToWorldPosition(const Vector3& localPosition) const
    {
        return convertLocalToWorldPosition(_worldRotation,
                                           _worldScale,
                                           _worldPosition,
                                           _worldFlipX,
                                           _worldFlipY,
                                           localPosition);
    }

    Vector3 Node::_convertWorldToLocalScale(const Vector3& worldScale) const
    {
        return convertWorldToLocalScale(_worldScale,
                                        worldScale);
    }

    Vector3 Node::_convertLocalToWorldScale(const Vector3& localScale) const
    {
        return convertLocalToWorldScale(_worldScale,
                                        localScale);
    }

    float Node::_convertWorldToLocalRotation(float worldRotation) const
    {
        return convertWorldToLocalRotation(_worldRotation, worldRotation);
    }

    float Node::_convertLocalToWorldRotation(float localRotation) const
    {
        return convertLocalToWorldRotation(_worldRotation, localRotation);
    }

    Vector3 Node::convertWorldToLocalPosition(float nodeRotation,
                                              const Vector3& nodeScale,
                                              const Vector3& nodePosition,
                                              bool nodeFlipX,
                                              bool nodeFlipY,
                                              const Vector3& worldPosition)
    {
        Matrix4 mat;

        mat.makeInverseTransform(nodePosition, nodeScale, nodeRotation, nodeFlipX, nodeFlipY);

        return mat * worldPosition;
    }

    Vector3 Node::convertLocalToWorldPosition(float nodeRotation,
                                              const Vector3& nodeScale,
                                              const Vector3& nodePosition,
                                              bool nodeFlipX,
                                              bool nodeFlipY,
                                              const Vector3& localPosition)
    {
        Matrix4 mat;

        mat.makeTransform(nodePosition, nodeScale, nodeRotation, nodeFlipX, nodeFlipY);

        return mat * localPosition;
    }

    Vector3 Node::convertWorldToLocalScale(const Vector3& nodeScale,
                                           const Vector3& worldScale)
    {
        return worldScale / nodeScale;
    }

    Vector3 Node::convertLocalToWorldScale(const Vector3& nodeScale,
                                           const Vector3& localScale)
    {
        return nodeScale * localScale;
    }

    float Node::convertWorldToLocalRotation(float nodeRotation,
                                            float worldRotation)
    {
        return worldRotation - nodeRotation;
    }

    float Node::convertLocalToWorldRotation(float nodeRotation,
                                            float localRotation)
    {
        return nodeRotation + localRotation;
    }

    const Matrix4& Node::getWorldTransform() const
    {
        return _worldTransform;
    }

    const Matrix4& Node::getWorldTransformInverse() const
    {
        return _worldTransformInverse;
    }

    float Node::getWorldRotation() const
    {
        return _worldRotation;
    }

    const Vector3& Node::getWorldPosition() const
    {
        return _worldPosition;
    }

    const Vector3& Node::getWorldScale() const
    {
        return _worldScale;
    }

    bool Node::getWorldFlipX() const
    {
        return _worldFlipX;
    }

    bool Node::getWorldFlipY() const
    {
        return _worldFlipY;
    }

    void Node::setTextureAtlasWithTextureAtlasSet(TextureAtlasSet* textureAtlasSet)
    {
        for (auto child : _children)
        {
            child->setTextureAtlasWithTextureAtlasSet(textureAtlasSet);
        }

        for (auto attachedObject : _attachedObjects)
        {
            attachedObject->setTextureAtlasWithTextureAtlasSet(textureAtlasSet);
        }
    }

    void Node::setPosition(float x, float y, float z)
    {
        setPosition(Vector3(x, y, z));
    }

    void Node::setPosition(const Vector3& position)
    {
        if (_position != position)
        {
            _position = position;
            _needToUpdateWorldTransform = true;
        }
    }

    void Node::setScale(float x, float y, float z)
    {
        setScale(Vector3(x, y, z));
    }

    void Node::setScale(const Vector3& scale)
    {
        if (_scale != scale)
        {
            _scale = scale;
            _needToUpdateWorldTransform = true;
        }
    }

    void Node::setRotation(float rotation)
    {
        if (_rotation != rotation)
        {
            _rotation = rotation;
            _needToUpdateWorldTransform = true;
        }
    }

    void Node::setFlipX(bool flipX)
    {
        if (_flipX != flipX)
        {
            _flipX = flipX;
            _needToUpdateWorldTransform = true;
        }
    }

    void Node::setFlipY(bool flipY)
    {
        if (_flipY != flipY)
        {
            _flipY = flipY;
            _needToUpdateWorldTransform = true;
        }
    }

    void Node::setInheritScale(bool inherit)
    {
        if (_inheritScale != inherit)
        {
            _inheritScale = inherit;
            _needToUpdateWorldTransform = true;
        }
    }

    void Node::setInheritRotation(bool inherit)
    {
        if (_inheritRotation != inherit)
        {
            _inheritRotation = inherit;
            _needToUpdateWorldTransform = true;
        }
    }

    const Vector3& Node::getPosition() const
    {
        return _position;
    }

    const Vector3& Node::getScale() const
    {
        return _scale;
    }

    float Node::getRotation() const
    {
        return _rotation;
    }

    bool Node::getFlipX() const
    {
        return _flipX;
    }

    bool Node::getFlipY() const
    {
        return _flipY;
    }

    bool Node::getInheritScale() const
    {
        return _inheritScale;
    }

    bool Node::getInheritRotation() const
    {
        return _inheritRotation;
    }

    void Node::_setNeedToUpdateWorldColor()
    {
        _needToUpdateWorldColor = true;
    }

    void Node::_updateWorldColorAscending()
    {
        if (_parent)
        {
            _parent->_updateWorldColorAscending();
        }
        else
        {
            _updateRootWorldColor();
        }

        _updateWorldColor();
    }

    void Node::_updateWorldColorDescending()
    {
        _updateWorldColor();

        for (auto child : _children)
        {
            child->_updateWorldColorDescending();
        }

        bool FoundRedBound = false;
        for (auto attachedObject : _attachedObjects)
        {
            attachedObject->_updateWorldColorDescending();

            //bx1111: 바운딩박스가 빨간색이 아니면 보여짐
            if(USE_EXCLUDED_RED && !FoundRedBound)
            {
                const SceneSlotInstance* CurSceneSlotInstance = dynamic_cast<const SceneSlotInstance*>(attachedObject);
                if(CurSceneSlotInstance && dynamic_cast<const TouchAreaInstance*>(CurSceneSlotInstance->getCurrentSceneInstance()))
                {
                    const ColourValue& Color = CurSceneSlotInstance->getLocalColor();
                    if(Color.r == 1 && Color.g == 0 && Color.b == 0)
                        FoundRedBound = true;
                }
            }
        }

        //bx1111: 빨간색을 전달
        if(USE_EXCLUDED_RED)
        {
            if(FoundRedBound)
                setLocalColorByBound(ColourValue(1, 0, 0, 1));
            else restoreLocalColorByBound();
        }
    }

    void Node::_updateWorldColor()
    {
        if (_needToUpdateWorldColor)
        {
            if (_parent)
            {
                _worldColor = _parent->getWorldColor() * _localColor;
            }
            else
            {
                _worldColor = _localColor;
            }

            for (auto child : _children)
            {
                child->_setNeedToUpdateWorldColor();
            }

            for (auto attachedObject : _attachedObjects)
            {
                attachedObject->_setNeedToUpdateWorldColor();
            }

            _needToUpdateWorldColor = false;
        }
    }

    const ColourValue& Node::getWorldColor() const
    {
        return _worldColor;
    }

    void Node::setLocalColor(const ColourValue& color)
    {
        delete _savedLocalColor;
        _savedLocalColor = nullptr;

        if (_localColor != color)
        {
            _localColor = color;

            _needToUpdateWorldColor = true;
        }
    }

    void Node::setLocalColor(float r, float g, float b, float a)
    {
        delete _savedLocalColor;
        _savedLocalColor = nullptr;

        if (_localColor.r != r ||
            _localColor.g != g ||
            _localColor.b != b ||
            _localColor.a != a)
        {
            _localColor.r = r;
            _localColor.g = g;
            _localColor.b = b;
            _localColor.a = a;

            _needToUpdateWorldColor = true;
        }
    }

    void Node::setLocalColorByBound(const ColourValue& color)
    {
        if(!_savedLocalColor)
            _savedLocalColor = new ColourValue(_localColor);

        if (_localColor != color)
        {
            _localColor = color;

            _needToUpdateWorldColor = true;
        }
    }

    void Node::restoreLocalColorByBound()
    {
        if(_savedLocalColor)
        {
            if (_localColor != *_savedLocalColor)
            {
                _localColor = *_savedLocalColor;

                _needToUpdateWorldColor = true;
            }
        }
    }

    const ColourValue& Node::getLocalColor() const
    {
        return _localColor;
    }

    void Node::setRenderer(Renderer* renderer)
    {
        if (_renderer != renderer)
        {
            if (_renderer)
            {
                _renderer->release();
            }

            _renderer = renderer;

            if (_renderer)
            {
                _renderer->retain();
            }

            for (auto attachedObject : _attachedObjects)
            {
                attachedObject->_setRenderer(_renderer);
            }
        }

        for (auto child : _children)
        {
            child->setRenderer(renderer);
        }
    }

    Renderer* Node::getRenderer() const
    {
        return _renderer;
    }

    void Node::setTouchEventDispatcher(TouchEventDispatcher* touchEventDispatcher)
    {
        if (_touchEventDispatcher != touchEventDispatcher)
        {
            if (_touchEventDispatcher)
            {
                _touchEventDispatcher->release();
            }

            _touchEventDispatcher = touchEventDispatcher;

            if (_touchEventDispatcher)
            {
                _touchEventDispatcher->release();
            }

            for (auto attachedObject : _attachedObjects)
            {
                attachedObject->_setTouchEventDispatcher(_touchEventDispatcher);
            }
        }

        for (auto child : _children)
        {
            child->setTouchEventDispatcher(touchEventDispatcher);
        }
    }

    TouchEventDispatcher* Node::getTouchEventDispatcher() const
    {
        return _touchEventDispatcher;
    }

    void Node::_preRender()
    {
        for (auto child : _children)
        {
            child->_preRender();
        }

        for (auto attachedObject : _attachedObjects)
        {
            attachedObject->_preRender();
        }
    }

    void Node::_postRender()
    {
        for (auto child : _children)
        {
            child->_postRender();
        }

        for (auto attachedObject : _attachedObjects)
        {
            attachedObject->_postRender();
        }
    }

    void Node::doFunctionSelfAndChildren(const std::function<void(Node*)>& function)
    {
        if (function)
        {
            function(this);

            for (auto child : _children)
            {
                child->doFunctionSelfAndChildren(function);
            }
        }
    }






}

