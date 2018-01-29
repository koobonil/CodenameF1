#include <boss.hpp>
#include "soundtool_f1.hpp"

#include <resource.hpp>

ZAY_DECLARE_VIEW_CLASS("soundtoolView", soundtoolData)

ZAY_VIEW_API OnCommand(CommandType type, chars topic, id_share in, id_cloned_share* out)
{
    if(type == CT_Create)
    {
        // 윈도우 타이틀
        Platform::SetWindowName("Codename F1 [SoundTool]");
    }
    else if(type == CT_Size)
    {
        m->mScrollUpdateCommand = 0.1f;
    }
    else if(type == CT_Tick)
    {
        static uint64 OldTick = Platform::Utility::CurrentTimeMsec();
        const uint64 CurTick = Platform::Utility::CurrentTimeMsec();
        if(0 < m->mPlayTimeMsec)
        {
            const auto TimeSpan = Platform::Utility::CurrentTimeMsec() - m->mPlayTimeMsec;
            for(sint32 i = 0, iend = m->mSoundElements.Count(); i < iend; ++i)
            {
                if(!m->mSoundElements[i].mIsPlayCheck) continue;
                const uint64 SoundMsec = m->mPlayTimeMsec + m->mSoundElements[i].mDelayTimeMsec;
                if(OldTick < SoundMsec && SoundMsec <= CurTick)
                    Platform::Sound::Play(m->mState.GetSound(m->mSoundElements[i].mSoundName));
            }
        }
        OldTick = CurTick;
    }
    m->Command(type, in);
}

ZAY_VIEW_API OnNotify(chars sender, chars topic, id_share in, id_cloned_share* out)
{
}

ZAY_VIEW_API OnGesture(GestureType type, sint32 x, sint32 y)
{
    if(type == GT_WheelUp || type == GT_WheelUpPeeked)
    {
        m->mScrollUpdateCommand = 0.4f;
        m->mCurSound = Math::Max(0, m->mCurSound - 1);
        m->invalidate();
    }
    else if(type == GT_WheelDown || type == GT_WheelDownPeeked)
    {
        m->mScrollUpdateCommand = 0.4f;
        m->mCurSound = Math::Min(m->mCurSound + 1, m->mSoundElements.Count() - 1);
        m->invalidate();
    }
}

ZAY_VIEW_API OnRender(ZayPanel& panel)
{
    // 출력미비영역 표현용
    ZAY_RGB(panel, 128, 128, 128)
        panel.fill();
    m->Render(panel);
}

soundtoolData::soundtoolData() : mScroll(updater())
{
    mScrollUpdateCommand = 0;
    mScroll.Reset(0);
    mPlayTimeMsec = 0;
    mCurSound = 0;
    Platform::File::Search("assets:/f1/sound/*",
        [](chars name, payload data)->void
        {
            String FileName = name;
            if(!FileName.Right(4).CompareNoCase(".wav"))
            {
                auto Self = (soundtoolData*) data;
                auto& NewElement = Self->mSoundElements.AtAdding();
                NewElement.mSoundName = FileName.Left(FileName.Length() - 4);
            }
        }, this, false);
}

soundtoolData::~soundtoolData()
{
}

