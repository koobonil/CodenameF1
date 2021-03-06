﻿#include <boss.hpp>
#include "classes.hpp"

#include <service/boss_parasource.hpp>
#include <resource.hpp>

////////////////////////////////////////////////////////////////////////////////
static sint32 gSpineRendererCount = 0;
SpineRenderer::SpineRenderer()
{
    gSpineRendererCount++;
    mSpine = nullptr;
}

SpineRenderer::~SpineRenderer()
{
    Release();
    if(--gSpineRendererCount == 0)
        ZAY::SpineBuilder::ClearCaches();
}

void SpineRenderer::Create(String projcode, chars spinepath, chars imagepath)
{
    Release();
    // 스파인 불러오기
    Context Pathes(ST_Json, SO_NeedCopy, String::FromAsset(projcode + imagepath));
    id_assetpath NewPath = Asset::CreatePath();
    for(sint32 i = 0, iend = Pathes.LengthOfIndexable(); i < iend; ++i)
        Asset::AddByPath(NewPath, projcode + Pathes[i].GetString());
    ZAY::SpineBuilder::SetAssetPath(projcode, NewPath);
    mSpine = ZAY::SpineBuilder::LoadSpine(projcode + spinepath, "image");
    ZAY::SpineBuilder::SetAssetPath("", nullptr);
    Asset::ReleasePath(NewPath);
}

void SpineRenderer::Release()
{
    ZAY::SpineBuilder::FreeSpine(mSpine);
    mSpine = nullptr;
}

void SpineRenderer::Render(ZAY::id_spine_instance instance, ZayPanel& panel,
    float sx, float sy, float sw, float sh, sint32 h, float scale, bool flip, bool outline) const
{
    if(!instance) return;
    const Point XY = panel.toview(0, 0);
    const float X = XY.x * panel.zoom();
    const float Y = (h - (XY.y + panel.h())) * panel.zoom();
    const float W = panel.w() * panel.zoom();
    const float H = panel.h() * panel.zoom();
    const float cx = ((X + W * 0.5f) - sx) / sw;
    const float cy = ((Y + H * 0.5f) - sy) / sh;

    if(outline) ZAY::SpineBuilder::Render(panel, instance, flip, cx, cy, scale, 1.0f, sx, sy, sw, sh);
    ZAY::SpineBuilder::Render(panel, instance, flip, cx, cy, scale, 0.0f, sx, sy, sw, sh);
}

void SpineRenderer::RenderShadow(ZAY::id_spine_instance instance, ZayPanel& panel,
    float sx, float sy, float sw, float sh, sint32 h, float scale, bool flip) const
{
    if(!instance) return;
    const Point XY = panel.toview(0, 0);
    const float X = XY.x * panel.zoom();
    const float Y = (h - (XY.y + panel.h())) * panel.zoom();
    const float W = panel.w() * panel.zoom();
    const float H = panel.h() * panel.zoom();
    const float cx = ((X + W * 0.5f) - sx) / sw;
    const float cy = ((Y + H * 0.5f) - sy) / sh;

    ZAY::SpineBuilder::Render(panel, instance, flip, cx, cy, scale, (flip)? 0.2f : 0.8f, sx, sy, sw, sh);
}

void SpineRenderer::RenderPanel(ZAY::id_spine_instance instance, ZayPanel& panel,
    float ox, float oy, float scale, bool flip, ZayPanel::SubRenderCB cb) const
{
    if(!instance) return;
    ZAY::SpineBuilder::RenderPanel(panel, instance, ox, oy, scale, flip, cb);
}

void SpineRenderer::RenderBound(ZAY::id_spine_instance instance, ZayPanel& panel,
    bool guideline, float ox, float oy, float scale, bool flip, chars uiname, ZayPanel::SubGestureCB cb) const
{
    if(!instance) return;
    ZAY::SpineBuilder::RenderBound(panel, instance, ox, oy, scale, flip, guideline, uiname, cb);
}

////////////////////////////////////////////////////////////////////////////////
MapSpine::MapSpine(SpineType type) : mSpineType(type)
{
    mSpineRenderer = nullptr;
    mSpineInstance = nullptr;
    mSpineMsecOld = 0;
    mSeekSec = 0;
    mSeekSecOld = -1;
    mStaffIdleMode = false;
    mStaffStartMode = false;
}

MapSpine::~MapSpine()
{
    ZAY::SpineBuilder::Release(mSpineInstance);
}

