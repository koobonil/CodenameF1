#pragma once
#include <service/boss_zay.hpp>
#include <service/boss_batchbuilder.hpp>

class sourcetoolData : public ZayObject
{
    BOSS_DECLARE_NONCOPYABLE_CLASS(sourcetoolData)
public:
    sourcetoolData()
    {
        OptionFileEnabled = false;
        TargetFolderEnabled = false;
    }
    ~sourcetoolData()
    {
    }

public:
    void Run()
    {
        BatchBuilder Builder(TargetFolder);
        Builder.Processing(OptionFile);
    }

public:
    String OptionFile;
    bool OptionFileEnabled;
    String TargetFolder;
    bool TargetFolderEnabled;
};
