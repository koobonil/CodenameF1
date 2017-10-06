#include "zay_types.h"
#include "zay_mesh_instance.h"
#include "zay_render_command.h"
#include "zay_mesh_data.h"
#include "zay_texture_atlas.h"
#include "zay_texture_atlas_set.h"
#include "zay_animation.h"
#include "zay_animation_track.h"
#include "zay_renderer.h"
#include "zay_animation_state_set.h"
#include "zay_forward_multiply_renderer.h"
#include "zay_texture2d.h"
#include "zay_skeleton_instance.h"
#include "zay_skeleton_bone.h"

namespace ZAY
{
    extern int USE_EXCLUDED_RED;

    MeshInstance::MeshInstance()
    {
        _textureAtlas = nullptr;
        _blendType = ForwardMultiplyRender::BlendType::Alpha;

        _meshData = nullptr;

        _renderCommand = nullptr;

        _skinning = false;
        _parentSkeletonInstance = nullptr;
        _parentSkeletonBone = nullptr;
    }

    MeshInstance::~MeshInstance()
    {
        releaseRenderCommand();

        if (_textureAtlas)
        {
            _textureAtlas->release();
            _textureAtlas = nullptr;
        }

        if (_meshData)
        {
            _meshData->release();
            _meshData = nullptr;
        }
    }

    void MeshInstance::_preRender()
    {
        SceneInstance::_preRender();

        updateRenderCommand();
    }

    void MeshInstance::_postRender()
    {
        SceneInstance::_postRender();
    }

    void MeshInstance::applyInitialAnimationPose(bool inheritChild, bool colorUpdateOnly) //bx
    {
        if (_meshData)
        {
            if (!colorUpdateOnly)
            {
                if(_meshAnimatedPositions.getBufferSize() > 0)
                {
                    _needToUpdateRenderCommandVertexPositions = true;

                    auto ptr = _meshAnimatedPositions.getBufferPointer();
                    auto count = _meshAnimatedPositions.getBufferSize();

                    memset(ptr, 0x00, sizeof(Vector3) * count);
                }
                else if(0 < _meshData->getSkinningOriginalVertexCount()) //bx: 자기정점은 없지만 원본정점이 있는 경우 업데이트필요
                    _needToUpdateRenderCommandVertexPositions = true;

                if (_meshAnimatedUVs.getBufferSize() > 0)
                {
                    _needToUpdateRenderCommandVertexUVs = true;

                    auto ptr = _meshAnimatedUVs.getBufferPointer();
                    auto count = _meshAnimatedUVs.getBufferSize();

                    memset(ptr, 0x00, sizeof(Vector2) * count);
                }
            }

            if (_meshAnimatedColors.getBufferSize() > 0)
            {
                _needToUpdateRenderCommandVertexColors = true;

                auto ptr = _meshAnimatedColors.getBufferPointer();
                auto count = _meshAnimatedColors.getBufferSize();

                auto srcPtr = _meshData->getVertexColors().getBufferPointer();
                auto srcCount = _meshData->getVertexColors().getBufferSize();

                if (ptr != srcPtr &&
                    count == srcCount)
                {
                    memcpy(ptr, srcPtr, sizeof(ColourValue) * count);
                }
            }
        }
    }

    void MeshInstance::applyAnimationStateSet(AnimationStateSet* animationStateSet, bool inheritChild, bool colorUpdateOnly) //bx
    {
        if (_meshData)
        {
            bool needToSetInitState = false;
            for (const auto& it : animationStateSet->getAnimationStates())
            {
                if (it.second->getEnabled())
                {
                    auto animation = _meshData->getAnimation(it.first);

                    if (animation)
                    {
                        auto animationState = it.second;

                        if (animationState->getNeedToSetInitState())
                        {
                            needToSetInitState = true;
                        }
                    }
                }
            }

            if (needToSetInitState)
            {
                applyInitialAnimationPose(inheritChild, colorUpdateOnly);
            }

            for (const auto& it : animationStateSet->getAnimationStates())
            {
                if (it.second->getEnabled())
                {
                    auto animation = _meshData->getAnimation(it.first);

                    if (animation)
                    {
                        auto animationState = it.second;

                        animation->applyToAnimatable(this, animationState->getCurrentTime(), colorUpdateOnly);
                    }
                }
            }
        }
    }

