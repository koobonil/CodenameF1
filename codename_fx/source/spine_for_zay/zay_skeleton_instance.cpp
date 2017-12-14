#include "zay_types.h"
#include "zay_skeleton_instance.h"
#include "zay_skeleton_data.h"
#include "zay_scene_slot_instance.h"
#include "zay_scene_slot_data.h"
#include "zay_skeleton_bone.h"
#include "zay_animation.h"
#include "zay_animation_state.h"
#include "zay_animation_state_set.h"
#include "zay_mesh_instance.h"
#include "zay_math.h"

namespace ZAY
{
    extern void SpineBuilder_CallEvent(const char* name);

    void SpineConstraintFunctor::setName(const std::string& name)
    {
        _name = name;
    }
    
    const std::string& SpineConstraintFunctor::getName() const
    {
        return _name;
    }
    
    
    
    SkeletonInstanceIKFunctor::SkeletonInstanceIKFunctor()
    {
        _mix = 1.0f;
        _direction = 1.0f;
    }
    
    SkeletonInstanceIKFunctor::~SkeletonInstanceIKFunctor()
    {
        _mix = 1.0f;
        _direction = 1.0f;
    }
    
    
    
    void SkeletonInstanceIKFunctor::setMix(float mix)
    {
        _mix = mix;
    }
    
    float SkeletonInstanceIKFunctor::getMix() const
    {
        return _mix;
    }
    
    
    
    void SkeletonInstanceIKFunctor::setDirection(float direction)
    {
        _direction = direction;
    }
    
    float SkeletonInstanceIKFunctor::getDirection() const
    {
        return _direction;
    }

    

    
    
    
    
    SkeletonInstanceIKFunctorApplyOne::SkeletonInstanceIKFunctorApplyOne()
    {
        _bone0 = nullptr;
        _targetBone = nullptr;
    }
    
    SkeletonInstanceIKFunctorApplyOne::~SkeletonInstanceIKFunctorApplyOne()
    {
    }

    void SkeletonInstanceIKFunctorApplyOne::operator ()(SkeletonInstance* SkeletonInstance, std::set<SkeletonBone*>& toUpdate)
    {
        if (_bone0 &&
            _targetBone)
        {
            auto parentRotation = (!_bone0->getInheritRotation() || !_bone0->getParentBone()) ? 0.0f : _bone0->getParentBone()->getWorldRotation();
            
            auto rotationIK = std::atan2(_targetBone->getWorldPosition().y - _bone0->getWorldPosition().y,
                                         _targetBone->getWorldPosition().x - _bone0->getWorldPosition().x) * Math::fRad2Deg;

            /*bx: -spine bug-
            if (_bone0->getWorldFlipX() != _bone0->getWorldFlipY())
            {
                rotationIK = -rotationIK;
            }*/

            rotationIK -= parentRotation;
            
            auto rotation = _bone0->getRotation();
            _bone0->setRotation(rotation + (rotationIK - rotation) * getMix());

            toUpdate.insert(_bone0);
        }
    }
    
    
    
    
    SkeletonInstanceIKFunctorApplyTwo::SkeletonInstanceIKFunctorApplyTwo()
    {
        _bone0 = nullptr;
        _bone1 = nullptr;
        _targetBone = nullptr;
    }
    
    SkeletonInstanceIKFunctorApplyTwo::~SkeletonInstanceIKFunctorApplyTwo()
    {
    }

