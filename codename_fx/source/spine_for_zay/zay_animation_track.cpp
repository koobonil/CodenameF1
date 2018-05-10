#include "zay_types.h"
#include "zay_animation_track.h"
#include "zay_scene_slot_instance.h"
#include "zay_skeleton_bone.h"
#include "zay_skeleton_instance.h"
#include "zay_mesh_instance.h"
#include "zay_renderer.h"
#include "zay_forward_multiply_renderer.h"

namespace ZAY
{
    int USE_MERGED_ANIMATION_COLOR = 0;

    AnimationTrack::AnimationTrack()
    {
    }

    AnimationTrack::~AnimationTrack()
    {
    }

    void AnimationTrack::registerAnimationTrackType(const std::string& typeName, const std::function<AnimationTrack*()>& creator)
    {
        s_animationTrackCreators[typeName] = creator;
    }
    
    void AnimationTrack::unregisterAnimationTrackType(const std::string& typeName)
    {
        s_animationTrackCreators.erase(typeName);
    }
    
    AnimationTrack* AnimationTrack::createAnimationTrack(const std::string& typeName)
    {
        auto it = s_animationTrackCreators.find(typeName);

        if (it != s_animationTrackCreators.end())
        {
            return it->second();
        }
        else
        {
            return nullptr;
        }
    }

    std::map<std::string, std::function<AnimationTrack*()>> AnimationTrack::s_animationTrackCreators;
    
