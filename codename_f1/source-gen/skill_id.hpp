class skill_id
{
public:
    enum Type {NoSkill, BreathSlow, BreathKnockBackUp, BreathStun, Max, Null = -1};

public:
    skill_id() {mName = Null;}
    skill_id(const skill_id& rhs) {operator=(rhs);}
    skill_id(chars rhs) {operator=(rhs);}
    skill_id& operator=(const skill_id& rhs) {mName = rhs.mName; return *this;}
    skill_id& operator=(Type rhs) {mName = rhs; return *this;}
    skill_id& operator=(chars rhs)
    {
        if(false) {}
        else if(!String::Compare(rhs, "NoSkill")) // 스킬이 없음
            mName = NoSkill;
        else if(!String::Compare(rhs, "BreathSlow")) // 브레스에 맞으면 몬스터가 느려지는 효과
            mName = BreathSlow;
        else if(!String::Compare(rhs, "BreathKnockBackUp")) // 브레스에 맞으면 몬스터의 넉백거리가 길어지는 효과
            mName = BreathKnockBackUp;
        else if(!String::Compare(rhs, "BreathStun")) // 브레스에 맞으면 몬스터가 경직되는 효과
            mName = BreathStun;
        else
        {
            mName = Null;
            if(!String::Compare(rhs, "Null"))
                BOSS_ASSERT("No valid keywords", false);
            else BOSS_ASSERT("Unknown keyword", false);
        }
        return *this;
    }
    bool operator==(Type rhs) const
    {return (mName == rhs);}
    bool operator!=(Type rhs) const
    {return (mName != rhs);}

protected:
    Type mName;
};
