namespace ZAY
{
    extern int USE_EXCLUDED_RED;
}

#include "zay_types.h"
#include "zay_scene_slot_instance.h"
#include "zay_scene_instance.h"
#include "zay_scene_slot_data.h"
#include "zay_scene_data.h"
#include "zay_animation.h"
#include "zay_render_command.h"
#include "zay_animation_state.h"
#include "zay_animation_state_set.h"
#include "zay_touch_area_instance.h"

namespace ZAY
{

    SceneSlotInstanceNode::SceneSlotInstanceNode(SceneSlotInstance* sceneObjectSlot)
    {
        _sceneObjectSlot = sceneObjectSlot;
    }

    SceneSlotInstanceNode::~SceneSlotInstanceNode()
    {

    }

    SceneSlotInstance* SceneSlotInstanceNode::getSceneSlotInstance()
    {
        return _sceneObjectSlot;
    }

    Node* SceneSlotInstanceNode::createChildImpl()
    {
        return nullptr;
    }

    void SceneSlotInstanceNode::_updateRootWorldTransform()
    {
        if (_sceneObjectSlot)
        {
            _sceneObjectSlot->_updateWorldTransformAscending();
        }
    }

    void SceneSlotInstanceNode::_updateRootWorldColor()
    {
        if (_sceneObjectSlot)
        {
            _sceneObjectSlot->_updateWorldColorAscending();
        }
    }

    SceneSlotInstance::SceneSlotInstance()
    {
        _sceneObjectSlotData = nullptr;
        _currentSceneInstance = nullptr;

        _slotNode = new SceneSlotInstanceNode(this);
        _slotNode->retain();
    }

    SceneSlotInstance::~SceneSlotInstance()
    {
        if (_slotNode)
        {
            _slotNode->release();
            _slotNode = nullptr;
        }

        if (_sceneObjectSlotData)
        {
            _sceneObjectSlotData->release();
            _sceneObjectSlotData = nullptr;
        }

        if (_currentSceneInstance)
        {
            _currentSceneInstance->release();
            _currentSceneInstance = nullptr;
        }

        for (const auto& it : _sceneObjects)
        {
            for (const auto& it2 : it.second)
            {
                it2.second->release();
            }
        }
        _sceneObjects.clear();
    }

    void SceneSlotInstance::_updateOneTick()
    {
        SceneInstance::_updateOneTick();

        _slotNode->_updateOneTick();
    }

    void SceneSlotInstance::_updateSubTicks(int32_t subTicks)
    {
        SceneInstance::_updateSubTicks(subTicks);

        _slotNode->_updateSubTicks(subTicks);
    }

    void SceneSlotInstance::_preRender()
    {
        SceneInstance::_preRender();

        _slotNode->_preRender();
    }

    void SceneSlotInstance::_postRender()
    {
        SceneInstance::_postRender();

        _slotNode->_postRender();
    }

    void SceneSlotInstance::_notifyAttachedToRenderer()
    {
        _slotNode->setRenderer(getAttachedRenderer());
    }

    void SceneSlotInstance::_notifyDetachedFromRenderer()
    {
        _slotNode->setRenderer(nullptr);
    }

    void SceneSlotInstance::_notifyAttachedToTouchEventDispatcher()
    {
        _slotNode->setTouchEventDispatcher(getAttachedTouchEventDispatcher());
    }

    void SceneSlotInstance::_notifyDetachedFromTouchEventDispatcher()
    {
        _slotNode->setTouchEventDispatcher(nullptr);
    }

    void SceneSlotInstance::_updateWorldTransformToChildren()
    {
        _slotNode->_updateWorldTransformDescending();
    }

    void SceneSlotInstance::_updateWorldColorToChildren()
    {
        _slotNode->_updateWorldColorDescending();
    }

    void SceneSlotInstance::_notifyWorldTransformUpdated(bool updated)
    {
        if (updated)
        {
            _slotNode->setRotation(getWorldRotation());
            _slotNode->setScale(getWorldScale());
            _slotNode->setPosition(getWorldPosition());
            _slotNode->setFlipX(getWorldFlipX());
            _slotNode->setFlipY(getWorldFlipY());
        }
    }

