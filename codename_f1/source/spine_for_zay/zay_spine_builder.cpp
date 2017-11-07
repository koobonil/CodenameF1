#include <boss.hpp>
#include "zay_spine_builder.hpp"
#include "zay_skeleton_data.h"
#include "zay_skeleton_instance.h"
#include "zay_skeleton_bone.h"
#include "zay_mesh_data.h"
#include "zay_touch_area_data.h"
#include "zay_animation_track.h"
#include "zay_forward_multiply_renderer.h"
#include "zay_scene_node.h"
#include "zay_image_data.h"
#include "zay_base.h"
#include "zay_image.h"

namespace ZAY
{
    #define _CACHE_IS_NON 0
    #define _CACHE_IS_ZEN 1
    #define _CACHE_IS_BIN 2
    int USE_SPINE_CACHE = _CACHE_IS_ZEN;

    template<typename TYPE>
    id_spine LoadSpineCore(const TYPE& prop, chars img_pathname);

    void SpineBuilder::ClearCaches()
    {
        if(auto Pool = ZAY::AutoreleasePool::getCurrentPool())
            Pool->clear();
        ZAY::Image::clearCachedTextures();
    }

    id_spine SpineBuilder::LoadSpine(chars json_filename, chars img_pathname, sint32 tone_r, sint32 tone_g, sint32 tone_b, sint32 tone_arg, sint32 tone_method)
    {
        Context NewContext;
        String NewJson;
        Zen NewZen;

        Profile::Start("LoadSpine");
        bool IsCacheLoaded = false;
        String CacheFilename = json_filename;
        if(USE_SPINE_CACHE == _CACHE_IS_ZEN)
            CacheFilename.Sub(5) += ".zen";
        else if(USE_SPINE_CACHE == _CACHE_IS_BIN)
            CacheFilename.Sub(5) += ".bin";

        if(USE_SPINE_CACHE != _CACHE_IS_NON)
        {
            Profile::Lap("CheckCache");
            if(Asset::ValidCache(json_filename, CacheFilename))
            if(id_asset_read CacheAsset = Asset::OpenForRead(CacheFilename))
            {
                const sint32 CacheSize = Asset::Size(CacheAsset);

                if(USE_SPINE_CACHE == _CACHE_IS_ZEN)
                {
                    Profile::Lap("LoadingCache:Zen");
                    sint32s ZenSource;
                    Asset::Read(CacheAsset, (uint08*) ZenSource.AtDumping(0, CacheSize / sizeof(sint32)), CacheSize);
                    Asset::Close(CacheAsset);
                    NewZen = Zen::Load(ZenSource, &IsCacheLoaded);
                }
                else if(USE_SPINE_CACHE == _CACHE_IS_BIN)
                {
                    Profile::Lap("LoadingCache:Bin");
                    uint08* CacheBytes = new uint08[CacheSize];
                    Asset::Read(CacheAsset, CacheBytes, CacheSize);
                    Asset::Close(CacheAsset);
                    IsCacheLoaded = NewContext.LoadBin(CacheBytes);
                    delete[] CacheBytes;
                }
            }
        }

        if(!IsCacheLoaded)
        {
            Profile::Lap("LoadingJson");
            NewJson = String::FromFile(json_filename, (id_assetpath) ZAY::MeshData::s_assetpath);
            NewContext.Clear();
            NewContext.LoadJson(SO_OnlyReference, NewJson);

            if(USE_SPINE_CACHE == _CACHE_IS_ZEN)
            {
                Profile::Lap("BuildCache:Zen");
                const sint32s ZenSource = Zen::Build(NewContext);
                NewZen = Zen::Load(ZenSource);

                Profile::Lap("SavingCache:Zen");
                id_asset CacheAsset = Asset::OpenForWrite(CacheFilename, true);
                Asset::Write(CacheAsset, (bytes) &ZenSource[0], sizeof(sint32) * ZenSource.Count());
                Asset::Close(CacheAsset);
            }
            else if(USE_SPINE_CACHE == _CACHE_IS_BIN)
            {
                Profile::Lap("SavingCache:Bin");
                uint08s BinBytes = NewContext.SaveBin();
                id_asset CacheAsset = Asset::OpenForWrite(CacheFilename, true);
                Asset::Write(CacheAsset, &BinBytes[0], BinBytes.Count());
                Asset::Close(CacheAsset);
            }
        }

        Profile::Stop();
        Profile::DebugPrint();
        Profile::Clear();

        ZAY::ImageData::s_tone_R = tone_r;
        ZAY::ImageData::s_tone_G = tone_g;
        ZAY::ImageData::s_tone_B = tone_b;
        ZAY::ImageData::s_tone_Arg = tone_arg;
        ZAY::ImageData::s_tone_Method = tone_method;

        if(USE_SPINE_CACHE == _CACHE_IS_ZEN)
            return LoadSpineCore<Zen>(NewZen, img_pathname);
        else return LoadSpineCore<Context>(NewContext, img_pathname);
    }

    void SpineBuilder::FreeSpine(id_spine spine)
    {
        if(spine)
            ((ZAY::SkeletonData*) spine)->release();
    }

    void SpineBuilder::SetAssetPath(id_assetpath_read assetpath)
    {
        ZAY::MeshData::s_assetpath = assetpath;
    }

    static sint32 _OriginalFBO = 0;
    void SpineBuilder::SetOriginalFBO(sint32 id)
    {
        _OriginalFBO = id;
    }

    class SpineInstance
    {
    public:
        SpineInstance(ZAY::SkeletonData* skel, SpineBuilder::MotionFinishedCB fcb, SpineBuilder::UserEventCB ecb)
        {
            Node = new ZAY::SceneNode();
            Node->retain();
            Renderer = new ZAY::ForwardMultiplyRender::ForwardMultiplyRenderer();
            Renderer->retain();
            Instance = new ZAY::SkeletonInstance();
            Instance->retain();
            StateSet = new ZAY::AnimationStateSet();

            Node->setRenderer(Renderer);
            Instance->setSkeletonData(skel);
            Instance->setBoneRenderAvailable(false);
            Instance->setSlotSkin("default");
            Node->attachObject(Instance);

            skel->createAnimationStates(*StateSet);
            for(auto it : StateSet->getAnimationStates())
            {
                it.second->setEnabled(false);
                it.second->setLoopAndSeek(0, false);
            }

            FinishedCB = fcb;
            EventCB = ecb;

            BoxFocus = nullptr;
        }

        ~SpineInstance()
        {
            Node->release();
            Renderer->release();
            Instance->release();
            delete StateSet;
        }

        static SpineInstance*& Current() {static SpineInstance* _ = nullptr; return _;}

        static SpineInstance* Clone(const SpineInstance* old)
        {return new SpineInstance(old->Instance->getSkeletonData(), old->FinishedCB, old->EventCB);}

        void ResetCB(SpineBuilder::MotionFinishedCB fcb, SpineBuilder::UserEventCB ecb)
        {
            FinishedCB = fcb;
            EventCB = ecb;
        }

        void SetBoundBox(chars name, int r, int g, int b, int a)
        {
            BoundBox& NewBoundBox = BoxMap(name);
            NewBoundBox.Name = name;
            NewBoundBox.Clr = Color(r, g, b, a);
            BoxFocus = &NewBoundBox;
        }

        void MergeBoundBox(float x, float y)
        {
            BOSS_ASSERT("BoxFocus정보가 없습니다", BoxFocus);
            BoundBox& CurBoundBox = *BoxFocus;
            const Point& NewPos = Point(x, -y);
            CurBoundBox.Pos.AtAdding() = NewPos;

            if(CurBoundBox.Pos.Count() == 1)
            {
                CurBoundBox.Box.l = NewPos.x;
                CurBoundBox.Box.t = NewPos.y;
                CurBoundBox.Box.r = NewPos.x;
                CurBoundBox.Box.b = NewPos.y;
            }
            else
            {
                CurBoundBox.Box.l = BOSS::Math::MinF(CurBoundBox.Box.l, NewPos.x);
                CurBoundBox.Box.t = BOSS::Math::MinF(CurBoundBox.Box.t, NewPos.y);
                CurBoundBox.Box.r = BOSS::Math::MaxF(CurBoundBox.Box.r, NewPos.x);
                CurBoundBox.Box.b = BOSS::Math::MaxF(CurBoundBox.Box.b, NewPos.y);
            }
        }

        void CallEvent(chars name)
        {
            if(EventCB) EventCB(name);
        }

    public:
        void InitBoundBox()
        {
            SpineInstance::Current() = this;
            Node->_updateWorldTransformDescending();
            Node->_updateWorldColorDescending();
            Node->_preRender();

            StateSet->updateAnimation(0, nullptr, nullptr);
            Instance->applyInitialAnimationPose(true, false);
            Instance->applyAnimationStateSet(StateSet, true, false);
            StateSet->clearNeedToSetInitState();
            SpineInstance::Current() = nullptr;
        }

        void SetSkin(chars skin)
        {
            LastSkin = skin;
            Instance->setSlotSkin(skin);
        }

        chars GetSkin() const
        {return LastSkin;}

        bool SetAttachment(chars slot, chars attachment)
        {
            ZAY::SceneSlotInstance* CurSlot = Instance->getSlot(slot);
            if(CurSlot)
            {
                CurSlot->setDefaultAttachmentName(attachment);
                return true;
            }
            return false;
        }

        void SetMotionOn(chars name, float beginpos, int loop)
        {
            if(auto motion = StateSet->getAnimationState(name))
            {
                motion->setEnabled(true);
                motion->setLoopAndSeek(loop, false);
                motion->setCurrentTime(beginpos);
            }
        }

        void SetMotionOnSeek(chars name, float beginpos, int loop)
        {
            if(auto motion = StateSet->getAnimationState(name))
            {
                motion->setEnabled(true);
                motion->setLoopAndSeek(loop, true);
                motion->setCurrentTime(beginpos);
            }
        }

        void ReserveMotionOn(chars target, chars name, float beginpos, int loop)
        {
            ReservedMotionMap(target).Enqueue(new ReservedMotion(name, beginpos, loop));
        }

        void SetMotionOff(chars name)
        {
            if(auto motion = StateSet->getAnimationState(name))
            {
                motion->setEnabled(false);
                motion->setLoopAndSeek(0, false);
            }
        }

        void SetMotionOffAll(bool with_reserve)
        {
            for(auto it : StateSet->getAnimationStates())
            {
                it.second->setEnabled(false);
                it.second->setLoopAndSeek(0, false);
            }
            if(with_reserve)
                ReservedMotionMap.Reset();
        }

