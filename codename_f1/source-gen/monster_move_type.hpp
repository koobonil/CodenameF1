class monster_move_type
{
public:
    enum Type {Straight, Max, Null = -1};

public:
    monster_move_type() {mName = Null;}
    monster_move_type(const monster_move_type& rhs) {operator=(rhs);}
    monster_move_type(chars rhs) {operator=(rhs);}
    monster_move_type& operator=(const monster_move_type& rhs) {mName = rhs.mName; return *this;}
    monster_move_type& operator=(Type rhs) {mName = rhs; return *this;}
    monster_move_type& operator=(chars rhs)
    {
        if(false) {}
        else if(!String::Compare(rhs, "Straight")) // 일반 길찾기 직선 이동
            mName = Straight;
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
