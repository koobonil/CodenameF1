#pragma once

#include "zay_types.h"
#include "zay_object_base.h"

namespace ZAY
{
    class SceneData
    : public ObjectBase
    {
    public:
        SceneData();
        virtual ~SceneData();

    public:
        virtual SceneInstance* createSceneInstance() = 0;
    };
}