        void SetMotionOffAllWithoutSeek(bool with_reserve)
        {
            for(auto it : StateSet->getAnimationStates())
            {
                if(!it.second->getSeek())
                {
                    it.second->setEnabled(false);
                    it.second->setLoopAndSeek(0, false);
                }
            }
            if(with_reserve)
                ReservedMotionMap.Reset();
        }

        bool IsMotionEnabled()
        {
            for(auto it : StateSet->getAnimationStates())
            {
                if(it.second->getEnabled())
                    return true;
            }
            return false;
        }

        Strings GetActiveMotions()
        {
            Strings Result;
            for(auto it : StateSet->getAnimationStates())
            {
                if(it.second->getEnabled())
                    Result.AtAdding() = it.first.c_str();
            }
            return Result;
        }

        void Seek(float sec)
        {
            StateSet->seekAnimation(sec);
        }

        void UpdateMotion(float deltaSec)
        {
            SpineInstance::Current() = this;
            BoxMap.Reset();

            StateSet->updateAnimation(deltaSec,
                [](void* payload, const char* motion, int loop) -> void
                {
                    SpineInstance* This = (SpineInstance*) payload;

                    // 1회성 모션처리
                    bool IsMotionAlived = true;
                    if(loop == -1)
                    {
                        IsMotionAlived = false;
                        This->SetMotionOff(motion);
                    }

                    // 예약모션 수행
                    if(ReservedMotionQueue* CurQueue = This->ReservedMotionMap.Access(motion))
                    if(sint32 CurCount = CurQueue->Count())
                    {
                        if(IsMotionAlived)
                        {
                            IsMotionAlived = false;
                            This->SetMotionOff(motion);
                        }

                        ReservedMotion* CurMotion = CurQueue->Dequeue();
                        This->SetMotionOn(CurMotion->Name, CurMotion->BeginPos, CurMotion->Loop);
                        delete CurMotion;

                        if(CurCount == 1)
                            This->ReservedMotionMap.Remove(motion);
                    }

                    // 현재 모션이 살아있고, 반복모드가 아니며, CB를 가진 경우, 콜백처리
                    if(IsMotionAlived && loop == 0 && This->FinishedCB)
                        This->FinishedCB(motion);
                }, this);
            Instance->applyInitialAnimationPose(true, false);
            Instance->applyAnimationStateSet(StateSet, true, false);
            StateSet->clearNeedToSetInitState();

            SpineInstance::Current() = nullptr;
        }

        void Render(uint32 fbo, sint32 sx, sint32 sy, sint32 sw, sint32 sh, float cx, float cy, bool flip, float scale, float rendermode)
        {
            // Scissor
            const bool enable_scissor = false;
            GLint OldScissor[4] = {0, 0, 0, 0};
            if(enable_scissor)
            {
                glEnable(GL_SCISSOR_TEST);
                glGetIntegerv(GL_SCISSOR_BOX, OldScissor);
                glScissor(sx, sy, sw, sh);
            }

            // Viewport
            GLint OldViewport[4] = {0, 0, 0, 0};
            glGetIntegerv(GL_VIEWPORT, OldViewport);
            const sint32 basesize = 4000;
            const float scaledsize = basesize * scale;
            Renderer->setViewport(sx + sw * cx - scaledsize, sy + sh * cy - scaledsize,
                scaledsize * 2, scaledsize * 2);

            // MVPMatrix
            const float l = (flip)? +basesize : -basesize;
            const float r = (flip)? -basesize : +basesize;
            const float b = -basesize;
            const float t = +basesize;
            const float zNear = -basesize;
            const float zFar = +basesize;
            ZAY::Matrix4 projectionMatrix(
                2.0f / (r - l), 0.0f, 0.0f, 0.0f,
                0.0f, 2.0f / (t - b), 0.0f, 0.0f,
                0.0f, 0.0f, 2.0f / (zNear - zFar), 0.0f,
                (l + r) / (l - r), (t + b) / (b - t), (zNear + zFar) / (zNear - zFar), 1.0f);
            Renderer->setMVPMatrix(projectionMatrix);

            // RenderMode
            Renderer->setRenderMode(rendermode);

            // Render Pipeline
            Node->_updateWorldTransformDescending();
            Node->_updateWorldColorDescending();
            Node->_preRender();
            Renderer->setOriginalFBO(fbo);
            Renderer->checkAndRemakeShaders();
            Renderer->update();
            Renderer->render();
            Node->_postRender();

            // Viewport And Scissor
            Renderer->setViewport(OldViewport[0], OldViewport[1], OldViewport[2], OldViewport[3]);
            if(enable_scissor)
            {
                glScissor(OldScissor[0], OldScissor[1], OldScissor[2], OldScissor[3]);
                glDisable(GL_SCISSOR_TEST);
            }
        }

        void RenderBound(ZayPanel& panel, float ox, float oy, float scale, bool flip, bool guideline, chars uiname = nullptr, ZayPanel::SubGestureCB cb = nullptr)
        {
            const float fv = (flip)? -1 : 1;
            for(sint32 i = 0, iend = BoxMap.Count(); i < iend; ++i)
            {
                const BoundBox& CurBox = *BoxMap.AccessByOrder(i);
                ZAY_COLOR_IF(panel, CurBox.Clr, guideline)
                {
                    // UI영역
                    BOSS::Rect NewRect;
                    if(flip)
                    {
                        NewRect.l = panel.w() / 2 + (-CurBox.Box.r - ox) * scale;
                        NewRect.t = panel.h() / 2 + (CurBox.Box.t - oy) * scale;
                        NewRect.r = panel.w() / 2 + (-CurBox.Box.l - ox) * scale;
                        NewRect.b = panel.h() / 2 + (CurBox.Box.b - oy) * scale;
                    }
                    else
                    {
                        NewRect.l = panel.w() / 2 + (CurBox.Box.l - ox) * scale;
                        NewRect.t = panel.h() / 2 + (CurBox.Box.t - oy) * scale;
                        NewRect.r = panel.w() / 2 + (CurBox.Box.r - ox) * scale;
                        NewRect.b = panel.h() / 2 + (CurBox.Box.b - oy) * scale;
                    }
                    if(uiname)
                    {
                        ZAY_RECT_UI(panel, NewRect, uiname + CurBox.Name, cb)
                        {
                            if(guideline)
                            {
                                ZAY_RGBA(panel, 128, 128, 128, 64)
                                    panel.fill();
                                ZAY_FONT(panel, 0.8f, "Arial")
                                ZAY_RGBA(panel, 0, 255, 255, 255)
                                    panel.text(CurBox.Name, UIFA_LeftTop, UIFE_Right);
                            }
                        }
                    }
                    else if(guideline)
                    {
                        ZAY_RECT(panel, NewRect)
                        ZAY_RGBA(panel, 128, 128, 128, 64)
                            panel.fill();
                    }

                    // 외곽폴리곤
                    if(guideline)
                    {
                        Points NewPos;
                        Point* PtrPos = NewPos.AtDumpingAdded(CurBox.Pos.Count());
                        for(sint32 i = 0, iend = CurBox.Pos.Count(); i < iend; ++i)
                        {
                            PtrPos[i].x = panel.w() / 2 + (((flip)? -CurBox.Pos[i].x : CurBox.Pos[i].x) - ox) * scale;
                            PtrPos[i].y = panel.h() / 2 + (CurBox.Pos[i].y - oy) * scale;
                        }
                        panel.polyline(NewPos, 1);
                    }
                }
            }
        }

        const BOSS::Rect* GetBoundRect(chars name)
        {
            for(sint32 i = 0, iend = BoxMap.Count(); i < iend; ++i)
            {
                const BoundBox& CurBox = *BoxMap.AccessByOrder(i);
                if(!CurBox.Name.Compare(name))
                    return &CurBox.Box;
            }
            return nullptr;
        }

    private:
        ZAY::SceneNode* Node;
        ZAY::ForwardMultiplyRender::ForwardMultiplyRenderer* Renderer;
        ZAY::SkeletonInstance* Instance;
        ZAY::AnimationStateSet* StateSet;
        String LastSkin;
        SpineBuilder::MotionFinishedCB FinishedCB;
        SpineBuilder::UserEventCB EventCB;

        class ReservedMotion
        {
        public:
            ReservedMotion() {BeginPos = 0; Loop = 0;}
            ReservedMotion(chars name, float beginpos, int loop)
            {
                Name = name;
                BeginPos = beginpos;
                Loop = loop;
            }
            ~ReservedMotion() {}

        public:
            String Name;
            float BeginPos;
            int Loop;
        };
        public:
        static void FreeReservedMotion(void* ptr) {delete (ReservedMotion*) ptr;}
        typedef Queue<ReservedMotion*, false, FreeReservedMotion> ReservedMotionQueue;
        Map<ReservedMotionQueue> ReservedMotionMap;

    private:
        class BoundBox
        {
        public:
            BoundBox() {}
            ~BoundBox() {}
        public:
            String Name;
            Color Clr;
            Points Pos;
            Rect Box;
        };
        Map<BoundBox> BoxMap;
        BoundBox* BoxFocus;
    };

    void SpineBuilder_SetBoundBox(const char* name, int r, int g, int b, int a)
    {
        SpineInstance* This = SpineInstance::Current();
        if(This) This->SetBoundBox(name, r, g, b, a);
    }

    void SpineBuilder_MergeBoundBox(float x, float y)
    {
        SpineInstance* This = SpineInstance::Current();
        if(This) This->MergeBoundBox(x, y);
    }

    void SpineBuilder_CallEvent(const char* name)
    {
        SpineInstance* This = SpineInstance::Current();
        if(This) This->CallEvent(name);
    }

    id_spine_instance SpineBuilder::Create(id_spine spine, chars skin, MotionFinishedCB fcb, UserEventCB ecb)
    {
        SpineInstance* NewInstance = new SpineInstance((ZAY::SkeletonData*) spine, fcb, ecb);
        NewInstance->InitBoundBox();
        NewInstance->SetSkin(skin);
        return (id_spine_instance) NewInstance;
    }

    id_spine_instance SpineBuilder::Clone(id_spine_instance spine_instance)
    {
        SpineInstance* OldInstance = (SpineInstance*) spine_instance;
        SpineInstance* NewInstance = SpineInstance::Clone(OldInstance);
        NewInstance->InitBoundBox();
        NewInstance->SetSkin(OldInstance->GetSkin());
        return (id_spine_instance) NewInstance;
    }

    void SpineBuilder::Release(id_spine_instance spine_instance)
    {
        delete (SpineInstance*) spine_instance;
    }

    void SpineBuilder::ResetCB(id_spine_instance spine_instance, MotionFinishedCB fcb, UserEventCB ecb)
    {
        SpineInstance* CurInstance = (SpineInstance*) spine_instance;
        CurInstance->ResetCB(fcb, ecb);
    }

