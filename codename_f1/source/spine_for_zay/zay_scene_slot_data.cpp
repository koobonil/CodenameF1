#include "zay_types.h"
#include "zay_scene_slot_data.h"
#include "zay_scene_slot_instance.h"
#include "zay_scene_data.h"

namespace ZAY
{
    SceneSlotData::SceneSlotData()
    {
        _colour = ColourValue::White;
        _blendType = ForwardMultiplyRender::BlendType::Alpha;
    }

    SceneSlotData::~SceneSlotData()
    {
        _sceneDatas.clear();
    }

    SceneInstance* SceneSlotData::createSceneInstance()
    {
        auto sceneObjectSlot = new SceneSlotInstance;

        sceneObjectSlot->setSceneSlotData(this);

        return sceneObjectSlot;
    }

    void SceneSlotData::setSkin(const std::string& attachmentName, const std::string& skinName, SceneData* sceneData)
    {
        _sceneDatas[attachmentName][skinName] = sceneData;
    }
    
    void SceneSlotData::removeSkinFromAttachment(const std::string& attachmentName, const std::string& skinName)
    {
        auto it = _sceneDatas.find(attachmentName);
        
        if (it != _sceneDatas.end())
        {
            auto it2 = it->second.find(skinName);
            
            if (it2 != it->second.end())
            {
                it->second.erase(it2);
            }
        }
    }
    
    void SceneSlotData::removeAttachment(const std::string& attachmentName)
    {
        _sceneDatas.erase(attachmentName);
    }
    
    const std::map<std::string, std::map<std::string, SceneData*>>& SceneSlotData::getSceneDatas() const
    {
        return _sceneDatas;
    }
    
    void SceneSlotData::setDefaultAttachmentName(const std::string& attachmentName)
    {
        _defaultAttachmentName = attachmentName;
    }
    
    const std::string& SceneSlotData::getDefaultAttachmentName() const
    {
        return _defaultAttachmentName;
    }
    
    const std::string& SceneSlotData::getDefaultSkinName()
    {
        static const std::string s_defaultSkineName("default");

        return s_defaultSkineName;
    }
    
    void SceneSlotData::setColour(const ColourValue& color)
    {
        _colour = color;
    }
    
    const ColourValue& SceneSlotData::getColour() const
    {
        return _colour;
    }
    
    void SceneSlotData::setBlendMode(ForwardMultiplyRender::BlendType blendType)
    {
        _blendType = blendType;
    }
    
    ForwardMultiplyRender::BlendType SceneSlotData::getBlendMode() const
    {
        return _blendType;
    }
}