void soundtoolData::Render(ZayPanel& panel)
{
    ZAY_RGB(panel, 255, 255, 255)
        panel.fill();

    // 사운드리스트
    ZAY_LTRB_SCISSOR(panel, InnerGap, ButtonSize + InnerGap, panel.w() - InnerGap, panel.h() - (ButtonSize + InnerGap))
    {
        const float ScrollHeight = panel.h();
        if(mScrollUpdateCommand != 0)
        {
            ScrollUpdate(ScrollHeight, mScrollUpdateCommand);
            mScrollUpdateCommand = 0;
        }

        const float ScrollPos = mScroll.value();        
        for(sint32 i = 0, iend = mSoundElements.Count(); i < iend; ++i)
        {
            ZAY_XYWH_UI(panel, 0, IconSize * (i - ScrollPos), panel.w(), IconSize, String::Format("sound-%d", i),
                ZAY_GESTURE_VNT(v, n, t, this)
                {
                    if(t == GT_InReleased)
                    {
                        mScrollUpdateCommand = 0.6f;
                        mCurSound = Parser::GetInt(&n[6]);
                    }
                })
            {
                ZAY_RGB_IF(panel, 224, 224, 224, i == mCurSound)
                ZAY_RGB_IF(panel, 192, 192, 192, i != mCurSound)
                    panel.fill();
                ZAY_RGBA(panel, 0, 0, 0, 64)
                    panel.rect(2);
                RenderSound(panel, i);
            }
        }
        ZAY_INNER(panel, 2)
        ZAY_RGB(panel, 0, 0, 0)
            panel.rect(2);
    }

    // 툴UI
    const float OuterSize = ButtonSize * 1.5 * mUITween.value() / 100;
    ZAY_LTRB(panel, -OuterSize, -OuterSize, panel.w() + OuterSize, panel.h() + OuterSize)
    ZAY_FONT(panel, 1.0, "Arial Black")
    {
        // 고정버튼
        ZAY_XYWH(panel, ButtonSize * 1, 0, ButtonSize, ButtonSize)
            RenderLockToggle(panel);

        // 홈버튼
        ZAY_XYWH(panel, 0, 0, ButtonSize, ButtonSize)
            RenderHomeButton(panel);

        // 선택된 사운드의 플레이버튼
        ZAY_XYWH(panel, 0, panel.h() - ButtonSize, ButtonSize, ButtonSize)
        ZAY_INNER_UI(panel, InnerGap, "play",
            ZAY_GESTURE_T(t, this)
            {
                if(t == GT_InReleased)
                    mPlayTimeMsec = Platform::Utility::CurrentTimeMsec();
            })
        {
            ZAY_RGBA(panel, 128, 255, 128, 192)
                panel.fill();
            ZAY_RGB(panel, 0, 0, 0)
            {
                panel.rect(2);
                ZAY_FONT(panel, 2.5)
                    panel.text("▶", UIFA_CenterMiddle);
            }
        }

        // 선택된 사운드의 중지버튼
        ZAY_XYWH(panel, ButtonSize, panel.h() - ButtonSize, ButtonSize, ButtonSize)
        ZAY_INNER_UI(panel, InnerGap, "stop",
            ZAY_GESTURE_T(t, this)
            {
                if(t == GT_InReleased)
                {
                    mPlayTimeMsec = 0;
                    for(sint32 i = 0, iend = m->mSoundElements.Count(); i < iend; ++i)
                    {
                        if(!m->mSoundElements[i].mIsPlayCheck) continue;
                        Platform::Sound::Stop(m->mState.GetSound(m->mSoundElements[i].mSoundName));
                    }
                }
            })
        {
            ZAY_RGBA(panel, 255, 128, 128, 192)
                panel.fill();
            ZAY_RGB(panel, 0, 0, 0)
            {
                panel.rect(2);
                ZAY_FONT(panel, 2.5)
                    panel.text("▣", UIFA_CenterMiddle);
            }
        }
    }
}