MapSpine::MapSpine(const MapSpine& rhs) : mSpineType(rhs.mSpineType)
{
    mSpineInstance = nullptr;
    operator=(rhs);
}

MapSpine& MapSpine::operator=(const MapSpine& rhs)
{
    mSpineRenderer = rhs.mSpineRenderer;
    ZAY::SpineBuilder::Release(mSpineInstance);
    mSpineInstance = nullptr;
    if(rhs.mSpineInstance)
        mSpineInstance = ZAY::SpineBuilder::Clone(rhs.mSpineInstance);
    mSpineMsecOld = rhs.mSpineMsecOld;
    mSeekSec = rhs.mSeekSec;
    mSeekSecOld = rhs.mSeekSecOld;
    mStaffIdleMode = rhs.mStaffIdleMode;
    mStaffStartMode = rhs.mStaffStartMode;
    return *this;
}

MapSpine::MapSpine(MapSpine&& rhs) : mSpineType(rhs.mSpineType)
{
    operator=(ToReference(rhs));
}

MapSpine& MapSpine::operator=(MapSpine&& rhs)
{
    mSpineRenderer = rhs.mSpineRenderer;
    mSpineInstance = rhs.mSpineInstance;
    rhs.mSpineInstance = nullptr;
    mSpineMsecOld = rhs.mSpineMsecOld;
    mSeekSec = rhs.mSeekSec;
    mSeekSecOld = rhs.mSeekSecOld;
    mStaffIdleMode = rhs.mStaffIdleMode;
    mStaffStartMode = rhs.mStaffStartMode;
    return *this;
}

MapSpine& MapSpine::InitSpine(const FXState* state, const SpineRenderer* renderer, chars skin,
    ZAY::SpineBuilder::MotionFinishedCB fcb, ZAY::SpineBuilder::UserEventCB ecb)
{
    // 초기화
    mSpineRenderer = renderer;
    ZAY::SpineBuilder::Release(mSpineInstance);
    mSpineInstance = nullptr;
    mSpineMsecOld = 0;
    mSeekSec = 0;
    mSeekSecOld = -1;
    mStaffIdleMode = false;
    mStaffStartMode = false;

    if(mSpineRenderer)
    {
        branch;
        jump(ecb)
            mSpineInstance = ZAY::SpineBuilder::Create(mSpineRenderer->spine(), skin, fcb, ecb);
        jump(!state)
            mSpineInstance = ZAY::SpineBuilder::Create(mSpineRenderer->spine(), skin, fcb, nullptr);
        else
            mSpineInstance = ZAY::SpineBuilder::Create(mSpineRenderer->spine(), skin, fcb,
                [](chars eventname)
                {
                    if(auto CurState = FXState::ST())
                    {
                        if(!String::Compare("sound_", eventname, 6))
                        {
                            if(FXSaver::Read("SoundFlag").GetInt())
                                FXState::PlaySound(CurState->GetSound(&eventname[6]));
                        }
                        else CurState->OnEvent(eventname);
                    }
                });
    }
    return *this;
}

void MapSpine::SetSkin(chars skin)
{
    if(!mSpineInstance) return;
    ZAY::SpineBuilder::SetSkin(mSpineInstance, skin);
}

void MapSpine::PlayMotion(chars motion, bool repeat, float beginsec)
{
    if(!mSpineInstance) return;
    ZAY::SpineBuilder::SetMotionOn(mSpineInstance, motion, repeat, beginsec);
}

void MapSpine::PlayMotionOnce(chars motion)
{
    if(!mSpineInstance) return;
    ZAY::SpineBuilder::SetMotionOnOnce(mSpineInstance, motion);
}

void MapSpine::PlayMotionAttached(chars first_motion, chars second_motion, bool repeat)
{
    if(!mSpineInstance) return;
    ZAY::SpineBuilder::SetMotionOnOnce(mSpineInstance, first_motion);
    ZAY::SpineBuilder::SetMotionOnAttached(mSpineInstance, first_motion, second_motion, repeat);
}

void MapSpine::PlayMotionSeek(chars seek_motion, bool repeat)
{
    if(!mSpineInstance) return;
    ZAY::SpineBuilder::SetMotionOnSeek(mSpineInstance, seek_motion, false);
}

