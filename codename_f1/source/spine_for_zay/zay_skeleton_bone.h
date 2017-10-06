#pragma once

#include "zay_types.h"
#include "zay_node.h"
#include "zay_skeleton_bone_render_instance.h"

namespace ZAY
{
    class SkeletonBone
    : public Node
    {
    public:
        SkeletonBone(SkeletonData* parentSkeletonData);
        SkeletonBone(SkeletonInstance* parentSkeletonInstance);
        virtual ~SkeletonBone();

        
        
    protected:
        virtual Node* createChildImpl() override;
        
    protected:
        virtual void _updateRootWorldTransform() override;
    protected:
        virtual void _updateRootWorldColor() override;

    public:
        virtual void addChild(Node* child) override;
        
        
    public:
        SkeletonData* getParentSkeletonData();
    private:
        SkeletonData* _parentSkeletonData;

        
        
    public:
        SkeletonInstance* getParentSkeletonInstance();
    private:
        SkeletonInstance* _parentSkeletonInstance;

        
        
    public:
        SkeletonBone* getParentBone() const;


        
    public:
        void setLength(float length);
    public:
        float getLength() const;
    private:
        float _length;
        
    public:
        void applyDefaultPose();
    public:
        void setDefaultPosition(float x, float y, float z = 0.0f);
        void setDefaultPosition(const Vector3& position);
        void setDefaultScale(float x, float y, float z = 1.0f);
        void setDefaultScale(const Vector3& scale);
        void setDefaultRotation(float rotation);
        void setDefaultFlipX(bool flipX);
        void setDefaultFlipY(bool flipY);
        void setDefaultInheritScale(bool inherit);
        void setDefaultInheritRotation(bool inherit);
        void setDefaultLength(float length);
    public:
        const Vector3& getDefaultPosition() const;
        const Vector3& getDefaultScale() const;
        float getDefaultRotation() const;
        bool getDefaultFlipX() const;
        bool getDefaultFlipY() const;
        bool getDefaultInheritScale() const;
        bool getDefaultInheritRotation() const;
        float getDefaultLength() const;
    private:
        float _defaultRotation;
        Vector3 _defaultPosition;
        Vector3 _defaultScale;
        bool _defaultFlipX;
        bool _defaultFlipY;
        bool _defaultInheritScale;
        bool _defaultInheritRotation;
        float _defaultLength;


    public:
        void setRenderPriorityGroup(int32_t priority);
        int32_t getRenderPriorityGroup() const;
        void setRenderPriorityDepth(float depth);
        float getRenderPriorityDepth() const;
    private:
        int32_t _renderPriorityGroup;
        float _renderPriorityDepth;

    public:
        void createSkeletonBoneRenderInstance();
        void destroySkeletonBoneRenderInstance();
        SkeletonBoneRenderInstance* getSkeletonBoneRenderInstance() const;
    private:
        SkeletonBoneRenderInstance* _skeletonBoneRenderObject;
    };
}
