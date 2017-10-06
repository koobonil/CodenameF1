#include "zay_types.h"
#include "zay_animation_keyframe.h"

namespace ZAY
{
    void AnimationKeyFrame::registerAnimationKeyFrameType(const std::string& typeName, const std::function<AnimationKeyFrame*()>& creator)
    {
        s_animationKeyFrameCreators[typeName] = creator;
    }
    
    void AnimationKeyFrame::unregisterAnimationKeyFrameType(const std::string& typeName)
    {
        s_animationKeyFrameCreators.erase(typeName);
    }
    
    AnimationKeyFrame* AnimationKeyFrame::createAnimationKeyFrame(const std::string& typeName)
    {
        auto it = s_animationKeyFrameCreators.find(typeName);
        
        if (it != s_animationKeyFrameCreators.end())
        {
            return it->second();
        }
        else
        {
            return nullptr;
        }
    }
    
    void AnimationKeyFrame::initAnimationKeyFrameCreators()
    {
        // float
        
        registerAnimationKeyFrameType("AnimationKeyFrame_Float_Stepped",
                                      []() -> AnimationKeyFrame*
                                      {
                                          auto animationKeyFrame = new AnimationKeyFrame_Float_Stepped();
                                          return animationKeyFrame;
                                      });
        
        registerAnimationKeyFrameType("AnimationKeyFrame_Float_Linear",
                                      []() -> AnimationKeyFrame*
                                      {
                                          auto animationKeyFrame = new AnimationKeyFrame_Float_Linear();
                                          return animationKeyFrame;
                                      });

        registerAnimationKeyFrameType("AnimationKeyFrame_Float_SpineBezierToTenLinearFake",
                                      []() -> AnimationKeyFrame*
                                      {
                                          auto animationKeyFrame = new AnimationKeyFrame_Float_SpineBezierToTenLinearFake();
                                          return animationKeyFrame;
                                      });
        
        registerAnimationKeyFrameType("AnimationKeyFrameArray_Float_Stepped",
                                      []() -> AnimationKeyFrame*
                                      {
                                          auto animationKeyFrame = new AnimationKeyFrameArray_Float_Stepped();
                                          return animationKeyFrame;
                                      });
        
        registerAnimationKeyFrameType("AnimationKeyFrameArray_Float_Linear",
                                      []() -> AnimationKeyFrame*
                                      {
                                          auto animationKeyFrame = new AnimationKeyFrameArray_Float_Linear();
                                          return animationKeyFrame;
                                      });
        
        registerAnimationKeyFrameType("AnimationKeyFrameArray_Float_SpineBezierToTenLinearFake",
                                      []() -> AnimationKeyFrame*
                                      {
                                          auto animationKeyFrame = new AnimationKeyFrameArray_Float_SpineBezierToTenLinearFake();
                                          return animationKeyFrame;
                                      });
        

        // Vector2
        
        registerAnimationKeyFrameType("AnimationKeyFrame_Vector2_Stepped",
                                      []() -> AnimationKeyFrame*
                                      {
                                          auto animationKeyFrame = new AnimationKeyFrame_Vector2_Stepped();
                                          return animationKeyFrame;
                                      });

        registerAnimationKeyFrameType("AnimationKeyFrame_Vector2_Linear",
                                      []() -> AnimationKeyFrame*
                                      {
                                          auto animationKeyFrame = new AnimationKeyFrame_Vector2_Linear();
                                          return animationKeyFrame;
                                      });

        registerAnimationKeyFrameType("AnimationKeyFrame_Vector2_SpineBezierToTenLinearFake",
                                      []() -> AnimationKeyFrame*
                                      {
                                          auto animationKeyFrame = new AnimationKeyFrame_Vector2_SpineBezierToTenLinearFake();
                                          return animationKeyFrame;
                                      });

        registerAnimationKeyFrameType("AnimationKeyFrameArray_Vector2_Stepped",
                                      []() -> AnimationKeyFrame*
                                      {
                                          auto animationKeyFrame = new AnimationKeyFrameArray_Vector2_Stepped();
                                          return animationKeyFrame;
                                      });
        
        registerAnimationKeyFrameType("AnimationKeyFrameArray_Vector2_Linear",
                                      []() -> AnimationKeyFrame*
                                      {
                                          auto animationKeyFrame = new AnimationKeyFrameArray_Vector2_Linear();
                                          return animationKeyFrame;
                                      });
        
        registerAnimationKeyFrameType("AnimationKeyFrameArray_Vector2_SpineBezierToTenLinearFake",
                                      []() -> AnimationKeyFrame*
                                      {
                                          auto animationKeyFrame = new AnimationKeyFrameArray_Vector2_SpineBezierToTenLinearFake();
                                          return animationKeyFrame;
                                      });

        
        
        // Vector3
        
        registerAnimationKeyFrameType("AnimationKeyFrame_Vector3_Stepped",
                                      []() -> AnimationKeyFrame*
                                      {
                                          auto animationKeyFrame = new AnimationKeyFrame_Vector3_Stepped();
                                          return animationKeyFrame;
                                      });

        registerAnimationKeyFrameType("AnimationKeyFrame_Vector3_Linear",
                                      []() -> AnimationKeyFrame*
                                      {
                                          auto animationKeyFrame = new AnimationKeyFrame_Vector3_Linear();
                                          return animationKeyFrame;
                                      });

        registerAnimationKeyFrameType("AnimationKeyFrame_Vector3_SpineBezierToTenLinearFake",
                                      []() -> AnimationKeyFrame*
                                      {
                                          auto animationKeyFrame = new AnimationKeyFrame_Vector3_SpineBezierToTenLinearFake();
                                          return animationKeyFrame;
                                      });

        registerAnimationKeyFrameType("AnimationKeyFrameArray_Vector3_Stepped",
                                      []() -> AnimationKeyFrame*
                                      {
                                          auto animationKeyFrame = new AnimationKeyFrameArray_Vector3_Stepped();
                                          return animationKeyFrame;
                                      });
        
        registerAnimationKeyFrameType("AnimationKeyFrameArray_Vector3_Linear",
                                      []() -> AnimationKeyFrame*
                                      {
                                          auto animationKeyFrame = new AnimationKeyFrameArray_Vector3_Linear();
                                          return animationKeyFrame;
                                      });
        
        registerAnimationKeyFrameType("AnimationKeyFrameArray_Vector3_SpineBezierToTenLinearFake",
                                      []() -> AnimationKeyFrame*
                                      {
                                          auto animationKeyFrame = new AnimationKeyFrameArray_Vector3_SpineBezierToTenLinearFake();
                                          return animationKeyFrame;
                                      });

        
        
        // Vector4
        
        registerAnimationKeyFrameType("AnimationKeyFrame_Vector4_Stepped",
                                      []() -> AnimationKeyFrame*
                                      {
                                          auto animationKeyFrame = new AnimationKeyFrame_Vector4_Stepped();
                                          return animationKeyFrame;
                                      });

        registerAnimationKeyFrameType("AnimationKeyFrame_Vector4_Linear",
                                      []() -> AnimationKeyFrame*
                                      {
                                          auto animationKeyFrame = new AnimationKeyFrame_Vector4_Linear();
                                          return animationKeyFrame;
                                      });

        registerAnimationKeyFrameType("AnimationKeyFrame_Vector4_SpineBezierToTenLinearFake",
                                      []() -> AnimationKeyFrame*
                                      {
                                          auto animationKeyFrame = new AnimationKeyFrame_Vector4_SpineBezierToTenLinearFake();
                                          return animationKeyFrame;
                                      });

        registerAnimationKeyFrameType("AnimationKeyFrameArray_Vector4_Stepped",
                                      []() -> AnimationKeyFrame*
                                      {
                                          auto animationKeyFrame = new AnimationKeyFrameArray_Vector4_Stepped();
                                          return animationKeyFrame;
                                      });
        
        registerAnimationKeyFrameType("AnimationKeyFrameArray_Vector4_Linear",
                                      []() -> AnimationKeyFrame*
                                      {
                                          auto animationKeyFrame = new AnimationKeyFrameArray_Vector4_Linear();
                                          return animationKeyFrame;
                                      });
        
        registerAnimationKeyFrameType("AnimationKeyFrameArray_Vector4_SpineBezierToTenLinearFake",
                                      []() -> AnimationKeyFrame*
                                      {
                                          auto animationKeyFrame = new AnimationKeyFrameArray_Vector4_SpineBezierToTenLinearFake();
                                          return animationKeyFrame;
                                      });

        
        
        // ColourValue
        
        registerAnimationKeyFrameType("AnimationKeyFrame_ColourValue_Stepped",
                                      []() -> AnimationKeyFrame*
                                      {
                                          auto animationKeyFrame = new AnimationKeyFrame_ColourValue_Stepped();
                                          return animationKeyFrame;
                                      });

        registerAnimationKeyFrameType("AnimationKeyFrame_ColourValue_Linear",
                                      []() -> AnimationKeyFrame*
                                      {
                                          auto animationKeyFrame = new AnimationKeyFrame_ColourValue_Linear();
                                          return animationKeyFrame;
                                      });

        registerAnimationKeyFrameType("AnimationKeyFrame_ColourValue_SpineBezierToTenLinearFake",
                                      []() -> AnimationKeyFrame*
                                      {
                                          auto animationKeyFrame = new AnimationKeyFrame_ColourValue_SpineBezierToTenLinearFake();
                                          return animationKeyFrame;
                                      });

        registerAnimationKeyFrameType("AnimationKeyFrameArray_ColourValue_Stepped",
                                      []() -> AnimationKeyFrame*
                                      {
                                          auto animationKeyFrame = new AnimationKeyFrameArray_ColourValue_Stepped();
                                          return animationKeyFrame;
                                      });
        
        registerAnimationKeyFrameType("AnimationKeyFrameArray_ColourValue_Linear",
                                      []() -> AnimationKeyFrame*
                                      {
                                          auto animationKeyFrame = new AnimationKeyFrameArray_ColourValue_Linear();
                                          return animationKeyFrame;
                                      });
        
        registerAnimationKeyFrameType("AnimationKeyFrameArray_ColourValue_SpineBezierToTenLinearFake",
                                      []() -> AnimationKeyFrame*
                                      {
                                          auto animationKeyFrame = new AnimationKeyFrameArray_ColourValue_SpineBezierToTenLinearFake();
                                          return animationKeyFrame;
                                      });

        

        // bool
        
        registerAnimationKeyFrameType("AnimationKeyFrame_Boolean_Stepped",
                                      []() -> AnimationKeyFrame*
                                      {
                                          auto animationKeyFrame = new AnimationKeyFrame_Boolean_Stepped();
                                          return animationKeyFrame;
                                      });
        
        registerAnimationKeyFrameType("AnimationKeyFrameArray_Boolean_Stepped",
                                      []() -> AnimationKeyFrame*
                                      {
                                          auto animationKeyFrame = new AnimationKeyFrameArray_Boolean_Stepped();
                                          return animationKeyFrame;
                                      });

        
        
        // string
        
        registerAnimationKeyFrameType("AnimationKeyFrame_String_Stepped",
                                      []() -> AnimationKeyFrame*
                                      {
                                          auto animationKeyFrame = new AnimationKeyFrame_String_Stepped();
                                          return animationKeyFrame;
                                      });

        registerAnimationKeyFrameType("AnimationKeyFrameArray_String_Stepped",
                                      []() -> AnimationKeyFrame*
                                      {
                                          auto animationKeyFrame = new AnimationKeyFrameArray_String_Stepped();
                                          return animationKeyFrame;
                                      });

        
        
        // std::map<std::string, float>
        
        registerAnimationKeyFrameType("AnimationKeyFrame_MapStringToFloat_Stepped",
                                      []() -> AnimationKeyFrame*
                                      {
                                          auto animationKeyFrame = new AnimationKeyFrame_MapStringToFloat_Stepped();
                                          return animationKeyFrame;
                                      });

        registerAnimationKeyFrameType("AnimationKeyFrameArray_MapStringToFloat_Stepped",
                                      []() -> AnimationKeyFrame*
                                      {
                                          auto animationKeyFrame = new AnimationKeyFrameArray_MapStringToFloat_Stepped();
                                          return animationKeyFrame;
                                      });
        
        // rotation
        
        registerAnimationKeyFrameType("AnimationKeyFrame_Rotation_Linear",
                                      []() -> AnimationKeyFrame*
                                      {
                                          auto animationKeyFrame = new AnimationKeyFrame_Rotation_Linear();
                                          return animationKeyFrame;
                                      });
        
        registerAnimationKeyFrameType("AnimationKeyFrame_Rotation_Stepped",
                                      []() -> AnimationKeyFrame*
                                      {
                                          auto animationKeyFrame = new AnimationKeyFrame_Rotation_Stepped();
                                          return animationKeyFrame;
                                      });

        registerAnimationKeyFrameType("AnimationKeyFrame_Rotation_SpineBezierToTenLinearFake",
                                      []() -> AnimationKeyFrame*
                                      {
                                          auto animationKeyFrame = new AnimationKeyFrame_Rotation_SpineBezierToTenLinearFake();
                                          return animationKeyFrame;
                                      });

    }