    void SceneSlotInstance::_notifyWorldColorUpdated(bool updated)
    {
        if (updated)
        {
            _slotNode->setLocalColor(getWorldColor());
        }
    }

    void SceneSlotInstance::setTextureAtlasWithTextureAtlasSet(TextureAtlasSet* textureAtlasSet)
    {
        for (const auto& it : _sceneObjects)
        {
            for (const auto& it2 : it.second)
            {
                it2.second->setTextureAtlasWithTextureAtlasSet(textureAtlasSet);
            }
        }
    }

    SceneSlotInstanceNode* SceneSlotInstance::getSlotNode() const
    {
        return _slotNode;
    }

    void SceneSlotInstance::setSceneSlotData(SceneSlotData* sceneObjectSlotData)
    {
        if (_sceneObjectSlotData != sceneObjectSlotData)
        {
            if (_sceneObjectSlotData)
            {
                _sceneObjectSlotData->release();
                _sceneObjectSlotData = nullptr;
            }



            _defaultAttachmentName.clear();
            removeSceneInstancesAll();




            _sceneObjectSlotData = sceneObjectSlotData;

            if (_sceneObjectSlotData)
            {
                _sceneObjectSlotData->retain();

                setLocalColor(_sceneObjectSlotData->getColour());

                for (const auto& it : _sceneObjectSlotData->getSceneDatas())
                {
                    auto attachmentName = it.first;

                    for (const auto& it2 : it.second)
                    {
                        auto skinName = it2.first;
                        auto _sceneData = it2.second;
                        assert(dynamic_cast<SceneData*>(_sceneData));

                        auto sceneData = static_cast<SceneData*>(_sceneData);
                        auto sceneInstance = sceneData->createSceneInstance();
                        sceneInstance->autorelease();

                        for (auto renderCommand : sceneInstance->getRenderCommands())
                        {
                            renderCommand->setRenderPriorityGroup(_renderPriority.getGroup());
                            renderCommand->setRenderPriorityDepth(_renderPriority.getDepth());
                            renderCommand->setRenderPrioritySceneData(_renderPriority.getSceneData());
                            renderCommand->setRenderPrioritySceneDataDepth(_renderPriority.getSceneDataDepth());
                        }

                        addSceneInstance(attachmentName, skinName, sceneInstance);
                    }
                }

                setDefaultAttachmentName(_sceneObjectSlotData->getDefaultAttachmentName());
                setCurrentAttachmentName(_sceneObjectSlotData->getDefaultAttachmentName());
            }
        }
    }

    SceneSlotData* SceneSlotInstance::getSceneSlotData() const
    {
        return _sceneObjectSlotData;
    }






    void SceneSlotInstance::_notifyRenderPriorityGroupChanged()
    {
        SceneInstance::_notifyRenderPriorityGroupChanged();

        auto priority = getRenderPriorityGroup();

        _renderPriority.setGroup(priority);

        for (const auto& it : _sceneObjects)
        {
            for (const auto& it2 : it.second)
            {
                auto sceneObject = it2.second;

                for (auto renderCommand : sceneObject->getRenderCommands())
                {
                    renderCommand->setRenderPriorityGroup(priority);
                }
            }
        }
    }

    void SceneSlotInstance::_notifyRenderPriorityDepthChanged()
    {
        SceneInstance::_notifyRenderPriorityDepthChanged();

        auto depth = getRenderPriorityDepth();

        _renderPriority.setDepth(depth);

        for (const auto& it : _sceneObjects)
        {
            for (const auto& it2 : it.second)
            {
                auto sceneObject = it2.second;

                for (auto renderCommand : sceneObject->getRenderCommands())
                {
                    renderCommand->setRenderPriorityDepth(depth);
                }
            }
        }
    }




    void SceneSlotInstance::setRenderPrioritySceneData(const SceneData* sceneData)
    {
        if (_renderPriority.getSceneData() != sceneData)
        {
            _renderPriority.setSceneData(sceneData);

            for (const auto& it : _sceneObjects)
            {
                for (const auto& it2 : it.second)
                {
                    auto sceneObject = it2.second;

                    for (auto renderCommand : sceneObject->getRenderCommands())
                    {
                        renderCommand->setRenderPrioritySceneData(sceneData);
                    }
                }
            }
        }
    }

