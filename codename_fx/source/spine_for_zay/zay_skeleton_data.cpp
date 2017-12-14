#include "zay_skeleton_data.h"
#include "zay_skeleton_instance.h"
#include "zay_skeleton_bone.h"
#include "zay_scene_slot_instance.h"
#include "zay_scene_slot_data.h"
#include "zay_mesh_data.h"
#include "zay_mesh_instance.h"
#include "zay_touch_area_data.h"
#include "zay_animation.h"
#include "zay_animation_track.h"
#include "zay_animation_state_set.h"
#include "zay_image_data.h"
#include "zay_image.h"

namespace ZAY
{
    void SpineConstraint::setName(const std::string& name)
    {
        _name = name;
    }
    
    const std::string& SpineConstraint::getName() const
    {
        return _name;
    }

    SkeletonData::SkeletonData()
    {
    }
    
    SkeletonData::~SkeletonData()
    {
        removeBonesAll();
        removeSlotsAll();
        removeSpineConstraintsAll();
    }

    SceneInstance* SkeletonData::createSceneInstance()
    {
        auto skeletonInstance = new SkeletonInstance;
        skeletonInstance->autorelease();
        
        skeletonInstance->setSkeletonData(this);
        
        return skeletonInstance;
    }

    SkeletonBone* SkeletonData::createBone(const std::string& name)
    {
        if (_bones.left.end() == _bones.left.find(name))
        {
            auto bone = new SkeletonBone(this);
            
            _bones.insert(BoneMapType(name, bone));
            _bonesIndexable.push_back(bone);
            
            return bone;
        }
        
        return nullptr;
    }

    SkeletonBone* SkeletonData::createRootBone(const std::string& name)
    {
        if (_bones.left.end() == _bones.left.find(name))
        {
            auto bone = new SkeletonBone(this);

            _bones.insert(BoneMapType(name, bone));
            _rootBones.insert(bone);
            _bonesIndexable.push_back(bone);

            return bone;
        }

        return nullptr;
    }

    SkeletonBone* SkeletonData::getBone(const std::string& name) const
    {
        auto it = _bones.left.find(name);
        
        if (_bones.left.end() != it)
        {
            return it->second;
        }
        
        return nullptr;
    }

    SkeletonBone* SkeletonData::getBone(int32_t index) const
    {
        if (index >= 0 &&
            index < static_cast<int32_t>(_bonesIndexable.size()))
        {
            return _bonesIndexable.at(index);
        }
        
        return nullptr;
    }

    int32_t SkeletonData::getBoneIndex(const std::string& name) const
    {
        auto bone = getBone(name);
        
        if (bone)
        {
            return getBoneIndex(bone);
        }
        
        return -1;
    }
    
    int32_t SkeletonData::getBoneIndex(SkeletonBone* bone) const
    {
        auto it = std::find(_bonesIndexable.begin(),
                            _bonesIndexable.end(),
                            bone);
        
        if (it != _bonesIndexable.end())
        {
            return static_cast<int32_t>(std::distance(_bonesIndexable.begin(), it));
        }

        return -1;
    }

    const std::string& SkeletonData::getBoneName(SkeletonBone* bone) const
    {
        auto it = _bones.right.find(bone);
        
        if (it != _bones.right.end())
        {
            return it->second;
        }
        
        return s_emptyString;
    }

    const std::string& SkeletonData::getBoneName(int32_t index) const
    {
        auto bone = getBone(index);
        
        if (bone)
        {
            return getBoneName(bone);
        }
        
        return s_emptyString;
    }

    void SkeletonData::removeBone(const std::string& name)
    {
        auto it = _bones.left.find(name);
        
        if (_bones.left.end() != it)
        {
            auto bone = it->second;

            _rootBones.erase(bone);
            _bones.left.erase(it);
            
            auto it2 = std::find(_bonesIndexable.begin(),
                                 _bonesIndexable.end(),
                                 bone);
            assert(it2 != _bonesIndexable.end());
            _bonesIndexable.erase(it2);
            
            delete bone;
        }
    }
    
    void SkeletonData::removeBone(SkeletonBone* bone)
    {
        auto it = _bones.right.find(bone);

        if (_bones.right.end() != it)
        {
            _rootBones.erase(bone);
            _bones.right.erase(it);
            
            auto it2 = std::find(_bonesIndexable.begin(),
                                 _bonesIndexable.end(),
                                 bone);
            assert(it2 != _bonesIndexable.end());
            _bonesIndexable.erase(it2);

            delete bone;
        }
    }
    
    void SkeletonData::removeBonesAll()
    {
        for (auto it : _bones.left)
        {
            delete it.second;
        }
        
        _bones.clear();
        _rootBones.clear();
        _bonesIndexable.clear();
    }

    const SkeletonData::BoneMap& SkeletonData::getBones() const
    {
        return _bones;
    }
    
    const SkeletonData::BoneSet& SkeletonData::getRootBones() const
    {
        return _rootBones;
    }
    
    const SkeletonData::BoneVector& SkeletonData::getBonesIndexable() const
    {
        return _bonesIndexable;
    }

    SceneSlotData* SkeletonData::createAndAddSlot(const std::string& name, float drawOrder)
    {
        auto it = _slots.left.find(name);
        
        if (it == _slots.left.end())
        {
            auto slot = new SceneSlotData;
            slot->autorelease();

            _slots.insert(SlotMapType(name, slot));
            _addSlotDrawOrder(slot, drawOrder);

            return slot;
        }
        else
        {
            return nullptr;
        }
    }
    