    std::map<std::string, std::function<AnimationKeyFrame*()>> AnimationKeyFrame::s_animationKeyFrameCreators;

    AnimationKeyFrame_Rotation_Linear::AnimationKeyFrame_Rotation_Linear()
    {
        _type = KeyFrameType::Linear;
    }

    float AnimationKeyFrame_Rotation_Linear::_getLerpValue(float left, float right, float percent)
    {
        auto diff = right - left;
        
        if (diff > 180.0f)
        {
            float rotation = left - (360.0f - diff) * percent;
            if (rotation < 0.0f)
            {
                rotation += 360.0f;
            }
            return rotation;
        }
        else if (diff < -180.0f)
        {
            float rotation = left + (360.0f + diff) * percent;
            if (rotation >= 360.0f)
            {
                rotation -= 360.0f;
            }
            return rotation;
        }
        else
        {
            return lerp(left, right, percent);
        }
    }

    float AnimationKeyFrame_Rotation_Linear::getLerpValue(const AnimationKeyFrameSingleValue<float>* right, float percent) const
    {
        return _getLerpValue(getValue(), right->getValue(), percent);
    }

    
    
    AnimationKeyFrame_Rotation_Stepped::AnimationKeyFrame_Rotation_Stepped()
    {
        _type = KeyFrameType::Stepped;
    }

