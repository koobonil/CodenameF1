#pragma once

#include <element/boss_image.hpp>
#include <service/boss_zay.hpp>
#include <format/boss_zen.hpp>

namespace ZAY
{
    BOSS_DECLARE_ID(id_spine);
    BOSS_DECLARE_ID(id_spine_instance);

    class SpineBuilder
    {
    public:
        typedef std::function<void(chars)> MotionFinishedCB;
        typedef std::function<void(chars)> UserEventCB;

    public:
        static int& GLScale();
        static void ClearCaches();

    public:
        static id_spine LoadSpine(chars json_filename, chars img_pathname,
            sint32 tone_r = 255, sint32 tone_g = 255, sint32 tone_b = 255, sint32 tone_arg = 0, sint32 tone_method = 0);
        static void FreeSpine(id_spine spine);
        static void SetAssetPath(chars projcode, id_assetpath_read assetpath);
        static void SetOriginalFBO(sint32 id);

    public:
        static id_spine_instance Create(id_spine spine, chars skin, MotionFinishedCB fcb = nullptr, UserEventCB ecb = nullptr);
        static id_spine_instance Clone(id_spine_instance spine_instance);
        static void Release(id_spine_instance spine_instance);
        static void ResetCB(id_spine_instance spine_instance, MotionFinishedCB fcb, UserEventCB ecb);
        static void Seek(id_spine_instance spine_instance, float sec);
        static void Update(id_spine_instance spine_instance, float delta_sec);
        static void SetSkin(id_spine_instance spine_instance, chars skin);
        static bool SetAttachment(id_spine_instance spine_instance, chars slot, chars attachment);
        static void SetMotionOn(id_spine_instance spine_instance, chars motion, bool repeat, float beginsec = 0.0f);
        static void SetMotionOnOnce(id_spine_instance spine_instance, chars motion);
        static void SetMotionOnAttached(id_spine_instance spine_instance, chars target_motion, chars motion, bool repeat);
        static void SetMotionOnOnceAttached(id_spine_instance spine_instance, chars target_motion, chars motion);
        static void SetMotionOnSeek(id_spine_instance spine_instance, chars motion, bool repeat);
        static void SetMotionOff(id_spine_instance spine_instance, chars motion);
        static void SetMotionOffAll(id_spine_instance spine_instance, bool with_reserve);
        static void SetMotionOffAllWithoutSeek(id_spine_instance spine_instance, bool with_reserve);
        static bool IsMotionEnabled(id_spine_instance spine_instance);
        static Strings GetActiveMotions(id_spine_instance spine_instance);
        static void Render(ZayPanel& panel, id_spine_instance spine_instance,
            bool flip, float cx, float cy, float scale, float rendermode, float sx, float sy, float sw, float sh);
        static void RenderPanel(ZayPanel& panel, id_spine_instance spine_instance, float ox, float oy, float scale, bool flip,
            ZayPanel::SubRenderCB cb);
        static void RenderBound(ZayPanel& panel, id_spine_instance spine_instance, float ox, float oy, float scale, bool flip,
            bool guideline, chars uiname = nullptr, ZayPanel::SubGestureCB cb = nullptr);
        static const BOSS::Rect* GetBoundRect(id_spine_instance spine_instance, chars name);
        static const BOSS::Points* GetBoundPolygon(id_spine_instance spine_instance, chars name);
    };
}