String MapSpine::PlayMotionScript(chars script)
{
    if(!mSpineInstance) return "";
    const String Text = script;
    Strings Motions;
    sint32 LastPos = 0;
    for(sint32 nextpos = 0; (nextpos = Text.Find(LastPos, '-')) != -1; LastPos = nextpos + 1)
        Motions.AtAdding() = String(((chars) Text) + LastPos, nextpos - LastPos);
    Motions.AtAdding() = String(((chars) Text) + LastPos, Text.Length() - LastPos);

    ZAY::SpineBuilder::SetMotionOffAllWithoutSeek(mSpineInstance, true);
    for(sint32 i = 0, iend = Motions.Count(); i < iend; ++i)
    {
        if(i == 0)
            ZAY::SpineBuilder::SetMotionOn(mSpineInstance, Motions[i], iend == 1);
        else ZAY::SpineBuilder::SetMotionOnAttached(mSpineInstance, Motions[i - 1], Motions[i], i == iend - 1);
    }
    return Motions[-1];
}

void MapSpine::StopMotion(chars motion)
{
    if(!mSpineInstance) return;
    ZAY::SpineBuilder::SetMotionOff(mSpineInstance, motion);
}

void MapSpine::StopMotionAll()
{
    if(!mSpineInstance) return;
    mSpineMsecOld = 0;
    ZAY::SpineBuilder::SetMotionOffAllWithoutSeek(mSpineInstance, true);
}

void MapSpine::Seek() const
{
    if(!mSpineInstance) return;
    mSeekSecOld = mSeekSec;
    ZAY::SpineBuilder::Seek(mSpineInstance, mSeekSec);
}

void MapSpine::Update() const
{
    if(mSpineInstance)
    {
        const uint64 SpineMsecOld = mSpineMsecOld;
        const uint64 SpineMsecNew = Platform::Utility::CurrentTimeMsec();
        mSpineMsecOld = SpineMsecNew;
        const float DeltaSec = (SpineMsecOld == 0)?
            0 : (SpineMsecNew - SpineMsecOld) * 0.001f;
        ZAY::SpineBuilder::Update(mSpineInstance, DeltaSec);
    }
}

void MapSpine::RenderObject(DebugMode debug, bool needupdate, ZayPanel& panel, bool flip, chars uiname,
    ZayPanel::SubGestureCB gcb, ZayPanel::SubRenderCB rcb) const
{
    if(needupdate)
    {
        if(IsSeekUpdated())
            Seek();
        Update();
        if(mSpineType == ST_MonsterToast && !enabled())
            return; // 업데이트후 즉시 사라져야 할 수도 있음
    }

    if(const Rect* AreaRect = GetBoundRect("area"))
    {
        const Point XY = panel.toview(0, 0);
        const float SX = XY.x * panel.zoom();
        const float SY = XY.y * panel.zoom();
        const float SW = panel.w() * panel.zoom();
        const float SH = panel.h() * panel.zoom();
        const float Width = AreaRect->Width();
        const float Height = AreaRect->Height();
        const float Rate = Math::MinF(SW / Width, SH / Height);
        const float CX = (flip)? -AreaRect->CenterX() : AreaRect->CenterX();
        const float CY = AreaRect->CenterY();

        ZAY_XYRR(panel, SW / 2 - CX * Rate, SH / 2 - CY * Rate, 0, 0)
        {
            Platform::Graphics::BeginGL();
            renderer()->Render(mSpineInstance, panel, SX, panel.screen_h() - (SY + SH), SW, SH, panel.screen_h(),
                Rate, flip, false); // 외곽선제거: mSpineType == ST_Monster || mSpineType == ST_Dragon);
            Platform::Graphics::EndGL();
        }

        if(rcb)
        ZAY_XYRR(panel, SW / 2, SH / 2, 0, 0)
            renderer()->RenderPanel(mSpineInstance, panel, CX, CY, Rate, flip, rcb);

        if(debug != DebugMode::None)
        ZAY_RGBA(panel, 0, 0, 0, 128)
            panel.fill();

        if(debug != DebugMode::None || uiname)
        ZAY_XYRR(panel, SW / 2, SH / 2, 0, 0)
            renderer()->RenderBound(mSpineInstance, panel, debug != DebugMode::None, CX, CY, Rate, flip, uiname, gcb);

        if(debug != DebugMode::None)
        ZAY_RGBA(panel, 255, 0, 0, 128)
            panel.rect(1);
    }
}