    float AnimationKeyFrame_Rotation_Stepped::getLerpValue(const AnimationKeyFrameSingleValue<float>* right, float percent) const
    {
        return getValue();
    }
    
    
    
    AnimationKeyFrame_Rotation_SpineBezierToTenLinearFake::AnimationKeyFrame_Rotation_SpineBezierToTenLinearFake()
    {
        _type = KeyFrameType::SpineBezierToTenLinearFake;
    }

    float AnimationKeyFrame_Rotation_SpineBezierToTenLinearFake::getLerpValue(const AnimationKeyFrameSingleValue<float>* right, float percent) const
    {
        for (int32_t i=0 ; i<s_bezierPointCount ; i++)
        {
            if (_x[i] >= percent)
            {
                float prevX, prevY;
                
                if (i == 0)
                {
                    prevX = 0.0f;
                    prevY = 0.0f;
                }
                else
                {
                    prevX = _x[i-1];
                    prevY = _y[i-1];
                }
                
                float ratio = prevY + (_y[i] - prevY) * (percent - prevX) / (_x[i] - prevX);
                
                return AnimationKeyFrame_Rotation_Linear::_getLerpValue(getValue(), right->getValue(), ratio);
            }
        }
        
        float x = _x[s_bezierPointCount-1];
        float y = _y[s_bezierPointCount-1];
        
        float ratio = y + (1.0f - y) * (percent - x) / (1.0f - x);
        
        return AnimationKeyFrame_Rotation_Linear::_getLerpValue(getValue(), right->getValue(), ratio);
    }