    void SkeletonInstanceIKFunctorApplyTwo::operator ()(SkeletonInstance* SkeletonInstance, std::set<SkeletonBone*>& toUpdate)
    {
        if (_bone0 &&
            _bone1 &&
            _targetBone)
        {
            auto parent = _bone0;
            auto child = _bone1;
            
            float childRotation = child->getRotation();
            float parentRotation = parent->getRotation();

            if (getMix() <= 0.0f)
            {
                return;
            }
            
            float targetX = _targetBone->getWorldPosition().x;
            float targetY = _targetBone->getWorldPosition().y;
            Vector3 position;
            
            auto parentParent = parent->getParentBone();
            if (parentParent)
            {
                position = parentParent->_convertWorldToLocalPosition(_targetBone->getWorldPosition());
                targetX = (position.x - parent->getPosition().x) * parentParent->getWorldScale().x;
                targetY = (position.y - parent->getPosition().y) * parentParent->getWorldScale().y;
            }
            else
            {
                targetX -= parent->getPosition().x;
                targetY -= parent->getPosition().y;
            }
            
            if (child->getParentBone() == parent)
            {
                position = child->getPosition();
            }
            else
            {
                position = child->getParentBone()->_convertLocalToWorldPosition(child->getPosition());
                position = parent->_convertWorldToLocalPosition(position);
            }
            
            auto childX = position.x * parent->getWorldScale().x;
            auto childY = position.y * parent->getWorldScale().y;
            auto offset = std::atan2(childY, childX);
            auto len1 = std::sqrt(childX * childX + childY * childY);
            auto len2 = child->getLength() * child->getWorldScale().x;

            auto cosDenom = 2.0f * len1 * len2;
            
            if (cosDenom < 0.0001f)
            {
                child->setRotation(childRotation + (std::atan2(targetY, targetX) * Math::fRad2Deg - parentRotation - childRotation) * _mix);
            }
            else
            {
                auto cos = (targetX * targetX + targetY * targetY - len1 * len1 - len2 * len2) / cosDenom;
                
                if (cos < -1.0f)
                {
                    cos = -1.0f;
                }
                else if (cos > 1.0f)
                {
                    cos = 1.0f;
                }

                auto childAngle = std::acos(cos) * _direction;
                auto adjacent = len1 + len2 * cos;
                auto opposite = len2 * std::sin(childAngle);
                auto parentAngle = std::atan2(targetY * adjacent - targetX * opposite, targetX * adjacent + targetY * opposite);
                auto rotation = (parentAngle - offset) * Math::fRad2Deg - parentRotation;
                if (rotation > 180.0f)
                    rotation -= 360.0f;
                else if (rotation < -180.0f)
                    rotation += 360.0f;
                parent->setRotation(parentRotation + rotation * _mix);
                rotation = (childAngle + offset) * Math::fRad2Deg - childRotation;
                if (rotation > 180.0f)
                    rotation -= 360.0f;
                else if (rotation < -180.0f)
                    rotation += 360.0f;
                child->setRotation(childRotation + (rotation + parent->getWorldRotation() - child->getParentBone()->getWorldRotation()) * _mix);
            }

            toUpdate.insert(_bone0);
            toUpdate.insert(_bone1);
        }
    }

    SkeletonInstance::SkeletonInstance()
    {
        _skeletonData = nullptr;
        _grandParentBone = new SkeletonBone(this);
    }

    SkeletonInstance::~SkeletonInstance()
    {
        removeSlotsAll();
        removeBonesAll();
        removeSpineConstraintFunctorsAll();
        
        if (_grandParentBone)
        {
            delete _grandParentBone;
            _grandParentBone = nullptr;
        }
        
        if (_skeletonData)
        {
            _skeletonData->release();
            _skeletonData = nullptr;
        }
    }

    void SkeletonInstance::_updateOneTick()
    {
        SceneInstance::_updateOneTick();

        _grandParentBone->_updateOneTick();
    }

    void SkeletonInstance::_updateSubTicks(int32_t subTicks)
    {
        SceneInstance::_updateSubTicks(subTicks);
        
        _grandParentBone->_updateSubTicks(subTicks);
    }

    void SkeletonInstance::_preRender()
    {
        SceneInstance::_preRender();

        _grandParentBone->_preRender();
    }

    void SkeletonInstance::_postRender()
    {
        SceneInstance::_postRender();
        
        _grandParentBone->_postRender();
    }
    