    const SceneData* SceneSlotInstance::getRenderPrioritySceneData() const
    {
        return _renderPriority.getSceneData();
    }

    void SceneSlotInstance::setRenderPrioritySceneDataDepth(float depth)
    {
        if (_renderPriority.getSceneDataDepth() != depth)
        {
            _renderPriority.setSceneDataDepth(depth);

            for (const auto& it : _sceneObjects)
            {
                for (const auto& it2 : it.second)
                {
                    auto sceneObject = it2.second;

                    for (auto renderCommand : sceneObject->getRenderCommands())
                    {
                        renderCommand->setRenderPrioritySceneDataDepth(depth);
                    }
                }
            }
        }
    }

    float SceneSlotInstance::getRenderPrioritySceneDataDepth() const
    {
        return _renderPriority.getSceneDataDepth();
    }

    const RenderPriority& SceneSlotInstance::getRenderPriority() const
    {
        return _renderPriority;
    }









    void SceneSlotInstance::applyInitialAnimationPose(bool inheritChild, bool colorUpdateOnly) //bx
    {
        setCurrentAttachmentName(getDefaultAttachmentName());

        if (_sceneObjectSlotData)
        {
            setLocalColor(_sceneObjectSlotData->getColour());
        }

        if (inheritChild)
        {
            for (const auto& it : _sceneObjects)
            {
                for (const auto& it2 : it.second)
                {
                    auto animatable = dynamic_cast<Animatable*>(it2.second);

                    if (animatable)
                    {
                        animatable->applyInitialAnimationPose(inheritChild, colorUpdateOnly);
                    }
                }
            }
        }
    }

