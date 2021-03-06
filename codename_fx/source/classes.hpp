﻿#pragma once
#include <service/boss_zay.hpp>
#include <service/boss_parasource.hpp>
#include <service/boss_firebaseservice.hpp>
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
    void SetAsset(const String& asset) {mAsset = asset;}
    void SetSpine(const String& spine) {mSpine = spine;}
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
            {
                sint32 Pos2 = mSpine.Find(0, '/');
                if(Pos < Pos2)
                    return String(((chars) mSpine) + Pos + 1, Pos2 - Pos - 1);
                else if(sint32 Length = mSpine.Length() - 1 - Pos)
                    return mSpine.Right(Length);
            }
        }
        return "default";
    }
    const String spineAnimationName() const
    {
        if(0 < mSpine.Length())
        {
            sint32 Pos = mSpine.Find(0, '/');
            if(0 <= Pos)
            {
                if(sint32 Length = mSpine.Length() - 1 - Pos)
                    return mSpine.Right(Length);
            }
        }
        return "idle";
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
enum class DebugMode {None, Weak, Strong};

////////////////////////////////////////////////////////////////////////////////
class FXState;
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
    MapSpine& InitSpine(const FXState* state, const SpineRenderer* renderer, chars skin = "default",
        ZAY::SpineBuilder::MotionFinishedCB fcb = nullptr, ZAY::SpineBuilder::UserEventCB ecb = nullptr);
    void SetSkin(chars skin);
    void PlayMotion(chars motion, bool repeat, float beginsec = 0.0f);
    void PlayMotionOnce(chars motion);
    void PlayMotionAttached(chars first_motion, chars second_motion, bool repeat);
    void PlayMotionSeek(chars seek_motion, bool repeat);
    String PlayMotionScript(chars script);
    void StopMotion(chars motion);
    void StopMotionAll();
    void Seek() const;
    void Update() const;
    void RenderObject(DebugMode debug, bool needupdate, ZayPanel& panel, bool flip, chars uiname = nullptr,
        ZayPanel::SubGestureCB gcb = nullptr, ZayPanel::SubRenderCB rcb = nullptr) const;
    void RenderObjectShadow(ZayPanel& panel, bool flip) const;
    const Rect* GetBoundRect(chars name) const;
    const Points* GetBoundPolygon(chars name) const;

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
    mutable Points mGroundPolygon;
    mutable Rect mGroundRect;
    bool mStaffIdleMode;
    bool mStaffStartMode;
};

////////////////////////////////////////////////////////////////////////////////
class FXPanel
{
    BOSS_DECLARE_NONCOPYABLE_CLASS(FXPanel)
public:
    FXPanel() {mCB = nullptr;}
    ~FXPanel() {}

public:
    class Data
    {
    public:
        Map<MapSpine> mSpines;
    };

public:
    typedef void (*InitCB)(const FXState& state, FXPanel::Data* data);
    typedef void (*RenderCB)(ZayPanel& panel, const FXPanel::Data* data);

public:
    RenderCB mCB;
    Data mData;
};

////////////////////////////////////////////////////////////////////////////////
class FXDoor
{
    BOSS_DECLARE_NONCOPYABLE_CLASS(FXDoor)
public:
    FXDoor();
    ~FXDoor();

private:
    String GetAuthCode();

public:
    bool IsLocked() const;
    bool Load();
    String GetGlobalWeight(chars id) const;
    void Render(ZayPanel& panel);
    static void RenderVersion(ZayPanel& panel);

public:
    static FXDoor& ST() {return *BOSS_STORAGE(FXDoor);}
    inline const Context* stage() {return mStagePack.GetContext();}
    inline const Context* language() {return mLanguagePack.GetContext();}
    inline chars authcode() {return mAuthCode;}

private:
    const String mAuthCode;
    bool mLoaded;
    String mService;
    String mComment;
    ParaJson mStagePack;
    ParaJson mLanguagePack;
    String mAccountCenter;
    String mAccountManager;
    Context mGlobalWeight;
    bool mIsParaAuth;
    bool mParaAuthSuccess;
    String mParaAuthName;
};

