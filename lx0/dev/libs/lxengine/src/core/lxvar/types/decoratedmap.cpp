//===========================================================================//
/*
                                   LxEngine

    LICENSE

    Copyright (c) 2011 athile@athile.net (http://www.athile.net)

    Permission is hereby granted, free of charge, to any person obtaining a 
    copy of this software and associated documentation files (the "Software"), 
    to deal in the Software without restriction, including without limitation 
    the rights to use, copy, modify, merge, publish, distribute, sublicense, 
    and/or sell copies of the Software, and to permit persons to whom the 
    Software is furnished to do so, subject to the following conditions:

    The above copyright notice and this permission notice shall be included in
    all copies or substantial portions of the Software.

    THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
    IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
    FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
    AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
    LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING 
    FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS 
    IN THE SOFTWARE.
*/
//===========================================================================//

#include <functional>

#include <lx0/core/lxvar/lxvar.hpp>

namespace lx0 
{ 
    namespace core 
    { 
        namespace lxvar_ns 
        {
            namespace detail
            {

//===========================================================================//

class lxdecoratedmap : public lxvalue
{
public:
    class Value
    {
    public:
        Value() : mFlags(0) {}
        Value (lx0::uint32 f, ValidateFunction vf, lxvar& v)
            : mFlags(f)
            , mValue (v)
            , mValidate(vf)
        {
        }
        lx0::uint32         mFlags;
        lxvar               mValue;
        ValidateFunction    mValidate;
    };

    typedef std::map<std::string, Value> Map;

    class iterator_imp : public lxvalue_iterator
    {
    public:
        iterator_imp (Map::iterator it) : mIter(it) {}
        virtual lxvalue_iterator* clone     (void)                         { return new iterator_imp(mIter); }

        virtual bool    equal               (const lxvalue_iterator& that) { return mIter == dynamic_cast<const iterator_imp&>(that).mIter; }
        virtual void    inc                 (void)                         { mIter++; }
        virtual std::string key             (void)                         { return mIter->first; }
        virtual lxvar   dereference         (void)                         { return mIter->second.mValue; }

    protected:
        Map::iterator    mIter;
    };


    virtual bool        sharedType  (void) const { return true; }
    virtual lxvalue*    clone       (void) const;

    virtual bool        is_map      (void) const    { return true; }

    virtual int         size        (void) const { return int( mMap.size() ); }

    lxvar::iterator     begin           (void) { return lxvar::iterator(new iterator_imp(mMap.begin())); }
    lxvar::iterator     end             (void) { return lxvar::iterator(new iterator_imp(mMap.end())); }

    virtual bool        has         (const char* key) const { return mMap.find(key) != mMap.end(); }
    virtual lxvar*      find        (const char* key) const;
    virtual void        insert      (const char* key, lxvar& value);

    virtual void        add         (const char* key, lx0::uint32 flags, ValidateFunction validate)
    {
        Value v;
        v.mFlags = flags;
        v.mValidate = validate;
        v.mValue = lxvar::undefined();

        mMap.erase(key);
        mMap.insert(std::make_pair(key, v));
    }
    virtual lx0::uint32 flags       (const char* key)
    {
        auto it = mMap.find(key);
        if (it != mMap.end())
            return it->second.mFlags;
        else
            return 0;
    }

    Map     mMap;
};

lxvalue*    
lxdecoratedmap::clone (void) const
{
    auto* pClone = new lxdecoratedmap;
    for (auto it = mMap.begin(); it != mMap.end(); ++it)
        pClone->mMap.insert( std::make_pair(it->first, it->second) );
    return pClone;
}

lxvar*
lxdecoratedmap::find (const char* key) const
{
    auto it = mMap.find(key);
    if (it != mMap.end())
        return const_cast<lxvar*>(&it->second.mValue);
    else
        return nullptr;
}

void 
lxdecoratedmap::insert (const char* key, lxvar& value) 
{
    auto it = mMap.find(key);
    
    if (it != mMap.end())
    {
        if (it->second.mValidate)
        {
            lxvar newvalue;
            if (it->second.mValidate(value, newvalue))
                it->second.mValue = newvalue;
        }
        else
            it->second.mValue = value;
    }
    else
        mMap.insert(std::make_pair(key, Value(0, ValidateFunction(), value)));
}


lxvalue* create_lxdecoratedmap() { return new lxdecoratedmap; }

            }
        }
    }
}