    const SceneSlotData* SkeletonData::getSlotObject(const std::string& name) const
    {
        auto it = _slots.left.find(name);
        
        if (it != _slots.left.end())
        {
            return it->second;
        }
        
        return nullptr;
    }
    
    void SkeletonData::removeSlot(const std::string& name)
    {
        auto it = _slots.left.find(name);
        
        if (it != _slots.left.end())
        {
            _removeSlotDrawOrder(it->second);

            _slots.left.erase(it);
        }
    }

    void SkeletonData::removeSlotsAll()
    {
        _slots.clear();
        
        _defaultDrawOrder.clear();
    }

    const SkeletonData::SlotMap& SkeletonData::getSlots() const
    {
        return _slots;
    }

    void SkeletonData::setSlotParentBoneName(const std::string& slotName, const std::string& parentBoneName)
    {
        _slotParentBones[slotName] = parentBoneName;
    }
    
    const std::string& SkeletonData::getSlotParentBoneName(const std::string& slotName) const
    {
        auto it = _slotParentBones.find(slotName);
        
        if (it != _slotParentBones.end())
        {
            return it->second;
        }
        else
        {
            return s_emptyString;
        }
    }
    
    void SkeletonData::removeSlotParentBoneName(const std::string& slotName)
    {
        _slotParentBones.erase(slotName);
    }
    
    void SkeletonData::removeSlotParentBonesAll()
    {
        _slotParentBones.clear();
    }

    const SkeletonData::SlotParentBoneMap& SkeletonData::getSlotParentBones() const
    {
        return _slotParentBones;
    }

    void SkeletonData::setSlotDrawOrder(const SceneSlotData* skeletonSlot, float drawOrder)
    {
        if (skeletonSlot != nullptr)
        {
            auto it = _defaultDrawOrder.right.find(skeletonSlot);

            if (it != _defaultDrawOrder.right.end())
            {
                _defaultDrawOrder.right.erase(it);

                _defaultDrawOrder.insert(DrawOrderMapType(drawOrder, skeletonSlot));
            }
        }
    }
    
    const SceneSlotData* SkeletonData::getSlotWithDrawOrder(float drawOrder, int32_t index) const
    {
        auto it_pair = _defaultDrawOrder.left.equal_range(drawOrder);
        
        auto count = std::distance(it_pair.first, it_pair.second);
        
        if (index < count)
        {
            auto it = it_pair.first;
            
            std::advance(it, index);
            
            return it->second;
        }
        else
        {
            return nullptr;
        }
    }

    void SkeletonData::_addSlotDrawOrder(const SceneSlotData* skeletonSlot, float drawOrder)
    {
        if (skeletonSlot != nullptr)
        {
            auto it = _defaultDrawOrder.right.find(skeletonSlot);
            
            if (it == _defaultDrawOrder.right.end())
            {
                _defaultDrawOrder.insert(DrawOrderMapType(drawOrder, skeletonSlot));
            }
        }
    }

    void SkeletonData::_removeSlotDrawOrder(const SceneSlotData* skeletonSlot)
    {
        if (skeletonSlot != nullptr)
        {
            auto it = _defaultDrawOrder.right.find(skeletonSlot);
            
            if (it != _defaultDrawOrder.right.end())
            {
                _defaultDrawOrder.right.erase(it);
            }
        }
    }

    void SkeletonData::_removeSlotDrawOrdersAll()
    {
        _defaultDrawOrder.clear();
    }

    const SkeletonData::DrawOrderMap& SkeletonData::getDefaultDrawOrder() const
    {
        return _defaultDrawOrder;
    }

    void SkeletonData::setWidth(float width)
    {
        _width = width;
    }
    
    void SkeletonData::setHeight(float height)
    {
        _height = height;
    }
    
    float SkeletonData::getWidth() const
    {
        return _width;
    }
    
    float SkeletonData::getHeight() const
    {
        return _height;
    }
    
    const std::vector<SpineConstraint*>& SkeletonData::getSpineConstraints() const
    {
        return _spineConstraints;
    }
    
    void SkeletonData::removeSpineConstraintsAll()
    {
        for (auto spineConstraint : _spineConstraints)
        {
            spineConstraint->release();
        }
        _spineConstraints.clear();

        _spineConstraintsMap.clear();
    }

    SpineConstraint* SkeletonData::getSpineConstraint(const std::string& name) const
    {
        auto it = _spineConstraintsMap.find(name);
        
        if (it != _spineConstraintsMap.end())
        {
            return it->second;
        }
        
        return nullptr;
    }

    void SkeletonData::createAnimationStates(AnimationStateSet& animationStatesSet)
    {
        animationStatesSet._createAnimationStatesFromAnimationContainer(this);

        for (auto it : getSlots())
        {
            auto slotDataObject = it.right;
            auto slotData = (SceneSlotData*) slotDataObject;

            animationStatesSet._createAnimationStatesFromAnimationContainer(slotData);
            
            for (auto it : slotData->getSceneDatas())
            {
                for (auto it2 : it.second)
                {
                    auto object = it2.second;
                    auto animationContainer = dynamic_cast<AnimationContainer*>(object);
                    
                    if (animationContainer)
                    {
                        animationStatesSet._createAnimationStatesFromAnimationContainer(animationContainer);
                    }
                }
            }
        }
    }
}
