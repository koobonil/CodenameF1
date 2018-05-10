#include <boss.hpp>
#include "f1.hpp"

#include <resource.hpp>
#include "outgame.hpp"
#include "classes_f1.hpp"

ZAY_DECLARE_VIEW_CLASS("f1View", f1Data)

ZAY_VIEW_API OnCommand(CommandType type, chars topic, id_share in, id_cloned_share* out)
{
    if(type == CT_Create)
    {
        // 윈도우 타이틀
        Platform::SetWindowName("Codename F1");
    }
    else if(type == CT_Tick)
    {
        if(0 < m->mGoNextCounter && --m->mGoNextCounter == 0)
            m->next("ingameView");
    }
    else if(type == CT_Size)
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
    if(!FXSaver::Read("DevMode").GetInt())
    {
        ZAY_RGB(panel, 255, 255, 255)
            panel.fill();

        // 배경
        const Image& LogoImage = R("logo");
        const sint32 Width = LogoImage.GetWidth();
        const sint32 Height = LogoImage.GetHeight();
        const float RateR = Math::MinF(panel.w() / Width, panel.h() / Height) / 2;
        ZAY_XYRR(panel, panel.w() / 2, panel.h() / 2, Width * RateR, Height * RateR)
            panel.stretch(LogoImage, true);

        // TAB
        /*ZAY_XYRR(panel, panel.w() / 2, panel.h() / 2 + Height * RateR * 1.5f, Width * RateR / 2, Height * RateR / 4)
        ZAY_RGBA(panel, 0, 0, 0, 32)
        {
            ZAY_FONT(panel, Width * RateR / 100, "Arial Black")
                panel.text("TAB", UIFA_CenterMiddle, UIFE_Right);
            panel.rect(Height * RateR / 50);
        }*/

        // 다음뷰로 이동
        if(m->mGoNextCounter == -1)
            m->mGoNextCounter = 5;
    }
    else
    {
        ZAY_RGB(panel, 255, 255, 255)
            panel.fill();

        const float FontSize = Math::MinF(panel.w(), panel.h()) / 400;
        ZAY_FONT(panel, FontSize, "Arial Black")
        {
            const sint32 Gap = panel.h() / 24;
            ZAY_LTRB_UI(panel, panel.w() * 0.10f, panel.h() * 0.15f, panel.w() * 0.70f, panel.h() * 0.25f, "stage",
                ZAY_GESTURE_VNT(v, n, t)
                {
                    if(t == GT_Pressed)
                    {
                        String Text = m->FirstStage();
                        const auto& VRect = v->rect(n);
                        if(Platform::Popup::OpenEditTracker(Text, UIET_String, VRect.l, VRect.t, VRect.r, VRect.b))
                        {
                            m->FirstStage() = Text;
                            m->invalidate();
                        }
                    }
                })
            {
                ZAY_RGBA(panel, 255, 255, 255, 128)
                    panel.fill();
                ZAY_RGB(panel, 0, 0, 0)
                {
                    if(0 < m->FirstStage().Length())
                        panel.text(m->FirstStage(), UIFA_CenterMiddle, UIFE_Right);
                    else
                    {
                        ZAY_RGB(panel, 192, 192, 192)
                        ZAY_FONT(panel, 0.75f, "Arial")
                            panel.text("로컬스테이지 플레이하기 (table_etc)", UIFA_CenterMiddle);
                    }
                    panel.rect(3);
                }
            }

            ZAY_LTRB_UI(panel, panel.w() * 0.75f, panel.h() * 0.15f, panel.w() * 0.90f, panel.h() * 0.25f, "run",
                ZAY_GESTURE_T(t)
                {
                    if(t == GT_InReleased)
                    {
                        // 로컬에서 제작된 스테이지를 플레이할 경우
                        if(0 < m->FirstStage().Length())
                            FXSaver::Write("LastStageJson").Set(String("f1/table_etc/") + m->FirstStage() + ".json");
                        FXSaver::Write("LastStageID").Set("Stage1");
                        Platform::Option::SetFlag("LandscapeMode", false);
                        Platform::Option::SetFlag("DirectPlay", false);
                        m->next("ingameView");
                    }
                })
            {
                ZAY_RGBA(panel, 0, 255, 255, 128)
                    panel.fill();
                ZAY_RGB(panel, 0, 0, 0)
                {
                    panel.text((m->mIsLandscape)? "RUN(W)" : "RUN", UIFA_CenterMiddle, UIFE_Right);
                    panel.rect(3);
                }
            }

            ZAY_LTRB_UI(panel, panel.w() * 0.10f, panel.h() * 0.30f, panel.w() * 0.90f, panel.h() * 0.40f, "maptool",
                ZAY_GESTURE_T(t)
                {
                    if(t == GT_InReleased)
                    {
                        Platform::Option::SetFlag("LandscapeMode", false);
                        m->next("maptoolView");
                    }
                })
            {
                ZAY_RGBA(panel, 255, 0, 255, 128)
                    panel.fill();
                ZAY_RGB(panel, 0, 0, 0)
                {
                    panel.text((m->mIsLandscape)? "MAP TOOL(W)" : "MAP TOOL", UIFA_CenterMiddle, UIFE_Right);
                    panel.rect(3);
                }
            }

            ZAY_LTRB_UI(panel, panel.w() * 0.10f, panel.h() * 0.45f, panel.w() * 0.90f, panel.h() * 0.55f, "stagetool",
                ZAY_GESTURE_T(t)
                {
                    if(t == GT_InReleased)
                    {
                        Platform::Option::SetFlag("LandscapeMode", false);
                        m->next("stagetoolView");
                    }
                })
            {
                ZAY_RGBA(panel, 255, 0, 255, 128)
                    panel.fill();
                ZAY_RGB(panel, 0, 0, 0)
                {
                    panel.text((m->mIsLandscape)? "STAGE TOOL(W)" : "STAGE TOOL", UIFA_CenterMiddle, UIFE_Right);
                    panel.rect(3);
                }
            }

            ZAY_LTRB_UI(panel, panel.w() * 0.10f, panel.h() * 0.60f, panel.w() * 0.90f, panel.h() * 0.70f, "soundtool",
                ZAY_GESTURE_T(t)
                {
                    if(t == GT_InReleased)
                        m->next("soundtoolView");
                })
            {
                ZAY_RGBA(panel, 255, 0, 255, 128)
                    panel.fill();
                ZAY_RGB(panel, 0, 0, 0)
                {
                    panel.text("SOUND TOOL", UIFA_CenterMiddle, UIFE_Right);
                    panel.rect(3);
                }
            }

            ZAY_LTRB_UI(panel, panel.w() * 0.10f, panel.h() * 0.75f, panel.w() * 0.90f, panel.h() * 0.85f, "pathtool",
                ZAY_GESTURE_T(t)
                {
                    if(t == GT_InReleased)
                        m->next("pathtoolView");
                })
            {
                ZAY_RGBA(panel, 255, 0, 255, 128)
                    panel.fill();
                ZAY_RGB(panel, 0, 0, 0)
                {
                    panel.text("PATH TOOL", UIFA_CenterMiddle, UIFE_Right);
                    panel.rect(3);
                }
            }
        }

        // 빌드버전
        FXDoor::RenderVersion(panel);
    }
}

f1Data::f1Data()
{
    mIsLandscape = false;
    mGoNextCounter = -1;
}

f1Data::~f1Data()
{
}