void MapSpine::RenderObjectShadow(ZayPanel& panel, bool flip) const
{
    if(const Rect* AreaRect = GetBoundRect("area"))
    {
        const Point XY = panel.toview(0, 0);
        const float SX = XY.x * panel.zoom();
        const float SY = XY.y * panel.zoom();
        const float SW = panel.w() * panel.zoom();
        const float SH = panel.h() * panel.zoom();
        const float Width = AreaRect->Width();
        const float Height = AreaRect->Height();
        const float Rate = Math::MinF(SW / Width, SH / Height);
        const float CX = (flip)? -AreaRect->CenterX() : AreaRect->CenterX();
        const float CY = AreaRect->CenterY();

        ZAY_XYRR(panel, SW / 2 - CX * Rate, SH / 2 - CY * Rate, 0, 0)
        {
            Platform::Graphics::BeginGL();
            renderer()->RenderShadow(mSpineInstance, panel, SX, panel.screen_h() - (SY + SH), SW, SH, panel.screen_h(), Rate, flip);
            Platform::Graphics::EndGL();
        }
    }
}

const Rect* MapSpine::GetBoundRect(chars name) const
{
    if(!mSpineInstance) return nullptr;
    return ZAY::SpineBuilder::GetBoundRect(mSpineInstance, name);
}

const Points* MapSpine::GetBoundPolygon(chars name) const
{
    if(!mSpineInstance) return nullptr;
    return ZAY::SpineBuilder::GetBoundPolygon(mSpineInstance, name);
}

void MapSpine::SetSeekSec(float sec)
{
    mSeekSec = sec;
}

bool MapSpine::IsSeekUpdated() const
{
    return (mSeekSec != mSeekSecOld);
}

void MapSpine::Staff_TryIdle()
{
    if(mSpineInstance && !mStaffIdleMode)
    {
        mStaffIdleMode = true;
        ZAY::SpineBuilder::SetMotionOffAllWithoutSeek(mSpineInstance, true);
        ZAY::SpineBuilder::SetMotionOn(mSpineInstance, "idle", true);
    }
}

void MapSpine::Staff_Start()
{
    if(mSpineInstance && !mStaffStartMode)
    {
        mStaffIdleMode = true;
        mStaffStartMode = true;
        ZAY::SpineBuilder::SetMotionOffAllWithoutSeek(mSpineInstance, true);
        ZAY::SpineBuilder::SetMotionOn(mSpineInstance, "start", false);
    }
}

////////////////////////////////////////////////////////////////////////////////
FXDoor::FXDoor() : mAuthCode(GetAuthCode())
{
    mLoaded = false;
    mIsParaAuth = false;
    mParaAuthSuccess = false;

}

FXDoor::~FXDoor()
{
}

String FXDoor::GetAuthCode()
{
    String AuthCode;
    const String AuthString = String::FromAsset("paraauth.json");
    if(0 < AuthString.Length())
    {
        Context AuthFile(ST_Json, SO_OnlyReference, AuthString, AuthString.Length());
        AuthCode = AuthFile("AuthCode").GetString();
    }
    else
    {
        AuthCode += 'A' + (Platform::Utility::Random() % ('Z' - 'A' + 1));
        AuthCode += 'A' + (Platform::Utility::Random() % ('Z' - 'A' + 1));
        AuthCode += String::Format("%04d", Platform::Utility::Random() % 10000);
        Context AuthFile;
        AuthFile.At("AuthCode").Set(AuthCode);
        AuthFile.SaveJson().ToAsset("paraauth.json");
    }
    return AuthCode;
}

bool FXDoor::IsLocked() const
{
    return (mIsParaAuth && !mParaAuthSuccess);
}

