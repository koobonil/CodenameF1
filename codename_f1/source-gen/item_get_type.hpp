class item_get_type
{
public:
    enum Type {Drop, Max, Null = -1};

public:
    item_get_type() {mName = Null;}
    item_get_type(const item_get_type& rhs) {operator=(rhs);}
    item_get_type(chars rhs) {operator=(rhs);}
    item_get_type& operator=(const item_get_type& rhs) {mName = rhs.mName; return *this;}
    item_get_type& operator=(Type rhs) {mName = rhs; return *this;}
    item_get_type& operator=(chars rhs)
    {
        if(false) {}
        else if(!String::Compare(rhs, "Drop")) // 바닥에 떨어지는 아이템 타입
            mName = Drop;
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