    void SceneSlotInstance::applyAnimationStateSet(AnimationStateSet* animationStateSet, bool inheritChild, bool colorUpdateOnly) //bx
    {
        if (_sceneObjectSlotData)
        {
            bool needToSetInitState = false;
            for (const auto& it : animationStateSet->getAnimationStates())
            {
                if (it.second->getEnabled())
                {
                    auto animation = _sceneObjectSlotData->getAnimation(it.first);

                    if (animation)
                    {
                        auto animationState = it.second;

                        if (animationState->getNeedToSetInitState())
                        {
                            needToSetInitState = true;
                        }
                    }
                }
            }

            if (needToSetInitState)
            {
                applyInitialAnimationPose(inheritChild, colorUpdateOnly);
            }

            for (const auto& it : animationStateSet->getAnimationStates())
            {
                if (it.second->getEnabled())
                {
                    auto animation = _sceneObjectSlotData->getAnimation(it.first);

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
            //bx1111: 빨간색이면 애니메이션 연산제외
            if(USE_EXCLUDED_RED && !colorUpdateOnly)
            {
                const ColourValue& Color = getWorldColor();
                if(Color.r == 1 && Color.g == 0 && Color.b == 0)
                    inheritChild = false;
            }

            if(inheritChild)
            {
                for (const auto& it : _sceneObjects)
                {
                    for (const auto& it2 : it.second)
                    {
                        auto animatable = dynamic_cast<Animatable*>(it2.second);

                        if (animatable)
                        {
                            animatable->applyAnimationStateSet(animationStateSet, inheritChild, colorUpdateOnly);
                        }
                    }
                }

                //bx1111: 바운딩박스 랜더링
                const SceneInstance* CurSceneInstance = getCurrentSceneInstance();
                const TouchAreaInstance* CurTouchAreaInstance = dynamic_cast<const TouchAreaInstance*>(CurSceneInstance);
                if(CurTouchAreaInstance)
                    CurTouchAreaInstance->renderBoundBox();
            }
        }
    }




    const std::string& SceneSlotInstance::getDefaultSkinName()
    {
        return SceneSlotData::getDefaultSkinName();
    }





    void SceneSlotInstance::setDefaultAttachmentName(const std::string& defaultAttachmentName)
    {
        _defaultAttachmentName = defaultAttachmentName;
    }

    const std::string& SceneSlotInstance::getDefaultAttachmentName() const
    {
        return _defaultAttachmentName;
    }





    SceneInstance* SceneSlotInstance::getSceneInstanceWithNames(const std::string& attachmentName, const std::string& skinName)
    {
        auto it = _sceneObjects.find(attachmentName);

        if (it != _sceneObjects.end())
        {
            {
                auto it2 = it->second.find(skinName);

                if (it2 != it->second.end())
                {
                    return it2->second;
                }
            }

            {
                auto it2 = it->second.find(getDefaultSkinName());

                if (it2 != it->second.end())
                {
                    return it2->second;
                }
            }
        }
        //bx:else assert(attachmentName.size() == 0);

        return nullptr;
    }

    void SceneSlotInstance::setCurrentAttachmentName(const std::string& attachmentName)
    {
        if (_currentAttachmentName != attachmentName)
        {
            _currentAttachmentName = attachmentName;

            _updateCurrentSceneInstance();
        }
    }

    void SceneSlotInstance::setCurrentSkinName(const std::string& skinName)
    {
        if (_currentSkinName != skinName)
        {
            _currentSkinName = skinName;

            _updateCurrentSceneInstance();
        }
    }

    void SceneSlotInstance::_updateCurrentSceneInstance()
    {
        auto sceneObject = getSceneInstanceWithNames(_currentAttachmentName, _currentSkinName);

        if (_currentSceneInstance != sceneObject)
        {
            if (_currentSceneInstance)
            {
                _slotNode->detachObject(_currentSceneInstance);
                _currentSceneInstance->release();
                _currentSceneInstance = nullptr;
            }

            _currentSceneInstance = sceneObject;

            if (_currentSceneInstance)
            {
                _currentSceneInstance->retain();
                _slotNode->attachObject(_currentSceneInstance);
            }
        }
    }

    const std::string& SceneSlotInstance::getCurrentAttachmentName() const
    {
        return _currentAttachmentName;
    }

    const std::string& SceneSlotInstance::getCurrentSkinName() const
    {
        return _currentSkinName;
    }

    SceneInstance* SceneSlotInstance::getCurrentSceneInstance() const
    {
        return _currentSceneInstance;
    }






    void SceneSlotInstance::addSceneInstance(const std::string& attachmentName, const std::string& skinName, SceneInstance* sceneObject)
    {
        removeSceneInstanceFromAttachment(attachmentName, skinName);

        _sceneObjects[attachmentName][skinName] = sceneObject;
        sceneObject->retain();

        _updateCurrentSceneInstance();
    }

    void SceneSlotInstance::removeSceneInstanceFromAttachment(const std::string& attachmentName, const std::string& skinName)
    {
        auto it = _sceneObjects.find(attachmentName);

        if (it != _sceneObjects.end())
        {
            auto it2 = it->second.find(skinName);

            if (it2 != it->second.end())
            {
                it2->second->release();
                it->second.erase(it2);

                if (it->second.size() == 0)
                {
                    _sceneObjects.erase(it);
                }

                _updateCurrentSceneInstance();
            }
        }
    }

    void SceneSlotInstance::removeAttachment(const std::string& attachmentName)
    {
        if (_currentAttachmentName == attachmentName)
        {
            if (_currentSceneInstance)
            {
                _slotNode->detachObject(_currentSceneInstance);

                _currentSceneInstance->release();
                _currentSceneInstance = nullptr;
            }
        }

        auto it = _sceneObjects.find(attachmentName);

        if (it != _sceneObjects.end())
        {
            for (const auto& it2 : it->second)
            {
                it2.second->release();
            }

            _sceneObjects.erase(it);
        }
    }

    void SceneSlotInstance::removeSceneInstancesAll()
    {
        for (const auto& it : _sceneObjects)
        {
            for (const auto& it2 : it.second)
            {
                it2.second->release();
            }
        }

        if (_currentSceneInstance)
        {
            _slotNode->detachObject(_currentSceneInstance);

            _currentSceneInstance->release();
            _currentSceneInstance = nullptr;
        }

        _sceneObjects.clear();
    }

    const std::map<std::string, std::map<std::string, SceneInstance*>>& SceneSlotInstance::getSceneInstancesAll() const
    {
        return _sceneObjects;
    }
}