bool FXDoor::Load()
{
    if(mLoaded)
        return true;

    mService = "";
    mComment = "";
    mIsParaAuth = true;
    mParaAuthSuccess = false;
    mParaAuthName = "";
    ParaSource DoorSource(ParaSource::IIS);
    DoorSource.SetContact("localhost", 80);

    // 도어인증
    Context Door;
    if(DoorSource.GetJson(Door, "f1/door.txt"))
    {
        const String BuildVersion = Platform::Option::GetText("BuildVersion");
        for(sint32 i = 0, iend = Door.LengthOfIndexable(); i < iend; ++i)
        {
            const sint32 BuildMatchingCount = Door[i]("BuildMatchingCount").GetInt(0);
            for(sint32 j = 0; j < BuildMatchingCount; ++j)
            {
                const String Version = Door[i](String::Format("BuildMatching%d_Version", j + 1)).GetString();
                if(!String::Compare(Version, BuildVersion, Version.Length()))
                {
                    mService = Door[i]("Service").GetString();
                    mComment = Door[i](String::Format("BuildMatching%d_Comment", j + 1)).GetString();
                    mStagePack.Init(Door[i]("StagePack").GetString());
                    mLanguagePack.Init(Door[i]("LanguagePack").GetString());
                    mAccountCenter = Door[i]("AccountCenter").GetString();
                    mAccountManager = Door[i]("AccountManager").GetString();
                    mGlobalWeight.LoadPrm(Door[i]("GlobalWeight").GetString());
                    if(0 < mAccountCenter.Length())
                    {
                        // 인증코드매칭
                        if(!String::CompareNoCase(mAccountCenter, "http://cafe.naver.com/", 22))
                        {
                            ParaSource AuthSource(ParaSource::NaverCafe);
                            AuthSource.SetContact("cafe.naver.com", 80);
                            Context Temp;
                            AuthSource.GetJson(Temp, ((chars) mAccountCenter) + 22);
                            Context AuthComment;
                            if(AuthSource.GetLastSpecialJson(AuthComment))
                            {
                                static const String ParaAuthText = "[paraauth/"; // [paraauth/AB1234/뽀로로]
                                static const String ParaAuthEndText = "]";
                                const Context& List = AuthComment("result")("list");
                                for(sint32 i = 0, iend = List.LengthOfIndexable(); i < iend && !mParaAuthSuccess; ++i)
                                {
                                    if(0 < mAccountManager.Length() && !!mAccountManager.Compare(List[i]("writerid").GetString()))
                                        continue;
                                    const String ContentText = List[i]("content").GetString();
                                    sint32 ParaAuthPos = 0;
                                    while((ParaAuthPos = ContentText.Find(ParaAuthPos, ParaAuthText)) != -1)
                                    {
                                        ParaAuthPos += ParaAuthText.Length();
                                        if(!String::Compare(mAuthCode, &((chars) ContentText)[ParaAuthPos], mAuthCode.Length()))
                                        {
                                            ParaAuthPos += mAuthCode.Length() + 1; // '/'
                                            const sint32 ParaAuthPosEnd = ContentText.Find(ParaAuthPos, ParaAuthEndText);
                                            if(ParaAuthPosEnd != -1)
                                            {
                                                mParaAuthSuccess = true;
                                                mParaAuthName = String(&((chars) ContentText)[ParaAuthPos], ParaAuthPosEnd - ParaAuthPos);
                                                break;
                                            }
                                        }
                                    }
                                }
                            }
                        }
                    }
                    else mIsParaAuth = false;
                    return (mLoaded = true);
                }
            }
        }
    }
    return (mLoaded = false);
}

String FXDoor::GetGlobalWeight(chars id) const
{
    return mGlobalWeight(id).GetString("");
}

