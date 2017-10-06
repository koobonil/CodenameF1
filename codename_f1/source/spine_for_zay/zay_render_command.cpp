#include "zay_types.h"
#include "zay_render_command.h"
#include "zay_renderer.h"

namespace ZAY
{
    RenderCommand::RenderCommand()
    {
        _renderer = nullptr;
        _visible = true;
    }
    
    RenderCommand::~RenderCommand()
    {
        _setRenderer(nullptr);
    }

    void RenderCommand::_setRenderer(Renderer* renderer)
    {
        if (_renderer != renderer)
        {
            if (_renderer)
            {
                _renderer->_removeRenderCommand(this);
            }
            
            _renderer = renderer;
            
            if (_renderer)
            {
                _renderer->_addRenderCommand(this);
            }
        }
    }

    Renderer* RenderCommand::getRenderer() const
    {
        return _renderer;
    }

    void RenderCommand::setVisible(bool visible)
    {
        _visible = visible;
    }
    
    bool RenderCommand::getVisible() const
    {
        return _visible;
    }

    void RenderCommand::setRenderPriorityGroup(int32_t group)
    {
        if (_renderPriority.getGroup() != group)
        {
            _renderPriority.setGroup(group);

            if (_renderer)
            {
                _renderer->_notifyModifyRenderPriority(this);
            }
        }
    }
    
    int32_t RenderCommand::getRenderPriorityGroup() const
    {
        return _renderPriority.getGroup();
    }
    
    void RenderCommand::setRenderPriorityDepth(float depth)
    {
        if (_renderPriority.getDepth() != depth)
        {
            _renderPriority.setDepth(depth);

            if (_renderer)
            {
                _renderer->_notifyModifyRenderPriority(this);
            }
        }
    }
    
    float RenderCommand::getRenderPriorityDepth() const
    {
        return _renderPriority.getDepth();
    }
    
    void RenderCommand::setRenderPrioritySceneData(const SceneData* sceneData)
    {
        if (_renderPriority.getSceneData() != sceneData)
        {
            _renderPriority.setSceneData(sceneData);
            
            if (_renderer)
            {
                _renderer->_notifyModifyRenderPriority(this);
            }
        }
    }
    
    const SceneData* RenderCommand::getRenderPrioritySceneData() const
    {
        return _renderPriority.getSceneData();
    }
    
    void RenderCommand::setRenderPrioritySceneDataDepth(float depth)
    {
        if (_renderPriority.getSceneDataDepth() != depth)
        {
            _renderPriority.setSceneDataDepth(depth);

            if (_renderer)
            {
                _renderer->_notifyModifyRenderPriority(this);
            }
        }
    }
    
    float RenderCommand::getRenderPrioritySceneDataDepth() const
    {
        return _renderPriority.getSceneDataDepth();
    }

    const RenderPriority& RenderCommand::getRenderPriority() const
    {
        return _renderPriority;
    }
}

