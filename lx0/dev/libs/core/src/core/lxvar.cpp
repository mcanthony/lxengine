#include <lx0/core.hpp>
#include <lx0/lxvar.hpp>
#include <lx0/point3.hpp>

namespace lx0 { namespace core {

    using namespace detail;

    namespace detail
    {
        std::weak_ptr<lxundefined> lxundefined::uwpSingleton;

        lxvalue*    
        lxarray::clone (void) const
        {
            lxarray* pClone = new lxarray;
            pClone->mValue.reserve(mValue.size());
            for (auto it = mValue.begin(); it != mValue.end(); ++it)
                pClone->mValue.push_back(it->clone());
            return pClone;
        }

        lxvar 
        lxarray::at (int i)      
        {
            lx_check_error(i >= 0 && i < int(mValue.size()));
            return mValue[i]; 
        }

        lxvalue*    
        lxstringmap::clone (void) const
        {
            lxstringmap* pClone = new lxstringmap;
            for (auto it = mValue.begin(); it != mValue.end(); ++it)
                pClone->mValue.insert( std::make_pair(it->first, it->second) );
            return pClone;
        }

        void
        lxvalue::_invalid (void) const
        {
            lx_error("Invalid operation for lxvar type");
        }
    }


    lxvar::lxvar()
        : mValue( lxundefined::acquire() )
    {
    }

    lxvar::lxvar (const lxvar& that)
        : mValue( lxundefined::acquire() )
    {
        if (mValue->sharedType())
            mValue = that.mValue;
        else
        {
            mValue = lxundefined::acquire();
            *this = that.clone();
        }
    }

    lxvar::lxvar(int i)
        : mValue( new lxint(i) )
    {
    }

    lxvar::lxvar(int a, int b, int c, int d)
        : mValue( lxundefined::acquire() )
    {
        push(a);
        push(b);
        push(c);
        push(d);
    }

    lxvar::lxvar(float a)
        : mValue ( new lxfloat(a) )
    {
    }

    lxvar::lxvar(float a, float b, float c)
        : mValue( lxundefined::acquire() )
    {
        push(a);
        push(b);
        push(c);
    }

    lxvar::lxvar (const char* s)
        : mValue(new lxstring(s))
    {
    }

    lxvar::lxvar (std::string s)
        : mValue(new lxstring(s))
    {   
    }

    lxvar
    lxvar::undefined (void)
    {
        return lxvar();
    }

    lxvar
    lxvar::map (void)
    {
        lxvar v;
        v._castTo<lxstringmap>();
        return v;
    }

    lxvar
    lxvar::array (void)
    {
        lxvar v;
        v._castTo<lxarray>();
        return v;
    }

    void 
    lxvar::undefine (void)
    {
        mValue = lxundefined::acquire();
    }

    lxvar
    lxvar::clone () const
    {
        lxvar deep;
        deep.mValue.reset( mValue->clone() );
        return deep;
    }

    template <typename T>
    T*    
    lxvar::_castTo () const
    {
        // mValue should always be set to a value.  lxundefined represents the case where
        // the lxvar does not point to any object or value.
        lxvalue* pBase = mValue.get();
        lx_assert(pBase != nullptr);

        if (_isType<lxundefined>())
        {
            T* pNew = new T;
            mValue.reset(pNew);
            return pNew;
        }
        else
        {
            T* pDerived = dynamic_cast<T*>(pBase);
            if (!pDerived)
                lx_error("lxvar treated as incompatible type");

            return pDerived;
        }
    }

    int 
    lxvar::asInt (void) const
    {
        return _castTo<lxint>()->mValue;
    }

    float 
    lxvar::asFloat (void) const
    {
        if (_isType<lxint>())
            return float( _castTo<lxint>()->mValue );
        else
            return _castTo<lxfloat>()->mValue;
    }

    std::string 
    lxvar::asString (void) const
    {
        return _castTo<lxstring>()->mValue;
    }

    bool    
    lxvar::equal (const char* s) const
    {
        return (isString() && asString() == s);
    }

    /*!
        Converts the undefined variable to an empty map.
     */
    void       
    lxvar::toArray (void)
    {
        _castTo<lxarray>();
    }

    lxvar::ArrayIterator
    lxvar::beginArray (void)
    {
        return _castTo<lxarray>()->mValue.begin();
    }

    lxvar::ArrayIterator 
    lxvar::endArray (void)
    {
        return _castTo<lxarray>()->mValue.end();
    }

    int
    lxvar::size (void) const
    {
        return mValue->size();
    }

    lxvar 
    lxvar::at (int index) const
    {
        return mValue->at(index);
    }

    void
    lxvar::push (const lxvar& v)
    {
        lxarray* pArray = _castTo<lxarray>();
        pArray->mValue.push_back(v);
    }

    bool
    lxvar::isMap (void) const
    {
        return _isType<lxstringmap>();
    }

    /*!
        Converts the undefined variable to an empty map.
     */
    void       
    lxvar::toMap (void)
    {
        _castTo<lxstringmap>();
    }

    lxvar::MapIterator
    lxvar::beginMap (void)
    {
        return _castTo<lxstringmap>()->mValue.begin();
    }

    lxvar::MapIterator 
    lxvar::endMap (void)
    {
        return _castTo<lxstringmap>()->mValue.end();
    }

    bool
    lxvar::containsKey (const char* key) const
    {
         auto map = _castTo<lxstringmap>()->mValue;
         return map.find(key) != map.end();
    }

    lxvar
    lxvar::find (const char* key) const
    {
        auto map = _castTo<lxstringmap>()->mValue;
        auto it = map.find(key);
        if (it != map.end())
            return it->second;
        else
            return lxvar();
    }

    void 
    lxvar::insert (const char* key, const lxvar& value)
    {
        _castTo<lxstringmap>()->mValue.insert(std::make_pair(key, value));
    }


    point3 
    asPoint3 (const lxvar& lx)
    {
        lx_check_error(lx.isArray());
        lx_check_error(lx.size() == 3);

        point3 p;
        p.x = lx.at(0).asFloat();
        p.y = lx.at(1).asFloat();
        p.z = lx.at(2).asFloat();
        return p;
    }
}};