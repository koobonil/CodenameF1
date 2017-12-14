#pragma once

#include "zay_types.h"

namespace ZAY
{
    class RenderPriority
    {
    public:
        RenderPriority();
        RenderPriority(const RenderPriority& other);

    public:
        void setGroup(int32_t group);
        int32_t getGroup() const;
    private:
        int32_t _group;
        
    public:
        void setDepth(float depth);
        float getDepth() const;
    private:
        float _depth;

    public:
        void setSceneData(const SceneData* sceneData);
		const SceneData* getSceneData() const;
    private:
		const SceneData* _sceneData;

    public:
        void setSceneDataDepth(float depth);
        float getSceneDataDepth() const;
    private:
        float _sceneDataDepth;
        
    public:
        const RenderPriority& operator = (const RenderPriority& other);
        
    public:
        bool operator < (const RenderPriority& other) const;
        bool operator > (const RenderPriority& other) const;
        bool operator <= (const RenderPriority& other) const;
        bool operator >= (const RenderPriority& other) const;
        bool operator != (const RenderPriority& other) const;
        bool operator == (const RenderPriority& other) const;
    };
}
