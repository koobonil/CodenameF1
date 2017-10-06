#include "zay_types.h"
#include "zay_scene_instance.h"
#include "zay_scene_node.h"
#include "zay_renderer.h"
#include "zay_render_command.h"

namespace ZAY
{
    SceneInstance::SceneInstance()
    {
        _attachedNode = nullptr;
        _visible = true;
        _renderPriorityGroup = 0;
        _renderPriorityDepth = 0.0f;
        _renderer = nullptr;
        _touchEventDispatcher = nullptr;
        _needToUpdateWorldTransform = false;
        _localColor = ColourValue::White;
        _hasFirstColor = false; //bx
        _needToUpdateWorldColor = false;
        _worldColor = ColourValue::White;
    }

    SceneInstance::~SceneInstance()
    {
    }

    void SceneInstance::detachFromAttachedNode()
    {
        if (_attachedNode)
        {
            _attachedNode->detachObject(this);
        }
    }

    Node* SceneInstance::getAttachedNode() const
    {
        return _attachedNode;
    }

    void SceneInstance::_notifyAttached()
    {
    }
    
    void SceneInstance::_setAttachedNode(Node* attachedNode)
    {
        assert(((_attachedNode == nullptr) && (attachedNode != nullptr)) ||
               ((_attachedNode != nullptr) && (attachedNode == nullptr)));

        if (_attachedNode != attachedNode)
        {
            _attachedNode = attachedNode;

            _needToUpdateWorldColor = true;
            _needToUpdateWorldTransform = true;
            
            _notifyAttached();
        }
    }

    void SceneInstance::_preRender()
    {
        
    }
    
    void SceneInstance::_postRender()
    {
        
    }

    void SceneInstance::setVisible(bool visible)
    {
        if (_visible != visible)
        {
            _visible = visible;

            for (auto renderCommand : _renderCommands)
            {
                renderCommand->setVisible(visible);
            }
        }
    }
    
    bool SceneInstance::getVisible() const
    {
        return _visible;
    }
    
    void SceneInstance::setRenderPriorityGroup(int32_t group)
    {
        if (_renderPriorityGroup != group)
        {
            _renderPriorityGroup = group;

            for (auto renderCommand : _renderCommands)
            {
                renderCommand->setRenderPriorityGroup(_renderPriorityGroup);
            }
            
            _notifyRenderPriorityGroupChanged();
        }
    }
    
    int32_t SceneInstance::getRenderPriorityGroup() const
    {
        return _renderPriorityGroup;
    }
    
    void SceneInstance::_notifyRenderPriorityGroupChanged()
    {
    }

    void SceneInstance::setRenderPriorityDepth(float depth)
    {
        if (_renderPriorityDepth != depth)
        {
            _renderPriorityDepth = depth;

            for (auto renderCommand : _renderCommandsUsingSceneInstanceDepth)
            {
                renderCommand->setRenderPriorityDepth(_renderPriorityDepth);
            }

            _notifyRenderPriorityDepthChanged();
        }
    }
    
    float SceneInstance::getRenderPriorityDepth() const
    {
        return _renderPriorityDepth;
    }

    void SceneInstance::_notifyRenderPriorityDepthChanged()
    {
    }

    
    
    

    void SceneInstance::_updateOneTick()
    {
    }

    void SceneInstance::_updateSubTicks(int32_t subTicks)
    {
    }

    Renderer* SceneInstance::getAttachedRenderer() const
    {
        return _renderer;
    }

    void SceneInstance::_setRenderer(Renderer* renderer)
    {
        if (_renderer != renderer)
        {
            if (_renderer)
            {
                for (auto renderCommand : _renderCommands)
                {
                    renderCommand->_setRenderer(nullptr);
                }

                _notifyDetachedFromRenderer();
            }
            
            _renderer = renderer;
            
            if (_renderer)
            {
                for (auto renderCommand : _renderCommands)
                {
                    renderCommand->_setRenderer(_renderer);
                }
                
                _notifyAttachedToRenderer();
            }
        }
    }

    void SceneInstance::_notifyAttachedToRenderer()
    {
    }
    