    void SkeletonInstance::_notifyAttachedToRenderer()
    {
        _grandParentBone->setRenderer(getAttachedRenderer());
    }

    void SkeletonInstance::_notifyDetachedFromRenderer()
    {
        _grandParentBone->setRenderer(nullptr);
    }
    
    void SkeletonInstance::_notifyAttachedToTouchEventDispatcher()
    {
        _grandParentBone->setTouchEventDispatcher(getAttachedTouchEventDispatcher());
    }
    
    void SkeletonInstance::_notifyDetachedFromTouchEventDispatcher()
    {
        _grandParentBone->setTouchEventDispatcher(nullptr);
    }

    void SkeletonInstance::_updateWorldTransformToChildren()
    {
        _grandParentBone->_updateWorldTransformDescending();
        
        _updateSpineConstraintFunctors();
    }
    
    void SkeletonInstance::_updateWorldColorToChildren()
    {
        _grandParentBone->_updateWorldColorDescending();
    }

    void SkeletonInstance::_notifyWorldTransformUpdated(bool updated)
    {
        if (updated)
        {
            _grandParentBone->setRotation(getWorldRotation());
            _grandParentBone->setScale(getWorldScale());
            _grandParentBone->setPosition(getWorldPosition());
            _grandParentBone->setFlipX(getWorldFlipX());
            _grandParentBone->setFlipY(getWorldFlipY());
        }
    }

    void SkeletonInstance::_notifyWorldColorUpdated(bool updated)
    {
        if (updated)
        {
            _grandParentBone->setLocalColor(getWorldColor());
        }
    }

    void SkeletonInstance::_notifyRenderPriorityGroupChanged()
    {
        for (auto it : _slots)
        {
            auto slot = it.right;
            
            slot->setRenderPriorityGroup(getRenderPriorityGroup());
        }
        
        for (auto it : _bones)
        {
            auto bone = it.right;
            
            bone->setRenderPriorityGroup(getRenderPriorityGroup());
        }
    }
    
    void SkeletonInstance::_notifyRenderPriorityDepthChanged()
    {
        for (auto it : _slots)
        {
            auto slot = it.right;
            
            slot->setRenderPriorityDepth(getRenderPriorityDepth());
        }
        
        for (auto it : _bones)
        {
            auto bone = it.right;
            
            bone->setRenderPriorityDepth(getRenderPriorityDepth());
        }
    }

    void SkeletonInstance::setTextureAtlasWithTextureAtlasSet(TextureAtlasSet* textureAtlasSet)
    {
        for (auto it : _bones)
        {
            it.right->setTextureAtlasWithTextureAtlasSet(textureAtlasSet);
        }

        for (auto it : _slots)
        {
            it.right->setTextureAtlasWithTextureAtlasSet(textureAtlasSet);
        }
    }

    void SkeletonInstance::setSkeletonData(SkeletonData* skeletonData)
    {
        if (_skeletonData != skeletonData)
        {
            removeBonesAll();
            removeSlotsAll();
            removeSpineConstraintFunctorsAll();
            
            if (_skeletonData)
            {
                _skeletonData->release();
            }
            
            _skeletonData = skeletonData;
            
            if (_skeletonData)
            {
                _skeletonData->retain();
            }
            
            _createBonesFromSkeleton();
            _createSlotsFromSkeleton();
            _createConstraintsFromSkeleton();
            
            applyInitialAnimationPose(true, false);
        }
    }

    SkeletonData* SkeletonInstance::getSkeletonData() const
    {
        return _skeletonData;
    }

    SkeletonBone* SkeletonInstance::addRootBone(const std::string& name)
    {
        if (_bones.left.end() == _bones.left.find(name))
        {
            auto bone = new SkeletonBone(this);
            
            _bones.insert(BoneMapType(name, bone));
            _rootBones.insert(bone);

            _grandParentBone->addChild(bone);
            
            if (_boneRenderable)
            {
                bone->createSkeletonBoneRenderInstance();
            }

            return bone;
        }
        
        return nullptr;
    }
    