////////////////////////////////////////////////////////////////////////////////
class FXState;
class FXData
{
    BOSS_DECLARE_NONCOPYABLE_CLASS(FXData)
private:
    FXData() {}
    ~FXData() {}

public:
    static FXData& ST() {static FXData _; return _;}
    static void SetResourcePathes(const Strings& pathes)
    {
        auto& Data = ST();
        Data.mAllResourcePathes = pathes;
    }
    static void ClearAll()
    {
        auto& Data = ST();
        Data.mAllParaViews.Reset();
        Data.mAllStrings.Reset();
        Data.mAllSpines.Reset();
        Data.mAllSounds.Reset();
        Data.mAllPanels.Reset();
    }

public:
    class Sound
    {
    public:
        Sound()
        {
            mLoop = false;
            mPlaying = false;
        }
        ~Sound() {}
    public:
        String mFilename;
        bool mLoop;
        mutable bool mPlaying;
    };

public:
    String mLanguage;
    Strings mAllResourcePathes;
    Map<ParaView> mAllParaViews;
    Map<String> mAllStrings;
    Map<SpineRenderer> mAllSpines;
    Map<Sound> mAllSounds;
    Map<FXPanel> mAllPanels;
    FirebaseService mAdService;
};

////////////////////////////////////////////////////////////////////////////////
class FXSaver
{
    BOSS_DECLARE_NONCOPYABLE_CLASS(FXSaver)
public:
    FXSaver() {mUpdated = false;}
    ~FXSaver() {}

public:
    static Context* Sync(chars assetname)
    {
        FXSaver& Saver = ST();
        Saver.mAssetName = assetname;
        String SaveString = String::FromAsset(assetname);
        if(0 < SaveString.Length())
        {
            Saver.mContext.LoadJson(SO_NeedCopy, SaveString, SaveString.Length());
            return nullptr;
        }
        return &Saver.mContext;
    }
    static const Context& Read(chars name)
    {
        FXSaver& Saver = ST();
        return Saver.mContext(name);
    }
    static Context& Write(chars name)
    {
        FXSaver& Saver = ST();
        Saver.mUpdated = true;
        return Saver.mContext.At(name);
    }
    static void Update()
    {
        FXSaver& Saver = ST();
        if(Saver.mUpdated)
        {
            Saver.mUpdated = false;
            Saver.mContext.SaveJson().ToAsset(Saver.mAssetName);
        }
    }

private:
    static FXSaver& ST() {return *BOSS_STORAGE(FXSaver);}

private:
    Context mContext;
    String mAssetName;
    bool mUpdated;
};

////////////////////////////////////////////////////////////////////////////////
class FXState
{
    BOSS_DECLARE_NONCOPYABLE_INITIALIZED_CLASS(FXState, mDoor(&FXDoor::ST()), mData(&FXData::ST()))
public:
    FXState();
    ~FXState();

public:
	static FXState* ST();
    void SetLanguage(chars language, bool do_next);
    const Context& GetStage(chars id);
    ZayPanel::SubRenderCB GetStageThumbnail(chars id);
    const String& GetString(sint32 id);
    const SpineRenderer* GetSpine(chars name) const;
    const FXData::Sound* GetSound(chars name, bool loop = false) const;
    static sint32 GetSoundThreadCount();
    static void PlaySound(const FXData::Sound* sound, float volume_rate = 1.0f);
    static void StopSound(const FXData::Sound* sound);
    void PlayBGSound(chars name, float volume_rate = 1.0f);
    void StopBGSound();
    void StopSoundAll();
    void SetPanel(chars name, FXPanel::InitCB icb, FXPanel::RenderCB rcb);
    void RenderPanel(chars name, ZayPanel& panel);

public:
    virtual void OnEvent(chars name, const Rect& rect = Rect(), const MapSpine* spine = nullptr) {}

public:
    inline FXDoor& door() {return *mDoor;}
    inline FirebaseService& ad() {return mData->mAdService;}

private:
    FXDoor* mDoor;
    FXData* mData;

public:
    ZayPanel::SubRenderCB mSubRenderer;
};
