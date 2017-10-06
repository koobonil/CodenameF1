#pragma once

#include "zay_types.h"
#include "zay_scene_data.h"
#include "zay_scene_instance.h"
#include "zay_scene_slot_data.h"
#include "zay_scene_slot_instance.h"
#include "zay_animation_container.h"

namespace ZAY
{
    enum class SpineConstraintType
    : std::uint32_t
    {
        IK,
        
        COUNT
    };
    
    class SpineConstraint
    : public Base
    {
    public:
        virtual SpineConstraintType getSpineConstraintType() const = 0;

        void setName(const std::string& name);
        const std::string& getName() const;

        std::string _name;
    };

    class SkeletonIKData
    : public SpineConstraint
    {
    public:
        SkeletonIKData()
        {
        }
        
        virtual ~SkeletonIKData() {}
        
        virtual SpineConstraintType getSpineConstraintType() const override
        {
            return SpineConstraintType::IK;
        }

        std::string _targetBoneName;
        std::vector<std::string> _boneNames;
        int32_t _bendPositive;
        float _mix;
    };
    
    
    
    
    class SkeletonData
    : public SceneData
    , public AnimationContainer
    {
    public:
        friend class SkeletonInstance;

    public:
        SkeletonData();
        virtual ~SkeletonData();

    public:
        virtual SceneInstance* createSceneInstance() override;

    public:
        typedef boost::bimap<std::string, SkeletonBone*> BoneMap;
        typedef BoneMap::value_type BoneMapType;
        typedef std::set<SkeletonBone*> BoneSet;
        typedef std::vector<SkeletonBone*> BoneVector;
    public:
        SkeletonBone* createBone(const std::string& name);
        SkeletonBone* createRootBone(const std::string& name);
        SkeletonBone* getBone(const std::string& name) const;
        SkeletonBone* getBone(int32_t index) const;
        int32_t getBoneIndex(const std::string& name) const;
        int32_t getBoneIndex(SkeletonBone* bone) const;
        const std::string& getBoneName(SkeletonBone* bone) const;
        const std::string& getBoneName(int32_t index) const;
        void removeBone(const std::string& name);
        void removeBone(SkeletonBone* bone);
        void removeBonesAll();
    public:
        const BoneMap& getBones() const;
        const BoneSet& getRootBones() const;
        const BoneVector& getBonesIndexable() const;
    private:
        BoneMap _bones;
        BoneSet _rootBones;
        BoneVector _bonesIndexable;
        
        
        
    public:
        typedef boost::bimap<std::string, SceneSlotData*> SlotMap;
        typedef SlotMap::value_type SlotMapType;
    public:
        SceneSlotData* createAndAddSlot(const std::string& name, float drawOrder = 0.0f);
        const SceneSlotData* getSlotObject(const std::string& name) const;
        void removeSlot(const std::string& name);
        void removeSlotsAll();
    public:
        const SlotMap& getSlots() const;
    private:
        SlotMap _slots;

    public:
        typedef std::map<std::string, std::string> SlotParentBoneMap;
        typedef SlotParentBoneMap::value_type SlotParentBoneMapType;
    public:
        void setSlotParentBoneName(const std::string& slotName, const std::string& parentBoneName);
        const std::string& getSlotParentBoneName(const std::string& slotName) const;
        void removeSlotParentBoneName(const std::string& slotName);
        void removeSlotParentBonesAll();
    public:
        const SlotParentBoneMap& getSlotParentBones() const;
    private:
        SlotParentBoneMap _slotParentBones;

        
        // skinning 처리
    public:
        struct SkinningData
        {
            SkinningData()
            {
                v_index = 0;
                offsetX = 0.0f;
                offsetY = 0.0f;
                weight = 0.0f;
            }

            SkinningData(int32_t v_index, float offsetX, float offsetY, float weight)
            {
                this->v_index = v_index;
                this->offsetX = offsetX;
                this->offsetY = offsetY;
                this->weight = weight;
            }
            
            int32_t v_index;
            float offsetX;
            float offsetY;
            float weight;
        };

    public:
        typedef boost::bimap<boost::bimaps::multiset_of<float>, const SceneSlotData*> DrawOrderMap;
        typedef DrawOrderMap::value_type DrawOrderMapType;
    public:
        void setSlotDrawOrder(const SceneSlotData* skeletonSlot, float drawOrder);
        const SceneSlotData* getSlotWithDrawOrder(float drawOrder, int32_t index = 0) const;
    private:
        void _addSlotDrawOrder(const SceneSlotData* skeletonSlot, float drawOrder = 0.0f);
        void _removeSlotDrawOrder(const SceneSlotData* skeletonSlot);
        void _removeSlotDrawOrdersAll();
    public:
        const DrawOrderMap& getDefaultDrawOrder() const;
    private:
        DrawOrderMap _defaultDrawOrder;

        
        


    public:
        void setWidth(float width);
        void setHeight(float height);
        float getWidth() const;
        float getHeight() const;
    private:
        float _width;
        float _height;

    public:
        const std::vector<SpineConstraint*>& getSpineConstraints() const;
        void removeSpineConstraintsAll();
        SpineConstraint* getSpineConstraint(const std::string& name) const;
    public:
        std::vector<SpineConstraint*> _spineConstraints;
        std::map<std::string, SpineConstraint*> _spineConstraintsMap;

    public:
        void createAnimationStates(AnimationStateSet& animationStatesSet);
    };
}
