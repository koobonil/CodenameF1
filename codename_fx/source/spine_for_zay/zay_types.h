#pragma once

#include <boss.h>

#define ZAY_LITTLE_ENDIAN

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// STD
#include <array>
#include <iostream>
#include <random>
#include <set>
#include <map>
#include <string>
#include <algorithm>
#include <vector>
#include <list>
#include <stack>
#include <cstdint>
#include <unordered_set>
#include <unordered_map>
#include <limits>
#include <iomanip>
#include <mutex>
#include <thread>
#include <condition_variable>
#include <functional>
#include <thread>
#include <queue>
#include <sstream>

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// BOOST
#define BOOST_ALL_NO_LIB
#define BOOST_BIMAP_DISABLE_SERIALIZATION
#include <boost/bimap.hpp>
#include <boost/bimap/set_of.hpp>
#include <boost/bimap/multiset_of.hpp>
#include <boost/bimap/unordered_set_of.hpp>
#include <boost/bimap/unordered_multiset_of.hpp>

#include <assert.h>
#undef assert
#define assert(EXP) BOSS_ASSERT("SPINE_FOR_ZAY", EXP)

namespace ZAY
{
    extern const std::string s_emptyString;

    // forward declaration
    class Animatable;
    class Animation;
    class AnimationContainer;
    class AnimationKeyFrame;
    class AnimationState;
    class AnimationStateSet;
    class AnimationTrack;
    class AnyType;
    class AnyValue;
    template<typename T> class ArrayBuffer;
    class Atlas;
    class Base;
    class ColourValue;
    class ForwardMultiplyRenderer;
    class Image;
    class ImageData;
    class Lerp;
    class Math;
    class Matrix3;
    class Matrix4;
    class MeshData;
    class MeshInstance;
    class Node;
    class ObjectBase;
    class Plane;
    class Quaternion;
    class RandomGenerator;
    class RenderCommand;
    class Renderer;
    class RenderPriority;
    class SceneData;
    class SceneInstance;
    class SceneNode;
    class SceneSlotData;
    class SceneSlotInstance;
    class SkeletonBone;
    class SkeletonBoneRenderInstance;
    class SkeletonData;
    class SkeletonInstance;
    class Texture2D;
    class TextureAtlas;
    class TextureAtlasSet;
    class Touch;
    class TouchAreaData;
    class TouchAreaInstance;
    class TouchEvent;
    class TouchEventDispatcher;
    class Vector2;
    class Vector3;
    class Vector4;

    namespace ForwardMultiplyRender
    {
        enum class BlendType
        : std::uint32_t
        {
            Alpha,
            Additive,
            Screen,
            Multiply,
            Outline,
            Shadow,

            DEFAULT
        };

        enum class VertexAttrib
        : std::uint32_t
        {
            Position,
            Color,
            TexCoord,
            MinSize,
            MaxSize,
            
            COUNT,
        };
        
        enum class RenderType
        : std::uint32_t
        {
            TrianglesMesh,
            
            MultiplyMap,
            
            COUNT
        };
    };

    enum class PixelFormat
    : std::uint32_t
    {
        NONE,
        
        RGBA8888,
        //! 24-bit texture: RGBA888
        RGB888,
        //! 16-bit texture without Alpha channel
        RGB565,
        //! 8-bit textures used as masks
        A8,
        //! 8-bit intensity texture
        I8,
        //! 16-bit textures used as masks
        AI88,
        //! 16-bit textures: RGBA4444
        RGBA4444,
        //! 16-bit textures: RGB5A1
        RGB5A1
    };

    enum class ValueType
    : std::uint32_t
    {
        NoneType = 0,
        Integer,
        Real,
        Boolean,
        String,
        Vector2,
        Vector3,
        Vector4,
        Quaternion,
        Colour,

        COUNT
    };

    enum class KeyFrameType
    : std::uint32_t
    {
        Linear,
        Stepped,
        SpineBezierToTenLinearFake,
        
        COUNT
    };

    typedef void (*MotionFinishedCB)(void*, const char*, int); //bx
}
