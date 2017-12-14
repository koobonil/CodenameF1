#pragma once

#include "zay_types.h"
#include "zay_scene_instance.h"
#include "zay_animatable.h"
#include "zay_array_buffer.h"

namespace ZAY
{
    class MeshInstance
    : public SceneInstance
    , public Animatable
    {
    public:
        MeshInstance();
        virtual ~MeshInstance();
    
    public:
        virtual void _preRender() override;
        virtual void _postRender() override;
        
    public:
        virtual void applyInitialAnimationPose(bool inheritChild, bool colorUpdateOnly) override; //bx
        virtual void applyAnimationStateSet(AnimationStateSet* animationStateSet, bool inheritChild, bool colorUpdateOnly) override; //bx

    protected:
        virtual void _notifyWorldTransformUpdated(bool updated) override;
        
    protected:
        virtual void _notifyWorldColorUpdated(bool updated) override;

    public:
        void _notifyVertexPositionChanged();
        
    public:
        void setMeshData(MeshData* meshData);
        MeshData* getMeshData() const;
    private:
        MeshData* _meshData;

    public:
        virtual void setTextureAtlasWithTextureAtlasSet(TextureAtlasSet* textureAtlasSet) override;
    public:
        void setTextureAtlas(TextureAtlas* textureAtlas);
        TextureAtlas* getTextureAtlas() const;
    private:
        TextureAtlas* _textureAtlas;

    public:
        void setBlendMode(ForwardMultiplyRender::BlendType blendType);
        ForwardMultiplyRender::BlendType getBlendMode() const;
    private:
        ForwardMultiplyRender::BlendType _blendType;

    public:
        void createRenderCommand();
        void releaseRenderCommand();
        void updateRenderCommand();
    public:
        RenderCommand* getRenderCommand() const;
    private:
        bool _needToUpdateRenderCommandVertexPositions;
        bool _needToUpdateRenderCommandVertexUVs;
        bool _needToUpdateRenderCommandVertexColors;
        bool _needToUpdateRenderCommandIndices;
        bool _needToUpdateRenderCommandTextures;
        RenderCommand* _renderCommand;

        
        
        // skinning 처리
    public:
        // TODO
        // 스키닝 되면
        // parent bone 의 world transform 을 쓰지 않고,
        // skeleton object 의 world transform 을 쓰되,
        // skeleton object 의 여러 bone 의 world transform 을 이용해서
        // position 을 설정해야 함.
        void setParentSkeletonInstance(SkeletonInstance* parentSkeletonInstance);
        void setParentSkeletonBone(SkeletonBone* parentSkeletonBone);
    private:
        bool _skinning;
        SkeletonInstance* _parentSkeletonInstance;
        SkeletonBone* _parentSkeletonBone;


        
    public:
        const ArrayBuffer<Vector3>& getAnimatedPositions() const;
        const ArrayBuffer<Vector2>& getAnimatedUVs() const;
        const ArrayBuffer<ColourValue>& getAnimatedColors() const;
    public:
        ArrayBuffer<Vector3>& _getAnimatedPositions();
        ArrayBuffer<Vector2>& _getAnimatedUVs();
        ArrayBuffer<ColourValue>& _getAnimatedColors();
    private:
        ArrayBuffer<Vector3> _meshAnimatedPositions;
        ArrayBuffer<Vector2> _meshAnimatedUVs;
        ArrayBuffer<ColourValue> _meshAnimatedColors;
    };
}
