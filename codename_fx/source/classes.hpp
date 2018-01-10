#pragma once
#include <service/boss_zay.hpp>
#include <service/boss_parasource.hpp>
#include "spine_for_zay/zay_spine_builder.hpp"

////////////////////////////////////////////////////////////////////////////////
class SpineAsset
{
    BOSS_DECLARE_NONCOPYABLE_CLASS(SpineAsset)
public:
    SpineAsset()
    {
        mAsset = "";
        mSpine = "";
    }
    ~SpineAsset()
    {
    }
    SpineAsset(SpineAsset&& rhs) {operator=(ToReference(rhs));}
    SpineAsset& operator=(SpineAsset&& rhs)
    {
        mAsset = ToReference(rhs.mAsset);
        mSpine = ToReference(rhs.mSpine);
        return *this;
    }

private:
    String mAsset;
    String mSpine;

public:
    void SetAsset(const String& asset) {mAsset = (!asset.Compare("None"))? "" : asset;}
    void SetSpine(const String& spine) {mSpine = (!spine.Compare("None"))? "" : spine;}
    chars imageName() const {return mAsset;}
    const String spineName() const
    {
        if(0 < mSpine.Length())
        {
            sint32 Pos = mSpine.Find(0, ':');
            if(Pos == -1) return mSpine;
            if(0 < Pos) return mSpine.Left(Pos);
        }
        return mAsset;
    }
    const String spineSkinName() const
    {
        if(0 < mSpine.Length())
        {
            sint32 Pos = mSpine.Find(0, ':');
            if(0 <= Pos)
            if(sint32 Length = mSpine.Length() - 1 - Pos)
                return mSpine.Right(Length);
        }
        return "default";
    }
};

////////////////////////////////////////////////////////////////////////////////
class SpineRenderer
{
public:
    SpineRenderer();
    ~SpineRenderer();

public:
    void Create(String projcode, chars spinepath, chars imagepath);
    void Release();
    void Render(ZAY::id_spine_instance instance, ZayPanel& panel, float sx, float sy, float sw, float sh, sint32 h, float scale, bool flip, bool outline) const;
    void RenderShadow(ZAY::id_spine_instance instance, ZayPanel& panel, float sx, float sy, float sw, float sh, sint32 h, float scale, bool flip) const;
    void RenderPanel(ZAY::id_spine_instance instance, ZayPanel& panel, float ox, float oy, float scale, bool flip, ZayPanel::SubRenderCB cb) const;
    void RenderBound(ZAY::id_spine_instance instance, ZayPanel& panel, bool guideline, float ox, float oy, float scale, bool flip, chars uiname, ZayPanel::SubGestureCB cb) const;

public:
    inline ZAY::id_spine spine() const {return mSpine;}

private:
    ZAY::id_spine mSpine;
};

////////////////////////////////////////////////////////////////////////////////
class MapSpine
{
public:
    enum SpineType {ST_Unknown, ST_Object, ST_Monster, ST_MonsterToast, ST_Dragon, ST_Item};

public:
    MapSpine(SpineType type = ST_Unknown);
    ~MapSpine();
    MapSpine(const MapSpine& rhs);
    MapSpine& operator=(const MapSpine& rhs);
    MapSpine(MapSpine&& rhs);
    MapSpine& operator=(MapSpine&& rhs);

public:
    MapSpine& InitSpine(const SpineRenderer* renderer, chars skin = "default",
        ZAY::SpineBuilder::MotionFinishedCB fcb = nullptr, ZAY::SpineBuilder::UserEventCB ecb = nullptr);
    void SetSkin(chars skin);
    void PlayMotion(chars motion, bool repeat, float beginsec = 0.0f);
    void PlayMotionOnce(chars motion);
    void PlayMotionAttached(chars first_motion, chars second_motion, bool repeat);
    void PlayMotionSeek(chars seek_motion, bool repeat);
    void PlayMotionScript(chars script);
    void StopMotion(chars motion);
    void StopMotionAll();
    void Seek() const;
    void Update() const;
    void RenderObject(bool needupdate, bool editmode, ZayPanel& panel, bool flip, chars uiname = nullptr,
        ZayPanel::SubGestureCB gcb = nullptr, ZayPanel::SubRenderCB rcb = nullptr) const;
    void RenderObjectShadow(ZayPanel& panel, bool flip) const;
    const Rect* GetBoundRect(chars name) const;

public:
    void SetSeekSec(float sec);
    bool IsSeekUpdated() const;
    void Staff_TryIdle();
    void Staff_Start();

public:
    inline const SpineRenderer* renderer() const {return mSpineRenderer;}
    inline const bool enabled() const
    {return (mSpineInstance && ZAY::SpineBuilder::IsMotionEnabled(mSpineInstance));}

public:
    const SpineType mSpineType;
    const SpineRenderer* mSpineRenderer;
    ZAY::id_spine_instance mSpineInstance;
    mutable uint64 mSpineMsecOld;
    float mSeekSec;
    mutable float mSeekSecOld;
    bool mStaffIdleMode;
    bool mStaffStartMode;
};

////////////////////////////////////////////////////////////////////////////////
class FXState;
class FXPanel
{
public:
    class Data
    {
    public:
        Map<MapSpine> mSpines;
    };

public:
    typedef void (*InitCB)(const FXState& state, FXPanel::Data& data);
    typedef void (*RenderCB)(ZayPanel& panel, const FXPanel::Data& data);

public:
    FXPanel() {mCB = nullptr;}
    ~FXPanel() {}

public:
    RenderCB mCB;
    Data mData;
};

////////////////////////////////////////////////////////////////////////////////
class FXData
{
    BOSS_DECLARE_NONCOPYABLE_CLASS(FXData)
public:
    FXData() {}
    ~FXData() {}

public:
    static FXData& ST() {static FXData _; return _;}

public:
    Context mStageTable;
    Context mStringTable;
    Map<ParaSource::View> mAllParaViews;
    Map<String> mAllStrings;
    Map<SpineRenderer> mAllSpines;
    Map<FXPanel> mAllPanels;
};

////////////////////////////////////////////////////////////////////////////////
class FXState
{
    BOSS_DECLARE_NONCOPYABLE_INITIALIZED_CLASS(FXState, mData(FXData::ST()))
public:
    FXState(chars defaultpath);
    ~FXState();

public:
    const Context& GetStage(sint32 index) const;
    ZayPanel::SubRenderCB GetStageThumbnail(sint32 index) const;
    const String& GetString(sint32 id) const;
    const SpineRenderer* GetSpine(chars name, chars path = nullptr) const;
    void SetPanel(chars name, FXPanel::InitCB icb, FXPanel::RenderCB rcb);
    void RenderPanel(chars name, ZayPanel& panel);

private:
    FXData& mData;
    const String mDefaultPath;

public:
    ZayPanel::SubRenderCB mSubRenderer;
};