void FXDoor::Render(ZayPanel& panel)
{
    const float FontSize = Math::MinF(panel.w(), panel.h()) / 240.0f;
    if(mService.Length())
    {
        // 버전
        ZAY_FONT(panel, FontSize, "Arial Black")
        {
            const sint32 FontHeightA = Platform::Graphics::GetStringHeight();
            ZAY_RGBA(panel, 0, 0, 0, 128)
                panel.text(panel.w() - 5 + 1, 1 + FontHeightA * 0.1f, mService, UIFA_RightTop);
            ZAY_RGB(panel, 255, 255, 255)
                panel.text(panel.w() - 5, 0 + FontHeightA * 0.1f, mService, UIFA_RightTop);

            if(0 < mComment.Length())
            ZAY_FONT(panel, 0.8)
            {
                const sint32 FontHeightB = Platform::Graphics::GetStringHeight();
                ZAY_RGBA(panel, 0, 0, 0, 128)
                    panel.text(panel.w() - 5 + 1, FontHeightA * 1.0f + 1 - FontHeightB * 0.1f, mComment, UIFA_RightTop);
                ZAY_RGB(panel, 255, 255, 0)
                    panel.text(panel.w() - 5, FontHeightA * 1.0f + 0 - FontHeightB * 0.1f, mComment, UIFA_RightTop);
            }
        }

        // 인증
        if(mIsParaAuth)
        {
            ZAY_XYWH(panel, 0, panel.h() / 2 - FontSize * 15, panel.w(), FontSize * 30)
            {
                ZAY_RGBA(panel, 0, 0, 0, 160)
                    panel.fill();
                ZAY_RGB(panel, 0, 0, 0)
                {
                    ZAY_LTRB(panel, -2, 0, panel.w() + 2, panel.h())
                        panel.rect(2);
                    ZAY_FONT(panel, FontSize / 2, "Arial Black")
                        panel.text(panel.w(), panel.h(), "PARAAUTH ", UIFA_RightBottom);
                }
                if(mParaAuthSuccess)
                {
                    ZAY_FONT(panel, FontSize)
                    ZAY_RGB(panel, 0, 255, 128)
                        panel.text(mParaAuthName, UIFA_CenterMiddle, UIFE_Right);
                }
                else
                {
                    ZAY_FONT(panel, FontSize, "Arial Black")
                    ZAY_XYRR(panel, panel.w() / 2, panel.h() / 2, panel.h() * 2.5, panel.h() / 2)
                    for(sint32 i = 0; i < 6; ++i)
                    {
                        ZAY_LTRB(panel, panel.w() * i / 6, 0, panel.w() * (i + 1) / 6, panel.h())
                        ZAY_INNER(panel, FontSize * 5)
                        {
                            ZAY_RGB(panel, 0, 0, 0)
                                panel.rect(Math::Ceil(FontSize));
                            ZAY_RGB(panel, 255, 0, 0)
                                panel.text(panel.w() / 2, panel.h() / 2, String(mAuthCode[i]), UIFA_CenterMiddle);
                        }
                    }
                }
            }
            if(!mParaAuthSuccess)
            {
                ZAY_XYRR_UI(panel, panel.w() / 2, panel.h() / 2 + FontSize * 30, FontSize * 50, FontSize * 10, "go_site",
                    ZAY_GESTURE_T(t, this)
                    {
                        if(t == GT_Pressed)
                            Platform::Popup::WebBrowserDialog(mAccountCenter);
                    })
                {
                    ZAY_RGBA(panel, 0, 0, 0, 160)
                        panel.fill();
                    ZAY_RGB(panel, 0, 0, 0)
                        panel.rect(2);
                    ZAY_FONT(panel, FontSize * 0.75f)
                    ZAY_RGB(panel, 255, 255, 0)
                        panel.text("인증하기", UIFA_CenterMiddle, UIFE_Right);
                }
            }
        }
    }
    else
    {
        // 메시지
        ZAY_XYWH(panel, 0, panel.h() / 2 - FontSize * 15, panel.w(), FontSize * 30)
        {
            ZAY_RGBA(panel, 0, 0, 0, 160)
                panel.fill();
            ZAY_RGB(panel, 0, 0, 0)
            {
                ZAY_LTRB(panel, -2, 0, panel.w() + 2, panel.h())
                    panel.rect(2);
                ZAY_FONT(panel, FontSize / 2, "Arial Black")
                    panel.text(panel.w(), panel.h(), "PARAAUTH ", UIFA_RightBottom);
            }
            ZAY_FONT(panel, FontSize * 0.75)
            ZAY_RGB(panel, 224, 192, 192)
                panel.text("This program is an unknown version.\r\n"
                    "Update may be required.", UIFA_CenterMiddle);
        }

        // 버전표시
        RenderVersion(panel);
    }
}

void FXDoor::RenderVersion(ZayPanel& panel)
{
    const float FontSize = Math::MinF(panel.w(), panel.h()) / 240.0f;
    const String BuildVersion = Platform::Option::GetText("BuildVersion");
    ZAY_XYRR(panel, panel.w() / 2, panel.h() / 14 + FontSize * 5, FontSize * 80, FontSize * 10)
    {
        ZAY_RGBA(panel, 0, 0, 0, 160)
            panel.fill();
        ZAY_FONT(panel, FontSize * 0.75f)
        ZAY_RGB(panel, 255, 255, 0)
            panel.text(panel.w() / 2, panel.h() / 2, BuildVersion, UIFA_CenterMiddle);
    }
}

////////////////////////////////////////////////////////////////////////////////
FXState* gState = nullptr;
FXState::FXState() : mDoor(&FXDoor::ST()), mData(&FXData::ST())
{
	gState = this;
    mSubRenderer = ZAY_RENDER_PN(p, n, this)
    {
        if(!String::Compare(n, "str_", 4))
        {
            ZAY_RGB(p, 255, 255, 255)
            ZAY_FONT(p, p.h() / 14)
                p.text(GetString(Parser::GetInt(n + 4)), UIFA_CenterMiddle, UIFE_Right);
        }
        else if(!String::Compare(n, "panel_", 6))
            RenderPanel(n + 6, p);
    };
}

