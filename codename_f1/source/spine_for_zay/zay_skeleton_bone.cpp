#include "zay_skeleton_bone.h"
#include "zay_skeleton_data.h"
#include "zay_skeleton_instance.h"
#include "zay_skeleton_bone_render_instance.h"

namespace ZAY
{
    SkeletonBone::SkeletonBone(SkeletonData* parentSkeletonData)
    {
        _parentSkeletonData = parentSkeletonData;
        _parentSkeletonInstance = nullptr;
        
        _length = 0.0f;
        
        _defaultRotation = 0.0f;
        _defaultPosition = Vector3::ZERO;
        _defaultScale = Vector3::UNIT_SCALE;
        _defaultFlipX = false;
        _defaultFlipY = false;
        _defaultInheritScale = true;
        _defaultInheritRotation = true;
        _defaultLength = 1.0f;
        
        _renderPriorityGroup = 0;
        _renderPriorityDepth = 0.0f;
        
        _skeletonBoneRenderObject = nullptr;
    }
    
    SkeletonBone::SkeletonBone(SkeletonInstance* parentSkeletonInstance)
    {
        _parentSkeletonData = nullptr;
        _parentSkeletonInstance = parentSkeletonInstance;
       
        _length = 0.0f;
        
        _defaultRotation = 0.0f;
        _defaultPosition = Vector3::ZERO;
        _defaultScale = Vector3::UNIT_SCALE;
        _defaultFlipX = false;
        _defaultFlipY = false;
        _defaultInheritScale = true;
        _defaultInheritRotation = true;
        _defaultLength = 1.0f;
        
        _renderPriorityGroup = 0;
        _renderPriorityDepth = 0.0f;
        
        _skeletonBoneRenderObject = nullptr;
    }

    SkeletonBone::~SkeletonBone()
    {
        destroySkeletonBoneRenderInstance();
    }
        
    
    
    Node* SkeletonBone::createChildImpl()
    {
        if (_parentSkeletonData)
        {
            return new SkeletonBone(_parentSkeletonData);
        }
        else if (_parentSkeletonInstance)
        {
            return new SkeletonBone(_parentSkeletonInstance);
        }
        
        return nullptr;
    }

    void SkeletonBone::_updateRootWorldTransform()
    {
        if (_parentSkeletonInstance)
        {
            _parentSkeletonInstance->_updateWorldTransformAscending();
        }
    }

    void SkeletonBone::_updateRootWorldColor()
    {
        if (_parentSkeletonInstance)
        {
            _parentSkeletonInstance->_updateWorldColorAscending();
        }
    }

    
    
    void SkeletonBone::addChild(Node* child)
    {
        assert(dynamic_cast<SkeletonBone*>(child));

        Node::addChild(child);
    }

    
    
    SkeletonData* SkeletonBone::getParentSkeletonData()
    {
        return _parentSkeletonData;
    }
    
    
    
    SkeletonInstance* SkeletonBone::getParentSkeletonInstance()
    {
        return _parentSkeletonInstance;
    }

    
    
    
    SkeletonBone* SkeletonBone::getParentBone() const
    {
        if (getParentNode())
        {
            assert(dynamic_cast<SkeletonBone*>(getParentNode()));
            return static_cast<SkeletonBone*>(getParentNode());
        }
        else
        {
            return nullptr;
        }
    }
    
    
    
    
    
    
    

    void SkeletonBone::setLength(float length)
    {
        _length = length;
    }

    float SkeletonBone::getLength() const
    {
        return _length;
    }
    
    
    
    
    
    
    

    void SkeletonBone::applyDefaultPose()
    {
        setPosition(getDefaultPosition());
        setScale(getDefaultScale());
        setRotation(getDefaultRotation());
        setFlipX(getDefaultFlipX());
        setFlipY(getDefaultFlipY());
        setInheritScale(getDefaultInheritScale());
        setInheritRotation(getDefaultInheritRotation());
        setLength(getDefaultLength());
    }

    void SkeletonBone::setDefaultPosition(float x, float y, float z)
    {
        _defaultPosition.x = x;
        _defaultPosition.y = y;
        _defaultPosition.z = z;
    }
    
