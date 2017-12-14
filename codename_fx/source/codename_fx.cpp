#include <boss.hpp>
#include "codename_fx.hpp"

#include <r.hpp>

ZAY_DECLARE_VIEW_CLASS("codename_fxView", codename_fxData)

ZAY_VIEW_API OnCommand(CommandType type, chars topic, id_share in, id_cloned_share* out)
{
}

ZAY_VIEW_API OnNotify(chars sender, chars topic, id_share in, id_cloned_share* out)
{
}

ZAY_VIEW_API OnGesture(GestureType type, sint32 x, sint32 y)
{
}

ZAY_VIEW_API OnRender(ZayPanel& panel)
{
    ZAY_RGB(panel, 255, 255, 255)
        panel.fill();
}

codename_fxData::codename_fxData()
{
}

codename_fxData::~codename_fxData()
{
}