    SkeletonBone* SkeletonInstance::getBone(const std::string& name) const
    {
        auto it = _bones.left.find(name);
        
        if (_bones.left.end() != it)
        {
            return it->second;
        }
        
        return nullptr;
    }
    
    SkeletonBone* SkeletonInstance::getBone(int32_t index) const
    {
        if (index >= 0 &&
            index < static_cast<int32_t>(_bonesIndexable.size()))
        {
            return _bonesIndexable.at(index);
        }
        
        return nullptr;
    }
    
    void SkeletonInstance::setBoneIndices(const BoneVector& boneIndices)
    {
        _bonesIndexable = boneIndices;
    }

    int32_t SkeletonInstance::getBoneIndex(const std::string& name) const
    {
        auto bone = getBone(name);
        
        if (bone)
        {
            return getBoneIndex(bone);
        }
        
        return -1;
    }
    
    int32_t SkeletonInstance::getBoneIndex(SkeletonBone* bone) const
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
    
    const std::string& SkeletonInstance::getBoneName(SkeletonBone* bone) const
    {
        auto it = _bones.right.find(bone);
        
        if (it != _bones.right.end())
        {
            return it->second;
        }
        
        return s_emptyString;
    }
    
    const std::string& SkeletonInstance::getBoneName(int32_t index) const
    {
        auto bone = getBone(index);
        
        if (bone)
        {
            return getBoneName(bone);
        }
        
        return s_emptyString;
    }

    void SkeletonInstance::removeBone(const std::string& name)
    {
        auto it = _bones.left.find(name);
        
        if (_bones.left.end() != it)
        {
            auto bone = it->second;
            
            _rootBones.erase(bone);
            _bones.left.erase(it);
            
            delete bone;
        }
    }
    
    void SkeletonInstance::removeBone(SkeletonBone* bone)
    {
        auto it = _bones.right.find(bone);
        
        if (_bones.right.end() != it)
        {
            _rootBones.erase(bone);
            _bones.right.erase(it);

            delete bone;
        }
    }
    
    void SkeletonInstance::removeBonesAll()
    {
        for (auto it : _bones.left)
        {
            delete it.second;
        }

        _bones.clear();
        _rootBones.clear();
    }
    
    const SkeletonInstance::BoneMap& SkeletonInstance::getBones() const
    {
        return _bones;
    }
    
    const SkeletonInstance::BoneSet& SkeletonInstance::getRootBones() const
    {
        return _rootBones;
    }