    void SkeletonBone::setDefaultPosition(const Vector3& position)
    {
        _defaultPosition = position;
    }
    
    void SkeletonBone::setDefaultScale(float x, float y, float z)
    {
        _defaultScale.x = x;
        _defaultScale.y = y;
        _defaultScale.z = z;
    }
    
    void SkeletonBone::setDefaultScale(const Vector3& scale)
    {
        _defaultScale = scale;
    }
    
    void SkeletonBone::setDefaultRotation(float rotation)
    {
        _defaultRotation = rotation;
    }
    
    void SkeletonBone::setDefaultFlipX(bool flipX)
    {
        _defaultFlipX = flipX;
    }
    
    void SkeletonBone::setDefaultFlipY(bool flipY)
    {
        _defaultFlipY = flipY;
    }
    
    void SkeletonBone::setDefaultInheritScale(bool inherit)
    {
        _defaultInheritScale = inherit;
    }
    
    void SkeletonBone::setDefaultInheritRotation(bool inherit)
    {
        _defaultInheritRotation = inherit;
    }
    
    void SkeletonBone::setDefaultLength(float length)
    {
        _defaultLength = length;
    }

    const Vector3& SkeletonBone::getDefaultPosition() const
    {
        return _defaultPosition;
    }
    
    const Vector3& SkeletonBone::getDefaultScale() const
    {
        return _defaultScale;
    }
    
    float SkeletonBone::getDefaultRotation() const
    {
        return _defaultRotation;
    }
    
    bool SkeletonBone::getDefaultFlipX() const
    {
        return _defaultFlipX;
    }
    
    bool SkeletonBone::getDefaultFlipY() const
    {
        return _defaultFlipY;
    }
    
    bool SkeletonBone::getDefaultInheritScale() const
    {
        return _defaultInheritScale;
    }
    
    bool SkeletonBone::getDefaultInheritRotation() const
    {
        return _defaultInheritRotation;
    }
    
    float SkeletonBone::getDefaultLength() const
    {
        return _defaultLength;
    }
    
    
    
    
    
    void SkeletonBone::setRenderPriorityGroup(int32_t priority)
    {
        if (_renderPriorityGroup != priority)
        {
            _renderPriorityGroup = priority;
            
            if (_skeletonBoneRenderObject)
            {
                _skeletonBoneRenderObject->setRenderPriorityGroup(_renderPriorityGroup);
            }
        }
    }
    
    int32_t SkeletonBone::getRenderPriorityGroup() const
    {
        return _renderPriorityGroup;
    }
    
    void SkeletonBone::setRenderPriorityDepth(float depth)
    {
        if (_renderPriorityDepth != depth)
        {
            _renderPriorityDepth = depth;
            
            if (_skeletonBoneRenderObject)
            {
                _skeletonBoneRenderObject->setRenderPriorityDepth(_renderPriorityDepth);
            }
        }
    }
    
    float SkeletonBone::getRenderPriorityDepth() const
    {
        return _renderPriorityDepth;
    }

    
    
    
    
    void SkeletonBone::createSkeletonBoneRenderInstance()
    {
        if (_skeletonBoneRenderObject == nullptr)
        {
            _skeletonBoneRenderObject = new SkeletonBoneRenderInstance;
            assert(_skeletonBoneRenderObject);
            
            _skeletonBoneRenderObject->retain();
            
            _skeletonBoneRenderObject->setRenderPriorityGroup(_renderPriorityGroup);
            _skeletonBoneRenderObject->setRenderPriorityDepth(_renderPriorityDepth);

            attachObject(_skeletonBoneRenderObject);
        }
    }
    
    void SkeletonBone::destroySkeletonBoneRenderInstance()
    {
        if (_skeletonBoneRenderObject != nullptr)
        {
            _skeletonBoneRenderObject->detachFromAttachedNode();
            _skeletonBoneRenderObject->release();

            _skeletonBoneRenderObject = nullptr;
        }
    }
    
    SkeletonBoneRenderInstance* SkeletonBone::getSkeletonBoneRenderInstance() const
    {
        return _skeletonBoneRenderObject;
    }

}