    void MeshInstance::_notifyWorldTransformUpdated(bool updated)
    {
        SceneInstance::_notifyWorldTransformUpdated(updated);

        if (updated)
        {
            _needToUpdateRenderCommandVertexPositions = true;
        }
    }

    void MeshInstance::_notifyWorldColorUpdated(bool updated)
    {
        SceneInstance::_notifyWorldColorUpdated(updated);

        if (updated)
        {
            _needToUpdateRenderCommandVertexColors = true;
        }
    }

    void MeshInstance::_notifyVertexPositionChanged()
    {
        _needToUpdateRenderCommandVertexPositions = true;
    }

    void MeshInstance::setMeshData(MeshData* meshData)
    {
        if (_meshData != meshData)
        {
            if (_meshData)
            {
                _meshData->release();
            }

            _meshData = meshData;

            _needToUpdateRenderCommandVertexPositions = true;
            _needToUpdateRenderCommandVertexUVs = true;
            _needToUpdateRenderCommandVertexColors = true;
            _needToUpdateRenderCommandIndices = true;
            _needToUpdateRenderCommandTextures = true;

            if (_meshData)
            {
                _meshData->retain();

                createRenderCommand();
                setBlendMode(_meshData->getBlendMode());

                if (_meshData->getSkinningData())
                {
                    _skinning = true;

                    assert(dynamic_cast<ForwardMultiplyRender::TrianglesMeshRenderCommand*>(_renderCommand));
                    auto renderCommand = static_cast<ForwardMultiplyRender::TrianglesMeshRenderCommand*>(_renderCommand);

                    auto skinningVertexCount = _meshData->getSkinningVertexCount();
                    auto skinningOriginalVertexCount = _meshData->getSkinningOriginalVertexCount();

                    renderCommand->getVerticesBuffer().createBuffer(skinningOriginalVertexCount);
                    renderCommand->getIndicesBuffer().createBuffer(_meshData->getIndices().getBufferSize());

                    bool foundVertexPositionAnimation = false;
                    bool foundVertexUVAnimation = false;
                    bool foundVertexColorAnimation = false;

                    for (const auto& it : _meshData->getAnimationsAll())
                    {
                        auto animation = it.second;

                        for (auto animationTrack : animation->getAnimationTracksAll())
                        {
                            if (animationTrack->getTypeName)
                            {
                                if (animationTrack->getTypeName() == "AnimationTrack_MeshInstanceVertexPosition")
                                {
                                    foundVertexPositionAnimation = true;
                                }
                                else if (animationTrack->getTypeName() == "AnimationTrack_MeshInstanceVertexUV")
                                {
                                    foundVertexUVAnimation = true;
                                }
                                else if (animationTrack->getTypeName() == "AnimationTrack_MeshInstanceVertexColor")
                                {
                                    foundVertexColorAnimation = true;
                                }
                            }
                        }
                    }

                    if (foundVertexPositionAnimation)
                    {
                        _meshAnimatedPositions.createBuffer(skinningVertexCount);
                        memset(_meshAnimatedPositions.getBufferPointer(),
                               0x00,
                               skinningVertexCount * sizeof(Vector3));
                    }

                    if (foundVertexUVAnimation)
                    {
                        _meshAnimatedUVs.createBuffer(_meshData->getVertexUVs().getBufferSize());
                        memcpy(_meshAnimatedUVs.getBufferPointer(),
                               _meshData->getVertexUVs().getBufferPointer(),
                               _meshData->getVertexUVs().getBufferSize() * sizeof(Vector2));
                    }

                    if (foundVertexColorAnimation)
                    {
                        _meshAnimatedColors.createBuffer(_meshData->getVertexColors().getBufferSize());
                        memcpy(_meshAnimatedColors.getBufferPointer(),
                               _meshData->getVertexColors().getBufferPointer(),
                               _meshData->getVertexColors().getBufferSize() * sizeof(ColourValue));
                    }
                }
                else
                {
                    _skinning = false;

                    assert(dynamic_cast<ForwardMultiplyRender::TrianglesMeshRenderCommand*>(_renderCommand));
                    auto renderCommand = static_cast<ForwardMultiplyRender::TrianglesMeshRenderCommand*>(_renderCommand);

                    renderCommand->getVerticesBuffer().createBuffer(_meshData->getVertexPositions().getBufferSize(), true); //bx:초기화
                    renderCommand->getIndicesBuffer().createBuffer(_meshData->getIndices().getBufferSize(), true); //bx:초기화

                    bool foundVertexPositionAnimation = false;
                    bool foundVertexUVAnimation = false;
                    bool foundVertexColorAnimation = false;

                    for (const auto& it : _meshData->getAnimationsAll())
                    {
                        auto animation = it.second;

                        for (auto animationTrack : animation->getAnimationTracksAll())
                        {
                            if (animationTrack->getTypeName)
                            {
                                if (animationTrack->getTypeName() == "AnimationTrack_MeshInstanceVertexPosition")
                                {
                                    foundVertexPositionAnimation = true;
                                }
                                else if (animationTrack->getTypeName() == "AnimationTrack_MeshInstanceVertexUV")
                                {
                                    foundVertexUVAnimation = true;
                                }
                                else if (animationTrack->getTypeName() == "AnimationTrack_MeshInstanceVertexColor")
                                {
                                    foundVertexColorAnimation = true;
                                }
                            }
                        }
                    }

                    if (foundVertexPositionAnimation)
                    {
                        _meshAnimatedPositions.createBuffer(_meshData->getVertexPositions().getBufferSize());
                        memcpy(_meshAnimatedPositions.getBufferPointer(),
                               _meshData->getVertexPositions().getBufferPointer(),
                               _meshData->getVertexPositions().getBufferSize() * sizeof(Vector3));
                    }

                    if (foundVertexUVAnimation)
                    {
                        _meshAnimatedUVs.createBuffer(_meshData->getVertexUVs().getBufferSize());
                        memcpy(_meshAnimatedUVs.getBufferPointer(),
                               _meshData->getVertexUVs().getBufferPointer(),
                               _meshData->getVertexUVs().getBufferSize() * sizeof(Vector2));
                    }

                    if (foundVertexColorAnimation)
                    {
                        _meshAnimatedColors.createBuffer(_meshData->getVertexColors().getBufferSize());
                        memcpy(_meshAnimatedColors.getBufferPointer(),
                               _meshData->getVertexColors().getBufferPointer(),
                               _meshData->getVertexColors().getBufferSize() * sizeof(ColourValue));
                    }
                }
            }
            else
            {
                releaseRenderCommand();
            }
        }
    }

