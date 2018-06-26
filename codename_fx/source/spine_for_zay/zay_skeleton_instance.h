#pragma once

#include "zay_types.h"
#include "zay_scene_instance.h"
#include "zay_scene_slot_instance.h"
#include "zay_animatable.h"

namespace ZAY
{
    class SpineConstraintFunctor
    : public Base
    {
    public:
        virtual void operator ()(SkeletonInstance* SkeletonInstance, std::set<SkeletonBone*>& toUpdate) = 0;
        
    public:
        void setName(const std::string& name);
        const std::string& getName() const;
    private:
        std::string _name;
    };
    
    class SkeletonInstanceIKFunctor
    : public SpineConstraintFunctor
    {
    public:
        SkeletonInstanceIKFunctor();
        virtual ~SkeletonInstanceIKFunctor();
        
    public:
        void setMix(float mix);
        float getMix() const;
    protected:
        float _mix;
        
    public:
        void setDirection(float direction);
        float getDirection() const;
    protected:
        float _direction;
    };
    
    class SkeletonInstanceIKFunctorApplyOne
    : public SkeletonInstanceIKFunctor
    {
    public:
        SkeletonInstanceIKFunctorApplyOne();
        virtual ~SkeletonInstanceIKFunctorApplyOne();
        
        virtual void operator ()(SkeletonInstance* SkeletonInstance, std::set<SkeletonBone*>& toUpdate) override;

        SkeletonBone* _bone0;
        SkeletonBone* _targetBone;
    };

    class SkeletonInstanceIKFunctorApplyTwo
    : public SkeletonInstanceIKFunctor
    {
    public:
        SkeletonInstanceIKFunctorApplyTwo();
        virtual ~SkeletonInstanceIKFunctorApplyTwo();
        
        virtual void operator ()(SkeletonInstance* SkeletonInstance, std::set<SkeletonBone*>& toUpdate) override;
        
        SkeletonBone* _bone0;
        SkeletonBone* _bone1;
        SkeletonBone* _targetBone;
    };

    class SkeletonInstance
    : public SceneInstance
    , public Animatable
    {
    public:
        SkeletonInstance();
        virtual ~SkeletonInstance();

    public:
        virtual void _updateOneTick() override;
        virtual void _updateSubTicks(int32_t subTicks) override;

    public:
        virtual void _preRender() override;
        virtual void _postRender() override;

    protected:
        virtual void _notifyAttachedToRenderer() override;
        virtual void _notifyDetachedFromRenderer() override;

    protected:
        virtual void _notifyAttachedToTouchEventDispatcher() override;
        virtual void _notifyDetachedFromTouchEventDispatcher() override;

    protected:
        virtual void _updateWorldTransformToChildren() override;
        virtual void _updateWorldColorToChildren() override;

    protected:
        virtual void _notifyWorldTransformUpdated(bool updated) override;
        virtual void _notifyWorldColorUpdated(bool updated) override;
        
    protected:
        virtual void _notifyRenderPriorityGroupChanged() override;
        virtual void _notifyRenderPriorityDepthChanged() override;

    public:
        virtual void setTextureAtlasWithTextureAtlasSet(TextureAtlasSet* textureAtlasSet) override;

    public:
        void setSkeletonData(SkeletonData* skeletonData);
        SkeletonData* getSkeletonData() const;
    private:
        SkeletonData* _skeletonData;

    public:
        typedef boost::bimap<std::string, SkeletonBone*> BoneMap;
        typedef BoneMap::value_type BoneMapType;
        typedef std::set<SkeletonBone*> BoneSet;
        typedef std::vector<SkeletonBone*> BoneVector;
    public:
        SkeletonBone* addRootBone(const std::string& name);
        SkeletonBone* getBone(const std::string& name) const;
        SkeletonBone* getBone(int32_t index) const;
        void setBoneIndices(const BoneVector& boneIndices);
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
    private:
        void _createBonesFromSkeleton();
    private:
        SkeletonBone* _grandParentBone;
        BoneMap _bones;
        BoneSet _rootBones;
        BoneVector _bonesIndexable;


        
    public:
        virtual void applyInitialAnimationPose(bool inheritChild, bool colorUpdateOnly) override; //bx
        virtual void applyAnimationStateSet(AnimationStateSet* animationStateSet, bool inheritChild, bool colorUpdateOnly) override; //bx

        
        
    public:
        typedef boost::bimap<std::string, SceneSlotInstance*> SlotMap;
        typedef SlotMap::value_type SlotMapType;
        typedef boost::bimap<boost::bimaps::multiset_of<SkeletonBone*>, boost::bimaps::set_of<SceneSlotInstance*>> SlotAttachMap;
        typedef SlotAttachMap::value_type SlotAttachMapType;
    public:
        SceneSlotInstance* addSlot(const std::string& name, const std::string& parentBoneName, float drawOrder = 0.0f);
        SceneSlotInstance* getSlot(const std::string& name) const;
        void removeSlot(const std::string& name);
        void removeSlot(SceneSlotInstance* skeletonSlot);
        void removeSlotsAll();
        void setSlotSkin(const std::string& skinName);
		void setSlotSkin(const std::string& slot, const std::string& name);
    public:
        const SlotMap& getSlots() const;
    private:
        void _createSlotsFromSkeleton();
    private:
        SlotMap _slots;
        SlotAttachMap _slotAttaches;

        
        
        
        
        
    public:
        typedef boost::bimap<boost::bimaps::multiset_of<float>, boost::bimaps::set_of<SceneSlotInstance*>> DrawOrderMap;
        typedef DrawOrderMap::value_type DrawOrderMapType;
    public:
        void setSlotDrawOrder(SceneSlotInstance* skeletonSlot, float drawOrder);
        SceneSlotInstance* getSlotWithDrawOrder(float drawOrder, int32_t index = 0) const;
    private:
        void _addSlotDrawOrder(SceneSlotInstance* skeletonSlot, float drawOrder = 0.0f);
        void _removeSlotDrawOrder(SceneSlotInstance* skeletonSlot);
    private:
        DrawOrderMap _defaultDrawOrder;

        
        
        
        
        
    public:
        void setBoneRenderAvailable(bool renderable);
        bool getBoneRenderAvailable() const;

    private:
        bool _boneRenderable;
        
        
        
        
        
    public:
        void _createConstraintsFromSkeleton();
        void _updateSpineConstraintFunctors();
    public:
        const std::vector<SpineConstraintFunctor*>& getSpineConstraintFunctors() const;
        void removeSpineConstraintFunctorsAll();
        SpineConstraintFunctor* getSpineConstraintFunctor(const std::string& name) const;
    private:
        std::vector<SpineConstraintFunctor*> _spineConstraintFunctors;
        std::map<std::string, SpineConstraintFunctor*> _spineConstraintFunctorsMap;



    //bx
    public:
        void setCurrentEventName(const std::string& name, bool pulse);
        const std::string& getCurrentEventName();
    private:
        std::string _eventName;
    };
}
