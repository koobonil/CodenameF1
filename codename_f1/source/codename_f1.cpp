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
        m->mIsLandscape = false;//(Height < Width);
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

    const sint32 Gap = panel.h() / 24;
    ZAY_LTRB_UI(panel, panel.w() * 1 / 4, panel.h() * 1 / 6, panel.w() * 2 / 4 - Gap, panel.h() * 2 / 6, "stage",
        ZAY_GESTURE_VNT(v, n, t)
        {
            if(t == GT_Pressed)
            {
                String Text = m->mFirstStage;
                const auto& VRect = v->rect(n);
                if(Platform::Popup::OpenEditTracker(Text, UIET_String, VRect.l, VRect.t, VRect.r, VRect.b))
                {
                    m->mFirstStage = Text;
                    m->invalidate();
                }
            }
        })
    {
        ZAY_RGBA(panel, 255, 255, 255, 128)
            panel.fill();
        ZAY_RGB(panel, 0, 0, 0)
        {
            panel.text(m->mFirstStage, UIFA_CenterMiddle, UIFE_Right);
            panel.rect(3);
        }
    }

    ZAY_LTRB_UI(panel, panel.w() * 2 / 4 + Gap, panel.h() * 1 / 6, panel.w() * 3 / 4, panel.h() * 2 / 6, "run",
        ZAY_GESTURE_T(t)
        {
            if(t == GT_InReleased)
            {
                F1State::landscape() = m->mIsLandscape;
                F1State::stage() = m->mFirstStage;
                m->next("ingameView");
            }
        })
    {
        ZAY_RGBA(panel, 0, 255, 255, 128)
            panel.fill();
        ZAY_RGB(panel, 0, 0, 0)
        {
            ZAY_FONT(panel, 2.0, "Arial Black")
                panel.text((m->mIsLandscape)? "RUN(W)" : "RUN", UIFA_CenterMiddle, UIFE_Right);
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
    if(0 < F1State::stage().Length())
        mFirstStage = F1State::stage();
    else mFirstStage = "stage_0_test";
}

codename_f1Data::~codename_f1Data()
{
}
