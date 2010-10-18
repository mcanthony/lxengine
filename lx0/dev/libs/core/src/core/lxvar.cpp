#include <lx0/core.hpp>
#include <lx0/lxvar.hpp>


namespace lx0 { namespace core {

    using namespace detail;

    namespace detail
    {
        std::weak_ptr<lxundefined> lxundefined::uwpSingleton;

        lxvalue*    
        lxarray::clone (void) const
        {
            lxarray* pClone = new lxarray;
            pClone->mValues.reserve(mValues.size());
            for (auto it = mValues.begin(); it != mValues.end(); ++it)
                pClone->mValues.push_back(it->clone());
            return pClone;
        }

        lxvalue*    
        lxstringmap::clone (void) const
        {
            lxstringmap* pClone = new lxstringmap;
            for (auto it = mValue.begin(); it != mValue.end(); ++it)
                pClone->mValue.insert( std::make_pair(it->first, it->second) );
            return pClone;
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
    {
        lxstring* t = new lxstring;
        t->mValue = s;
        mValue.reset(t);
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
    lxvar::_castTo ()
    {
        // mValue should always be set to a value.  lxundefined represents the case where
        // the lxvar does not point to any object or value.
        lxvalue* pBase = mValue.get();
        lx_assert(pBase != nullptr);

        if (typeid(*pBase) == typeid(lxundefined))
        {
            T* pNew = new T;
            mValue.reset(pNew);
            return pNew;
        }
        else
        {
            T* pDerived = dynamic_cast<T*>(pBase);
            if (!pDerived)
                error("lxvar treated as incompatible type");

            return pDerived;
        }
    }

    void
    lxvar::push (const lxvar& v)
    {
        lxarray* pArray = _castTo<lxarray>();
        pArray->mValues.push_back(v);
    }

    void 
    lxvar::insert (const char* key, const lxvar& value)
    {
        _castTo<lxstringmap>()->mValue.insert(std::make_pair(key, value));
    }

}};