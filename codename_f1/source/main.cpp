﻿#include <boss.hpp>
#include <platform/boss_platform.hpp>
#include <service/boss_zay.hpp>

#include <resource.hpp>
#include "classes.hpp"

// 빌드시간 참고용 헤더
#include "../source-gen/buildtime.h"

void PlatformInit()
{
    #if BOSS_WINDOWS || BOSS_LINUX || BOSS_MAC_OSX
        // buildtime.h의 빌드시간을 현재 빌드버전으로 로드
        Platform::Option::SetText("BuildVersion", BuildTime);
        // 다음 빌드를 위해 buildtime.h를 갱신
        if(id_file BuildTimeFile = Platform::File::OpenForWrite("../source-gen/buildtime.h"))
        {
            id_clock BuildTimeClock = Platform::Clock::CreateAsCurrent();
            sint32 Year = 0, Month = 0, Day = 0, Hour = 0, Min = 0, Sec = 0;
            Platform::Clock::GetDetail(BuildTimeClock, nullptr, &Sec, &Min, &Hour, &Day, &Month, &Year);
            Platform::Clock::Release(BuildTimeClock);
            const String BuildTimeText = String::Format(
                "static const char* BuildTime = \"%s/ %04d-%02d-%02d/ %02d:%02d:%02d\";\r\n",
                Platform::Utility::GetOSName(), Year, Month, Day, Hour, Min, Sec);
            Platform::File::Write(BuildTimeFile, (bytes) "\xef\xbb\xbf", 3); // UTF-8 Bom: EF BB BF
            Platform::File::Write(BuildTimeFile, (bytes)(chars) BuildTimeText, BuildTimeText.Length());
            Platform::File::Close(BuildTimeFile);
        }
        else // buildtime.h가 없으면 개발상황이 아니라 배포된 빌드
        {
            // Data폴더를 Asset쓰기용폴더로 지정
            Platform::File::ResetAssetsRemRoot(Platform::File::RootForData());
        }
    #else
        Platform::Option::SetText("BuildVersion",
            String::Format("%s/ %s/ %s", Platform::Utility::GetOSName(), __DATE__, __TIME__));
    #endif

    Platform::InitForGL();
    Platform::SetViewCreator(ZayView::Creator);

    if(auto FirstSaver = FXSaver::Sync("save.json"))
    {
        FirstSaver->At("DevMode").Set("0");
        FirstSaver->At("SoundFlag").Set("1");
        FirstSaver->At("BGMFlag").Set("1");
        FirstSaver->At("LastStageJson").Set("f1/table/stage_tutorial.json");
        FirstSaver->At("LastStageID").Set("Stage1");
        FXSaver::Update();
    }
    Platform::Option::SetText("ParaTalkCount", "0");
    Platform::Option::SetText("ParaViewCount", "0");
    Platform::Option::SetFlag("LandscapeMode", false);
    Platform::Option::SetFlag("DirectPlay", false);

    if(FXSaver::Read("DevMode").GetInt())
        Platform::SetWindowView("codename_f1View");
    else Platform::SetWindowView("ingameView");

    String InfoString = String::FromAsset("windowinfo.json");
    if(0 < InfoString.Length())
    {
        Context Info(ST_Json, SO_OnlyReference, InfoString, InfoString.Length());
        Platform::SetWindowPos(Info("x").GetInt(0), Info("y").GetInt(0));
        Platform::SetWindowSize(Info("w").GetInt(640), Info("h").GetInt(480));
    }
    else Platform::SetWindowSize(475, 844);

    String AtlasInfoString = String::FromAsset("atlasinfo.json");
    Context AtlasInfo(ST_Json, SO_OnlyReference, AtlasInfoString, AtlasInfoString.Length());
    R::SetAtlasDir("f1/image");
    R::AddAtlas("ui_atlaskey.png", "atlas_latest.png", AtlasInfo);
    if(R::IsAtlasUpdated())
        R::RebuildAll();

    Platform::AddWindowProcedure(WE_Tick,
        [](payload data)->void
        {
            static uint64 LastUpdateCheckTime = Platform::Utility::CurrentTimeMsec();
            uint64 CurUpdateCheckTime = Platform::Utility::CurrentTimeMsec();
            if(LastUpdateCheckTime + 1000 < CurUpdateCheckTime)
            {
                LastUpdateCheckTime = CurUpdateCheckTime;
                if(R::IsAtlasUpdated())
                {
                    R::RebuildAll();
                    Platform::UpdateAllViews();
                }
            }
        });
}

void PlatformQuit()
{
    rect128 WindowRect;
    Platform::GetWindowRect(WindowRect);

    Context Info;
    Info.At("x").Set(String::FromInteger(WindowRect.l));
    Info.At("y").Set(String::FromInteger(WindowRect.t));
    Info.At("w").Set(String::FromInteger(WindowRect.r - WindowRect.l));
    Info.At("h").Set(String::FromInteger(WindowRect.b - WindowRect.t));
    Info.SaveJson().ToAsset("windowinfo.json");

    Context AtlasInfo;
    R::SaveAtlas(AtlasInfo);
    AtlasInfo.SaveJson().ToAsset("atlasinfo.json");

    // 글로벌 리소스의 제거
    FXData::ClearAll();
}