    void AnimationTrack::initAnimationTrackCreators()
    {
        static const std::string AnimationTrack_SceneSlotInstanceColor("AnimationTrack_SceneSlotInstanceColor");
        registerAnimationTrackType(AnimationTrack_SceneSlotInstanceColor,
                                   []() -> AnimationTrack*
                                   {
                                       auto track = new AnimationTrackColour();
                                       track->getTypeName = []() -> const std::string& { return AnimationTrack_SceneSlotInstanceColor; };
                                       
                                       auto setter = [](const AnimationTrack*, Animatable* animatable, const ColourValue& color)
                                       {
                                           assert(dynamic_cast<SceneSlotInstance*>(animatable));
                                           SceneSlotInstance* skeletonSlot = static_cast<SceneSlotInstance*>(animatable);
                                           if(USE_MERGED_ANIMATION_COLOR)
                                               skeletonSlot->setLocalColorMultiply(color);
                                           else skeletonSlot->setLocalColor(color);
                                       };
                                       
                                       auto getter = [](const AnimationTrack*, Animatable* animatable) -> ColourValue
                                       {
                                           assert(dynamic_cast<SceneSlotInstance*>(animatable));
                                           SceneSlotInstance* skeletonSlot = static_cast<SceneSlotInstance*>(animatable);
                                           return skeletonSlot->getLocalColor();
                                       };
                                       
                                       track->setSetter(setter);
                                       track->setGetter(getter);
                                       
                                       return track;
                                   });
        
        static const std::string AnimationTrack_SceneSlotInstanceAttachment("AnimationTrack_SceneSlotInstanceAttachment");
        registerAnimationTrackType(AnimationTrack_SceneSlotInstanceAttachment,
                                   []() -> AnimationTrack*
                                   {
                                       auto track = new AnimationTrackString();
                                       track->getTypeName = []() -> const std::string& { return AnimationTrack_SceneSlotInstanceAttachment; };
                                       
                                       auto setter = [](const AnimationTrack*, Animatable* animatable, const std::string& name)
                                       {
                                           assert(dynamic_cast<SceneSlotInstance*>(animatable));
                                           SceneSlotInstance* skeletonSlot = static_cast<SceneSlotInstance*>(animatable);
                                           skeletonSlot->setCurrentAttachmentName(name);
                                       };
                                       
                                       auto getter = [](const AnimationTrack*, Animatable* animatable) -> std::string
                                       {
                                           assert(dynamic_cast<SceneSlotInstance*>(animatable));
                                           SceneSlotInstance* skeletonSlot = static_cast<SceneSlotInstance*>(animatable);
                                           return skeletonSlot->getCurrentAttachmentName();
                                       };
                                       
                                       track->setSetter(setter);
                                       track->setGetter(getter);
                                       
                                       return track;
                                   });
        
        static const std::string AnimationTrack_SkeletonBoneRotate("AnimationTrack_SkeletonBoneRotate");
        registerAnimationTrackType(AnimationTrack_SkeletonBoneRotate,
                                   []() -> AnimationTrack*
                                   {
                                       auto track = new AnimationTrackFloat();
                                       track->getTypeName = []() -> const std::string& { return AnimationTrack_SkeletonBoneRotate; };
                                       
                                       auto setter = [](const AnimationTrack* animationTrack, Animatable* animatable, const float& rotation)
                                       {
                                           auto bone_name = animationTrack->boneName;
                                           
                                           assert(dynamic_cast<SkeletonInstance*>(animatable));
                                           SkeletonInstance* skeleton = static_cast<SkeletonInstance*>(animatable);
                                           auto bone = skeleton->getBone(bone_name);
                                           bone->setRotation(rotation);
                                       };
                                       
                                       auto getter = [](const AnimationTrack* animationTrack, Animatable* animatable) -> float
                                       {
                                           auto bone_name = animationTrack->boneName;

                                           assert(dynamic_cast<SkeletonInstance*>(animatable));
                                           SkeletonInstance* skeleton = static_cast<SkeletonInstance*>(animatable);
                                           auto bone = skeleton->getBone(bone_name);

                                           return bone->getRotation();
                                       };
                                       
                                       track->setSetter(setter);
                                       track->setGetter(getter);
                                       
                                       return track;
                                   });
        
        static const std::string AnimationTrack_SkeletonBoneScale("AnimationTrack_SkeletonBoneScale");
        registerAnimationTrackType(AnimationTrack_SkeletonBoneScale,
                                   []() -> AnimationTrack*
                                   {
                                       auto track = new AnimationTrackVector3();
                                       track->getTypeName = []() -> const std::string& { return AnimationTrack_SkeletonBoneScale; };
                                       
                                       auto setter = [](const AnimationTrack* animationTrack, Animatable* animatable, const Vector3& scale)
                                       {
                                           auto bone_name = animationTrack->boneName;

                                           assert(dynamic_cast<SkeletonInstance*>(animatable));
                                           SkeletonInstance* skeleton = static_cast<SkeletonInstance*>(animatable);
                                           auto bone = skeleton->getBone(bone_name);
                                           bone->setScale(scale);
                                       };
                                       
                                       auto getter = [](const AnimationTrack* animationTrack, Animatable* animatable) -> Vector3
                                       {
                                           auto bone_name = animationTrack->boneName;

                                           assert(dynamic_cast<SkeletonInstance*>(animatable));
                                           SkeletonInstance* skeleton = static_cast<SkeletonInstance*>(animatable);
                                           auto bone = skeleton->getBone(bone_name);
                                           
                                           return bone->getScale();
                                       };
                                       
                                       track->setSetter(setter);
                                       track->setGetter(getter);
                                       
                                       return track;
                                   });

        static const std::string AnimationTrack_SkeletonBoneTranslate("AnimationTrack_SkeletonBoneTranslate");
        registerAnimationTrackType(AnimationTrack_SkeletonBoneTranslate,
                                   []() -> AnimationTrack*
                                   {
                                       auto track = new AnimationTrackVector3();
                                       track->getTypeName = []() -> const std::string& { return AnimationTrack_SkeletonBoneTranslate; };

                                       auto setter = [](const AnimationTrack* animationTrack, Animatable* animatable, const Vector3& position)
                                       {
                                           auto bone_name = animationTrack->boneName;

                                           assert(dynamic_cast<SkeletonInstance*>(animatable));
                                           SkeletonInstance* skeleton = static_cast<SkeletonInstance*>(animatable);
                                           auto bone = skeleton->getBone(bone_name);
                                           bone->setPosition(position);
                                       };
                                       
                                       auto getter = [](const AnimationTrack* animationTrack, Animatable* animatable) -> Vector3
                                       {
                                           auto bone_name = animationTrack->boneName;

                                           assert(dynamic_cast<SkeletonInstance*>(animatable));
                                           SkeletonInstance* skeleton = static_cast<SkeletonInstance*>(animatable);
                                           auto bone = skeleton->getBone(bone_name);
                                           return bone->getPosition();
                                       };
                                       
                                       track->setSetter(setter);
                                       track->setGetter(getter);

                                       
                                       return track;
                                   });

        static const std::string AnimationTrack_SkeletonBoneFlipX("AnimationTrack_SkeletonBoneFlipX");
        registerAnimationTrackType(AnimationTrack_SkeletonBoneFlipX,
                                   []() -> AnimationTrack*
                                   {
                                       auto track = new AnimationTrackBoolean();
                                       track->getTypeName = []() -> const std::string& { return AnimationTrack_SkeletonBoneFlipX; };

                                       auto setter = [](const AnimationTrack* animationTrack, Animatable* animatable, const bool& x)
                                       {
                                           auto bone_name = animationTrack->boneName;

                                           assert(dynamic_cast<SkeletonInstance*>(animatable));
                                           SkeletonInstance* skeleton = static_cast<SkeletonInstance*>(animatable);
                                           auto bone = skeleton->getBone(bone_name);
                                           bone->setFlipX(x);
                                       };
                                       
                                       auto getter = [](const AnimationTrack* animationTrack, Animatable* animatable) -> bool
                                       {
                                           auto bone_name = animationTrack->boneName;

                                           assert(dynamic_cast<SkeletonInstance*>(animatable));
                                           SkeletonInstance* skeleton = static_cast<SkeletonInstance*>(animatable);
                                           return skeleton->getBone(bone_name)->getFlipX();
                                       };
                                       
                                       track->setSetter(setter);
                                       track->setGetter(getter);
                                       
                                       return track;
                                   });

        static const std::string AnimationTrack_SkeletonBoneFlipY("AnimationTrack_SkeletonBoneFlipY");
        registerAnimationTrackType(AnimationTrack_SkeletonBoneFlipY,
                                   []() -> AnimationTrack*
                                   {
                                       auto track = new AnimationTrackBoolean();
                                       track->getTypeName = []() -> const std::string& { return AnimationTrack_SkeletonBoneFlipY; };

                                       auto setter = [](const AnimationTrack* animationTrack, Animatable* animatable, const bool& y)
                                       {
                                           auto bone_name = animationTrack->boneName;

                                           assert(dynamic_cast<SkeletonInstance*>(animatable));
                                           SkeletonInstance* skeleton = static_cast<SkeletonInstance*>(animatable);
                                           auto bone = skeleton->getBone(bone_name);
                                           bone->setFlipY(y);
                                       };
                                       
                                       auto getter = [](const AnimationTrack* animationTrack, Animatable* animatable) -> bool
                                       {
                                           auto bone_name = animationTrack->boneName;

                                           assert(dynamic_cast<SkeletonInstance*>(animatable));
                                           SkeletonInstance* skeleton = static_cast<SkeletonInstance*>(animatable);
                                           return skeleton->getBone(bone_name)->getFlipY();
                                       };
                                       
                                       track->setSetter(setter);
                                       track->setGetter(getter);
                                       
                                       return track;
                                   });

        static const std::string AnimationTrack_MeshInstanceVertexPosition("AnimationTrack_MeshInstanceVertexPosition");
        registerAnimationTrackType(AnimationTrack_MeshInstanceVertexPosition,
                                   []() -> AnimationTrack*
                                   {
                                       auto track = new AnimationTrackVector3Array();
                                       track->getTypeName = []() -> const std::string& { return AnimationTrack_MeshInstanceVertexPosition; };

                                       auto setter = [](const AnimationTrack*, Animatable* animatable, int32_t index, const Vector3& position)
                                       {
                                           assert(dynamic_cast<MeshInstance*>(animatable));
                                           MeshInstance* meshObject = static_cast<MeshInstance*>(animatable);

                                           meshObject->_notifyVertexPositionChanged();
                                           *(meshObject->_getAnimatedPositions().getBufferPointer() + index) = position;
                                       };
                                       
                                       auto getter = [](const AnimationTrack*, Animatable* animatable, int32_t index) -> Vector3
                                       {
                                           assert(dynamic_cast<MeshInstance*>(animatable));
                                           MeshInstance* meshObject = static_cast<MeshInstance*>(animatable);
                                           
                                           return *(meshObject->_getAnimatedPositions().getBufferPointer() + index);
                                       };
                                       
                                       track->setSetter(setter);
                                       track->setGetter(getter);
                                       
                                       return track;
                                   });

        static const std::string AnimationTrack_SkeletonInstanceSlotDrawOrder("AnimationTrack_SkeletonInstanceSlotDrawOrder");
        registerAnimationTrackType(AnimationTrack_SkeletonInstanceSlotDrawOrder,
                                   []() -> AnimationTrack*
                                   {
                                       auto track = new AnimationTrackMapStringToFloat();
                                       track->getTypeName = []() -> const std::string& { return AnimationTrack_SkeletonInstanceSlotDrawOrder; };

                                       auto setter = [](const AnimationTrack*, Animatable* animatable, const std::map<std::string, float>& drawOrderMap)
                                       {
                                           SkeletonInstance* skeletonObject = static_cast<SkeletonInstance*>(animatable);
                                           
                                           for (auto it : drawOrderMap)
                                           {
                                               auto slot = skeletonObject->getSlot(it.first);
                                               slot->setRenderPrioritySceneDataDepth(it.second);
                                           }
                                       };
                                       
                                       auto getter = [](const AnimationTrack*, Animatable* animatable) -> std::map<std::string, float>
                                       {
                                           return std::map<std::string, float>();
                                       };
                                       
                                       track->setSetter(setter);
                                       track->setGetter(getter);

                                       return track;
                                   });

        static const std::string AnimationTrack_SkeletonConstraintIK_Mix("AnimationTrack_SkeletonConstraintIK_Mix");
        registerAnimationTrackType(AnimationTrack_SkeletonConstraintIK_Mix,
                                   []() -> AnimationTrack*
                                   {
                                       auto track = new AnimationTrackFloat();
                                       track->getTypeName = []() -> const std::string& { return AnimationTrack_SkeletonConstraintIK_Mix; };
                                       
                                       auto setter = [](const AnimationTrack* animationTrack, Animatable* animatable, const float& mix)
                                       {
                                           SkeletonInstance* skeletonObject = static_cast<SkeletonInstance*>(animatable);

                                           auto ik_name = animationTrack->ikName;

                                           auto constraint = skeletonObject->getSpineConstraintFunctor(ik_name);

                                           assert(constraint);
                                           assert(dynamic_cast<SkeletonInstanceIKFunctor*>(constraint));
                                           auto ik_functor = static_cast<SkeletonInstanceIKFunctor*>(constraint);

                                           ik_functor->setMix(mix);
                                       };
                                       
                                       auto getter = [](const AnimationTrack* animationTrack, Animatable* animatable) -> float
                                       {
                                           SkeletonInstance* skeletonObject = static_cast<SkeletonInstance*>(animatable);
                                           
                                           auto ik_name = animationTrack->ikName;
                                           
                                           auto constraint = skeletonObject->getSpineConstraintFunctor(ik_name);
                                           
                                           assert(constraint);
                                           assert(dynamic_cast<SkeletonInstanceIKFunctor*>(constraint));
                                           auto ik_functor = static_cast<SkeletonInstanceIKFunctor*>(constraint);
                                           
                                           return ik_functor->getMix();
                                       };
                                       
                                       track->setSetter(setter);
                                       track->setGetter(getter);
                                       
                                       return track;
                                   });

        static const std::string AnimationTrack_SkeletonConstraintIK_BendPositive("AnimationTrack_SkeletonConstraintIK_BendPositive");
        registerAnimationTrackType(AnimationTrack_SkeletonConstraintIK_BendPositive,
                                   []() -> AnimationTrack*
                                   {
                                       auto track = new AnimationTrackFloat();
                                       track->getTypeName = []() -> const std::string& { return AnimationTrack_SkeletonConstraintIK_BendPositive; };
                                       
                                       auto setter = [](const AnimationTrack* animationTrack, Animatable* animatable, const float& positive)
                                       {
                                           SkeletonInstance* skeletonObject = static_cast<SkeletonInstance*>(animatable);
                                           
                                           auto ik_name = animationTrack->ikName;
                                           
                                           auto constraint = skeletonObject->getSpineConstraintFunctor(ik_name);
                                           
                                           assert(constraint);
                                           assert(dynamic_cast<SkeletonInstanceIKFunctor*>(constraint));
                                           auto ik_functor = static_cast<SkeletonInstanceIKFunctor*>(constraint);
                                           
                                           ik_functor->setDirection(positive);
                                       };
                                       
                                       auto getter = [](const AnimationTrack* animationTrack, Animatable* animatable) -> float
                                       {
                                           SkeletonInstance* skeletonObject = static_cast<SkeletonInstance*>(animatable);
                                           
                                           auto ik_name = animationTrack->ikName;
                                           
                                           auto constraint = skeletonObject->getSpineConstraintFunctor(ik_name);
                                           
                                           assert(constraint);
                                           assert(dynamic_cast<SkeletonInstanceIKFunctor*>(constraint));
                                           auto ik_functor = static_cast<SkeletonInstanceIKFunctor*>(constraint);
                                           
                                           return ik_functor->getDirection();
                                       };
                                       
                                       track->setSetter(setter);
                                       track->setGetter(getter);
                                       
                                       return track;
                                   });

        //bx
        static const std::string AnimationTrack_Event("AnimationTrack_Event");
        registerAnimationTrackType(AnimationTrack_Event,
                                   []() -> AnimationTrack*
                                   {
                                       auto track = new AnimationTrackString();
                                       track->getTypeName = []() -> const std::string& { return AnimationTrack_Event; };

                                       auto setter = [](const AnimationTrack*, Animatable* animatable, const std::string& name)
                                       {
                                           assert(dynamic_cast<SkeletonInstance*>(animatable));
                                           SkeletonInstance* skeleton = static_cast<SkeletonInstance*>(animatable);
                                           skeleton->setCurrentEventName(name, animatable->getEventPulse());
                                       };

                                       auto getter = [](const AnimationTrack*, Animatable* animatable) -> std::string
                                       {
                                           BOSS_ASSERT("개발필요", false);
                                           assert(dynamic_cast<SkeletonInstance*>(animatable));
                                           SkeletonInstance* skeleton = static_cast<SkeletonInstance*>(animatable);
                                           return skeleton->getCurrentEventName();
                                       };

                                       track->setSetter(setter);
                                       track->setGetter(getter);

                                       return track;
                                   });
    }
}
