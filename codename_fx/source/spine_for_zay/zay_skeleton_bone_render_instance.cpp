#include "zay_types.h"
#include "zay_skeleton_bone_render_instance.h"
#include "zay_forward_multiply_renderer.h"
#include "zay_render_command.h"
#include "zay_skeleton_bone.h"
#include "zay_skeleton_instance.h"
#include "zay_skeleton_data.h"

namespace ZAY
{
    SkeletonBoneRenderInstance::SkeletonBoneRenderInstance()
    {
        _needToUpdateRenderCommand = false;
        _renderCommand = nullptr;
        
        createRenderCommand();
    }
    

    SkeletonBoneRenderInstance::~SkeletonBoneRenderInstance()
    {
        releaseRenderCommand();
    }

    void SkeletonBoneRenderInstance::_preRender()
    {
        SceneInstance::_preRender();
        
        updateRenderCommand();
    }
    
    void SkeletonBoneRenderInstance::_postRender()
    {
        SceneInstance::_postRender();
    }
        
    void SkeletonBoneRenderInstance::_notifyWorldTransformUpdated(bool updated)
    {
        SceneInstance::_notifyWorldTransformUpdated(updated);
        
        if (updated)
        {
            _needToUpdateRenderCommand = true;
        }
    }

    void SkeletonBoneRenderInstance::_notifyAttached()
    {
        auto node = getAttachedNode();
        
        if (node)
        {
            assert(dynamic_cast<SkeletonBone*>(node));
            auto bone = static_cast<SkeletonBone*>(node);

            _renderCommand->setRenderPrioritySceneData(bone->getParentSkeletonInstance()->getSkeletonData());
        }
    }

    void SkeletonBoneRenderInstance::createRenderCommand()
    {
        if (_renderCommand == nullptr)
        {
            auto renderCommand = new ForwardMultiplyRender::TrianglesMeshRenderCommand();
            _renderCommand = renderCommand;
            _renderCommand->retain();

            renderCommand->getIndicesBuffer().createBuffer(6, true); //bx:초기화
            renderCommand->getVerticesBuffer().createBuffer(4, true); //bx:초기화

            auto indices = renderCommand->getIndicesBuffer().getBufferPointer();

            indices[0] = 0;
            indices[1] = 1;
            indices[2] = 2;
            indices[3] = 0;
            indices[4] = 2;
            indices[5] = 3;

            ColourValue color(0.5f, 0.5f, 0.5f, 0.5f);
            
            auto vertices = renderCommand->getVerticesBuffer().getBufferPointer();
            for (auto i = 0 ; i<4 ; i++)
            {
                vertices[i].vertices = Vector3::ZERO;
                vertices[i].texCoords = Vector2::ZERO;
                vertices[i].colors = color.getAsRGBA();
            }
            
            addRenderCommand(_renderCommand);

            _renderCommand->setRenderPrioritySceneDataDepth(std::numeric_limits<float>::max());
            
            _needToUpdateRenderCommand = true;
        }
    }
    
    void SkeletonBoneRenderInstance::releaseRenderCommand()
    {
        if (_renderCommand != nullptr)
        {
            removeRenderCommand(_renderCommand);
            
            _renderCommand->release();
            _renderCommand = nullptr;
        }
    }
    
    void SkeletonBoneRenderInstance::updateRenderCommand()
    {
        if (_renderCommand)
        {
            assert(dynamic_cast<ForwardMultiplyRender::TrianglesMeshRenderCommand*>(_renderCommand));
            auto renderCommand = static_cast<ForwardMultiplyRender::TrianglesMeshRenderCommand*>(_renderCommand);

            auto node = getAttachedNode();
            assert(dynamic_cast<SkeletonBone*>(node));
            auto bone = static_cast<SkeletonBone*>(node);

            if (_needToUpdateRenderCommand)
            {
                auto _renderer = getAttachedRenderer();
                assert(dynamic_cast<ForwardMultiplyRender::ForwardMultiplyRenderer*>(_renderer));
                auto renderer = static_cast<ForwardMultiplyRender::ForwardMultiplyRenderer*>(_renderer);

                renderCommand->setTexture(renderer->getWhiteTexture());

                float boneSize = 2.0f;

                float minX, maxX;
                float minY, maxY;

                if (bone->getLength() > 0.0f)
                {
                    minX = - boneSize;
                    maxX = bone->getLength() + boneSize;
                }
                else
                {
                    minX = bone->getLength() - boneSize;
                    maxX = + boneSize;
                }
                
                minY = - boneSize;
                maxY = + boneSize;

                Vector3 boneV1 = bone->getWorldTransform() * Vector3(minX, minY, 0.0f);
                Vector3 boneV2 = bone->getWorldTransform() * Vector3(minX, maxY, 0.0f);
                Vector3 boneV3 = bone->getWorldTransform() * Vector3(maxX, maxY, 0.0f);
                Vector3 boneV4 = bone->getWorldTransform() * Vector3(maxX, minY, 0.0f);

                assert(renderCommand->getVerticesBuffer().getBufferSize() == 4);
                
                auto vertices = renderCommand->getVerticesBuffer().getBufferPointer();
                
                vertices[0].vertices = boneV1;
                vertices[1].vertices = boneV2;
                vertices[2].vertices = boneV3;
                vertices[3].vertices = boneV4;

                _needToUpdateRenderCommand = false;
            }
        }
    }

    RenderCommand* SkeletonBoneRenderInstance::getRenderCommand() const
    {
        return _renderCommand;
    }
    
    
    
    void SkeletonBoneRenderInstance::_notifyRenderPriorityGroupChanged()
    {
        SceneInstance::_notifyRenderPriorityGroupChanged();
        
        assert(_renderCommand);
        
        _renderCommand->setRenderPriorityGroup(getRenderPriorityGroup());
    }
    
    void SkeletonBoneRenderInstance::_notifyRenderPriorityDepthChanged()
    {
        SceneInstance::_notifyRenderPriorityDepthChanged();
        
        assert(_renderCommand);
        
        _renderCommand->setRenderPriorityDepth(getRenderPriorityDepth());
    }
}