FXState::~FXState()
{
	if(gState == this)
		gState = nullptr;
}

FXState* FXState::ST()
{
	return gState;
}

void FXState::SetLanguage(chars language, bool do_next)
{
    String OldLanguage = FXSaver::Read("Language").GetString("null");
    sint32 FindPos = String(language).Find(0, OldLanguage);
    if(FindPos == -1) FindPos = 0;
    else if(do_next)
    {
        while(language[FindPos] != '\0' && language[FindPos++] != '|');
        if(language[FindPos] == '\0') FindPos = 0;
    }
    String CurLanguage;
    while(language[FindPos] != '\0' && language[FindPos] != '|')
        CurLanguage += language[FindPos++];

    if(!!mData->mLanguage.Compare(CurLanguage))
    {
        mData->mLanguage = CurLanguage;
        mData->mAllStrings.Reset();
        FXSaver::Write("Language").Set(CurLanguage);
    }
}

const Context& FXState::GetStage(chars id)
{
    if(auto CurStage = mDoor->stage())
    for(sint32 i = 0, iend = CurStage->LengthOfIndexable(); i < iend; ++i)
    {
        if(!String::Compare(id, (*CurStage)[i]("ID").GetString()))
            return (*CurStage)[i];
    }

    static const Context NullContext;
    return NullContext;
}

ZayPanel::SubRenderCB FXState::GetStageThumbnail(chars id)
{
    if(auto Result = mData->mAllParaViews.Access(id))
        return Result->GetRenderer();

    auto& NewParaView = mData->mAllParaViews(id);
    NewParaView.Init(GetStage(id)("Thumbnail").GetString());
    return NewParaView.GetRenderer();
}

const String& FXState::GetString(sint32 id)
{
    if(auto Result = mData->mAllStrings.Access(id))
        return *Result;

    if(auto CurLanguage = mDoor->language())
    {
        if(mData->mAllStrings.Count() == 0)
        {
            for(sint32 i = 0, iend = CurLanguage->LengthOfIndexable(); i < iend; ++i)
            {
                sint32 CurID = (*CurLanguage)[i]("Index").GetInt(0);
                auto& CurString = (*CurLanguage)[i](mData->mLanguage);
                if(CurString.IsValid())
                {
                    String MultilineText = CurString.GetString("-blank-");
                    MultilineText.Replace("<br>", "\r\n");
                    mData->mAllStrings[CurID] = MultilineText;
                }
                else mData->mAllStrings[CurID] = "-unknown language-";
            }
            if(auto Result = mData->mAllStrings.Access(id))
                return *Result;
        }
        static String Null = "-null-";
        return Null;
    }
    static String Null = "";
    return Null;
}

const SpineRenderer* FXState::GetSpine(chars name) const
{
    BOSS_ASSERT("ResourcePath를 등록하지 않았습니다", 0 < mData->mAllResourcePathes.Count());
    if(auto Result = mData->mAllSpines.Access(name))
        return Result;

    for(sint32 i = 0, iend = mData->mAllResourcePathes.Count(); i < iend; ++i)
    {
        const String SpineJsonPath = String::Format("spine/%s/spine.json", name);
        const String PathJsonPath = String::Format("spine/%s/path.json", name);
        if(Asset::Exist(mData->mAllResourcePathes[i] + SpineJsonPath) != roottype_null)
        {
            mData->mAllSpines(name).Create(mData->mAllResourcePathes[i], SpineJsonPath, PathJsonPath);
            return &mData->mAllSpines(name);
        }
    }
    return nullptr;
}

const FXData::Sound* FXState::GetSound(chars name, bool loop) const
{
    BOSS_ASSERT("ResourcePath를 등록하지 않았습니다", 0 < mData->mAllResourcePathes.Count());
    const String Name = String::Format("%s%s", name, (loop)? "*" : "");
    if(auto Result = mData->mAllSounds.Access(Name))
        return Result;

    for(sint32 i = 0, iend = mData->mAllResourcePathes.Count(); i < iend; ++i)
    {
        String OggPath = mData->mAllResourcePathes[i] + String::Format("sound_ogg/%s.ogg", name);
        auto RootType = Asset::Exist(OggPath);
        if(RootType == roottype_assets)
            OggPath = Platform::File::RootForAssets() + OggPath;
        else if(RootType == roottype_assetsrem)
            OggPath = Platform::File::RootForAssetsRem() + OggPath;
        else continue;
        auto& NewSound = mData->mAllSounds(Name);
        NewSound.mFilename = OggPath;
        NewSound.mLoop = loop;
        return &NewSound;
    }
    return nullptr;
}

