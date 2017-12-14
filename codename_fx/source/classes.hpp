#pragma once
#include <service/boss_zay.hpp>
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
    void Render(ZAY::id_spine_instance instance, ZayPanel& panel, sint32 sx, sint32 sy, sint32 sw, sint32 sh, sint32 h, float scale, bool flip, bool outline) const;
    void RenderShadow(ZAY::id_spine_instance instance, ZayPanel& panel, sint32 sx, sint32 sy, sint32 sw, sint32 sh, sint32 h, float scale, bool flip) const;
    void RenderBound(ZAY::id_spine_instance instance, ZayPanel& panel, bool guideline, float ox, float oy, float scale, bool flip,
        chars uiname, ZayPanel::SubGestureCB cb) const;

public:
    inline ZAY::id_spine spine() const {return mSpine;}

private:
    ZAY::id_spine mSpine;
};
typedef Map<SpineRenderer> SpineRendererMap;

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
    void PlayMotion(chars motion, bool repeat);
    void PlayMotionOnce(chars motion);
    void PlayMotionAttached(chars first_motion, chars second_motion, bool repeat);
    void PlayMotionSeek(chars seek_motion, bool repeat);
    void StopMotionAll();
    void Seek() const;
    void Update() const;
    void RenderObject(bool needupdate, bool editmode, ZayPanel& panel, bool flip, chars uiname = nullptr, ZayPanel::SubGestureCB cb = nullptr) const;
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
