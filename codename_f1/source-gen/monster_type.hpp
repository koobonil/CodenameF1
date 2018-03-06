class monster_type
{
public:
    enum Type {Enemy, Ally, Max, Null = -1};

public:
    monster_type() {mName = Null;}
    monster_type(const monster_type& rhs) {operator=(rhs);}
    monster_type(chars rhs) {operator=(rhs);}
    monster_type& operator=(const monster_type& rhs) {mName = rhs.mName; return *this;}
    monster_type& operator=(Type rhs) {mName = rhs; return *this;}
    monster_type& operator=(chars rhs)
    {
        if(false) {}
        else if(!String::Compare(rhs, "Enemy")) // 브레스에 죽는 일반 몬스터 타입
            mName = Enemy;
        else if(!String::Compare(rhs, "Ally")) // 터치하면 효과를 발휘하는 아군 타입
            mName = Ally;
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