    MeshData* MeshInstance::getMeshData() const
    {
        return _meshData;
    }

    void MeshInstance::setTextureAtlasWithTextureAtlasSet(TextureAtlasSet* textureAtlasSet)
    {
        if (textureAtlasSet &&
            _meshData)
        {
            auto textureAtlas = textureAtlasSet->getTextureAtlas(_meshData->getTextureImage());

            if (textureAtlas)
            {
                setTextureAtlas(textureAtlas);
            }
        }
    }

    void MeshInstance::setTextureAtlas(TextureAtlas* textureAtlas)
    {
        if (_textureAtlas != textureAtlas)
        {
            if (_textureAtlas)
            {
                _textureAtlas->release();
            }

            _textureAtlas = textureAtlas;

            if (_textureAtlas)
            {
                _textureAtlas->retain();
            }

            _needToUpdateRenderCommandVertexUVs = true;
            _needToUpdateRenderCommandTextures = true;
        }
    }

    TextureAtlas* MeshInstance::getTextureAtlas() const
    {
        return _textureAtlas;
    }

    void MeshInstance::setBlendMode(ForwardMultiplyRender::BlendType blendType)
    {
        _blendType = blendType;
    }

    ForwardMultiplyRender::BlendType MeshInstance::getBlendMode() const
    {
        return _blendType;
    }







