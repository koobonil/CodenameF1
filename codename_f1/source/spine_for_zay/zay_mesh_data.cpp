#include "zay_types.h"
#include "zay_mesh_data.h"
#include "zay_mesh_instance.h"
#include "zay_image.h"

#include <boss.hpp>

namespace ZAY
{
    const void* MeshData::s_assetpath = nullptr;

    MeshData::MeshData()
    {
        _blendType = ForwardMultiplyRender::BlendType::Alpha;

        _textureImage = nullptr;

        _renderPriorityGroup = 0;

        _skinningData = nullptr;
        _skinningVertexCount = 0;
        _skinningOriginalVertexCount = 0;
        _skinningDataSize = 0;
    }

    MeshData::~MeshData()
    {
        delete _textureImage;

        if (_skinningData)
        {
            delete [] _skinningData;
            _skinningData = nullptr;
        }
    }

    SceneInstance* MeshData::createSceneInstance()
    {
        auto meshInstance = new MeshInstance;

        meshInstance->setMeshData(this);

        return meshInstance;
    }
    
    void MeshData::setBlendMode(ForwardMultiplyRender::BlendType blendType)
    {
        _blendType = blendType;
    }

    ForwardMultiplyRender::BlendType MeshData::getBlendMode() const
    {
        return _blendType;
    }

    ArrayBuffer<Vector3>& MeshData::_getVertexPositions()
    {
        return _vertexPositions;
    }
    
    ArrayBuffer<ColourValue>& MeshData::_getVertexColors()
    {
        return _vertexColors;
    }
    
    ArrayBuffer<Vector2>& MeshData::_getVertexUVs()
    {
        return _vertexUVs;
    }

    const ArrayBuffer<Vector3>& MeshData::getVertexPositions() const
    {
        return _vertexPositions;
    }
    
    const ArrayBuffer<ColourValue>& MeshData::getVertexColors() const
    {
        return _vertexColors;
    }
    
    const ArrayBuffer<Vector2>& MeshData::getVertexUVs() const
    {
        return _vertexUVs;
    }

    ArrayBuffer<GLushort>& MeshData::_getIndices()
    {
        return _indices;
    }
    
    const ArrayBuffer<GLushort>& MeshData::getIndices() const
    {
        return _indices;
    }
    
    void MeshData::loadTextureImage(const char* path)
    {
        _textureImage = new Image;

        id_asset_read AssetID = Asset::OpenForRead(path, (id_assetpath_read) s_assetpath);
        if(!AssetID) AssetID = Asset::OpenForRead("image/noimage.png");

        sint32 AssetLen = Asset::Size(AssetID);
        buffer AssetBuf = Buffer::Alloc(BOSS_DBG AssetLen);
        Asset::Read(AssetID, (uint08*) AssetBuf, AssetLen);
        Asset::Close(AssetID);

        _textureImage->loadImageData((uint08*) AssetBuf, AssetLen);
        Buffer::Free(AssetBuf);
    }

    Image* MeshData::getTextureImage() const
    {
        return _textureImage;
    }

    void MeshData::setRenderPriorityGroup(int32_t renderPriorityGroup)
    {
        _renderPriorityGroup = renderPriorityGroup;
    }
    
    int32_t MeshData::getRenderPriorityGroup() const
    {
        return _renderPriorityGroup;
    }

    void MeshData::_setSkinningData(int32_t skinningVertexCount, int32_t skinningOriginalVertexCount, char* skinningData, int32_t skinningDataSize)
    {
        if (_skinningData != skinningData)
        {
            if (_skinningData)
            {
                delete [] _skinningData;
            }

            _skinningVertexCount = skinningVertexCount;
            _skinningOriginalVertexCount = skinningOriginalVertexCount;
            _skinningData = skinningData;
            _skinningDataSize = skinningDataSize;
        }
    }
    
    char* MeshData::getSkinningData() const
    {
        return _skinningData;
    }

    int32_t MeshData::getSkinningOriginalVertexCount() const
    {
        return _skinningOriginalVertexCount;
    }

    int32_t MeshData::getSkinningVertexSize() const
    {
        return _skinningDataSize;
    }
    
    int32_t MeshData::getSkinningVertexCount() const
    {
        return _skinningVertexCount;
    }
}
