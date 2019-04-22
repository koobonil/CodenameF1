#include <boss.hpp>
#include "sourcetool.hpp"

ZAY_DECLARE_VIEW_CLASS("sourcetoolView", sourcetoolData)

ZAY_VIEW_API OnCommand(CommandType type, chars topic, id_share in, id_cloned_share* out)
{
}

ZAY_VIEW_API OnNotify(NotifyType type, chars topic, id_share in, id_cloned_share* out)
{
}

ZAY_VIEW_API OnGesture(GestureType type, sint32 x, sint32 y)
{
}

ZAY_VIEW_API OnRender(ZayPanel& panel)
{
    ZAY_RGB(panel, 96, 96, 96)
        panel.fill();

    ZAY_FONT(panel, 2.0f, "Arial Black")
    ZAY_RGB(panel, 240, 160, 160)
        panel.text("*소스코드 일괄변경*", UIFA_CenterTop);

    // 실행버튼
    const bool RunEnabled = (m->OptionFileEnabled && m->TargetFolderEnabled);
    ZAY_XYWH_UI_SCISSOR(panel, panel.w() - 110, 10, 100, 30, "Run",
        ZAY_GESTURE_T(type)
        {
            if(type == GT_InReleased)
            {
                m->Run();
                m->invalidate();
            }
        })
    {
        ZAY_RGBA_IF(panel, 255, 0, 0, 128, RunEnabled)
        ZAY_RGBA_IF(panel, 255, 0, 0, 64, !RunEnabled)
            panel.fill();
        ZAY_FONT(panel, 1.5f, "Arial")
        ZAY_RGBA_IF(panel, 255, 255, 255, 160, RunEnabled)
        ZAY_RGBA_IF(panel, 255, 255, 255, 32, !RunEnabled)
            panel.text(panel.w() / 2, panel.h() / 2, "실행", UIFA_CenterMiddle);
    }

    // 명령어파일
    ZAY_XYWH(panel, 0, 50, panel.w(), 40)
    {
        ZAY_RGBA(panel, 255, 255, 255, 192)
            panel.fill();
        ZAY_FONT(panel, 2.0f, "Arial Black")
        ZAY_RGB(panel, 0, 0, 0)
            panel.text(150, panel.h() / 2, "명령어파일:", UIFA_RightMiddle);
        ZAY_LTRB(panel, 150, 0, panel.w(), panel.h())
        ZAY_INNER_UI_SCISSOR(panel, 5, "Option",
            ZAY_GESTURE_NT(name, type)
            {
                if(type == GT_InReleased)
                {
                    const rect128& CurRect = m->rect(name);
                    String GetString = m->OptionFile;
                    if(Platform::Popup::OpenEditTracker(GetString, UIET_String, CurRect.l, CurRect.t, CurRect.r, CurRect.b))
                    {
                        m->OptionFile = GetString;
                        m->OptionFileEnabled = Platform::File::Exist(m->OptionFile);
                    }
                    m->invalidate();
                }
            })
        {
            ZAY_RGBA(panel, 96, 96, 96, 64)
                panel.fill();
            ZAY_FONT(panel, 1.5f, "Arial")
            ZAY_RGB_IF(panel, 0, 0, 192, m->OptionFileEnabled)
            ZAY_RGB_IF(panel, 96, 96, 96, !m->OptionFileEnabled)
                panel.text(5, panel.h() / 2, m->OptionFile, UIFA_LeftMiddle);
        }
    }

    // 대상폴더
    ZAY_XYWH(panel, 0, 100, panel.w(), 40)
    {
        ZAY_RGBA(panel, 255, 255, 255, 192)
            panel.fill();
        ZAY_FONT(panel, 2.0f, "Arial Black")
        ZAY_RGB(panel, 0, 0, 0)
            panel.text(150, panel.h() / 2, "대상폴더:", UIFA_RightMiddle);
        ZAY_LTRB(panel, 150, 0, panel.w(), panel.h())
        ZAY_INNER_UI_SCISSOR(panel, 5, "Target",
            ZAY_GESTURE_NT(name, type)
            {
                if(type == GT_InReleased)
                {
                    const rect128& CurRect = m->rect(name);
                    String GetString = m->TargetFolder;
                    if(Platform::Popup::OpenEditTracker(GetString, UIET_String, CurRect.l, CurRect.t, CurRect.r, CurRect.b))
                    {
                        m->TargetFolder = GetString;
                        m->TargetFolderEnabled = Platform::File::ExistForDir(m->TargetFolder);
                    }
                    m->invalidate();
                }
            })
        {
            ZAY_RGBA(panel, 96, 96, 96, 64)
                panel.fill();
            ZAY_FONT(panel, 1.5f, "Arial")
            ZAY_RGB_IF(panel, 0, 0, 192, m->TargetFolderEnabled)
            ZAY_RGB_IF(panel, 96, 96, 96, !m->TargetFolderEnabled)
                panel.text(5, panel.h() / 2, m->TargetFolder, UIFA_LeftMiddle);
        }
    }
}