    void SkeletonInstance::_createBonesFromSkeleton()
    {
        assert(_bones.size() == 0);

        if (_skeletonData)
        {
            BoneVector bonesIndexable;
            
            // copy bone
            for (auto srcBone : _skeletonData->getBonesIndexable())
            {
                auto boneName = _skeletonData->getBoneName(srcBone);

                auto bone = new SkeletonBone(this);
                _bones.insert(BoneMapType(boneName, bone));
                
                bone->setPosition(srcBone->getPosition());
                bone->setScale(srcBone->getScale());
                bone->setRotation(srcBone->getRotation());
                bone->setFlipX(srcBone->getFlipX());
                bone->setFlipY(srcBone->getFlipY());
                bone->setLength(srcBone->getLength());
                bone->setInheritScale(srcBone->getInheritScale());
                bone->setInheritRotation(srcBone->getInheritRotation());
                
                bone->setDefaultPosition(srcBone->getDefaultPosition());
                bone->setDefaultScale(srcBone->getDefaultScale());
                bone->setDefaultRotation(srcBone->getDefaultRotation());
                bone->setDefaultFlipX(srcBone->getDefaultFlipX());
                bone->setDefaultFlipY(srcBone->getDefaultFlipY());
                bone->setDefaultLength(srcBone->getDefaultLength());
                bone->setDefaultInheritScale(srcBone->getDefaultInheritScale());
                bone->setDefaultInheritRotation(srcBone->getDefaultInheritRotation());
                
                if (_boneRenderable)
                {
                    bone->createSkeletonBoneRenderInstance();
                }

                bonesIndexable.push_back(bone);
            }

            // add root bone
            for (auto rootBone : _skeletonData->getRootBones())
            {
                auto found = _skeletonData->getBones().right.find(rootBone);
                
                assert(found != _skeletonData->getBones().right.end());
                if ((found != _skeletonData->getBones().right.end()))
                {
                    auto rootBoneName = found->second;

                    auto it = _bones.left.find(rootBoneName);
                    assert(it != _bones.left.end());
                    if (it != _bones.left.end())
                    {
                        _rootBones.insert(it->second);
                        _grandParentBone->addChild(it->second);
                    }
                }
            }
            
            // set bone parent
            for (auto it : _skeletonData->getBones())
            {
                auto boneName = it.left;
                auto srcBone = it.right;
                
                auto srcParentBone = srcBone->getParentBone();
                
                if (srcParentBone)
                {
                    auto it = _skeletonData->getBones().right.find(srcParentBone);
                    assert(it != _skeletonData->getBones().right.end());
                    if (it != _skeletonData->getBones().right.end())
                    {
                        auto parentBoneName = it->second;

                        auto bone = getBone(boneName);
                        auto parentBone = getBone(parentBoneName);
                        
                        parentBone->addChild(bone);
                    }
                }
            }
            
            setBoneIndices(bonesIndexable);
        }
    }

    void SkeletonInstance::applyInitialAnimationPose(bool inheritChild, bool colorUpdateOnly) //bx
    {
        for (auto it : _defaultDrawOrder)
        {
            it.right->setRenderPrioritySceneDataDepth(it.left);
        }
        
        for (auto it : _bones)
        {
            auto bone = it.right;

            bone->applyDefaultPose();
        }

        if (inheritChild)
        {
            for (auto it : _slots)
            {
                it.right->applyInitialAnimationPose(inheritChild, colorUpdateOnly);
            }
        }

        if (!colorUpdateOnly && _skeletonData)
        {
            int32_t index = 0;
            for (auto spineConstraint : _skeletonData->getSpineConstraints())
            {
                switch (spineConstraint->getSpineConstraintType())
                {
                    default:
                        assert(0);
                        break;
                        
                    case SpineConstraintType::IK:
                    {
                        assert(dynamic_cast<SkeletonIKData*>(spineConstraint));
                        
                        auto ikData = static_cast<SkeletonIKData*>(spineConstraint);
                        
                        assert(ikData->_targetBoneName.size() > 0);
                        assert(ikData->_boneNames.size() == 1 ||
                               ikData->_boneNames.size() == 2);
                        assert(ikData->_bendPositive == 1 ||
                               ikData->_bendPositive == -1);
                        assert(ikData->_mix >= 0.0f &&
                               ikData->_mix <= 1.0f);
                        
                        if (ikData->_boneNames.size() == 1)
                        {
                            auto c = _spineConstraintFunctors.at(index++);
                            assert(dynamic_cast<SkeletonInstanceIKFunctorApplyOne*>(c));
                            auto ik = static_cast<SkeletonInstanceIKFunctorApplyOne*>(c);

                            ik->setDirection(1.0f);
                            
                            ik->setMix(ikData->_mix);
                        }
                        else if (ikData->_boneNames.size() == 2)
                        {
                            auto c = _spineConstraintFunctors.at(index++);
                            assert(dynamic_cast<SkeletonInstanceIKFunctorApplyTwo*>(c));
                            auto ik = static_cast<SkeletonInstanceIKFunctorApplyTwo*>(c);

                            ik->setDirection(ikData->_bendPositive == 1 ? 1.0f : -1.0f);
                            
                            ik->setMix(ikData->_mix);
                        }
                        else
                        {
                            assert(0);
                        }
                    }
                        break;
                }
            }
        }
    }