    void SpineBuilder::Seek(id_spine_instance spine_instance, float sec)
    {
        SpineInstance* CurInstance = (SpineInstance*) spine_instance;
        CurInstance->Seek(sec);
    }

    void SpineBuilder::Update(id_spine_instance spine_instance, float delta_sec)
    {
        SpineInstance* CurInstance = (SpineInstance*) spine_instance;
        CurInstance->UpdateMotion(delta_sec);
    }

    void SpineBuilder::SetSkin(id_spine_instance spine_instance, chars skin)
    {
        SpineInstance* CurInstance = (SpineInstance*) spine_instance;
        CurInstance->SetSkin(skin);
    }

    bool SpineBuilder::SetAttachment(id_spine_instance spine_instance, chars slot, chars attachment)
    {
        SpineInstance* CurInstance = (SpineInstance*) spine_instance;
        return CurInstance->SetAttachment(slot, attachment);
    }

    void SpineBuilder::SetMotionOn(id_spine_instance spine_instance, chars motion, bool repeat)
    {
        SpineInstance* CurInstance = (SpineInstance*) spine_instance;
        CurInstance->SetMotionOn(motion, 0.0f, repeat? 1 : 0);
    }

    void SpineBuilder::SetMotionOnOnce(id_spine_instance spine_instance, chars motion)
    {
        SpineInstance* CurInstance = (SpineInstance*) spine_instance;
        CurInstance->SetMotionOn(motion, 0.0f, -1);
    }

    void SpineBuilder::SetMotionOnAttached(id_spine_instance spine_instance, chars target_motion, chars motion, bool repeat)
    {
        SpineInstance* CurInstance = (SpineInstance*) spine_instance;
        CurInstance->ReserveMotionOn(target_motion, motion, 0.0f, repeat? 1 : 0);
    }

    void SpineBuilder::SetMotionOnOnceAttached(id_spine_instance spine_instance, chars target_motion, chars motion)
    {
        SpineInstance* CurInstance = (SpineInstance*) spine_instance;
        CurInstance->ReserveMotionOn(target_motion, motion, 0.0f, -1);
    }

    void SpineBuilder::SetMotionOnSeek(id_spine_instance spine_instance, chars motion, bool repeat)
    {
        SpineInstance* CurInstance = (SpineInstance*) spine_instance;
        CurInstance->SetMotionOnSeek(motion, 0.0f, repeat? 1 : 0);
    }

    void SpineBuilder::SetMotionOff(id_spine_instance spine_instance, chars motion)
    {
        SpineInstance* CurInstance = (SpineInstance*) spine_instance;
        CurInstance->SetMotionOff(motion);
    }

    void SpineBuilder::SetMotionOffAll(id_spine_instance spine_instance, bool with_reserve)
    {
        SpineInstance* CurInstance = (SpineInstance*) spine_instance;
        CurInstance->SetMotionOffAll(with_reserve);
    }

    void SpineBuilder::SetMotionOffAllWithoutSeek(id_spine_instance spine_instance, bool with_reserve)
    {
        SpineInstance* CurInstance = (SpineInstance*) spine_instance;
        CurInstance->SetMotionOffAllWithoutSeek(with_reserve);
    }

    bool SpineBuilder::IsMotionEnabled(id_spine_instance spine_instance)
    {
        SpineInstance* CurInstance = (SpineInstance*) spine_instance;
        return CurInstance->IsMotionEnabled();
    }

    Strings SpineBuilder::GetActiveMotions(id_spine_instance spine_instance)
    {
        SpineInstance* CurInstance = (SpineInstance*) spine_instance;
        return CurInstance->GetActiveMotions();
    }

    void SpineBuilder::Render(ZayPanel& panel, id_spine_instance spine_instance,
        bool flip, float cx, float cy, float scale, float rendermode, sint32 sx, sint32 sy, sint32 sw, sint32 sh)
    {
        GLint OldArrayBuffer = 0;
        GLint OldElementArrayBuffer = 0;
        GLint OldVertexArray = 0;
        GLint OldFrameBuffer = 0;
        GLint OldRenderBuffer = 0;
        GLint OldTexture = 0;
        GLint OldProgram = 0;
        //bx:glGetIntegerv(GL_ARRAY_BUFFER_BINDING, &OldArrayBuffer);
        //bx:glGetIntegerv(GL_ELEMENT_ARRAY_BUFFER_BINDING, &OldElementArrayBuffer);
        //bx:glGetIntegerv(GL_VERTEX_ARRAY_BINDING, &OldVertexArray);
        //bx:glGetIntegerv(GL_DRAW_FRAMEBUFFER_BINDING, &OldFrameBuffer);
        //bx:glGetIntegerv(GL_RENDERBUFFER_BINDING, &OldRenderBuffer);
        //bx:glGetIntegerv(GL_TEXTURE_BINDING_2D, &OldTexture);
        //bx:glGetIntegerv(GL_CURRENT_PROGRAM, &OldProgram);
        //bx:glPushMatrix();

        #if BOSS_IPHONE
            #define GLSCALE 3
        #else
            #define GLSCALE 1
        #endif

        SpineInstance* CurInstance = (SpineInstance*) spine_instance;
        CurInstance->Render(panel.fboid(), sx * GLSCALE, sy * GLSCALE, sw * GLSCALE, sh * GLSCALE, cx, cy, flip, scale * GLSCALE, rendermode);

        //bx:glPopMatrix();
        BOSS_GL(BindBuffer, GL_ARRAY_BUFFER, OldArrayBuffer);
        BOSS_GL(BindBuffer, GL_ELEMENT_ARRAY_BUFFER, OldElementArrayBuffer);
        #ifdef USE_BUFFER_OBJECT
            glBindVertexArray(OldVertexArray);
        #endif
        BOSS_GL(BindFramebuffer, GL_FRAMEBUFFER, OldFrameBuffer);
        BOSS_GL(BindRenderbuffer, GL_RENDERBUFFER, OldRenderBuffer);
        glBindTexture(GL_TEXTURE_2D, OldTexture);
        BOSS_GL(UseProgram, OldProgram);
    }

    void SpineBuilder::RenderBound(ZayPanel& panel, id_spine_instance spine_instance, float ox, float oy, float scale, bool flip, bool guideline,
        chars uiname, ZayPanel::SubGestureCB cb)
    {
        SpineInstance* CurInstance = (SpineInstance*) spine_instance;
        CurInstance->RenderBound(panel, ox, oy, scale, flip, guideline, uiname, cb);
    }

    const BOSS::Rect* SpineBuilder::GetBoundRect(id_spine_instance spine_instance, chars name)
    {
        SpineInstance* CurInstance = (SpineInstance*) spine_instance;
        return CurInstance->GetBoundRect(name);
    }