    void MeshInstance::createRenderCommand()
    {
        if (_renderCommand == nullptr)
        {
            _renderCommand = new ForwardMultiplyRender::TrianglesMeshRenderCommand();
            _renderCommand->retain();

            addRenderCommand(_renderCommand);
        }
    }

    void MeshInstance::releaseRenderCommand()
    {
        if (_renderCommand != nullptr)
        {
            removeRenderCommand(_renderCommand);

            _renderCommand->release();
            _renderCommand = nullptr;
        }
    }

    void MeshInstance::updateRenderCommand()
    {
        //bx1111: 빨간색이면 클리핑
        if(USE_EXCLUDED_RED)
        {
            const ColourValue& Color = getWorldColor();
            if(Color.r == 1 && Color.g == 0 && Color.b == 0)
                return;
        }

        if (_renderCommand)
        {
            assert(dynamic_cast<ForwardMultiplyRender::TrianglesMeshRenderCommand*>(_renderCommand));
            auto renderCommand = static_cast<ForwardMultiplyRender::TrianglesMeshRenderCommand*>(_renderCommand);

            //bx: need update at next time... for slot-timing
            renderCommand->setBlendType(getBlendMode());

            // texture atlas 자동 생성 때문에 UV 업데이트보다 먼저 해야 함.
            if (_needToUpdateRenderCommandTextures)
            {
                if (_textureAtlas)
                {
                    renderCommand->setTexture(_textureAtlas->getTexture());
                }
                else
                {
                    Image* image = _meshData->getTextureImage();
                    if (image)
                    {
                        auto imageTextureAtlas = image->getTextureAtlas();

                        if (imageTextureAtlas)
                        {
                            setTextureAtlas(imageTextureAtlas);
                            renderCommand->setTexture(imageTextureAtlas->getTexture());
                        }
                    }
                    else
                    {
                        renderCommand->setTexture(nullptr);
                    }
                }

                _needToUpdateRenderCommandTextures = false;
            }

            if (_needToUpdateRenderCommandVertexPositions)
            {
                if (_skinning)
                {
                    ZAY_V3F_C4B_T2F* dstVertices = renderCommand->getVerticesBuffer().getBufferPointer();
                    int32_t dstVerticesCount = renderCommand->getVerticesBuffer().getBufferSize();

                    assert(_parentSkeletonInstance);
                    assert(_parentSkeletonBone);

                    int32_t* skinningData = reinterpret_cast<int32_t*>(_meshData->getSkinningData());
                    int32_t srcVertexPositionsCount = _meshData->getSkinningVertexCount();

                    int32_t originalVertexCount = _meshData->getSkinningOriginalVertexCount();

                    if (_meshAnimatedPositions.getBufferSize() > 0)
                    {
                        Vector3* animatedVertexPositions = _getAnimatedPositions().getBufferPointer();
                        int32_t animatedVertexPositionsCount = _getAnimatedPositions().getBufferSize();

                        assert(dstVerticesCount == originalVertexCount);
                        assert(animatedVertexPositionsCount == srcVertexPositionsCount);

                        for (auto i=0 ; i<originalVertexCount ; i++)
                        {
                            int32_t boneCount = *(skinningData++);

                            dstVertices->vertices = Vector3::ZERO;

                            for (auto j=0 ; j<boneCount ; j++)
                            {
                                int32_t boneIndex = *(skinningData++);
                                float offsetX = *reinterpret_cast<float*>(skinningData++);
                                float offsetY = *reinterpret_cast<float*>(skinningData++);
                                float weight = *reinterpret_cast<float*>(skinningData++);

                                auto bone = _parentSkeletonInstance->getBone(boneIndex);

                                dstVertices->vertices += (bone->getWorldTransform() * (Vector3(offsetX, offsetY, 0.0f) + *animatedVertexPositions)) * weight;
                                animatedVertexPositions++;
                            }

                            dstVertices++;
                        }
                    }
                    else
                    {
                        assert(dstVerticesCount == originalVertexCount);

                        for (auto i=0 ; i<originalVertexCount ; i++)
                        {
                            int32_t boneCount = *(skinningData++);

                            dstVertices->vertices = Vector3::ZERO;

                            for (auto j=0 ; j<boneCount ; j++)
                            {
                                int32_t boneIndex = *(skinningData++);
                                float offsetX = *reinterpret_cast<float*>(skinningData++);
                                float offsetY = *reinterpret_cast<float*>(skinningData++);
                                float weight = *reinterpret_cast<float*>(skinningData++);

                                auto bone = _parentSkeletonInstance->getBone(boneIndex);

                                dstVertices->vertices += bone->getWorldTransform() * Vector3(offsetX, offsetY, 0.0f) * weight;
                            }

                            dstVertices++;
                        }
                    }
                }
                else if (_meshAnimatedPositions.getBufferSize() > 0)
                {
                    Vector3* srcVertexPositions = _meshData->getVertexPositions().getBufferPointer();
                    int32_t srcVertexPositionsCount = _meshData->getVertexPositions().getBufferSize();

                    Vector3* animatedVertexPositions = _getAnimatedPositions().getBufferPointer();
                    int32_t animatedVertexPositionsCount = _getAnimatedPositions().getBufferSize();

                    ZAY_V3F_C4B_T2F* dstVertices = renderCommand->getVerticesBuffer().getBufferPointer();
                    int32_t dstVerticesCount = renderCommand->getVerticesBuffer().getBufferSize();

                    assert(dstVerticesCount == srcVertexPositionsCount);
                    assert(animatedVertexPositionsCount == srcVertexPositionsCount);

                    for (auto i=0 ; i<srcVertexPositionsCount ; i++)
                    {
                        (dstVertices++)->vertices = getWorldTransform() * (*(srcVertexPositions++) + *(animatedVertexPositions++));
                    }
                }
                else if (_meshData)
                {
                    Vector3* srcVertexPositions = _meshData->getVertexPositions().getBufferPointer();
                    int32_t srcVertexPositionsCount = _meshData->getVertexPositions().getBufferSize();

                    ZAY_V3F_C4B_T2F* dstVertices = renderCommand->getVerticesBuffer().getBufferPointer();
                    int32_t dstVerticesCount = renderCommand->getVerticesBuffer().getBufferSize();

                    assert(srcVertexPositionsCount == dstVerticesCount);

                    for (auto i=0 ; i<srcVertexPositionsCount ; i++)
                    {
                        (dstVertices++)->vertices = getWorldTransform() * *(srcVertexPositions++);
                    }
                }

                _needToUpdateRenderCommandVertexPositions = false;
            }

            if (_needToUpdateRenderCommandVertexUVs)
            {
                ZAY_V3F_C4B_T2F* dstVertices = renderCommand->getVerticesBuffer().getBufferPointer();
                int32_t dstVerticesCount = renderCommand->getVerticesBuffer().getBufferSize();

                Vector2* srcVertexUVs = nullptr;
                int32_t srcVertexUVsCount = 0;

                if (_meshAnimatedUVs.getBufferSize() > 0)
                {
                    srcVertexUVs = _meshAnimatedUVs.getBufferPointer();
                    srcVertexUVsCount = _meshAnimatedUVs.getBufferSize();

                    assert(dstVerticesCount == srcVertexUVsCount);
                }
                else if (_meshData)
                {
                    srcVertexUVs = _meshData->getVertexUVs().getBufferPointer();
                    srcVertexUVsCount = _meshData->getVertexUVs().getBufferSize();

                    assert(dstVerticesCount == srcVertexUVsCount);
                }

                if (dstVerticesCount == srcVertexUVsCount)
                {
                    if (_textureAtlas &&
                        _textureAtlas->getTexture())
                    {
                        auto texture = _textureAtlas->getTexture();
                        auto textureWidthf = static_cast<float>(texture->getPixelsWide());
                        auto textureHeightf = static_cast<float>(texture->getPixelsHigh());

                        auto minU = static_cast<float>(_textureAtlas->getAtlasX()) / textureWidthf;
                        auto widthU = static_cast<float>(_textureAtlas->getAtlasWidth()) / textureWidthf;

                        auto minV = static_cast<float>(_textureAtlas->getAtlasY()) / textureHeightf;
                        auto heightV = static_cast<float>(_textureAtlas->getAtlasHeight()) / textureHeightf;

                        if (_textureAtlas->getAtlasRotated())
                        {
                            // rotated
                            for (auto i=0 ; i<srcVertexUVsCount ; i++)
                            {
                                (dstVertices)->texCoords.x = minU + (srcVertexUVs->y) * heightV;
                                (dstVertices)->texCoords.y = minV + (srcVertexUVs->x) * widthU;

                                dstVertices++;
                                srcVertexUVs++;
                            }
                        }
                        else
                        {
                            // no-rotated

                            for (auto i=0 ; i<srcVertexUVsCount ; i++)
                            {
                                (dstVertices)->texCoords.x = minU + (srcVertexUVs->x) * widthU;
                                (dstVertices)->texCoords.y = minV + (srcVertexUVs->y) * heightV;

                                dstVertices++;
                                srcVertexUVs++;
                            }
                        }
                    }
                }

                _needToUpdateRenderCommandVertexUVs = false;
            }

            if (_needToUpdateRenderCommandVertexColors)
            {
                ZAY_V3F_C4B_T2F* dstVertices = renderCommand->getVerticesBuffer().getBufferPointer();
                int32_t dstVerticesCount = renderCommand->getVerticesBuffer().getBufferSize();

                ColourValue* srcVertexColors = nullptr;
                int32_t srcVertexColorsCount = 0;

                if (_meshAnimatedColors.getBufferSize() > 0)
                {
                    srcVertexColors = _meshAnimatedColors.getBufferPointer();
                    srcVertexColorsCount = _meshAnimatedColors.getBufferSize();

                    assert(dstVerticesCount == srcVertexColorsCount);
                }
                else if (_meshData)
                {
                    srcVertexColors = _meshData->getVertexColors().getBufferPointer();
                    srcVertexColorsCount = _meshData->getVertexColors().getBufferSize();

                    assert(dstVerticesCount == srcVertexColorsCount);
                }

                if (dstVerticesCount == srcVertexColorsCount)
                {
                    for (auto i=0 ; i<srcVertexColorsCount ; i++)
                    {
                        (dstVertices++)->colors = (*(srcVertexColors++) * getWorldColor()).getAsPremultipliedABGR();
                    }
                }

                _needToUpdateRenderCommandVertexColors = false;
            }

            if (_needToUpdateRenderCommandIndices)
            {
                if (_meshData)
                {
                    renderCommand->getIndicesBuffer().referenceBuffer(_meshData->getIndices().getBufferPointer(),
                                                                      _meshData->getIndices().getBufferSize(),
                                                                      _meshData);
                }
                else
                {
                    renderCommand->getIndicesBuffer().clearBuffer();
                }

                _needToUpdateRenderCommandIndices = false;
            }
        }
    }