void soundtoolData::RenderSound(ZayPanel& panel, sint32 i)
{
    ZAY_INNER_SCISSOR(panel, 4)
    ZAY_RGB(panel, 0, 0, 0)
    {
        // 체크버튼
        ZAY_LTRB_UI(panel, 0, 0, IconSize * 1, panel.h(), String::Format("sound-%d-check", i),
            ZAY_GESTURE_T(t, this, i)
            {
                if(t == GT_InReleased)
                    mSoundElements.At(i).mIsPlayCheck ^= 1;
            })
        ZAY_INNER(panel, 4)
        {
            if(mSoundElements[i].mIsPlayCheck)
            {
                ZAY_RGB(panel, -192, -192, -240)
                    panel.fill();
                ZAY_FONT(panel, 2.0)
                    panel.text(panel.w() / 2, panel.h() / 2, "√", UIFA_CenterMiddle);
            }
            else ZAY_RGB(panel, -128, -128, -128)
                panel.fill();
            panel.rect(2);
        }

        // 슬라이더
        ZAY_LTRB_UI(panel, IconSize * 1, 0, IconSize * 3, panel.h(), String::Format("sound-%d-slide", i),
            ZAY_GESTURE_VNTXY(v, n, t, x, y, this, i)
            {
                if(t == GT_Pressed || t == GT_InDragging || t == GT_OutDragging)
                {
                    rect128 Area = v->rect(n);
                    Area.l += 8;
                    Area.r -= 8;
                    const sint32 Width = Area.r - Area.l;
                    mSoundElements.At(i).mDelayTimeMsec = Math::Clamp(x - Area.l, 0, Width) * 1000 / Width;
                    v->invalidate();
                }
            })
        ZAY_INNER(panel, 4)
        {
            // 슬라이더 라인
            ZAY_XYWH(panel, 0, panel.h() / 2 - 1, panel.w(), 2)
            {
                ZAY_RGB(panel, -128, -128, -128)
                    panel.fill();
                panel.rect(2);
            }
            // 슬라이더 바
            ZAY_LTRB(panel, 4, 0, panel.w() - 4, panel.h())
            ZAY_XYRR(panel, panel.w() * mSoundElements[i].mDelayTimeMsec / 1000, panel.h() / 2, 4, panel.h() / 2)
            {
                ZAY_RGB(panel, -64, -64, -96)
                    panel.fill();
                panel.rect(2);
            }
        }

        // 사운드명칭
        ZAY_LTRB(panel, IconSize * 3, 0, panel.w() - IconSize * 2, panel.h())
        ZAY_INNER(panel, 4)
        {
            if(-2 < panel.w())
            {
                ZAY_RGB(panel, -128, 255, 255)
                    panel.fill();
                ZAY_FONT(panel, 1.25)
                    panel.text(String::Format("%s", (chars) mSoundElements[i].mSoundName), UIFA_CenterMiddle, UIFE_Right);
                panel.rect(2);
            }
        }

        // 플레이버튼
        const String PlayButton = String::Format("sound-%d-play", i);
        ZAY_LTRB_UI(panel, panel.w() - IconSize * 2, 0, panel.w() - IconSize * 1, panel.h(), PlayButton,
            ZAY_GESTURE_T(t, this, i)
            {
                if(t == GT_InReleased)
                    Platform::Sound::Play(mState.GetSound(mSoundElements[i].mSoundName));
            })
        ZAY_INNER(panel, 4)
        {
            if(panel.state(PlayButton) & PS_Pressed)
            {
                ZAY_RGBA(panel, 255, 255, 0, 128)
                    panel.fill();
            }
            ZAY_FONT(panel, 1.25)
                panel.text("▶", UIFA_CenterMiddle);
            panel.rect(2);
        }

        // 중지버튼
        const String StopButton = String::Format("sound-%d-stop", i);
        ZAY_LTRB_UI(panel, panel.w() - IconSize * 1, 0, panel.w(), panel.h(), StopButton,
            ZAY_GESTURE_T(t, this, i)
            {
                if(t == GT_InReleased)
                    Platform::Sound::Stop(mState.GetSound(mSoundElements[i].mSoundName));
            })
        ZAY_INNER(panel, 4)
        {
            if(panel.state(StopButton) & PS_Pressed)
            {
                ZAY_RGBA(panel, 255, 255, 0, 128)
                    panel.fill();
            }
            ZAY_FONT(panel, 1.25)
                panel.text("▣", UIFA_CenterMiddle);
            panel.rect(2);
        }
    }
}

void soundtoolData::ScrollUpdate(sint32 height, float sec)
{
    const float ScrollMid = (height - IconSize) / 2;
    const float ScrollEnd = IconSize * mSoundElements.Count() - height;
    mScroll.MoveTo(Math::ClampF(mCurSound * IconSize - ScrollMid, 0, ScrollEnd) / IconSize, sec);
}

void soundtoolData::OnModeChanged()
{
}

void soundtoolData::OnSelectSub(chars name)
{
}

void soundtoolData::InitSelectBox(sint32 index)
{
}

void soundtoolData::QuitSelectBox(sint32 index)
{
}

void soundtoolData::ChangeSelectBox(sint32 type, sint32 index)
{
}

void soundtoolData::OnSelectBoxMoving(sint32 index, float addx, float addy)
{
}

void soundtoolData::OnSelectBoxMoved(sint32 index)
{
}

void soundtoolData::OnSelectBoxSizing(sint32 index, float addx, float addy)
{
}

void soundtoolData::OnSelectBoxSized(sint32 index)
{
}

void soundtoolData::OnSelectBoxClone(sint32 index)
{
}
