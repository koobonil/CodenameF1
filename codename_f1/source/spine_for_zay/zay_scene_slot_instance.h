#pragma once

#include "zay_types.h"
#include "zay_scene_instance.h"
#include "zay_animatable.h"
#include "zay_render_priority.h"
#include "zay_node.h"

namespace ZAY
{
    class SceneSlotInstanceNode
    : public Node
    {
    public:
        SceneSlotInstanceNode(SceneSlotInstance* sceneObjectSlot);
        virtual ~SceneSlotInstanceNode();
        
    public:
        SceneSlotInstance* getSceneSlotInstance();
    private:
        SceneSlotInstance* _sceneObjectSlot;

    protected:
        virtual Node* createChildImpl() override;
        
    protected:
        virtual void _updateRootWorldTransform() override;
    protected:
        virtual void _updateRootWorldColor() override;

    };
    
    class SceneSlotInstance
    : public SceneInstance
    , public Animatable
    {
    public:
        SceneSlotInstance();
        virtual ~SceneSlotInstance();

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
        
    public:
        virtual void setTextureAtlasWithTextureAtlasSet(TextureAtlasSet* textureAtlasSet) override;
    
    public:
        SceneSlotInstanceNode* getSlotNode() const;
    private:
        SceneSlotInstanceNode* _slotNode;
        
    public:
        void setSceneSlotData(SceneSlotData* sceneObjectSlotData);
        SceneSlotData* getSceneSlotData() const;
    private:
        SceneSlotData* _sceneObjectSlotData;
        
        
        
    protected:
        virtual void _notifyRenderPriorityGroupChanged() override;
        virtual void _notifyRenderPriorityDepthChanged() override;

    public:
        void setRenderPrioritySceneData(const SceneData* sceneData);
        const SceneData* getRenderPrioritySceneData() const;
        void setRenderPrioritySceneDataDepth(float depth);
        float getRenderPrioritySceneDataDepth() const;
        const RenderPriority& getRenderPriority() const;
    private:
        RenderPriority _renderPriority;
        
        
        
    public:
        virtual void applyInitialAnimationPose(bool inheritChild, bool colorUpdateOnly) override; //bx
        virtual void applyAnimationStateSet(AnimationStateSet* animationStateSet, bool inheritChild, bool colorUpdateOnly) override; //bx

        
        
        
        
    public:
        static const std::string& getDefaultSkinName();

    public:
        void setDefaultAttachmentName(const std::string& defaultAttachmentName);
        const std::string& getDefaultAttachmentName() const;
    private:
        std::string _defaultAttachmentName;

        
        
    public:
        SceneInstance* getSceneInstanceWithNames(const std::string& attachmentName, const std::string& skinName);
    public:
        void setCurrentAttachmentName(const std::string& attachmentName);
        void setCurrentSkinName(const std::string& skinName);
        void _updateCurrentSceneInstance();
        const std::string& getCurrentAttachmentName() const;
        const std::string& getCurrentSkinName() const;
        SceneInstance* getCurrentSceneInstance() const;
    public:
        void addSceneInstance(const std::string& attachmentName, const std::string& skinName, SceneInstance* sceneObject);
        void removeSceneInstanceFromAttachment(const std::string& attachmentName, const std::string& skinName);
        void removeAttachment(const std::string& attachmentName);
        void removeSceneInstancesAll();
        const std::map<std::string, std::map<std::string, SceneInstance*>>& getSceneInstancesAll() const;
    private:
        std::map<std::string, std::map<std::string, SceneInstance*>> _sceneObjects;
        std::string _currentAttachmentName;
        std::string _currentSkinName;
        SceneInstance* _currentSceneInstance;

        
    };
}
