﻿#pragma once

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
        static void ClearCaches();

    public:
        static id_spine LoadSpine(chars json_filename, chars img_pathname,
            sint32 tone_r = 255, sint32 tone_g = 255, sint32 tone_b = 255, sint32 tone_arg = 0, sint32 tone_method = 0);
        static void FreeSpine(id_spine spine);
        static void SetAssetPath(id_assetpath_read assetpath);
        static void SetOriginalFBO(sint32 id);

    public:
        static id_spine_instance Create(id_spine spine, chars skin, MotionFinishedCB fcb = nullptr, UserEventCB ecb = nullptr);
        static void Release(id_spine_instance spine_instance);
        static void Update(id_spine_instance spine_instance, float delta_sec);
        static void SetSkin(id_spine_instance spine_instance, chars skin);
        static bool SetAttachment(id_spine_instance spine_instance, chars slot, chars attachment);
        static void SetMotionOn(id_spine_instance spine_instance, chars motion, bool repeat);
        static void SetMotionOnOnce(id_spine_instance spine_instance, chars motion);
        static void SetMotionOnAttached(id_spine_instance spine_instance, chars target_motion, chars motion, bool once);
        static void SetMotionOff(id_spine_instance spine_instance, chars motion);
        static void SetMotionOffAll(id_spine_instance spine_instance);
        static Strings GetActiveMotions(id_spine_instance spine_instance);
        static void Render(ZayPanel& panel, id_spine_instance spine_instance, float cx, float cy, bool flip, float scale, sint32 sx, sint32 sy, sint32 sw, sint32 sh);
        static void RenderBound(ZayPanel& panel, id_spine_instance spine_instance, float cx, float cy, bool guideline, ZayPanel::SubGestureCB cb = nullptr);
    };
}
