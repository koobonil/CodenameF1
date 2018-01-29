#include "zay_types.h"
#include "zay_touch_area_instance.h"
#include "zay_skeleton_instance.h"
#include "zay_scene_node.h"

namespace ZAY
{
    extern void SpineBuilder_SetBoundBox(const char* name, int r, int g, int b, int a);
    extern void SpineBuilder_MergeBoundBox(float x, float y);

    TouchAreaInstance::TouchAreaInstance()
    {
        _enabled = true;

        _worldVerticesValid = false;
        _bossVerticesValid = false; //bx1116
        _localVertciesXY = nullptr;
        _worldVertciesXY = nullptr;
        _bossVertciesXY = nullptr; //bx1116
        _vertciesCount = 0;
    }

    TouchAreaInstance::~TouchAreaInstance()
    {
        if (_localVertciesXY)
        {
            delete [] _localVertciesXY;
            _localVertciesXY = nullptr;
        }
        
        if (_worldVertciesXY)
        {
            delete [] _worldVertciesXY;
            _worldVertciesXY = nullptr;
        }

        //bx1116
        if (_bossVertciesXY)
        {
            delete [] _bossVertciesXY;
            _bossVertciesXY = nullptr;
        }

        _vertciesCount = 0;
    }

    void TouchAreaInstance::_preRender() // bx
    {
         SceneInstance::_preRender();

         updateBoundBox();
    }

    void TouchAreaInstance::_postRender() // bx
    {
        SceneInstance::_postRender();
    }

    void TouchAreaInstance::setLocalVertices(std::string name, float* vertices, int32_t verticesCount)
    {
        if (_vertciesCount != verticesCount)
        {
            _name = name;

            if (_localVertciesXY)
            {
                delete [] _localVertciesXY;
                _localVertciesXY = nullptr;
            }
            
            if (_worldVertciesXY)
            {
                delete [] _worldVertciesXY;
                _worldVertciesXY = nullptr;
            }

            //bx1116
            if (_bossVertciesXY)
            {
                delete [] _bossVertciesXY;
                _bossVertciesXY = nullptr;
            }

            _vertciesCount = verticesCount;
            _worldVerticesValid = false;
            
            if (_vertciesCount > 0)
            {
                _localVertciesXY = new float[_vertciesCount];
                _worldVertciesXY = new float[_vertciesCount];
                _bossVertciesXY = new float[_vertciesCount]; //bx1116

                if (vertices)
                {
                    memcpy(_localVertciesXY, vertices, sizeof(float) * _vertciesCount);
                }
                
                memset(_worldVertciesXY, 0x00, sizeof(float) * _vertciesCount);
                memset(_bossVertciesXY, 0x00, sizeof(float) * _vertciesCount); //bx1116
            }
        }
    }

    void TouchAreaInstance::updateBoundBox() //bx
    {
        auto node = getAttachedNode();

        if (node)
        {
            _updateWorldTransformAscending();

            const Matrix4& worldTransform = getWorldTransform();

            // 바운드박스 추가
            const ColourValue& Color = getWorldColor();
            SpineBuilder_SetBoundBox(_name.c_str(), Color.r * 255, Color.g * 255, Color.b * 255, Color.a * 255);

            for(int32_t i = 0; i < _vertciesCount; i += 2)
            {
                const ZAY::Vector2 dst = worldTransform * ZAY::Vector2(_localVertciesXY[i], _localVertciesXY[i + 1]);
                _bossVertciesXY[i] = dst.x;
                _bossVertciesXY[i + 1] = dst.y;
                // 바운드박스 좌표추가
                SpineBuilder_MergeBoundBox(dst.x, dst.y);
            }
            // 바운드박스 좌표마감처리
            SpineBuilder_MergeBoundBox(_bossVertciesXY[0], _bossVertciesXY[1]);

            _bossVerticesValid = true;
        }
    }

    void TouchAreaInstance::updateAreaBoxOnly(int r, int g, int b, int a) const //bx
    {
        const char* Name = _name.c_str();
        if(boss_strcmp(Name, "area") == 0)
        if(6 <= _vertciesCount && _bossVerticesValid)
        {
            SpineBuilder_SetBoundBox(Name, r, g, b, a);
            for(int i = 0; i < _vertciesCount; i += 2)
                SpineBuilder_MergeBoundBox(_bossVertciesXY[i], _bossVertciesXY[i + 1]);
            SpineBuilder_MergeBoundBox(_bossVertciesXY[0], _bossVertciesXY[1]);
        }
    }

    void TouchAreaInstance::setEnabled(bool flag)
    {
        _enabled = flag;
    }
    
    bool TouchAreaInstance::getEnabled() const
    {
        return _enabled;
    }

    void TouchAreaInstance::setEnabledForController(bool flag)
    {
        _enabledForController = flag;
    }
    
    bool TouchAreaInstance::getEnabledForController() const
    {
        return _enabledForController;
    }

    void TouchAreaInstance::_updateOneTick()
    {
        SceneInstance::_updateOneTick();

        updateBoundBox();
    }
    
    void TouchAreaInstance::_updateSubTicks(int32_t subTicks)
    {
        SceneInstance::_updateSubTicks(subTicks);
        
        auto node = getAttachedNode();
        
        if (node)
        {
            _updateWorldTransformAscending();
            
            const Matrix4& worldTransform = getWorldTransform();
            
            for (int32_t i=0 ; i<(_vertciesCount/2) ; i++)
            {
                ZAY::Vector2 dst = worldTransform * ZAY::Vector2(*(_localVertciesXY+i*2+0), *(_localVertciesXY+i*2+1));
                
                *(_worldVertciesXY+i*2+0) = dst.x;
                *(_worldVertciesXY+i*2+1) = dst.y;
            }
            
            _worldVerticesValid = true;
        }
    }
    
    
    
    void TouchAreaInstance::_notifyRenderPriorityGroupChanged()
    {
        SceneInstance::_notifyRenderPriorityGroupChanged();
        
        _renderPriority.setGroup(getRenderPriorityGroup());
    }
    
    void TouchAreaInstance::_notifyRenderPriorityDepthChanged()
    {
        SceneInstance::_notifyRenderPriorityDepthChanged();

        _renderPriority.setDepth(getRenderPriorityDepth());
    }

    void TouchAreaInstance::setRenderPrioritySceneData(const SceneData* sceneData)
    {
        _renderPriority.setSceneData(sceneData);
    }
    
    const SceneData* TouchAreaInstance::getRenderPrioritySceneData() const
    {
        return _renderPriority.getSceneData();
    }
    
    void TouchAreaInstance::setRenderPrioritySceneDataDepth(float depth)
    {
        _renderPriority.setSceneDataDepth(depth);
    }
    
    float TouchAreaInstance::getRenderPrioritySceneDataDepth() const
    {
        return _renderPriority.getSceneDataDepth();
    }

    const RenderPriority& TouchAreaInstance::getRenderPriority() const
    {
        return _renderPriority;
    }
}