    template<typename TYPE>
    id_spine LoadSpineCore(const TYPE& prop, chars img_pathname)
    {
        static bool IsInit = true;
        if(IsInit)
        {
            IsInit = false;
            ZAY::AutoreleasePool::createDefaultPool();
            ZAY::AnimationTrack::initAnimationTrackCreators();
        }

        auto ToColor = [](chars value, sint32 index) -> float
        {
            char digits[3];
            char *error;
            int32_t color;

            if (boss_strlen(value) != 8) return -1;
            value += index * 2;

            digits[0] = *value;
            digits[1] = *(value + 1);
            digits[2] = '\0';
            color = (int32_t)strtoul(digits, &error, 16);
            if (*error != 0) return -1;
            return color / (float)255;
        };

        ZAY::SkeletonData* Data = new ZAY::SkeletonData;
        Data->retain();

        ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        // skeleton
        ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        const TYPE& json_skeleton = prop("skeleton");
        if(json_skeleton.IsValid())
        {
            float spine_width = json_skeleton("width").GetFloat(0.0f);
            float spine_height = json_skeleton("height").GetFloat(0.0f);

            Data->setWidth(spine_width);
            Data->setHeight(spine_height);
        }

        ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        // bones
        ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        const TYPE& json_bones = prop("bones");
        if(json_bones.IsValid())
        {
            std::map<ZAY::SkeletonBone*, std::string> parentNames;

            for(sint32 i = 0, iend = json_bones.LengthOfIndexable(); i < iend; ++i)
            {
                const TYPE& bone = json_bones[i];
                chars bone_name = bone("name").GetString("");
                chars bone_parent = bone("parent").GetString(nullptr);
                float bone_length = bone("length").GetFloat(0.0f);
                float bone_x = bone("x").GetFloat(0.0f);
                float bone_y = bone("y").GetFloat(0.0f);
                float bone_scaleX = bone("scaleX").GetFloat(1.0f);
                float bone_scaleY = bone("scaleY").GetFloat(1.0f);
                float bone_rotation = bone("rotation").GetFloat(0.0f);
                chars bone_flipX = bone("flipX").GetString(nullptr);
                chars bone_flipY = bone("flipY").GetString(nullptr);
                sint32 bone_inheritScale = bone("inheritScale").GetInt(1);
                sint32 bone_inheritRotation = bone("inheritRotation").GetInt(1);

                ZAY::SkeletonBone* skeleton_bone = nullptr;
                if(bone_parent)
                {
                    skeleton_bone = Data->createBone(bone_name);
                    parentNames[skeleton_bone] = bone_parent;
                }
                else
                {
                    skeleton_bone = Data->createRootBone(bone_name);
                }

                if (bone_rotation > 180.0f)
                {
                    bone_rotation = bone_rotation - 360.0f;
                }

                skeleton_bone->setPosition(bone_x, bone_y);
                skeleton_bone->setLength(bone_length);
                skeleton_bone->setScale(bone_scaleX, bone_scaleY);
                skeleton_bone->setRotation(bone_rotation);
                skeleton_bone->setInheritScale(bone_inheritScale ? true : false);
                skeleton_bone->setInheritRotation(bone_inheritRotation ? true : false);

                skeleton_bone->setDefaultPosition(skeleton_bone->getPosition());
                skeleton_bone->setDefaultLength(skeleton_bone->getLength());
                skeleton_bone->setDefaultScale(skeleton_bone->getScale());
                skeleton_bone->setDefaultRotation(skeleton_bone->getRotation());
                skeleton_bone->setDefaultFlipX(bone_flipX && !strcmp(bone_flipX, "true"));
                skeleton_bone->setDefaultFlipY(bone_flipY && !strcmp(bone_flipY, "true"));
                skeleton_bone->setDefaultInheritScale(skeleton_bone->getInheritScale());
                skeleton_bone->setDefaultInheritRotation(skeleton_bone->getInheritRotation());
            }

            for(auto it : parentNames)
            {
                auto skeleton_bone = Data->getBone(it.second);
                skeleton_bone->addChild(it.first);
            }
        }

        for(auto bone : Data->getRootBones())
        {
            bone->_updateWorldTransformDescending();
        }

        ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        // ik
        ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        const TYPE& json_iks = prop("ik");
        if(json_iks.IsValid())
        {
            for(sint32 i = 0, iend = json_iks.LengthOfIndexable(); i < iend; ++i)
            {
                const TYPE& ikMap = json_iks[i];
                ZAY::SkeletonIKData* ikData = new ZAY::SkeletonIKData();
                ikData->autorelease();

                chars ikName = ikMap("name").GetString(nullptr);
                const TYPE& boneMap = ikMap("bones");

                for(sint32 j = 0, jend = boneMap.LengthOfIndexable(); j < jend; ++j)
                {
                    chars boneName = boneMap[j].GetString();
                    BOSS_ASSERT("LoadSpine실패", Data->getBone(boneName));
                    ikData->_boneNames.push_back(boneName);
                }

                chars targetBoneName = ikMap("target").GetString(nullptr);
                BOSS_ASSERT("LoadSpine실패", Data->getBone(targetBoneName));
                ikData->_targetBoneName = targetBoneName;

                sint32 bendDirection = (!strcmp(ikMap("bendPositive").GetString("true"), "true"))? 1 : -1;
                ikData->_bendPositive = bendDirection;

                float mix = ikMap("mix").GetFloat(1.0f);
                ikData->_mix = mix;

                ikData->setName(ikName);

                Data->_spineConstraints.push_back(ikData);
                ikData->retain();

                if (ikData->getName().size() > 0)
                {
                    Data->_spineConstraintsMap[ikData->getName()] = ikData;
                }
            }
        }

        boost::bimap<int32_t, std::string> defaultSlotIndexToSlotNames;
        std::map<std::string, std::string> defaultSlotAttachmentNames;
        std::map<std::string, ZAY::SceneSlotData*> slotMaps;
        std::map<std::string, Image*> images;

        ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        // slots
        ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        const TYPE& json_slots = prop("slots");
        if(json_slots.IsValid())
        {
            int32_t drawOrder = 0;

            for(sint32 i = 0, iend = json_slots.LengthOfIndexable(); i < iend; ++i)
            {
                const TYPE& slot = json_slots[i];
                chars slot_name = slot("name").GetString(nullptr);
                chars slot_bone = slot("bone").GetString(nullptr);
                chars slot_color = slot("color").GetString("ffffffff");
                chars slot_attachment = slot("attachment").GetString(nullptr);
                chars slot_blend = slot("blend").GetString("normal");

                defaultSlotIndexToSlotNames.insert(boost::bimap<int32_t, std::string>::value_type(drawOrder, slot_name));

                ZAY::SceneSlotData* skeleton_slot = Data->createAndAddSlot(slot_name, static_cast<float>(drawOrder++));
                if (skeleton_slot == nullptr)
                {
                    delete Data;
                    return nullptr;
                }

                Data->setSlotParentBoneName(slot_name, slot_bone);

                if (slotMaps.find(slot_name) != slotMaps.end())
                {
                    delete Data;
                    return nullptr;
                }

                slotMaps[slot_name] = skeleton_slot;

                if (slot_color)
                {
                    float r = ToColor(slot_color, 0);
                    float g = ToColor(slot_color, 1);
                    float b = ToColor(slot_color, 2);
                    float a = ToColor(slot_color, 3);

                    skeleton_slot->setColour(ZAY::ColourValue(r, g, b, a));
                }

                if (slot_attachment)
                {
                    defaultSlotAttachmentNames[slot_name] = slot_attachment;
                }

                if (!strcmp(slot_blend, "normal"))
                    skeleton_slot->setBlendMode(ZAY::ForwardMultiplyRender::BlendType::Alpha);
                else if (!strcmp(slot_blend, "additive"))
                    skeleton_slot->setBlendMode(ZAY::ForwardMultiplyRender::BlendType::Additive);
                else if (!strcmp(slot_blend, "multiply"))
                    skeleton_slot->setBlendMode(ZAY::ForwardMultiplyRender::BlendType::Multiply);
                else if (!strcmp(slot_blend, "screen"))
                    skeleton_slot->setBlendMode(ZAY::ForwardMultiplyRender::BlendType::Screen);
                else
                {
                    BOSS_ASSERT(String::Format("Unknown Spine's blend type : %s", slot_blend), false);
                    skeleton_slot->setBlendMode(ZAY::ForwardMultiplyRender::BlendType::Alpha);
                }
            }
        }

        std::map<std::string, std::map<std::string, std::map<std::string, std::pair<std::string, ZAY::SceneData*>>>> attachments;

        ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        // skins
        ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        const TYPE& json_skins = prop("skins");
        if(json_skins.IsValid())
        {
            for(sint32 i = 0, iend = json_skins.LengthOfNamable(); i < iend; ++i)
            {
                chararray _skinName;
                const TYPE& skin = json_skins(i, &_skinName);
                chars skinName = &_skinName[0];

                for(sint32 j = 0, jend = skin.LengthOfNamable(); j < jend; ++j)
                {
                    chararray _slot_name;
                    const TYPE& slot = skin(j, &_slot_name);
                    chars slot_name = &_slot_name[0];

                    auto it = slotMaps.find(slot_name);
                    if (it == slotMaps.end())
                    {
                        delete Data;
                        return nullptr;
                    }

                    auto skeleton_slot = it->second;

                    if (skeleton_slot)
                    {
                        for(sint32 k = 0, kend = slot.LengthOfNamable(); k < kend; ++k)
                        {
                            chararray _skinAttachmentName;
                            const TYPE& attachment = slot(k, &_skinAttachmentName);
                            chars skinAttachmentName = &_skinAttachmentName[0];
                            chars attachmentName = attachment("name").GetString(skinAttachmentName);
                            chars path = attachment("path").GetString(attachmentName);

                            chars attachment_type = attachment("type").GetString("region");

                            if (strcmp(attachment_type, "region") == 0)
                            {
                                float attachment_x = attachment("x").GetFloat(0.0f);
                                float attachment_y = attachment("y").GetFloat(0.0f);
                                float attachment_scaleX = attachment("scaleX").GetFloat(1.0f);
                                float attachment_scaleY = attachment("scaleY").GetFloat(1.0f);
                                float attachment_rotation = attachment("rotation").GetFloat(0.0f);
                                float attachment_width = attachment("width").GetFloat(32.0f);
                                float attachment_height = attachment("height").GetFloat(32.0f);
                                chars attachment_color = attachment("color").GetString(nullptr);

                                ZAY::ColourValue color = ZAY::ColourValue::White;

                                if (attachment_color)
                                {
                                    float r = ToColor(attachment_color, 0);
                                    float g = ToColor(attachment_color, 1);
                                    float b = ToColor(attachment_color, 2);
                                    float a = ToColor(attachment_color, 3);

                                    color.r = r;
                                    color.g = g;
                                    color.b = b;
                                    color.a = a;
                                }

                                ZAY::Matrix4 transform;
                                transform.makeTransform(ZAY::Vector3(attachment_x, attachment_y, 0.0f),
                                                        ZAY::Vector3(attachment_scaleX, attachment_scaleY, 1.0f),
                                                        attachment_rotation,
                                                        false,
                                                        false);

                                ZAY::MeshData* meshData = new ZAY::MeshData;
                                meshData->autorelease();

                                int32_t verticesCount = 4;
                                int32_t indicesCount = 6;

                                meshData->_getVertexPositions().createBuffer(verticesCount);
                                meshData->_getVertexUVs().createBuffer(verticesCount);
                                meshData->_getVertexColors().createBuffer(verticesCount);

                                meshData->_getIndices().createBuffer(indicesCount);

                                auto vertexPositions = meshData->getVertexPositions().getBufferPointer();
                                auto vertexTexCoords = meshData->getVertexUVs().getBufferPointer();
                                auto vertexColors = meshData->getVertexColors().getBufferPointer();

                                auto indices = meshData->getIndices().getBufferPointer();

                                vertexPositions[0] = transform * ZAY::Vector3(+ attachment_width / 2.0f,
                                                                            + attachment_height / 2.0f,
                                                                            0.0f);
                                vertexTexCoords[0] = ZAY::Vector2(1.0f, 0.0f);
                                vertexColors[0] = color;

                                vertexPositions[1] = transform * ZAY::Vector3(+ attachment_width / 2.0f,
                                                                            - attachment_height / 2.0f,
                                                                            0.0f);
                                vertexTexCoords[1] = ZAY::Vector2(1.0f, 1.0f);
                                vertexColors[1] = color;

                                vertexPositions[2] = transform * ZAY::Vector3(- attachment_width / 2.0f,
                                                                            - attachment_height / 2.0f,
                                                                            0.0f);
                                vertexTexCoords[2] = ZAY::Vector2(0.0f, 1.0f);
                                vertexColors[2] = color;

                                vertexPositions[3] = transform * ZAY::Vector3(- attachment_width / 2.0f,
                                                                            + attachment_height / 2.0f,
                                                                            0.0f);
                                vertexTexCoords[3] = ZAY::Vector2(0.0f, 0.0f);
                                vertexColors[3] = color;

                                indices[0] = 0;
                                indices[1] = 1;
                                indices[2] = 2;
                                indices[3] = 0;
                                indices[4] = 2;
                                indices[5] = 3;

                                meshData->loadTextureImage(String::Format("%s/%s.png", img_pathname, path));
                                meshData->setBlendMode(skeleton_slot->getBlendMode());
                                skeleton_slot->setSkin(skinAttachmentName, skinName, meshData);
                                if (attachments[skinName][slot_name].find(skinAttachmentName) != attachments[skinName][slot_name].end())
                                {
                                    delete Data;
                                    return nullptr;
                                }

                                attachments[skinName][slot_name][skinAttachmentName] = std::make_pair(attachmentName, meshData);

                            }
                            else if (strcmp(attachment_type, "regionsequence") == 0)
                            {
                                delete Data;
                                return nullptr;
                            }
                            else if (strcmp(attachment_type, "mesh") == 0)
                            {
                                const TYPE& attachment_mesh_vertices = attachment("vertices");
                                const TYPE& attachment_mesh_indices = attachment("triangles");
                                const TYPE& attachment_mesh_uvs = attachment("uvs");
                                chars attachment_mesh_color = attachment("color").GetString(nullptr);

                                ZAY::MeshData* meshData = new ZAY::MeshData;
                                meshData->autorelease();

                                int32_t verticesCount = 0;

                                if(attachment_mesh_vertices.IsValid())
                                {
                                    sint32 attachment_mesh_vertices_count = attachment_mesh_vertices.LengthOfIndexable();
                                    verticesCount = attachment_mesh_vertices_count / 2;

                                    if (verticesCount > 0)
                                    {
                                        meshData->_getVertexPositions().createBuffer(verticesCount);

                                        ZAY::Vector3* v = meshData->getVertexPositions().getBufferPointer();

                                        for(sint32 l = 0, lend = attachment_mesh_vertices.LengthOfIndexable(); l < lend;)
                                        {
                                            float x = attachment_mesh_vertices[l++].GetFloat();
                                            float y = attachment_mesh_vertices[l++].GetFloat();

                                            v->x = x;
                                            v->y = y;
                                            v->z = 0.0f;

                                            v++;
                                        }
                                    }
                                }

                                if(attachment_mesh_indices.IsValid())
                                {
                                    sint32 attachment_mesh_indices_count = attachment_mesh_indices.LengthOfIndexable();
                                    sint32 indicesCount = attachment_mesh_indices_count;

                                    if (indicesCount > 0)
                                    {
                                        meshData->_getIndices().createBuffer(indicesCount);

                                        GLushort* indices = meshData->getIndices().getBufferPointer();

                                        for(sint32 l = 0, lend = attachment_mesh_indices.LengthOfIndexable(); l < lend; ++l)
                                        {
                                            sint32 value = attachment_mesh_indices[l].GetInt();

                                            *indices = value;

                                            indices++;
                                        }
                                    }
                                }

                                if(attachment_mesh_uvs.IsValid())
                                {
                                    sint32 attachment_mesh_uvs_count = attachment_mesh_uvs.LengthOfIndexable();
                                    sint32 count = attachment_mesh_uvs_count / 2;

                                    if (count != verticesCount)
                                    {
                                        delete Data;
                                        return nullptr;
                                    }

                                    meshData->_getVertexUVs().createBuffer(count);

                                    auto p_uv = meshData->getVertexUVs().getBufferPointer();

                                    for(sint32 l = 0, lend = attachment_mesh_uvs.LengthOfIndexable(); l < lend;)
                                    {
                                        float u = attachment_mesh_uvs[l++].GetFloat();
                                        float v = attachment_mesh_uvs[l++].GetFloat();

                                        p_uv->x = u;
                                        p_uv->y = v;

                                        p_uv++;
                                    }
                                }

                                ZAY::ColourValue color = ZAY::ColourValue::White;

                                if (attachment_mesh_color)
                                {
                                    float r = ToColor(attachment_mesh_color, 0);
                                    float g = ToColor(attachment_mesh_color, 1);
                                    float b = ToColor(attachment_mesh_color, 2);
                                    float a = ToColor(attachment_mesh_color, 3);

                                    color.r = r;
                                    color.g = g;
                                    color.b = b;
                                    color.a = a;
                                }

                                if (verticesCount > 0)
                                {
                                    meshData->_getVertexColors().createBuffer(verticesCount);

                                    auto p_color = meshData->getVertexColors().getBufferPointer();

                                    for (sint32 l = 0; l < verticesCount ; l++)
                                    {
                                        *(p_color++) = color;
                                    }
                                }

                                meshData->loadTextureImage(String::Format("%s/%s.png", img_pathname, path));
                                meshData->setBlendMode(skeleton_slot->getBlendMode());
                                skeleton_slot->setSkin(skinAttachmentName, skinName, meshData);
                                if (attachments[skinName][slot_name].find(skinAttachmentName) != attachments[skinName][slot_name].end())
                                {
                                    delete Data;
                                    return nullptr;
                                }

                                attachments[skinName][slot_name][skinAttachmentName] = std::make_pair(attachmentName, meshData);
                            }
                            else if (strcmp(attachment_type, "skinnedmesh") == 0)
                            {
                                const TYPE& attachment_mesh_vertices = attachment("vertices");
                                const TYPE& attachment_mesh_indices = attachment("triangles");
                                const TYPE& attachment_mesh_uvs = attachment("uvs");
                                chars attachment_mesh_color = attachment("color").GetString(nullptr);

                                ZAY::MeshData* meshData = new ZAY::MeshData;
                                meshData->autorelease();

                                int32_t verticesCount = 0;

                                if(attachment_mesh_vertices.IsValid())
                                {
                                    sint32s datas;
                                    int32_t skinningVertexCount = 0;
                                    for(sint32 l = 0, lend = attachment_mesh_vertices.LengthOfIndexable(); l < lend;)
                                    {
                                        const sint32 weightsCount = attachment_mesh_vertices[l++].GetInt();
                                        datas.AtAdding() = weightsCount;

                                        verticesCount++;
                                        skinningVertexCount += weightsCount;

                                        for(sint32 m = 0; m < weightsCount; m++)
                                        {
                                            const sint32 boneIndex = attachment_mesh_vertices[l++].GetInt();
                                            datas.AtAdding() = boneIndex;
                                            const float offsetX = attachment_mesh_vertices[l++].GetFloat();
                                            datas.AtAdding() = *((sint32*) &offsetX);
                                            const float offsetY = attachment_mesh_vertices[l++].GetFloat();
                                            datas.AtAdding() = *((sint32*) &offsetY);
                                            const float weight = attachment_mesh_vertices[l++].GetFloat();
                                            datas.AtAdding() = *((sint32*) &weight);
                                        }
                                    }

                                    sint32 NewDataSize = sizeof(sint32) * datas.Count();
                                    char* NewData = new char[NewDataSize];
                                    Memory::Copy(NewData, datas.AtDumping(0, 0), NewDataSize);
                                    meshData->_setSkinningData(skinningVertexCount, verticesCount, NewData, NewDataSize);
                                }

                                if(attachment_mesh_indices.IsValid())
                                {
                                    sint32 attachment_mesh_indices_count = attachment_mesh_indices.LengthOfIndexable();
                                    sint32 indicesCount = attachment_mesh_indices_count;

                                    if(indicesCount > 0)
                                    {
                                        meshData->_getIndices().createBuffer(indicesCount);

                                        auto indices = meshData->getIndices().getBufferPointer();

                                        for(sint32 l = 0, lend = attachment_mesh_indices.LengthOfIndexable(); l < lend; ++l)
                                        {
                                            sint32 value = attachment_mesh_indices[l].GetInt();
                                            *indices = value;
                                            indices++;
                                        }
                                    }
                                }

                                if(attachment_mesh_uvs.IsValid())
                                {
                                    sint32 attachment_mesh_uvs_count = attachment_mesh_uvs.LengthOfIndexable();
                                    sint32 count = attachment_mesh_uvs_count / 2;

                                    if (count != verticesCount)
                                    {
                                        delete Data;
                                        return nullptr;
                                    }

                                    meshData->_getVertexUVs().createBuffer(count);

                                    ZAY::Vector2* p_uv = meshData->getVertexUVs().getBufferPointer();

                                    for(sint32 l = 0, lend = attachment_mesh_uvs.LengthOfIndexable(); l < lend;)
                                    {
                                        float u = attachment_mesh_uvs[l++].GetFloat();
                                        float v = (l < lend)? attachment_mesh_uvs[l++].GetFloat() : 0.0f;

                                        p_uv->x = u;
                                        p_uv->y = v;

                                        p_uv++;
                                    }
                                }

                                ZAY::ColourValue color = ZAY::ColourValue::White;

                                if (attachment_mesh_color)
                                {
                                    float r = ToColor(attachment_mesh_color, 0);
                                    float g = ToColor(attachment_mesh_color, 1);
                                    float b = ToColor(attachment_mesh_color, 2);
                                    float a = ToColor(attachment_mesh_color, 3);

                                    color.r = r;
                                    color.g = g;
                                    color.b = b;
                                    color.a = a;
                                }

                                if (verticesCount > 0)
                                {
                                    meshData->_getVertexColors().createBuffer(verticesCount);

                                    auto p_color = meshData->getVertexColors().getBufferPointer();

                                    for(sint32 l = 0; l < verticesCount; l++)
                                    {
                                        *(p_color++) = color;
                                    }
                                }

                                meshData->loadTextureImage(String::Format("%s/%s.png", img_pathname, path));
                                meshData->setBlendMode(skeleton_slot->getBlendMode());
                                skeleton_slot->setSkin(skinAttachmentName, skinName, meshData);
                                if (attachments[skinName][slot_name].find(skinAttachmentName) != attachments[skinName][slot_name].end())
                                {
                                    delete Data;
                                    return nullptr;
                                }

                                attachments[skinName][slot_name][skinAttachmentName] = std::make_pair(attachmentName, meshData);
                            }
                            else if (strcmp(attachment_type, "boundingbox") == 0)
                            {
                                const TYPE& attachment_boundingbox_vertices = attachment("vertices");

                                ZAY::TouchAreaData* touchAreaData = new ZAY::TouchAreaData;
                                touchAreaData->autorelease();

                                if(attachment_boundingbox_vertices.IsValid())
                                {
                                    sint32 attachment_boundingbox_vertices_size = attachment_boundingbox_vertices.LengthOfIndexable();

                                    float* vertices = new float[attachment_boundingbox_vertices_size];
                                    float* v = vertices;

                                    for(sint32 l = 0, lend = attachment_boundingbox_vertices.LengthOfIndexable(); l < lend; ++l)
                                    {
                                        float value = attachment_boundingbox_vertices[l].GetFloat();
                                        *v = value;
                                        v++;
                                    }

                                    touchAreaData->setLocalVertices(slot_name, vertices, attachment_boundingbox_vertices_size);
                                    delete [] vertices;
                                }

                                skeleton_slot->setSkin(skinAttachmentName, skinName, touchAreaData);
                                if (attachments[skinName][slot_name].find(skinAttachmentName) != attachments[skinName][slot_name].end())
                                {
                                    delete Data;
                                    return nullptr;
                                }
                                attachments[skinName][slot_name][skinAttachmentName] = std::make_pair(attachmentName, touchAreaData);
                            }
                            else
                            {
                                delete Data;
                                return nullptr;
                            }
                        }
                    }
                }
            }
        }

        ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        // events
        ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        const TYPE& json_events = prop("events");
        if(json_events.IsValid())
        {
            // TODO
        }

        ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        // animations
        ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        const TYPE& json_animations = prop("animations");
        if(json_animations.IsValid())
        {
            for(sint32 i = 0, iend = json_animations.LengthOfNamable(); i < iend; ++i)
            {
                chararray _animation_name;
                const TYPE& json_animation = json_animations(i, &_animation_name);
                chars animation_name = &_animation_name[0];

                const TYPE& json_animation_bones = json_animation("bones");
                const TYPE& json_animation_slots = json_animation("slots");
                const TYPE& json_animation_ik = json_animation("ik");
                const TYPE& json_animation_ffd = json_animation("ffd");
                const TYPE& json_animation_events = json_animation("events"); //bx

                const TYPE& json_animation_drawOrder = json_animation("drawOrder");

                ZAY::Animation* skeletonAnimation = new ZAY::Animation();
                skeletonAnimation->autorelease();
                Data->setAnimation(animation_name, skeletonAnimation);

                if(json_animation_slots.IsValid())
                {
                    for(sint32 j = 0, jend = json_animation_slots.LengthOfNamable(); j < jend; ++j)
                    {
                        chararray _slotName;
                        const TYPE& slot = json_animation_slots(j, &_slotName);
                        chars slotName = &_slotName[0];

                        auto it = slotMaps.find(slotName);
                        if (it == slotMaps.end())
                        {
                            delete Data;
                            return nullptr;
                        }

                        ZAY::SceneSlotData* skeleton_slot = it->second;

                        if(skeleton_slot)
                        {
                            ZAY::Animation* animation = new ZAY::Animation();
                            animation->autorelease();

                            skeleton_slot->setAnimation(animation_name, animation);

                            for(sint32 k = 0, kend = slot.LengthOfNamable(); k < kend; ++k)
                            {
                                chararray _timelineName;
                                const TYPE& timeline = slot(k, &_timelineName);
                                chars timelineName = &_timelineName[0];

                                if (strcmp(timelineName, "color") == 0)
                                {
                                    ZAY::AnimationTrackColour* track = ZAY::AnimationTrack::
                                        createAnimationTrackWithTypeCasting<ZAY::AnimationTrackColour*>("AnimationTrack_SceneSlotInstanceColor");
                                    track->setDefaultValue(skeleton_slot->getColour());
                                    animation->addAnimationTrack(track);

                                    for(sint32 l = 0, lend = timeline.LengthOfIndexable(); l < lend; ++l)
                                    {
                                        const TYPE& frame = timeline[l];
                                        chars frame_color = frame("color").GetString(nullptr);

                                        float r = ToColor(frame_color, 0);
                                        float g = ToColor(frame_color, 1);
                                        float b = ToColor(frame_color, 2);
                                        float a = ToColor(frame_color, 3);

                                        float frame_time = frame("time").GetFloat(0.0f);

                                        const TYPE& frame_curve = frame("curve");

                                        ZAY::AnimationKeyFrameSingleValue<ZAY::ColourValue>* animationKeyFrame = nullptr;

                                        if(frame_curve.IsValid() && strcmp(frame_curve.GetString(), "stepped") == 0)
                                        {
                                            animationKeyFrame = new ZAY::AnimationKeyFrame_ColourValue_Stepped();
                                            animationKeyFrame->autorelease();
                                        }
                                        else if(frame_curve.IsValid() && 0 < frame_curve.LengthOfIndexable())
                                        {
                                            // bezier curve
                                            float cx1 = frame_curve[0].GetFloat();
                                            float cy1 = frame_curve[1].GetFloat();
                                            float cx2 = frame_curve[2].GetFloat();
                                            float cy2 = frame_curve[3].GetFloat();

                                            ZAY::AnimationKeyFrame_ColourValue_SpineBezierToTenLinearFake* akf
                                                = new ZAY::AnimationKeyFrame_ColourValue_SpineBezierToTenLinearFake();
                                            akf->autorelease();

                                            akf->createSpineFakeLinears(cx1, cy1, cx2, cy2);

                                            animationKeyFrame = akf;
                                        }
                                        else
                                        {
                                            animationKeyFrame = new ZAY::AnimationKeyFrame_ColourValue_Linear();
                                            animationKeyFrame->autorelease();
                                        }

                                        animationKeyFrame->setValue(ZAY::ColourValue(r, g, b, a));

                                        track->setKeyFrame(frame_time, animationKeyFrame);
                                    }
                                }
                                else if (strcmp(timelineName, "attachment") == 0)
                                {
                                    ZAY::AnimationTrackString* track = ZAY::AnimationTrack::
                                        createAnimationTrackWithTypeCasting<ZAY::AnimationTrackString*>("AnimationTrack_SceneSlotInstanceAttachment");

                                    auto it = defaultSlotAttachmentNames.find(slotName);

                                    if (it != defaultSlotAttachmentNames.end())
                                    {
                                        track->setDefaultValue(it->second);
                                    }

                                    animation->addAnimationTrack(track);

                                    for(sint32 l = 0, lend = timeline.LengthOfIndexable(); l < lend; ++l)
                                    {
                                        const TYPE& frame = timeline[l];
                                        chars frame_attachment_name = frame("name").GetString("");

                                        float frame_time = frame("time").GetFloat(0.0f);

                                        ZAY::AnimationKeyFrame_String_Stepped* animationKeyFrame = nullptr;
                                        animationKeyFrame = new ZAY::AnimationKeyFrame_String_Stepped();
                                        animationKeyFrame->autorelease();

                                        if (frame_attachment_name)
                                        {
                                            animationKeyFrame->setValue(frame_attachment_name);
                                        }
                                        else
                                        {
                                            animationKeyFrame->setValue(ZAY::s_emptyString);
                                        }

                                        track->setKeyFrame(frame_time, animationKeyFrame);
                                    }
                                }
                            }

                            animation->findAndSetAnimationLengthWithTracks();
                        }
                    }
                }

                if(json_animation_bones.IsValid())
                {
                    for(sint32 j = 0, jend = json_animation_bones.LengthOfNamable(); j < jend; ++j)
                    {
                        chararray _bone_name;
                        const TYPE& bone = json_animation_bones(j, &_bone_name);
                        std::string bone_name = &_bone_name[0];
                        ZAY::SkeletonBone* targetBone = Data->getBone(bone_name);
                        if(targetBone == nullptr)
                        {
                            delete Data;
                            return nullptr;
                        }

                        ZAY::AnimationTrackFloat* rotateTrack = nullptr;
                        ZAY::AnimationTrackVector3* scaleTrack = nullptr;
                        ZAY::AnimationTrackVector3* translateTrack = nullptr;
                        ZAY::AnimationTrackBoolean* flipXTrack = nullptr;
                        ZAY::AnimationTrackBoolean* flipYTrack = nullptr;

                        for(sint32 k = 0, kend = bone.LengthOfNamable(); k < kend; ++k)
                        {
                            chararray _timeline_name;
                            const TYPE& timeline = bone(k, &_timeline_name);
                            chars timeline_name = &_timeline_name[0];

                            if (strcmp(timeline_name, "rotate") == 0)
                            {
                                for(sint32 l = 0, lend = timeline.LengthOfIndexable(); l < lend; ++l)
                                {
                                    const TYPE& frame = timeline[l];

                                    if (rotateTrack == nullptr)
                                    {
                                        rotateTrack = ZAY::AnimationTrack::
                                            createAnimationTrackWithTypeCasting<ZAY::AnimationTrackFloat*>("AnimationTrack_SkeletonBoneRotate");
                                        rotateTrack->setDefaultValue(targetBone->getDefaultRotation());
                                        rotateTrack->boneName = bone_name;
                                        skeletonAnimation->addAnimationTrack(rotateTrack);
                                    }

                                    float time = frame("time").GetFloat(0.0f);
                                    float angle = frame("angle").GetFloat(0.0f);
                                    const TYPE& frame_curve = frame("curve");

                                    ZAY::AnimationKeyFrameSingleValue<float>* animationKeyFrame = nullptr;

                                    if(frame_curve.IsValid() && strcmp(frame_curve.GetString(), "stepped") == 0)
                                    {
                                        animationKeyFrame = new ZAY::AnimationKeyFrame_Rotation_Stepped();
                                        animationKeyFrame->autorelease();
                                    }
                                    else if(frame_curve.IsValid() && 0 < frame_curve.LengthOfIndexable())
                                    {
                                        // bezier curve
                                        float cx1 = frame_curve[0].GetFloat();
                                        float cy1 = frame_curve[1].GetFloat();
                                        float cx2 = frame_curve[2].GetFloat();
                                        float cy2 = frame_curve[3].GetFloat();

                                        ZAY::AnimationKeyFrame_Rotation_SpineBezierToTenLinearFake* akf
                                            = new ZAY::AnimationKeyFrame_Rotation_SpineBezierToTenLinearFake();
                                        akf->autorelease();

                                        akf->createSpineFakeLinears(cx1, cy1, cx2, cy2);

                                        animationKeyFrame = akf;
                                    }
                                    else
                                    {
                                        animationKeyFrame = new ZAY::AnimationKeyFrame_Rotation_Linear();
                                        animationKeyFrame->autorelease();
                                    }

                                    animationKeyFrame->setValue(targetBone->getDefaultRotation() + angle);

                                    rotateTrack->setKeyFrame(time, animationKeyFrame);
                                }
                            }
                            else if (strcmp(timeline_name, "scale") == 0)
                            {
                                for(sint32 l = 0, lend = timeline.LengthOfIndexable(); l < lend; ++l)
                                {
                                    const TYPE& frame = timeline[l];

                                    if (scaleTrack == nullptr)
                                    {
                                        scaleTrack = ZAY::AnimationTrack::
                                            createAnimationTrackWithTypeCasting<ZAY::AnimationTrackVector3*>("AnimationTrack_SkeletonBoneScale");
                                        scaleTrack->setDefaultValue(targetBone->getDefaultScale());
                                        scaleTrack->boneName = bone_name;
                                        skeletonAnimation->addAnimationTrack(scaleTrack);
                                    }

                                    float time = frame("time").GetFloat(0.0f);
                                    float x = frame("x").GetFloat(0.0f);
                                    float y = frame("y").GetFloat(0.0f);
                                    const TYPE& frame_curve = frame("curve");

                                    ZAY::AnimationKeyFrameSingleValue<ZAY::Vector3>* animationKeyFrame = nullptr;

                                    if(frame_curve.IsValid() && strcmp(frame_curve.GetString(), "stepped") == 0)
                                    {
                                        animationKeyFrame = new ZAY::AnimationKeyFrame_Vector3_Stepped();
                                        animationKeyFrame->autorelease();
                                    }
                                    else if(frame_curve.IsValid() && 0 < frame_curve.LengthOfIndexable())
                                    {
                                        // bezier curve
                                        float cx1 = frame_curve[0].GetFloat();
                                        float cy1 = frame_curve[1].GetFloat();
                                        float cx2 = frame_curve[2].GetFloat();
                                        float cy2 = frame_curve[3].GetFloat();

                                        ZAY::AnimationKeyFrame_Vector3_SpineBezierToTenLinearFake* akf
                                            = new ZAY::AnimationKeyFrame_Vector3_SpineBezierToTenLinearFake();
                                        akf->autorelease();

                                        akf->createSpineFakeLinears(cx1, cy1, cx2, cy2);

                                        animationKeyFrame = akf;
                                    }
                                    else
                                    {
                                        animationKeyFrame = new ZAY::AnimationKeyFrame_Vector3_Linear();
                                        animationKeyFrame->autorelease();
                                    }

                                    animationKeyFrame->setValue(targetBone->getDefaultScale() * ZAY::Vector3(x, y, 1.0f));

                                    scaleTrack->setKeyFrame(time, animationKeyFrame);
                                }
                            }
                            else if (strcmp(timeline_name, "translate") == 0)
                            {
                                for(sint32 l = 0, lend = timeline.LengthOfIndexable(); l < lend; ++l)
                                {
                                    const TYPE& frame = timeline[l];

                                    if (translateTrack == nullptr)
                                    {
                                        translateTrack = ZAY::AnimationTrack::
                                            createAnimationTrackWithTypeCasting<ZAY::AnimationTrackVector3*>("AnimationTrack_SkeletonBoneTranslate");
                                        translateTrack->setDefaultValue(targetBone->getDefaultPosition());
                                        translateTrack->boneName = bone_name;
                                        skeletonAnimation->addAnimationTrack(translateTrack);
                                    }

                                    float time = frame("time").GetFloat(0.0f);
                                    float x = frame("x").GetFloat(0.0f);
                                    float y = frame("y").GetFloat(0.0f);
                                    const TYPE& frame_curve = frame("curve");

                                    ZAY::AnimationKeyFrameSingleValue<ZAY::Vector3>* animationKeyFrame = nullptr;

                                    if(frame_curve.IsValid() && strcmp(frame_curve.GetString(), "stepped") == 0)
                                    {
                                        animationKeyFrame = new ZAY::AnimationKeyFrame_Vector3_Stepped();
                                        animationKeyFrame->autorelease();
                                    }
                                    else if(frame_curve.IsValid() && 0 < frame_curve.LengthOfIndexable())
                                    {
                                        // bezier curve
                                        float cx1 = frame_curve[0].GetFloat();
                                        float cy1 = frame_curve[1].GetFloat();
                                        float cx2 = frame_curve[2].GetFloat();
                                        float cy2 = frame_curve[3].GetFloat();

                                        ZAY::AnimationKeyFrame_Vector3_SpineBezierToTenLinearFake* akf
                                            = new ZAY::AnimationKeyFrame_Vector3_SpineBezierToTenLinearFake();
                                        akf->autorelease();

                                        akf->createSpineFakeLinears(cx1, cy1, cx2, cy2);

                                        animationKeyFrame = akf;
                                    }
                                    else
                                    {
                                        animationKeyFrame = new ZAY::AnimationKeyFrame_Vector3_Linear();
                                        animationKeyFrame->autorelease();
                                    }

                                    animationKeyFrame->setValue(targetBone->getDefaultPosition() + ZAY::Vector3(x, y, 0.0f));

                                    translateTrack->setKeyFrame(time, animationKeyFrame);
                                }
                            }
                            else if (strcmp(timeline_name, "flipX") == 0)
                            {
                                for(sint32 l = 0, lend = timeline.LengthOfIndexable(); l < lend; ++l)
                                {
                                    const TYPE& frame = timeline[l];

                                    if (flipXTrack == nullptr)
                                    {
                                        flipXTrack = ZAY::AnimationTrack::
                                            createAnimationTrackWithTypeCasting<ZAY::AnimationTrackBoolean*>("AnimationTrack_SkeletonBoneFlipX");
                                        flipXTrack->setDefaultValue(targetBone->getDefaultFlipX());
                                        flipXTrack->boneName = bone_name;
                                        skeletonAnimation->addAnimationTrack(flipXTrack);
                                    }

                                    ZAY::AnimationKeyFrame_Boolean_Stepped* animationKeyFrame = nullptr;
                                    animationKeyFrame = new ZAY::AnimationKeyFrame_Boolean_Stepped();
                                    animationKeyFrame->autorelease();

                                    const TYPE& x_item = frame("x");
                                    animationKeyFrame->setValue(x_item.IsValid() && !strcmp(x_item.GetString(), "true"));

                                    float time = frame("time").GetFloat(0.0f);
                                    flipXTrack->setKeyFrame(time, animationKeyFrame);
                                }
                            }
                            else if (strcmp(timeline_name, "flipY") == 0)
                            {
                                for(sint32 l = 0, lend = timeline.LengthOfIndexable(); l < lend; ++l)
                                {
                                    const TYPE& frame = timeline[l];

                                    if (flipYTrack == nullptr)
                                    {
                                        flipYTrack = ZAY::AnimationTrack::
                                            createAnimationTrackWithTypeCasting<ZAY::AnimationTrackBoolean*>("AnimationTrack_SkeletonBoneFlipY");
                                        flipYTrack->setDefaultValue(targetBone->getDefaultFlipY());
                                        flipYTrack->boneName = bone_name;
                                        skeletonAnimation->addAnimationTrack(flipYTrack);
                                    }

                                    ZAY::AnimationKeyFrame_Boolean_Stepped* animationKeyFrame = nullptr;
                                    animationKeyFrame = new ZAY::AnimationKeyFrame_Boolean_Stepped();
                                    animationKeyFrame->autorelease();

                                    const TYPE& y_item = frame("y");
                                    animationKeyFrame->setValue(y_item.IsValid() && !strcmp(y_item.GetString(), "true"));

                                    float time = frame("time").GetFloat(0.0f);
                                    flipYTrack->setKeyFrame(time, animationKeyFrame);
                                }
                            }
                        }
                    }
                }

                if(json_animation_ik.IsValid())
                {
                    for(sint32 j = 0, jend = json_animation_ik.LengthOfNamable(); j < jend; ++j)
                    {
                        chararray _ik_name;
                        const TYPE& ik = json_animation_ik(j, &_ik_name);
                        chars ik_name = &_ik_name[0];

                        ZAY::SpineConstraint* spineConstraint = Data->getSpineConstraint(ik_name);
                        if (dynamic_cast<ZAY::SkeletonIKData*>(spineConstraint) == nullptr)
                        {
                            delete Data;
                            return nullptr;
                        }

                        ZAY::SkeletonIKData* ikData = static_cast<ZAY::SkeletonIKData*>(spineConstraint);

                        if(0 < ik.LengthOfIndexable())
                        {
                            // bendDirection 는 -1, 1 로 무조건 stepped animation track 으로

                            // mix 는 curve 값에 맞춰서

                            // 즉, 무조건 2개 track 이 필요함.

                            ZAY::AnimationTrackFloat* mixTrack = ZAY::AnimationTrack::
                                createAnimationTrackWithTypeCasting<ZAY::AnimationTrackFloat*>("AnimationTrack_SkeletonConstraintIK_Mix");
                            mixTrack->setDefaultValue(ikData->_mix);

                            ZAY::AnimationTrackFloat* bendPositiveTrack = ZAY::AnimationTrack::
                                createAnimationTrackWithTypeCasting<ZAY::AnimationTrackFloat*>("AnimationTrack_SkeletonConstraintIK_BendPositive");
                            bendPositiveTrack->setDefaultValue((ikData->_bendPositive == 1) ? 1.0f : -1.0f);

                            mixTrack->ikName = std::string(ik_name);
                            bendPositiveTrack->ikName = std::string(ik_name);

                            skeletonAnimation->addAnimationTrack(mixTrack);
                            skeletonAnimation->addAnimationTrack(bendPositiveTrack);

                            for(sint32 k = 0, kend = ik.LengthOfIndexable(); k < kend; ++k)
                            {
                                const TYPE& frame = ik[k];

                                float time = frame("time").GetFloat(0.0f);
                                const TYPE& frame_curve = frame("curve");
                                float mix = frame("mix").GetFloat(1.0f);


                                ZAY::AnimationKeyFrameSingleValue<float>* animationKeyFrameMix = nullptr;

                                if(frame_curve.IsValid() && strcmp(frame_curve.GetString(), "stepped") == 0)
                                {
                                    animationKeyFrameMix = new ZAY::AnimationKeyFrame_Float_Stepped();
                                    animationKeyFrameMix->autorelease();
                                }
                                else if(frame_curve.IsValid() && 0 < frame_curve.LengthOfIndexable())
                                {
                                    // bezier curve
                                    float cx1 = frame_curve[0].GetFloat();
                                    float cy1 = frame_curve[1].GetFloat();
                                    float cx2 = frame_curve[2].GetFloat();
                                    float cy2 = frame_curve[3].GetFloat();

                                    ZAY::AnimationKeyFrame_Float_SpineBezierToTenLinearFake* akf
                                        = new ZAY::AnimationKeyFrame_Float_SpineBezierToTenLinearFake();
                                    akf->autorelease();
                                    akf->createSpineFakeLinears(cx1, cy1, cx2, cy2);

                                    animationKeyFrameMix = akf;
                                }
                                else
                                {
                                    animationKeyFrameMix = new ZAY::AnimationKeyFrame_Float_Linear();
                                    animationKeyFrameMix->autorelease();
                                }

                                animationKeyFrameMix->setValue(mix);

                                mixTrack->setKeyFrame(time, animationKeyFrameMix);

                                auto bendDirection = (!strcmp(frame("bendPositive").GetString("true"), "true"))? 1.0f : -1.0f;

                                ZAY::AnimationKeyFrame_Float_Stepped* animationKeyFrameBendPositive = new ZAY::AnimationKeyFrame_Float_Stepped();
                                animationKeyFrameBendPositive->autorelease();
                                animationKeyFrameBendPositive->setValue(bendDirection);
                                bendPositiveTrack->setKeyFrame(time, animationKeyFrameBendPositive);
                            }
                        }
                    }
                }

                if(json_animation_ffd.IsValid())
                {
                    for(sint32 j = 0, jend = json_animation_ffd.LengthOfNamable(); j < jend; ++j)
                    {
                        chararray _skin_name;
                        const TYPE& skin = json_animation_ffd(j, &_skin_name);
                        chars skin_name = &_skin_name[0];

                        auto found = attachments.find(skin_name);
                        if (found == attachments.end())
                        {
                            delete Data;
                            return nullptr;
                        }

                        const auto& skin_attachments = found->second;

                        for(sint32 k = 0, kend = skin.LengthOfNamable(); k < kend; ++k)
                        {
                            chararray _slot_name;
                            const TYPE& slot = skin(k, &_slot_name);
                            chars slot_name = &_slot_name[0];

                            auto found = skin_attachments.find(slot_name);
                            if (found == skin_attachments.end())
                            {
                                delete Data;
                                return nullptr;
                            }

                            const auto& slot_attachments = found->second;

                            for(sint32 l = 0, lend = slot.LengthOfNamable(); l < lend; ++l)
                            {
                                chararray _attachment_name;
                                const TYPE& timeline = slot(l, &_attachment_name);
                                chars attachment_name = &_attachment_name[0];

                                auto it = slot_attachments.find(attachment_name);

                                if (it == slot_attachments.end())
                                {
                                    delete Data;
                                    return nullptr;
                                }

                                ZAY::SceneData* sceneData = it->second.second;

                                if (dynamic_cast<ZAY::MeshData*>(sceneData) == nullptr)
                                {
                                    delete Data;
                                    return nullptr;
                                }
                                ZAY::MeshData* meshData = static_cast<ZAY::MeshData*>(sceneData);

                                int32_t vertices_count = 0;

                                if (meshData->getSkinningVertexCount() > 0)
                                    vertices_count = meshData->getSkinningVertexCount();
                                else vertices_count = meshData->getVertexPositions().getBufferSize();

                                ZAY::Animation* meshAnimation = new ZAY::Animation();
                                meshAnimation->autorelease();

                                meshData->setAnimation(animation_name, meshAnimation);

                                ZAY::AnimationTrackVector3Array* vertexPositionTrack = nullptr;

                                for(sint32 m = 0, mend = timeline.LengthOfIndexable(); m < mend; ++m)
                                {
                                    const TYPE& frame = timeline[m];

                                    float time = frame("time").GetFloat(0.0f);
                                    const TYPE& json_vertices = frame("vertices");
                                    sint32 json_offset = frame("offset").GetInt(0);
                                    const TYPE& frame_curve = frame("curve");

                                    if (vertexPositionTrack == nullptr)
                                    {
                                        vertexPositionTrack = ZAY::AnimationTrack::
                                            createAnimationTrackWithTypeCasting<ZAY::AnimationTrackVector3Array*>("AnimationTrack_MeshInstanceVertexPosition");
                                        vertexPositionTrack->setDefaultValue(ZAY::Vector3::ZERO);
                                        meshAnimation->addAnimationTrack(vertexPositionTrack);

                                        vertexPositionTrack->setArraySize(vertices_count);
                                    }

                                    // create keyframe
                                    ZAY::AnimationKeyFrameArray<ZAY::Vector3>* animationKeyFrame = nullptr;

                                    if(frame_curve.IsValid() && strcmp(frame_curve.GetString(), "stepped") == 0)
                                    {
                                        animationKeyFrame = new ZAY::AnimationKeyFrameArray_Vector3_Stepped();
                                        animationKeyFrame->autorelease();
                                        animationKeyFrame->setArraySize(vertices_count);
                                    }
                                    else if(frame_curve.IsValid() && 0 < frame_curve.LengthOfIndexable())
                                    {
                                        // bezier curve
                                        float cx1 = frame_curve[0].GetFloat();
                                        float cy1 = frame_curve[1].GetFloat();
                                        float cx2 = frame_curve[2].GetFloat();
                                        float cy2 = frame_curve[3].GetFloat();

                                        ZAY::AnimationKeyFrameArray_Vector3_SpineBezierToTenLinearFake* akf
                                            = new ZAY::AnimationKeyFrameArray_Vector3_SpineBezierToTenLinearFake();
                                        akf->autorelease();

                                        akf->setArraySize(vertices_count);

                                        akf->createSpineFakeLinears(cx1, cy1, cx2, cy2);

                                        animationKeyFrame = akf;
                                    }
                                    else
                                    {
                                        animationKeyFrame = new ZAY::AnimationKeyFrameArray_Vector3_Linear();
                                        animationKeyFrame->autorelease();
                                        animationKeyFrame->setArraySize(vertices_count);
                                    }

                                    // set array to keyframe
                                    if(!json_vertices.IsValid())
                                    {
                                        ZAY::Vector3* t = animationKeyFrame->getArrayPointer();
                                        //auto s = meshData->getVertexPositions().getBufferPointer();

                                        if (vertices_count > 0)
                                        {
                                            // offset
                                            Memory::Set(t, 0x00, sizeof(ZAY::Vector3) * vertices_count);

                                            // value
                                            //memcpy(t, s, sizeof(Vector3) * vertices_count);
                                        }
                                    }
                                    else
                                    {
                                        ZAY::Vector3* t = animationKeyFrame->getArrayPointer();
                                        //auto s = meshData->getVertexPositions().getBufferPointer();

                                        if (t == nullptr)
                                        {
                                            delete Data;
                                            return nullptr;
                                        }

                                        if (vertices_count > 0)
                                        {
                                            // offset
                                            Memory::Set(t, 0x00, sizeof(ZAY::Vector3) * vertices_count);

                                            // value
                                            //memcpy(t, s, sizeof(Vector3) * vertices_count);
                                        }

                                        int32_t offset = json_offset;
                                        for(sint32 n = 0, nend = json_vertices.LengthOfIndexable(); n < nend; ++n)
                                        {
                                            const TYPE& json_v = json_vertices[n];

                                            if ((offset % 2) == 0)
                                            {
                                                (t + (offset / 2))->x += json_v.GetFloat();
                                            }
                                            else
                                            {
                                                (t + (offset / 2))->y += json_v.GetFloat();
                                            }

                                            if ((offset / 2) >= vertices_count)
                                            {
                                                delete Data;
                                                return nullptr;
                                            }
                                        }
                                    }

                                    // set keyframe
                                    vertexPositionTrack->setKeyFrame(time, animationKeyFrame);
                                }

                                meshAnimation->findAndSetAnimationLengthWithTracks();
                            }
                        }
                    }
                }

                // bx
                if(json_animation_events.IsValid())
                {
                    ZAY::AnimationTrackString* track = ZAY::AnimationTrack::
                        createAnimationTrackWithTypeCasting<ZAY::AnimationTrackString*>("AnimationTrack_Event");
                    skeletonAnimation->addAnimationTrack(track);

                    for(sint32 j = 0, jend = json_animation_events.LengthOfIndexable(); j < jend; ++j)
                    {
                        const TYPE& event = json_animation_events[j];
                        chars event_name = event("name").GetString("unknown event");
                        float event_time = event("time").GetFloat(0.0f);

                        ZAY::AnimationKeyFrame_String_Stepped* animationKeyFrame = nullptr;
                        animationKeyFrame = new ZAY::AnimationKeyFrame_String_Stepped();
                        animationKeyFrame->autorelease();

                        animationKeyFrame->setValue((event_name)? event_name : ZAY::s_emptyString);
                        track->setKeyFrame(event_time, animationKeyFrame);
                    }
                }

                // draw order 데이터가 이해하기 힘듬.
                // spine runtime 가져다가 그대로 씀.
                // drawOrder [index] = order 인지
                // drawOrder [order] = index 인지 헷갈림
                // 일단 drawOrder [index] = order 로 처리함.
                if(json_animation_drawOrder.IsValid())
                {
                    int32_t slots_count = Data->getSlots().size();

                    ZAY::AnimationTrackTemplate<std::map<std::string, float>>* track = ZAY::AnimationTrack::
                        createAnimationTrackWithTypeCasting<ZAY::AnimationTrackTemplate<std::map<std::string, float>>*>("AnimationTrack_SkeletonInstanceSlotDrawOrder");

                    {
                        std::map<std::string, float> defaultDrawOrderMap;
                        for (auto it : defaultSlotIndexToSlotNames)
                        {
                            defaultDrawOrderMap[it.right] = static_cast<float>(it.left);
                        }
                        track->setDefaultValue(defaultDrawOrderMap);
                    }

                    skeletonAnimation->addAnimationTrack(track);

                    for(sint32 j = 0, jend = json_animation_drawOrder.LengthOfIndexable(); j < jend; ++j)
                    {
                        const TYPE& frame = json_animation_drawOrder[j];

                        float frame_time = frame("time").GetFloat(0.0f);
                        const TYPE& json_offsets = frame("offsets");

                        if(!json_offsets.IsValid())
                        {
                            track->setKeyFrame(frame_time, nullptr);
                        }
                        else
                        {
                            int32_t offsets_count = json_offsets.LengthOfIndexable();

                            int32_t* unchanged = new int32_t[slots_count - offsets_count];
                            int32_t* drawOrder = new int32_t[slots_count];

                            int32_t originalIndex = 0;
                            int32_t unchangedIndex = 0;

                            for(sint32 k = slots_count - 1; 0 <= k; --k)
                            {
                                drawOrder[k] = -1;
                            }

                            for(sint32 k = 0, kend = json_offsets.LengthOfIndexable(); k < kend; ++k)
                            {
                                const TYPE& json_offset = json_offsets[k];

                                chars slot_name = json_offset("slot").GetString(nullptr);
                                sint32 offset = json_offset("offset").GetInt(0);

                                auto it = defaultSlotIndexToSlotNames.right.find(slot_name);
                                if (it == defaultSlotIndexToSlotNames.right.end())
                                {
                                    delete Data;
                                    return nullptr;
                                }

                                auto slotIndex = it->second;

                                while (originalIndex != slotIndex)
                                    unchanged[unchangedIndex++] = originalIndex++;
                                drawOrder[originalIndex + offset] = originalIndex;
                                originalIndex++;
                            }

                            while (originalIndex < slots_count)
                                unchanged[unchangedIndex++] = originalIndex++;

                            for(sint32 k = slots_count - 1; 0 <= k; k--)
                                if(drawOrder[k] == -1)
                                    drawOrder[k] = unchanged[--unchangedIndex];

                            std::map<std::string, float> drawOrderMap;

                            for(sint32 k = 0; k < slots_count; k++)
                            {
                                auto it = defaultSlotIndexToSlotNames.left.find(drawOrder[k]);
                                if (it == defaultSlotIndexToSlotNames.left.end())
                                {
                                    delete Data;
                                    return nullptr;
                                }
                                drawOrderMap[it->second] = static_cast<float>(k);
                            }

                            ZAY::AnimationKeyFrame_MapStringToFloat_Stepped* keyframe = new ZAY::AnimationKeyFrame_MapStringToFloat_Stepped();
                            keyframe->setValue(drawOrderMap);

                            track->setKeyFrame(frame_time, keyframe);

                            delete [] unchanged;
                            delete [] drawOrder;
                        }
                    }
                }

                skeletonAnimation->findAndSetAnimationLengthWithTracks();
            }
        }

        for (auto it : defaultSlotAttachmentNames)
        {
            auto it2 = slotMaps.find(it.first);
            if (it2 == slotMaps.end())
            {
                delete Data;
                return nullptr;
            }

            it2->second->setDefaultAttachmentName(it.second);
        }

        return (id_spine) Data;
    }
}
