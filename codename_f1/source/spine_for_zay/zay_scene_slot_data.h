#pragma once

#include "zay_types.h"
#include "zay_scene_data.h"
#include "zay_animation_container.h"
#include "zay_colour_value.h"

namespace ZAY
{
    class SceneSlotData
    : public SceneData
    , public AnimationContainer
    {
    public:
        SceneSlotData();
        virtual ~SceneSlotData();
        
    public:
        virtual SceneInstance* createSceneInstance() override;

    public:
        void setSkin(const std::string& attachmentName, const std::string& skinName, SceneData* sceneData);
        void removeSkinFromAttachment(const std::string& attachmentName, const std::string& skinName);
        void removeAttachment(const std::string& attachmentName);
        const std::map<std::string, std::map<std::string, SceneData*>>& getSceneDatas() const;
    private:
        std::map<std::string, std::map<std::string, SceneData*>> _sceneDatas;

    public:
        void setDefaultAttachmentName(const std::string& attachmentName);
        const std::string& getDefaultAttachmentName() const;
    private:
        std::string _defaultAttachmentName;
        
    public:
        static const std::string& getDefaultSkinName();

        
        
    public:
        void setColour(const ColourValue& color);
        const ColourValue& getColour() const;
    private:
        ColourValue _colour;
        
        
        
    public:
        void setBlendMode(ForwardMultiplyRender::BlendType blendType);
        ForwardMultiplyRender::BlendType getBlendMode() const;
    private:
        ForwardMultiplyRender::BlendType _blendType;
        
        
        
        
    };
}