    void SkeletonInstance::applyAnimationStateSet(AnimationStateSet* animationStateSet, bool inheritChild, bool colorUpdateOnly) //bx
    {
        if (_skeletonData)
        {
            for (auto it : animationStateSet->getAnimationStates())
            {
                if (it.second->getEnabled())
                {
                    auto animation = _skeletonData->getAnimation(it.first);
                    
                    if (animation)
                    {
                        auto animationState = it.second;
                        
                        animation->applyToAnimatable(this, animationState->getCurrentTime(), colorUpdateOnly);
                    }
                }
            }
        }

        if (inheritChild)
        {
            for (auto it : _slots)
            {
                it.right->applyAnimationStateSet(animationStateSet, inheritChild, colorUpdateOnly);
            }
        }
    }

    SceneSlotInstance* SkeletonInstance::addSlot(const std::string& name, const std::string& parentBoneName, float drawOrder)
    {
        auto it = _slots.left.find(name);
        auto it2 = _bones.left.find(parentBoneName);
        
        if (it == _slots.left.end() &&
            it2 != _bones.left.end())
        {
            auto parentBone = it2->second;

            auto slot = new SceneSlotInstance;

            _slots.insert(SlotMapType(name, slot));
            slot->retain();
            
            _addSlotDrawOrder(slot, drawOrder);

            _slotAttaches.insert(SlotAttachMapType(parentBone, slot));
            parentBone->attachObject(slot);

            return slot;
        }
        else
        {
            return nullptr;
        }
    }
    
    SceneSlotInstance* SkeletonInstance::getSlot(const std::string& name) const
    {
        auto it = _slots.left.find(name);
        
        if (it != _slots.left.end())
        {
            return it->second;
        }
        
        return nullptr;
    }
    
    void SkeletonInstance::removeSlot(const std::string& name)
    {
        auto it = _slots.left.find(name);
        
        if (it != _slots.left.end())
        {
            _removeSlotDrawOrder(it->second);
            
            it->second->release();
            _slots.left.erase(it);
        }
    }
    
    void SkeletonInstance::removeSlot(SceneSlotInstance* skeletonSlot)
    {
        auto it = _slots.right.find(skeletonSlot);
        
        if (it != _slots.right.end())
        {
            _removeSlotDrawOrder(skeletonSlot);

            skeletonSlot->release();
            
            _slots.right.erase(it);
        }
    }
    
    void SkeletonInstance::removeSlotsAll()
    {
        for (auto it : _slots)
        {
            it.right->release();
        }
        _slots.clear();
        
        _defaultDrawOrder.clear();
    }
    
    void SkeletonInstance::setSlotSkin(const std::string& skinName)
    {
        for (auto it : _slots)
        {
            it.right->setCurrentSkinName(skinName);
        }
    }

    const SkeletonInstance::SlotMap& SkeletonInstance::getSlots() const
    {
        return _slots;
    }

    void SkeletonInstance::_createSlotsFromSkeleton()
    {
        assert(_slots.size() == 0);
        
        if (_skeletonData)
        {
            for (auto it : _skeletonData->getSlots())
            {
                auto slotName = it.left;
                auto srcSlotData = it.right;

                auto it2 = _skeletonData->getDefaultDrawOrder().right.find(srcSlotData);
                
                if (it2 != _skeletonData->getDefaultDrawOrder().right.end())
                {
                    auto slot = addSlot(slotName, _skeletonData->getSlotParentBoneName(slotName), it2->second);
                    auto bone = getBone(_skeletonData->getSlotParentBoneName(slotName));

                    auto slotData = srcSlotData;
                    assert(slotData);
                    
                    slot->setSceneSlotData(slotData);

                    slot->setRenderPriorityGroup(getRenderPriorityGroup());
                    slot->setRenderPriorityDepth(getRenderPriorityDepth());
                    slot->setRenderPrioritySceneData(_skeletonData);
                    slot->setRenderPrioritySceneDataDepth(it2->second);
                    
                    for (auto it : slot->getSceneInstancesAll())
                    {
                        for (auto it2 : it.second)
                        {
                            auto meshObject = dynamic_cast<MeshInstance*>(it2.second);
                            
                            if (meshObject)
                            {
                                meshObject->setParentSkeletonInstance(this);
                                meshObject->setParentSkeletonBone(bone);
                            }
                        }
                    }
                }
                else
                {
                    assert(0);
                }
            }
        }
    }

    
    
    
    
    
    
    
    
