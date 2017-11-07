#include <boss.hpp>
#include "codename_f1.hpp"

#include <resource.hpp>
#include "classes.hpp"

ZAY_DECLARE_VIEW_CLASS("codename_f1View", codename_f1Data)

ZAY_VIEW_API OnCommand(CommandType type, chars topic, id_share in, id_cloned_share* out)
{
    if(type == CT_Create)
    {
        // 윈도우 타이틀
        Platform::SetWindowName("Codename F1");
    }
    if(type == CT_Size)
    {
        sint32s WH = in;
        const sint32 Width = WH[0];
        const sint32 Height = WH[1];
        m->mIsLandscape = (Height < Width);
    }
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
            {
                F1State::landscape() = m->mIsLandscape;
                m->next("ingameView");
            }
        })
    {
        ZAY_RGBA(panel, 0, 255, 255, 128)
            panel.fill();
        ZAY_RGB(panel, 0, 0, 0)
        {
            ZAY_FONT(panel, 2.0, "Arial Black")
                panel.text((m->mIsLandscape)? "IN GAME(W)" : "IN GAME", UIFA_CenterMiddle, UIFE_Right);
            panel.rect(3);
        }
    }

    ZAY_XYRR_UI(panel, panel.w() / 2, panel.h() * 0.50, panel.w() / 4, panel.h() / 12, "maptool",
        ZAY_GESTURE_T(t)
        {
            if(t == GT_InReleased)
            {
                F1State::landscape() = m->mIsLandscape;
                m->next("maptoolView");
            }
        })
    {
        ZAY_RGBA(panel, 255, 0, 255, 128)
            panel.fill();
        ZAY_RGB(panel, 0, 0, 0)
        {
            ZAY_FONT(panel, 2.0, "Arial Black")
                panel.text((m->mIsLandscape)? "MAP TOOL(W)" : "MAP TOOL", UIFA_CenterMiddle, UIFE_Right);
            panel.rect(3);
        }
    }

    ZAY_XYRR_UI(panel, panel.w() / 2, panel.h() * 0.75, panel.w() / 4, panel.h() / 12, "stagetool",
        ZAY_GESTURE_T(t)
        {
            if(t == GT_InReleased)
            {
                F1State::landscape() = m->mIsLandscape;
                m->next("stagetoolView");
            }
        })
    {
        ZAY_RGBA(panel, 255, 0, 255, 128)
            panel.fill();
        ZAY_RGB(panel, 0, 0, 0)
        {
            ZAY_FONT(panel, 2.0, "Arial Black")
                panel.text((m->mIsLandscape)? "STAGE TOOL(W)" : "STAGE TOOL", UIFA_CenterMiddle, UIFE_Right);
            panel.rect(3);
        }
    }
}

codename_f1Data::codename_f1Data()
{
    mIsLandscape = false;
}

codename_f1Data::~codename_f1Data()
{
}
