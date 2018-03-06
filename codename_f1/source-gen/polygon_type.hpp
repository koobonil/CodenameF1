class polygon_type
{
public:
    enum Type {Wall, DynamicChain, Max, Null = -1};

public:
    polygon_type() {mName = Null;}
    polygon_type(const polygon_type& rhs) {operator=(rhs);}
    polygon_type(chars rhs) {operator=(rhs);}
    polygon_type& operator=(const polygon_type& rhs) {mName = rhs.mName; return *this;}
    polygon_type& operator=(Type rhs) {mName = rhs; return *this;}
    polygon_type& operator=(chars rhs)
    {
        if(false) {}
        else if(!String::Compare(rhs, "Wall")) // 못가는 일반벽의 폴리곤
            mName = Wall;
        else if(!String::Compare(rhs, "DynamicChain")) // 파괴되는 벽의 폴리곤
            mName = DynamicChain;
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
