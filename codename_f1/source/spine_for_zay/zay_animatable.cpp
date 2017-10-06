#include "zay_types.h"
#include "zay_animatable.h"

namespace ZAY
{
    Animatable::Animatable()
    {
    }
    
    Animatable::~Animatable()
    {
        clearKeyframeRangeDatas();
    }

    void Animatable::clearKeyframeRangeDatas()
    {
        _keyframeRangeDatas.clear();
    }
}