    RenderCommand* MeshInstance::getRenderCommand() const
    {
        return _renderCommand;
    }









    void MeshInstance::setParentSkeletonInstance(SkeletonInstance* parentSkeletonInstance)
    {
        _parentSkeletonInstance = parentSkeletonInstance;
    }

    void MeshInstance::setParentSkeletonBone(SkeletonBone* parentSkeletonBone)
    {
        _parentSkeletonBone = parentSkeletonBone;
    }









    const ArrayBuffer<Vector3>& MeshInstance::getAnimatedPositions() const
    {
        return _meshAnimatedPositions;
    }

    const ArrayBuffer<Vector2>& MeshInstance::getAnimatedUVs() const
    {
        return _meshAnimatedUVs;
    }

    const ArrayBuffer<ColourValue>& MeshInstance::getAnimatedColors() const
    {
        return _meshAnimatedColors;
    }

    ArrayBuffer<Vector3>& MeshInstance::_getAnimatedPositions()
    {
        return _meshAnimatedPositions;
    }

    ArrayBuffer<Vector2>& MeshInstance::_getAnimatedUVs()
    {
        return _meshAnimatedUVs;
    }

    ArrayBuffer<ColourValue>& MeshInstance::_getAnimatedColors()
    {
        return _meshAnimatedColors;
    }
}
