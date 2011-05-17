//===========================================================================//
/*
                                   LxEngine

    LICENSE

    Copyright (c) 2010-2011 athile@athile.net (http://www.athile.net)

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

//===========================================================================//
//   H E A D E R S
//===========================================================================//

#include <lx0/lxengine.hpp>
#include <lx0/core/lxvar/lxvar.hpp>
#include <glgeom/glgeom.hpp>

#include "lxvar_parser.hpp"

namespace lx0 { namespace core { namespace lxvar_ns {

    using namespace detail;

    namespace detail
    {
        lxundefined lxundefined::s_singleton;

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

        void 
        lxarray::at (int i, lxvar value)      
        {
            lx_check_error(i >= 0 && i < int(mValue.size()));
            mValue[i] = value;; 
        }

        lxvalue*    
        lxstringmap::clone (void) const
        {
            lxstringmap* pClone = new lxstringmap;
            for (auto it = mValue.begin(); it != mValue.end(); ++it)
                pClone->mValue.insert( std::make_pair(it->first, it->second) );
            return pClone;
        }

        lxvar
        lxstringmap::find (const char* key) const
        {
            auto it = mValue.find(key);
            if (it != mValue.end())
                return it->second;
            else
                return lxvar();
        }

        void
        lxvalue::_invalid (void) const
        {
            lx_error("Invalid operation for lxvar type");
        }


        void
        lxvalue_iterator::_invalid (void) const
        {
            lx_error("Invalid operation for lxvar iterator type");
        }
    }

    lxvar::iterator::iterator (void)
        : mValue(new lxvalue_iterator)
    {
    }

    lxvar::iterator::iterator (const lxvar::iterator& that) 
        : mValue( that.mValue->clone() ) 
    {
    }

    void        
    lxvar::iterator::operator= (const lxvar::iterator& that) 
    { 
        mValue.reset( that.mValue->clone() ); 
    }

    bool
    lxvar::iterator::operator== (iterator& that)
    {
        return mValue->equal(*that.mValue);
    }

    bool
    lxvar::iterator::operator!= (iterator& that)
    {
        return !mValue->equal(*that.mValue);
    }

    void
    lxvar::iterator::operator++ (void)
    {
        return mValue->inc();
    }

    void
    lxvar::iterator::operator++ (int)
    {
        return mValue->inc();
    }

    lxvar 
    lxvar::iterator::operator-> (void)
    {
        return mValue->dereference();
    }

    lxvar 
    lxvar::iterator::operator* (void)
    {
        return mValue->dereference();
    }

    std::string 
    lxvar::iterator::key (void)
    {
        return mValue->key();
    }

    lxvar::lxvar()
        : mValue( lxundefined::acquire() )
    {
    }

    lxvar::lxvar (const lxvar& that)
        : mValue( lxundefined::acquire() )
    {
        if (that.mValue->sharedType())
            mValue = that.mValue;
        else
            mValue = that.mValue->clone();
    }

    lxvar::lxvar (detail::lxvalue* imp)
        : mValue ( imp )
    {
        if (!imp->sharedType())
            mValue = mValue->clone();
    }

    lxvar::lxvar(int i)
        : mValue( new lxint(i) )
    {
    }

    lxvar::lxvar(int a, int b)
        : mValue( lxundefined::acquire() )
    {
        push(a);
        push(b);
    }

    lxvar::lxvar(int a, int b, int c)
        : mValue( lxundefined::acquire() )
    {
        push(a);
        push(b);
        push(c);
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

    lxvar::lxvar(float a, float b, float c, float d)
        : mValue( lxundefined::acquire() )
    {
        push(a);
        push(b);
        push(c);
        push(d);
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

    bool
    lxvar::isShared () const
    {
        return (mValue->_refCount() > 0);
    }

    bool
    lxvar::isSharedType () const
    {
        return mValue->sharedType();
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

    lxvar::iterator
    lxvar::begin (void)
    {
        return mValue->begin();
    }

    lxvar::iterator
    lxvar::end (void)
    {
        return mValue->end();
    }

    /*!
        Converts the undefined variable to an empty map.
     */
    void       
    lxvar::toArray (void)
    {
        _castTo<lxarray>();
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
    lxvar::at (int index, lxvar value)
    {
        return mValue->at(index, value);
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
        Returns if the current lxvar is actually an opaque handle to a 
        native C++ object.  
     */
    bool
    lxvar::isHandle (void) const
    {
        return mValue->isHandle();
    }

    /*!
        This method is sub-optimal in terms of design: it is effectively introducing
        a new type system, as each natively wrapped object needs to be given a unique
        name - which may or may not correspond to the name it is given in C++ or
        Javascript.

        Yet - do something more automated requires adding a dependency between lxvar and
        the runtime type system, which is possibly worse since this is supposed to be a
        small, isolated, encapsulated class.
     */
    std::string
    lxvar::handleType (void) const
    {
        return mValue->handleType();
    }

    void*
    lxvar::unwrap (void)
    {
        return mValue->unwrap();
    }

    /*!
        Converts the undefined variable to an empty map.
     */
    void       
    lxvar::toMap (void)
    {
        _castTo<lxstringmap>();
    }

    bool
    lxvar::containsKey (const char* key) const
    {
         auto map = _castTo<lxstringmap>()->mValue;
         return map.find(key) != map.end();
    }

    lxvar
    lxvar::find (const std::string& s) const
    {
        return find(s.c_str());
    }

    lxvar
    lxvar::find (const char* key) const
    {
        return mValue->find(key);
    }

    void 
    lxvar::insert (const char* key, const lxvar& value)
    {
        _castTo<lxstringmap>()->mValue.insert(std::make_pair(key, value));
    }

    lxvar    
    lxvar::parse (const char* s)
    {
        lx0::core::detail::LxsonParser parser;
        return parser.parse(s);
    }

}}}