    void AnimationKeyFrame_Rotation_SpineBezierToTenLinearFake::createSpineFakeLinears(float cx1, float cy1, float cx2, float cy2)
    {
        static const int BEZIER_SEGMENTS = 10;
        
        float subdiv1 = 1.0f / BEZIER_SEGMENTS, subdiv2 = subdiv1 * subdiv1, subdiv3 = subdiv2 * subdiv1;
        float pre1 = 3 * subdiv1, pre2 = 3 * subdiv2, pre4 = 6 * subdiv2, pre5 = 6 * subdiv3;
        float tmp1x = -cx1 * 2 + cx2, tmp1y = -cy1 * 2 + cy2, tmp2x = (cx1 - cx2) * 3 + 1, tmp2y = (cy1 - cy2) * 3 + 1;
        float dfx = cx1 * pre1 + tmp1x * pre2 + tmp2x * subdiv3, dfy = cy1 * pre1 + tmp1y * pre2 + tmp2y * subdiv3;
        float ddfx = tmp1x * pre4 + tmp2x * pre5, ddfy = tmp1y * pre4 + tmp2y * pre5;
        float dddfx = tmp2x * pre5, dddfy = tmp2y * pre5;
        float x = dfx, y = dfy;
        
        for (auto i=0 ; i < s_bezierPointCount ; i ++)
        {
            _x[i] = x;
            _y[i] = y;
            
            dfx += ddfx;
            dfy += ddfy;
            ddfx += dddfx;
            ddfy += dddfy;
            x += dfx;
            y += dfy;
        }
    }
}