    void SceneInstance::_notifyDetachedFromRenderer()
    {
    }

    
    
    
    
    TouchEventDispatcher* SceneInstance::getAttachedTouchEventDispatcher() const
    {
        return _touchEventDispatcher;
    }
    
    void SceneInstance::_setTouchEventDispatcher(TouchEventDispatcher* touchEventDispatcher)
    {
        if (_touchEventDispatcher != touchEventDispatcher)
        {
            if (_touchEventDispatcher)
            {
                _notifyDetachedFromRenderer();
            }

            _touchEventDispatcher = touchEventDispatcher;

            if (_touchEventDispatcher)
            {
                _notifyAttachedToRenderer();
            }
        }
    }
    
    void SceneInstance::_notifyAttachedToTouchEventDispatcher()
    {
    }
    
    void SceneInstance::_notifyDetachedFromTouchEventDispatcher()
    {
    }

    
    
    
    
    void SceneInstance::addRenderCommand(RenderCommand* renderCommand, bool usingSceneInstanceDepth)
    {
        if (_renderCommands.find(renderCommand) == _renderCommands.end())
        {
            _renderCommands.insert(renderCommand);
            renderCommand->retain();

            if (usingSceneInstanceDepth)
            {
                _renderCommandsUsingSceneInstanceDepth.insert(renderCommand);
                renderCommand->retain();
            }
            
            if (_renderer)
            {
                renderCommand->_setRenderer(_renderer);
            }

            renderCommand->setVisible(_visible);
        }
    }
    
    void SceneInstance::removeRenderCommand(RenderCommand* renderCommand)
    {
        auto it = _renderCommands.find(renderCommand);
        
        if (it != _renderCommands.end())
        {
            renderCommand->_setRenderer(nullptr);
            
            renderCommand->release();
            
            _renderCommands.erase(it);
        }
        
        auto it2 = _renderCommandsUsingSceneInstanceDepth.find(renderCommand);
        
        if (it2 != _renderCommandsUsingSceneInstanceDepth.end())
        {
            renderCommand->release();

            _renderCommandsUsingSceneInstanceDepth.erase(it2);
        }
    }
    
    void SceneInstance::removeRenderCommandsAll()
    {
        if (_renderer)
        {
            for (auto renderCommand : _renderCommands)
            {
                renderCommand->_setRenderer(_renderer);
            }
        }
        
        for (auto renderCommand : _renderCommands)
        {
            renderCommand->release();
        }

        _renderCommands.clear();
        
        for (auto renderCommand : _renderCommandsUsingSceneInstanceDepth)
        {
            renderCommand->release();
        }
        _renderCommandsUsingSceneInstanceDepth.clear();
    }
    
    const std::set<RenderCommand*>& SceneInstance::getRenderCommands() const
    {
        return _renderCommands;
    }
    
    void SceneInstance::setTextureAtlasWithTextureAtlasSet(TextureAtlasSet* textureAtlasSet)
    {
    }

    void SceneInstance::_setNeedToUpdateWorldTransform()
    {
        _needToUpdateWorldTransform = true;
    }

    void SceneInstance::_updateWorldTransformAscending()
    {
        auto attachedNode = getAttachedNode();
        
        if (attachedNode)
        {
            attachedNode->_updateWorldTransformAscending();
        
            _updateWorldTransform();
        }
    }
    
    void SceneInstance::_updateWorldTransformDescending()
    {
        _updateWorldTransform();

        _updateWorldTransformToChildren();
    }

    void SceneInstance::_updateWorldTransform()
    {
        bool updated = _needToUpdateWorldTransform;
        
        if (_needToUpdateWorldTransform)
        {
            auto attachedNode = getAttachedNode();

            if (attachedNode)
            {
                // TODO
                // 경우의 수를 복잡하게 가야 될 수도 있음.
                // 지금은 일단 SceneNode의 z 값만 렌더 순서에 영향을 미침
                if (dynamic_cast<SceneNode*>(attachedNode))
                {
                    setRenderPriorityDepth(attachedNode->getWorldPosition().z);
                }
            }

            _needToUpdateWorldTransform = false;
        }

        _notifyWorldTransformUpdated(updated);
    }

