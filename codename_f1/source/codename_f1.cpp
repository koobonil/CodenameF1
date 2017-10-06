﻿#include <boss.hpp>
#include "codename_f1.hpp"

#include <r.hpp>

ZAY_DECLARE_VIEW_CLASS("codename_f1View", codename_f1Data)

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

    ZAY_XYRR_UI(panel, panel.w() / 2, panel.h() * 0.25, panel.w() / 4, panel.h() / 12, "game",
        ZAY_GESTURE_T(t)
        {
            if(t == GT_InReleased)
                m->next("ingameView");
        })
    {
        ZAY_RGBA(panel, 0, 255, 255, 128)
            panel.fill();
        ZAY_RGB(panel, 0, 0, 0)
        {
            ZAY_FONT(panel, 2.0, "Arial Black")
                panel.text("IN GAME", UIFA_CenterMiddle, UIFE_Right);
            panel.rect(3);
        }
    }

    ZAY_XYRR_UI(panel, panel.w() / 2, panel.h() * 0.50, panel.w() / 4, panel.h() / 12, "maptool",
        ZAY_GESTURE_T(t)
        {
            if(t == GT_InReleased)
                m->next("maptoolView");
        })
    {
        ZAY_RGBA(panel, 255, 0, 255, 128)
            panel.fill();
        ZAY_RGB(panel, 0, 0, 0)
        {
            ZAY_FONT(panel, 2.0, "Arial Black")
                panel.text("MAP TOOL", UIFA_CenterMiddle, UIFE_Right);
            panel.rect(3);
        }
    }

    ZAY_XYRR_UI(panel, panel.w() / 2, panel.h() * 0.75, panel.w() / 4, panel.h() / 12, "stagetool",
        ZAY_GESTURE_T(t)
        {
            if(t == GT_InReleased)
                m->next("stagetoolView");
        })
    {
        ZAY_RGBA(panel, 255, 0, 255, 128)
            panel.fill();
        ZAY_RGB(panel, 0, 0, 0)
        {
            ZAY_FONT(panel, 2.0, "Arial Black")
                panel.text("STAGE TOOL", UIFA_CenterMiddle, UIFE_Right);
            panel.rect(3);
        }
    }
}

codename_f1Data::codename_f1Data()
{
}

codename_f1Data::~codename_f1Data()
{
}