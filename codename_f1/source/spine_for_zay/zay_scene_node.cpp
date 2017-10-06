#include "zay_types.h"
#include "zay_scene_node.h"
#include "zay_scene_instance.h"

namespace ZAY
{
    SceneNode::SceneNode()
    {
        
    }

    SceneNode::~SceneNode()
    {
        
    }

    Node* SceneNode::createChildImpl()
    {
        SceneNode* sceneNode = new SceneNode;
        return sceneNode;
    }

    void SceneNode::_updateRootWorldTransform()
    {
    }

    void SceneNode::_updateRootWorldColor()
    {
    }

    SceneNode* SceneNode::getParentSceneNode() const
    {
        assert(dynamic_cast<SceneNode*>(getParentNode()));
        return static_cast<SceneNode*>(getParentNode());
    }
}
