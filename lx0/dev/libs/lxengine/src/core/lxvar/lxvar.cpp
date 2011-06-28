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
#include <boost/format.hpp>

#include "lxvar_parser.hpp"

namespace lx0 { namespace core { namespace lxvar_ns {

    using namespace detail;

    namespace detail
    {

        //===========================================================================//

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

    //===========================================================================//

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

    //===========================================================================//

    lxvar::lxvar()
        : mValue( create_lxundefined() )
    {
    }

    lxvar::lxvar (const lxvar& that)
        : mValue( create_lxundefined() )
    {
        if (that.mValue->sharedType())
            mValue = that.mValue;
        else
            mValue = that.mValue->clone();
    }

    const lxvar& lxvar::operator= (const lxvar& that)
    {
        if (that.mValue->sharedType())
            mValue = that.mValue;
        else
            mValue = that.mValue->clone();

        return *this;
    }


    lxvar::lxvar (detail::lxvalue* imp)
        : mValue ( imp )
    {
        if (!imp->sharedType())
            mValue = mValue->clone();
    }

    lxvar::lxvar (bool b)
        : mValue( create_lxbool(b) )
    {
    }

    lxvar::lxvar(int i)
        : mValue( create_lxint(i) )
    {
    }

    lxvar::lxvar(int a, int b)
        : mValue( create_lxarray() )
    {
        push(a);
        push(b);
    }

    lxvar::lxvar(int a, int b, int c)
        : mValue( create_lxarray() )
    {
        push(a);
        push(b);
        push(c);
    }

    lxvar::lxvar(int a, int b, int c, int d)
        : mValue( create_lxarray() )
    {
        push(a);
        push(b);
        push(c);
        push(d);
    }

    lxvar::lxvar(float a)
        : mValue ( create_lxfloat(a) )
    {
    }

    lxvar::lxvar(float a, float b, float c)
        : mValue( create_lxarray() )
    {
        push(a);
        push(b);
        push(c);
    }

    lxvar::lxvar(float a, float b, float c, float d)
        : mValue( create_lxarray() )
    {
        push(a);
        push(b);
        push(c);
        push(d);
    }

    lxvar::lxvar (const char* s)
        : mValue(create_lxstring(s))
    {
    }

    lxvar::lxvar (std::string s)
        : mValue(create_lxstring(s))
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
        return lxvar(create_lxstringmap());
    }

    /*!
        Creates a map that retains the original insertion order upon iteration of the map.

        This implementation requires overhead than an unordered map.  
     */
    lxvar
    lxvar::ordered_map (void)
    {
        return lxvar(create_lxorderedmap());
    }

    /*!
        A decorated map is a work-in-progress class designed as standard string-based
        map that adds practical features such as different types of flags and validation 
        for specific keys in the map.  This is particularly useful for settings maps.

        For example, the "view_width" key could be set to have a validator that only
        allows 320 to 1024 as value values.  Or it could do the same but automatically
        clamp out-of-range values.
     */
    lxvar
    lxvar::decorated_map (void)
    {
        return lxvar(create_lxdecoratedmap());
    }

    lxvar
    lxvar::array (void)
    {
        return lxvar(create_lxarray());
    }

    bool
    lxvar::isShared () const
    {
        return (mValue->_refCount() > 1);
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


    int
    lxvar::size (void) const
    {
        return mValue->size();
    }

    lxvar 
    lxvar::at (int index) const
    {
        auto p = mValue->at(index);
        return p ? *p : lxvar::undefined();
    }

    void
    lxvar::at (int index, lxvar value)
    {
        return mValue->at(index, value);
    }

    void
    lxvar::push (const lxvar& v)
    {
        if (is_undefined())
            *this = array();

        mValue->push(v);
    }

    bool
    lxvar::is_undefined (void) const
    {
        return mValue->is_undefined();
    }

    bool
    lxvar::is_int (void) const
    {
        return mValue->is_int();
    }

    bool
    lxvar::is_float (void) const
    {
        return mValue->is_float();
    }

    bool
    lxvar::is_string (void) const
    {
        return mValue->is_string();
    }

    bool
    lxvar::is_array (void) const
    {
        return mValue->is_array();
    }

    bool
    lxvar::is_map (void) const
    {
        return mValue->is_map();
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

    bool
    lxvar::has (const char* key) const
    {
        return mValue->has(key);
    }

    lxvar&
    lxvar::operator[] (int i)
    {
        if (!is_defined()) 
            *this = array();

        return *mValue->at(i);
    }

    lxvar&
    lxvar::operator[] (const char* s)
    {
        if (!is_defined()) 
            *this = map();

        auto p = mValue->find(s);
        if (!p)
        {
            mValue->insert(s, lxvar::undefined());
            p = mValue->find(s);
        }
        return *p;
    }

    lxvar
    lxvar::find (const std::string& s) const
    {
        return find(s.c_str());
    }

    lxvar
    lxvar::find (const char* key) const
    {
        auto p = mValue->find(key);
        return p ? *p : lxvar::undefined();
    }

    bool
    lxvar::operator== (const lxvar& that) const
    {
        if (!is_defined())
            return !that.is_defined();
        if (is_int())
            return that.is_int() && as<int>() == that.as<int>();
        if (is_float())
            return that.is_float() && as<float>() == that.as<float>();
        if (is_string())
            return that.is_string() && as<std::string>() == that.as<std::string>();
        
        lx_error("Not yet support type!");
        return false;
    }


    static lxvar
    _query_path (lxvar v, std::string path)
    {
        // Split the path into components
        std::vector<std::string> keys;
        size_t s = 0;
        size_t i = 0;
        while (i < path.size())
        {
            while (i < path.size() && path[i] != '/')
                i++;

            keys.push_back( path.substr(s, i - s) );
            s = i + 1;
        }

        // Walk the path
        for (auto it = keys.begin(); it != keys.end(); ++it)
        {
            if (v.is_map())
            {
                v = v.find(*it);
            }
            else if (v.is_array())
            {
                int index;
                std::istringstream iss (*it);
                iss >> index;
                if (!iss.eof())
                    v = lxvar();
                else
                    v = v.at(index);
            }
            else
            {
                v = lxvar();
                break;
            }
        }

        return v;
    }

    std::string lxvar::query (std::string path, std::string def)
    {
        return _query_path(*this, path).query(def);
    }

    int lxvar::query (std::string path, int def)
    {
        return _query_path(*this, path).query(def);
    }

    void 
    lxvar::insert (const char* key, const lxvar& value)
    {
        if (!is_defined())
            *this = map();

        mValue->insert(key, const_cast<lxvar&>(value));
    }

    void
    lxvar::add (const char* key, lx0::uint32 flags, ValidateFunction validate, lxvar def)
    {
        if (!is_defined())
            *this = decorated_map();

        mValue->add(key, flags, validate);
        mValue->insert(key, def);
    }

    lx0::uint32
    lxvar::flags (const char* key)
    {
        return mValue->flags(key);
    }

    lxvar    
    lxvar::parse (const char* s)
    {
        lx0::core::detail::LxsonParser parser;
        return parser.parse(s);
    }

    //===========================================================================//


}}}
