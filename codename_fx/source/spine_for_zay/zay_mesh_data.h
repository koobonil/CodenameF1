#pragma once

#include "zay_types.h"
#include "zay_scene_data.h"
#include "zay_animation_container.h"
#include "zay_renderer.h"
#include "zay_array_buffer.h"

namespace ZAY
{
    class MeshData
    : public SceneData
    , public AnimationContainer
    {
    public:
        MeshData();
        virtual ~MeshData();

    public:
        virtual SceneInstance* createSceneInstance() override;

    public:
        void setBlendMode(ForwardMultiplyRender::BlendType blendType);
        ForwardMultiplyRender::BlendType getBlendMode() const;
    private:
        ForwardMultiplyRender::BlendType _blendType;

    public:
        ArrayBuffer<Vector3>& _getVertexPositions();
        ArrayBuffer<ColourValue>& _getVertexColors();
        ArrayBuffer<Vector2>& _getVertexUVs();
    public:
        const ArrayBuffer<Vector3>& getVertexPositions() const;
        const ArrayBuffer<ColourValue>& getVertexColors() const;
        const ArrayBuffer<Vector2>& getVertexUVs() const;
    private:
        ArrayBuffer<Vector3> _vertexPositions;
        ArrayBuffer<ColourValue> _vertexColors;
        ArrayBuffer<Vector2> _vertexUVs;

    public:
        ArrayBuffer<GLushort>& _getIndices();
    public:
        const ArrayBuffer<GLushort>& getIndices() const;
    private:
        ArrayBuffer<GLushort> _indices;
        
    public:
        void loadTextureImage(const char* path);
        Image* getTextureImage() const;
    private:
        Image* _textureImage;

    public:
        void setRenderPriorityGroup(int32_t renderPriorityGroup);
        int32_t getRenderPriorityGroup() const;
    private:
        int32_t _renderPriorityGroup;
        
        
        
        // temp
    public:
        void _setSkinningData(int32_t skinningVertexCount, int32_t skinningOriginalVertexCount, char* skinningData, int32_t skinningDataSize);
        char* getSkinningData() const;
        int32_t getSkinningOriginalVertexCount() const;
        int32_t getSkinningVertexSize() const;
        int32_t getSkinningVertexCount() const;
    private:
        char* _skinningData;
        int32_t _skinningDataSize;
        int32_t _skinningOriginalVertexCount;
        int32_t _skinningVertexCount;

    public:
        static String s_projcode;
        static const void* s_assetpath;
    };
}