    void SceneInstance::_updateWorldTransformToChildren()
    {
    }

    void SceneInstance::_notifyWorldTransformUpdated(bool updated)
    {
    }

    const Matrix4& SceneInstance::getWorldTransform() const
    {
        auto attachedNode = getAttachedNode();
        
        if (attachedNode)
        {
            return attachedNode->getWorldTransform();
        }
        else
        {
            return Matrix4::IDENTITY;
        }
    }
    
    const Matrix4& SceneInstance::getWorldTransformInverse() const
    {
        auto attachedNode = getAttachedNode();
        
        if (attachedNode)
        {
            return attachedNode->getWorldTransformInverse();
        }
        else
        {
            return Matrix4::IDENTITY;
        }
    }
    
    float SceneInstance::getWorldRotation() const
    {
        auto attachedNode = getAttachedNode();
        
        if (attachedNode)
        {
            return attachedNode->getWorldRotation();
        }
        else
        {
            return 0.0f;
        }
    }
    
    const Vector3& SceneInstance::getWorldPosition() const
    {
        auto attachedNode = getAttachedNode();
        
        if (attachedNode)
        {
            return attachedNode->getWorldPosition();
        }
        else
        {
            return Vector3::ZERO;
        }
    }
    
    const Vector3& SceneInstance::getWorldScale() const
    {
        auto attachedNode = getAttachedNode();
        
        if (attachedNode)
        {
            return attachedNode->getWorldScale();
        }
        else
        {
            return Vector3::UNIT_SCALE;
        }
    }

    bool SceneInstance::getWorldFlipX() const
    {
        auto attachedNode = getAttachedNode();
        
        if (attachedNode)
        {
            return attachedNode->getWorldFlipX();
        }
        else
        {
            return false;
        }
    }
    
    bool SceneInstance::getWorldFlipY() const
    {
        auto attachedNode = getAttachedNode();
        
        if (attachedNode)
        {
            return attachedNode->getWorldFlipY();
        }
        else
        {
            return false;
        }
    }

    void SceneInstance::setLocalColor(const ColourValue& color)
    {
        _hasFirstColor = true; //bx

        if (_localColor != color)
        {
            _localColor = color;
            _needToUpdateWorldColor = true;
        }
    }

    void SceneInstance::setLocalColorMultiply(const ColourValue& color) //bx
    {
        ColourValue NewColor = (_hasFirstColor)? color : _localColor * color;
        _hasFirstColor = false;

        if(_localColor != NewColor)
        {
            _localColor = NewColor;
            _needToUpdateWorldColor = true;
        }
    }
    
    const ColourValue& SceneInstance::getLocalColor() const
    {
        return _localColor;
    }

    void SceneInstance::_setNeedToUpdateWorldColor()
    {
        _needToUpdateWorldColor = true;
    }

    void SceneInstance::_updateWorldColorAscending()
    {
        auto attachedNode = getAttachedNode();

        if (attachedNode)
        {
            attachedNode->_updateWorldColorAscending();

            _updateWorldColor();
        }
    }
    
    void SceneInstance::_updateWorldColorDescending()
    {
        _updateWorldColor();
        
        _updateWorldColorToChildren();
    }

    void SceneInstance::_updateWorldColor()
    {
        bool updated = _needToUpdateWorldColor;
        
        if (_needToUpdateWorldColor)
        {
            auto attachedNode = getAttachedNode();
            
            if (attachedNode)
            {
                _worldColor = attachedNode->getWorldColor() * _localColor;
            }
            else
            {
                _worldColor = _localColor;
            }

            _needToUpdateWorldColor = false;
        }
        
        _notifyWorldColorUpdated(updated);
    }

    void SceneInstance::_updateWorldColorToChildren()
    {
    }

    void SceneInstance::_notifyWorldColorUpdated(bool updated)
    {
    }

    const ColourValue& SceneInstance::getWorldColor() const
    {
        return _worldColor;
    }
}