static SafeCounter gSoundCounter;
sint32 FXState::GetSoundThreadCount()
{
    return gSoundCounter.Get();
}

void FXState::PlaySound(const FXData::Sound* sound, float volume_rate)
{
    if(sound)
    {
        sound->mPlaying = true;
        payload* Payloads = new payload[2];
        Payloads[0] = *((payload*) &sound);
        Payloads[1] = *((payload*) &volume_rate);
        Platform::Utility::Threading([](void* data)->void
        {
            gSoundCounter.Set(true);
            auto Payloads = (payload*) data;
            auto SoundData = (FXData::Sound*) Payloads[0];
            auto VolumeRate = *((const float*) &Payloads[1]);
            delete[] Payloads;

            if(auto OggFile = Platform::File::OpenForRead(SoundData->mFilename))
            {
                do
                {
                    Platform::File::Seek(OggFile, 0);
                    id_sound SoundID = nullptr;
                    payload Payloads[3] = {&SoundID, &VolumeRate, &SoundData->mPlaying};

                    AddOn::Ogg::ToPcmStream(OggFile,
                        [](sint32 channel, sint32 sample_rate, sint32 sample_size, payload data)->void
                        {
                            id_sound& SoundID = *((id_sound*) ((payload*) data)[0]);
                            float VolumeRate = *((float*) ((payload*) data)[1]);
                            Platform::Sound::Close(SoundID);
                            SoundID = Platform::Sound::OpenForStream(channel, sample_rate, sample_size);
                            Platform::Sound::Play(SoundID, VolumeRate);
                        },
                        [](void* ptr, size_t size, payload data)->sint32
                        {
                            id_sound& SoundID = *((id_sound*) ((payload*) data)[0]);
                            bool& Playing = *((bool*) ((payload*) data)[2]);
                            if(!Playing) return -1;
                            return Platform::Sound::AddStreamForPlay(SoundID, (bytes) ptr, size);
                        }, Payloads);

                    // StopSound호출에 따른 사운드종료
                    if(!SoundData->mPlaying)
                        Platform::Sound::Stop(SoundID);
                    // 사운드제거
                    Platform::Sound::Close(SoundID);
                }
                while(SoundData->mPlaying && SoundData->mLoop);
                Platform::File::Close(OggFile);
            }
            else BOSS_ASSERT(String::Format("해당 파일이 없습니다(%s)", (chars) SoundData->mFilename), false);
            gSoundCounter.Set(false);
        }, Payloads);
    }
}

void FXState::StopSound(const FXData::Sound* sound)
{
    if(sound)
        sound->mPlaying = false;
}

static String gBGSoundName;
void FXState::PlayBGSound(chars name, float volume_rate)
{
    if(!gBGSoundName.CompareNoCase(name))
        return;
    StopBGSound();
    gBGSoundName = name;
    auto NewBGSound = GetSound(gBGSoundName, true);
    PlaySound(NewBGSound, volume_rate);
}

void FXState::StopBGSound()
{
    if(0 < gBGSoundName.Length())
    {
        auto OldBGSound = GetSound(gBGSoundName, true);
        StopSound(OldBGSound);
        gBGSoundName = "";
    }
}

void FXState::StopSoundAll()
{
    for(sint32 i = 0, iend = mData->mAllSounds.Count(); i < iend; ++i)
        mData->mAllSounds.AccessByOrder(i)->mPlaying = false;
}

void FXState::SetPanel(chars name, FXPanel::InitCB icb, FXPanel::RenderCB rcb)
{
    mData->mAllPanels.Remove(name);
    auto& CurPanel = mData->mAllPanels(name);
    CurPanel.mCB = rcb;
    icb(*this, &CurPanel.mData);
}

void FXState::RenderPanel(chars name, ZayPanel& panel)
{
    if(auto CurPanel = mData->mAllPanels.Access(name))
        CurPanel->mCB(panel, &CurPanel->mData);
    else ZAY_RGBA(panel, 255, 0, 0, -128)
        panel.fill();
}
