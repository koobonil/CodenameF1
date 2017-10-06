#pragma once

#include "zay_node.h"

namespace ZAY
{
    class SceneNode
    : public Node
    {
    public:
        SceneNode();
        virtual ~SceneNode();
        
    protected:
        virtual Node* createChildImpl() override;

    protected:
        virtual void _updateRootWorldTransform() override;
    protected:
        virtual void _updateRootWorldColor() override;
        
    public:
        SceneNode* getParentSceneNode() const;
    };
    
    
    
    
    class SceneNodeManager
    : public Base
    {
    public:
        static void createInstance();
        static SceneNodeManager* getInstance();
        static void destroyInstance();
    private:
        static SceneNodeManager* s_manager;
        
    public:
        SceneNodeManager();
        virtual ~SceneNodeManager();

    public:
        SceneNode* createSceneNode();
    };
}