    void SkeletonInstance::setSlotDrawOrder(SceneSlotInstance* skeletonSlot, float drawOrder)
    {
        if (skeletonSlot)
        {
            auto it = _defaultDrawOrder.right.find(skeletonSlot);
            
            if (it != _defaultDrawOrder.right.end())
            {
                _defaultDrawOrder.right.erase(it);

                _defaultDrawOrder.insert(DrawOrderMapType(drawOrder, skeletonSlot));

                skeletonSlot->setRenderPrioritySceneDataDepth(drawOrder);
            }
        }
    }
    
    SceneSlotInstance* SkeletonInstance::getSlotWithDrawOrder(float drawOrder, int32_t index) const
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
    
    void SkeletonInstance::_addSlotDrawOrder(SceneSlotInstance* skeletonSlot, float drawOrder)
    {
        if (skeletonSlot)
        {
            auto it = _defaultDrawOrder.right.find(skeletonSlot);
            
            if (it == _defaultDrawOrder.right.end())
            {
                _defaultDrawOrder.insert(DrawOrderMapType(drawOrder, skeletonSlot));

                skeletonSlot->setRenderPrioritySceneDataDepth(drawOrder);
            }
        }
    }
    
    void SkeletonInstance::_removeSlotDrawOrder(SceneSlotInstance* skeletonSlot)
    {
        if (skeletonSlot)
        {
            auto it = _defaultDrawOrder.right.find(skeletonSlot);
            
            if (it != _defaultDrawOrder.right.end())
            {
                _defaultDrawOrder.right.erase(it);
            }
        }
    }

    
    
    
    
    
    

    
    
    void SkeletonInstance::setBoneRenderAvailable(bool renderable)
    {
        if (_boneRenderable != renderable)
        {
            _boneRenderable = renderable;
            
            if (_boneRenderable)
            {
                for (auto it : _bones)
                {
                    it.right->createSkeletonBoneRenderInstance();
                }
            }
            else
            {
                for (auto it : _bones)
                {
                    it.right->destroySkeletonBoneRenderInstance();
                }
            }
        }
    }
    
