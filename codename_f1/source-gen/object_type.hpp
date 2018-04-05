class object_type
{
public:
    enum Type {Static, Wall, Dynamic, Trigger, Target, AllyTarget, View, Ground, Spot, AllySpot, Hole, Max, Null = -1};

public:
    object_type() {mName = Null;}
    object_type(const object_type& rhs) {operator=(rhs);}
    object_type(chars rhs) {operator=(rhs);}
    object_type& operator=(const object_type& rhs) {mName = rhs.mName; return *this;}
    object_type& operator=(Type rhs) {mName = rhs; return *this;}
    object_type& operator=(chars rhs)
    {
        if(false) {}
        else if(!String::Compare(rhs, "Static")) // 길찾기에 영향을 주고 고정되어 있는 타입
            mName = Static;
        else if(!String::Compare(rhs, "Wall")) // 벽타입
            mName = Wall;
        else if(!String::Compare(rhs, "Dynamic")) // Static'과 동일하지만 파괴되는 타입
            mName = Dynamic;
        else if(!String::Compare(rhs, "Trigger")) // 타임라인 이벤트의 오브젝트
            mName = Trigger;
        else if(!String::Compare(rhs, "Target")) // 몬스터가 길찾기의 '목표'가 될 타입
            mName = Target;
        else if(!String::Compare(rhs, "AllyTarget")) // 아군몬스터가 길찾기의 '목표'가 될 타입
            mName = AllyTarget;
        else if(!String::Compare(rhs, "View")) // 광고판
            mName = View;
        else if(!String::Compare(rhs, "Ground")) // 길찾기에 영향을 받지 않는 바닥에 깔리는 타입
            mName = Ground;
        else if(!String::Compare(rhs, "Spot")) // 몬스터가 출현하는 오브젝트 타입
            mName = Spot;
        else if(!String::Compare(rhs, "AllySpot")) // 아군몬스터가 출현하는 오브젝트 타입
            mName = AllySpot;
        else if(!String::Compare(rhs, "Hole")) // 길찾기에 영향을 받고 몬스터가 닿게 되면 떨어지는 타입
            mName = Hole;
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
