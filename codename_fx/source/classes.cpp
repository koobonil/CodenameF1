#include <boss.hpp>
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
    Context Pathes(ST_Json, SO_NeedCopy, String::FromFile(projcode + imagepath));
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

MapSpine& MapSpine::InitSpine(const SpineRenderer* renderer, chars skin,
    ZAY::SpineBuilder::MotionFinishedCB fcb, ZAY::SpineBuilder::UserEventCB ecb)
{
    if(mSpineRenderer = renderer)
        mSpineInstance = ZAY::SpineBuilder::Create(mSpineRenderer->spine(), skin, fcb, ecb);
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
    ZAY::SpineBuilder::SetMotionOn(mSpineInstance, first_motion, false);
    ZAY::SpineBuilder::SetMotionOnAttached(mSpineInstance, first_motion, second_motion, repeat);
}

void MapSpine::PlayMotionSeek(chars seek_motion, bool repeat)
{
    if(!mSpineInstance) return;
    ZAY::SpineBuilder::SetMotionOnSeek(mSpineInstance, seek_motion, false);
}

void MapSpine::PlayMotionScript(chars script)
{
    if(!mSpineInstance) return;
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
}

void MapSpine::StopMotion(chars motion)
{
    if(!mSpineInstance) return;
    ZAY::SpineBuilder::SetMotionOff(mSpineInstance, motion);
}

void MapSpine::StopMotionAll()
{
    if(!mSpineInstance) return;
    ZAY::SpineBuilder::SetMotionOffAllWithoutSeek(mSpineInstance, true);
}

void MapSpine::Seek() const
{
    if(mSpineInstance)
    {
        mSeekSecOld = mSeekSec;
        ZAY::SpineBuilder::Seek(mSpineInstance, mSeekSec);
    }
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

void MapSpine::RenderObject(bool needupdate, bool editmode, ZayPanel& panel, bool flip, chars uiname,
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
                Rate, flip, mSpineType == ST_Monster || mSpineType == ST_Dragon);
            Platform::Graphics::EndGL();
        }

        if(rcb)
        ZAY_XYRR(panel, SW / 2, SH / 2, 0, 0)
            renderer()->RenderPanel(mSpineInstance, panel, CX, CY, Rate, flip, rcb);

        if(editmode)
        ZAY_RGBA(panel, 0, 0, 0, 128)
            panel.fill();

        if(editmode || uiname)
        ZAY_XYRR(panel, SW / 2, SH / 2, 0, 0)
            renderer()->RenderBound(mSpineInstance, panel, editmode, CX, CY, Rate, flip, uiname, gcb);

        if(editmode)
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
FXState::FXState(chars defaultpath) : mData(FXData::ST()), mDefaultPath(defaultpath)
{
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
}

const Context& FXState::GetStage(sint32 index) const
{
    if(!mData.mStageTable[0].IsValid())
    {
        ParaSource Source(ParaSource::IIS);
        Source.SetContact("www.finalbossbehindthedoor.com", 80);
        Source.GetJson(mData.mStageTable, "f1/stage_table.txt");
    }
    return mData.mStageTable[index];
}

ZayPanel::SubRenderCB FXState::GetStageThumbnail(sint32 index) const
{
    if(auto Result = mData.mAllParaViews.Access(index))
        return Result->GetRenderer();

    auto& NewParaView = mData.mAllParaViews[index];
    NewParaView.Init(GetStage(index)("ParaView").GetString());
    return NewParaView.GetRenderer();
}

const String& FXState::GetString(sint32 id) const
{
    if(auto Result = mData.mAllStrings.Access(id))
        return *Result;

    if(!mData.mStringTable[0].IsValid())
    {
        ParaSource Source(ParaSource::IIS);
        Source.SetContact("www.finalbossbehindthedoor.com", 80);
        Source.GetJson(mData.mStringTable, "f1/string_table.txt");
    }
    if(mData.mAllStrings.Count() == 0)
    {
        for(sint32 i = 0, iend = mData.mStringTable.LengthOfIndexable(); i < iend; ++i)
        {
            sint32 CurID = mData.mStringTable[i]("Index").GetInt(0);
            mData.mAllStrings[CurID] = mData.mStringTable[i]("kor").GetString("-blank-");
        }
        if(auto Result = mData.mAllStrings.Access(id))
            return *Result;
    }
    static String Null = "-null-";
    return Null;
}

const SpineRenderer* FXState::GetSpine(chars name, chars path) const
{
    if(auto Result = mData.mAllSpines.Access(name))
        return Result;

    const String ResRoot = (path)? String(path) : mDefaultPath;
    const String SpineJsonPath = String::Format("spine/%s/spine.json", name);
    const String PathJsonPath = String::Format("spine/%s/path.json", name);
    if(Asset::Exist(ResRoot + SpineJsonPath) != roottype_null)
    {
        mData.mAllSpines(name).Create(ResRoot, SpineJsonPath, PathJsonPath);
        return &mData.mAllSpines(name);
    }
    return nullptr;
}

void FXState::SetPanel(chars name, FXPanel::InitCB icb, FXPanel::RenderCB rcb)
{
    mData.mAllPanels.Remove(name);
    auto& CurPanel = mData.mAllPanels(name);
    CurPanel.mCB = rcb;
    icb(*this, CurPanel.mData);
}

void FXState::RenderPanel(chars name, ZayPanel& panel)
{
    if(auto CurPanel = mData.mAllPanels.Access(name))
        CurPanel->mCB(panel, CurPanel->mData);
    else ZAY_RGBA(panel, 255, 0, 0, -128)
        panel.fill();
}
