#include <boss.hpp>
#include <platform/boss_platform.hpp>
#include <service/boss_zay.hpp>

#include <resource.hpp>
#include "classes.hpp"

// 빌드시간
#include "../source-gen/buildtime.h"
void SetBuildTime()
{
    #if BOSS_WINDOWS || BOSS_LINUX || BOSS_MAC_OSX
        // 빌드시간을 빌드버전으로 기록
        Platform::Option::SetText("BuildVersion", BuildTime);
        // 빌드시간을 갱신저장
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
        else BOSS_ASSERT("프로젝트의 빌드시간을 갱신저장할 수 없습니다", false);
    #else
        Platform::Option::SetText("BuildVersion",
            String::Format("%s/ %s/ %s", Platform::Utility::GetOSName(), __DATE__, __TIME__));
    #endif
}

void PlatformInit()
{
    SetBuildTime();
    Platform::InitForGL();
    Platform::SetViewCreator(ZayView::Creator);

    Context SaveFile;
    String SaveString = String::FromFile("save.json");
    if(0 < SaveString.Length())
        SaveFile.LoadJson(SO_NeedCopy, SaveString, SaveString.Length());
    else
    {
        SaveFile.At("DevMode").Set("0");
        SaveFile.At("LastStageJson").Set("f1/table/stage_tutorial.json");
        SaveFile.At("LastStageID").Set("");
        SaveFile.SaveJson().ToFile("save.json");
    }

    Platform::Option::SetFlag("DevMode", SaveFile("DevMode").GetInt());
    Platform::Option::SetText("StageName", SaveFile("LastStageJson").GetString());
    Platform::Option::SetText("LastStageID", SaveFile("LastStageID").GetString());
    Platform::Option::SetText("ParaTalkCount", "0");
    Platform::Option::SetText("ParaViewCount", "0");
    Platform::Option::SetFlag("LandscapeMode", false);
    Platform::Option::SetFlag("DirectPlay", false);

    if(Platform::Option::GetFlag("DevMode"))
        Platform::SetWindowView("codename_f1View");
    else Platform::SetWindowView("ingameView");

    String InfoString = String::FromFile("windowinfo.json");
    if(0 < InfoString.Length())
    {
        Context Info(ST_Json, SO_OnlyReference, InfoString, InfoString.Length());
        Platform::SetWindowPos(Info("x").GetInt(0), Info("y").GetInt(0));
        Platform::SetWindowSize(Info("w").GetInt(640), Info("h").GetInt(480));
    }
    else Platform::SetWindowSize(475, 844);

    String AtlasInfoString = String::FromFile("atlasinfo.json");
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
    Info.SaveJson().ToFile("windowinfo.json");

    Context AtlasInfo;
    R::SaveAtlas(AtlasInfo);
    AtlasInfo.SaveJson().ToFile("atlasinfo.json");

    // 글로벌 리소스의 제거
    FXData::ClearAll();
}