    bool SkeletonInstance::getBoneRenderAvailable() const
    {
        return _boneRenderable;
    }
    
    
    
    
    
    
    void SkeletonInstance::_createConstraintsFromSkeleton()
    {
        assert(_spineConstraintFunctors.size() == 0);
        assert(_spineConstraintFunctorsMap.size() == 0);

        if (_skeletonData)
        {
            for (auto spineConstraint : _skeletonData->getSpineConstraints())
            {
                switch (spineConstraint->getSpineConstraintType())
                {
                    default:
                        assert(0);
                        break;

                    case SpineConstraintType::IK:
                    {
                        assert(dynamic_cast<SkeletonIKData*>(spineConstraint));

                        auto ikData = static_cast<SkeletonIKData*>(spineConstraint);

                        assert(ikData->_targetBoneName.size() > 0);
                        assert(ikData->_boneNames.size() == 1 ||
                               ikData->_boneNames.size() == 2);
                        assert(ikData->_bendPositive == 1 ||
                               ikData->_bendPositive == -1);
                        assert(ikData->_mix >= 0.0f &&
                               ikData->_mix <= 1.0f);

                        if (ikData->_boneNames.size() == 1)
                        {
                            auto ik = new SkeletonInstanceIKFunctorApplyOne();
                            ik->autorelease();

                            ik->setName(ikData->getName());
                            
                            ik->_targetBone = getBone(ikData->_targetBoneName);
                            assert(ik->_targetBone);

                            ik->_bone0 = getBone(ikData->_boneNames[0]);
                            assert(ik->_bone0);

                            ik->setDirection(1.0f);
                            
                            ik->setMix(ikData->_mix);

                            _spineConstraintFunctors.push_back(ik);
                            ik->retain();
                            
                            if (ik->getName().size() > 0)
                            {
                                // TODO
                                // check duplicated
                                _spineConstraintFunctorsMap[ik->getName()] = ik;
                            }
                        }
                        else if (ikData->_boneNames.size() == 2)
                        {
                            auto ik = new SkeletonInstanceIKFunctorApplyTwo();
                            ik->autorelease();

                            ik->setName(ikData->getName());

                            ik->_targetBone = getBone(ikData->_targetBoneName);
                            assert(ik->_targetBone);

                            ik->_bone0 = getBone(ikData->_boneNames[0]);
                            assert(ik->_bone0);
                            
                            ik->_bone1 = getBone(ikData->_boneNames[1]);
                            assert(ik->_bone1);

                            ik->setDirection(ikData->_bendPositive == 1 ? 1.0f : -1.0f);

                            ik->setMix(ikData->_mix);
                            
                            _spineConstraintFunctors.push_back(ik);
                            ik->retain();
                            
                            if (ik->getName().size() > 0)
                            {
                                // TODO
                                // check duplicated
                                _spineConstraintFunctorsMap[ik->getName()] = ik;
                            }

                        }
                        else
                        {
                            assert(0);
                        }
                    }
                        break;
                }
            }

            // COMMENT
            // _spineConstraintFunctors 에 들어가는 순서도 중요함.
            // 현재 spine json 데이터가 정렬되어서 나오는 것 같아서 sorting 은 지금 하지 않음.
            // target bone 기준으로 bone hierarchy 순서에 맞춰서 정렬되어 있으면 됨.
        }
    }
    
    void SkeletonInstance::_updateSpineConstraintFunctors()
    {
        std::set<SkeletonBone*> bonesToUpdate;

        for (auto spineConstraintFunctor : _spineConstraintFunctors)
        {
            (*spineConstraintFunctor)(this, bonesToUpdate);
            
            for (auto bone : bonesToUpdate)
            {
                bone->_updateWorldTransformDescending();
            }

            bonesToUpdate.clear();
        }
        
    }

    const std::vector<SpineConstraintFunctor*>& SkeletonInstance::getSpineConstraintFunctors() const
    {
        return _spineConstraintFunctors;
    }
    
    void SkeletonInstance::removeSpineConstraintFunctorsAll()
    {
        for (auto spineConstraintFunctor : _spineConstraintFunctors)
        {
            spineConstraintFunctor->release();
        }
        _spineConstraintFunctors.clear();
        _spineConstraintFunctorsMap.clear();
    }
    
    SpineConstraintFunctor* SkeletonInstance::getSpineConstraintFunctor(const std::string& name) const
    {
        auto it = _spineConstraintFunctorsMap.find(name);
        
        if (it != _spineConstraintFunctorsMap.end())
        {
            return it->second;
        }
        
        return nullptr;
    }

    //bx
    void SkeletonInstance::setCurrentEventName(const std::string& name)
    {
        if(_eventName != name)
        {
            _eventName = name;
            if(0 < name.length())
                SpineBuilder_CallEvent(name.c_str());
        }
    }

    //bx
    const std::string& SkeletonInstance::getCurrentEventName()
    {
        return _eventName;
    }
}
