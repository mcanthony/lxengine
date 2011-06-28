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

        class lxundefined : public lxvalue
        {
        public:
            static lxvalue*     acquire() { return &s_singleton; }

            virtual bool        sharedType() const { return true; }
            virtual lxvalue*    clone() const { return const_cast<lxundefined*>(this); }

        private:
            lxundefined() { _incRef(); }
            ~lxundefined() {}
            static lxundefined s_singleton;
        };

        lxundefined lxundefined::s_singleton;

        //===========================================================================//

        template <typename Derived, typename T>
        class lxvalue_basic : public lxvalue
        {
        public:
            typedef lxvalue_basic<Derived, T>   Base;
            lxvalue_basic() {}
            lxvalue_basic(T v) : mValue (v) {}
            virtual bool sharedType (void) const { return false; }
            virtual lxvalue* clone (void) const { return new Derived(mValue); }

            virtual void as(T& v) const { v = mValue; }
            T mValue;
        };

        //===========================================================================//

        class lxbool : public lxvalue_basic<lxbool, bool>
        {
        public:
            lxbool(bool b) : Base (b) {}
        };

        //===========================================================================//

        class lxint : public lxvalue_basic<lxint, int>
        {
        public:
            lxint() : Base (0) {}
            lxint(int i) : Base (i) {}

            //@name Implicit up-casts
            //@{
            virtual void as (float& v)  const { v = float(mValue); }
            virtual void as (double& v) const { v = double(mValue); }
            //@}
        };

        //===========================================================================//

        class lxfloat : public lxvalue_basic<lxfloat, float>
        {
        public:
            lxfloat() : Base (0.0f) {}
            lxfloat(float f) : Base(f) {}

            //@name Implicit up-casts
            //@{
            virtual void as (double& v) const { v = mValue; }
            //@}
        };

        //===========================================================================//

        class lxdouble : public lxvalue_basic<lxdouble, double>
        {
        public:
            lxdouble() : Base (0.0) {}
            lxdouble(double f) : Base(f) {}
        };

        //===========================================================================//

        class lxstring : public lxvalue_basic<lxstring, std::string>
        {
        public:
            lxstring() {}
            lxstring(std::string s) : Base(s) {}
            lxstring(const char* s) : Base(s) {}
        };

        //===========================================================================//
        /*!
            Implementation for a generic array of lxvars.
            */
        class lxarray : public lxvalue
        {
        public:
            class iterator_imp : public lxvalue_iterator
            {
            public:
                iterator_imp (std::vector<lxvar>::iterator it) : mIter(it) {}
                virtual lxvalue_iterator* clone     (void)                         { return new iterator_imp(mIter); }

                virtual bool    equal               (const lxvalue_iterator& that) { return mIter == dynamic_cast<const iterator_imp&>(that).mIter; }
                virtual void    inc                 (void)                         { mIter++; }
                virtual lxvar   dereference         (void)                         { return *mIter; }

            protected:
                std::vector<lxvar>::iterator    mIter;

            };

            virtual bool        sharedType  (void) const { return true; }
            virtual lxvalue*    clone       (void) const;

            virtual int         size        (void) const { return int( mValue.size() ); }
            virtual lxvar*      at          (int i);
            virtual void        at          (int index, lxvar value);

            lxvar::iterator     begin           (void) { return lxvar::iterator(new iterator_imp(mValue.begin())); }
            lxvar::iterator     end             (void) { return lxvar::iterator(new iterator_imp(mValue.end())); }

            std::vector<lxvar> mValue;
        };

        lxvalue*    
        lxarray::clone (void) const
        {
            lxarray* pClone = new lxarray;
            pClone->mValue.reserve(mValue.size());
            for (auto it = mValue.begin(); it != mValue.end(); ++it)
                pClone->mValue.push_back(it->clone());
            return pClone;
        }

        lxvar*
        lxarray::at (int i)      
        {
            lx_check_error(i >= 0 && i < int(mValue.size()));
            return &mValue[i]; 
        }

        void 
        lxarray::at (int i, lxvar value)      
        {
            lx_check_error(i >= 0 && i < int(mValue.size()));
            mValue[i] = value;; 
        }

        //===========================================================================//

        class lxstringmap : public lxvalue
        {
        public:
            class iterator_imp : public lxvalue_iterator
            {
            public:
                iterator_imp (std::map<std::string, lxvar>::iterator it) : mIter(it) {}
                virtual lxvalue_iterator* clone     (void)                         { return new iterator_imp(mIter); }

                virtual bool    equal               (const lxvalue_iterator& that) { return mIter == dynamic_cast<const iterator_imp&>(that).mIter; }
                virtual void    inc                 (void)                         { mIter++; }
                virtual std::string key             (void)                         { return mIter->first; }
                virtual lxvar   dereference         (void)                         { return mIter->second; }

            protected:
                std::map<std::string, lxvar>::iterator    mIter;
            };


            virtual bool        sharedType  (void) const { return true; }
            virtual lxvalue*    clone       (void) const;

            virtual bool        is_map      (void) const    { return true; }

            virtual int         size        (void) const { return int( mValue.size() ); }

            lxvar::iterator     begin           (void) { return lxvar::iterator(new iterator_imp(mValue.begin())); }
            lxvar::iterator     end             (void) { return lxvar::iterator(new iterator_imp(mValue.end())); }

            virtual lxvar*      find        (const char* key) const;
            virtual void        insert      (const char* key, lxvar& value);

            typedef std::map<std::string, lxvar> Map;
            Map mValue;
        };

        lxvalue*    
        lxstringmap::clone (void) const
        {
            lxstringmap* pClone = new lxstringmap;
            for (auto it = mValue.begin(); it != mValue.end(); ++it)
                pClone->mValue.insert( std::make_pair(it->first, it->second) );
            return pClone;
        }

        lxvar*
        lxstringmap::find (const char* key) const
        {
            auto it = mValue.find(key);
            if (it != mValue.end())
                return const_cast<lxvar*>(&it->second);
            else
                return nullptr;
        }

        void 
        lxstringmap::insert (const char* key, lxvar& value) 
        {
            mValue.erase(key);
            mValue.insert(std::make_pair(key, value));
        }

        //===========================================================================//

        class lxorderedmap : public lxvalue
        {
        public:
            typedef std::map<std::string, lxvar> ValueMap;
            typedef std::map<size_t, std::string> OrderMap;

            class iterator_imp : public lxvalue_iterator
            {
            public:
                iterator_imp (lxorderedmap* pMap, OrderMap::iterator it) : mpMap(pMap), mIter(it) {}
                virtual lxvalue_iterator* clone     (void)                         { return new iterator_imp(mpMap, mIter); }

                virtual bool    equal               (const lxvalue_iterator& that) { return mIter == dynamic_cast<const iterator_imp&>(that).mIter; }
                virtual void    inc                 (void)                         { mIter++; }
                virtual std::string key             (void)                         { return mIter->second; }
                virtual lxvar   dereference         (void)                         { return mpMap->mValues.find(key())->second; }

            protected:
                lxorderedmap*       mpMap;
                OrderMap::iterator  mIter;
            };

            lxorderedmap();


            virtual bool        sharedType  (void) const { return true; }
            virtual lxvalue*    clone       (void) const;

            virtual bool        is_map      (void) const    { return true; }

            virtual int         size        (void) const { return int( mValues.size() ); }

            lxvar::iterator     begin           (void) { return lxvar::iterator(new iterator_imp(this, mOrder.begin())); }
            lxvar::iterator     end             (void) { return lxvar::iterator(new iterator_imp(this, mOrder.end())); }

            virtual lxvar*      find        (const char* key) const;
            virtual void        insert      (const char* key, lxvar& value);

            OrderMap     mOrder;
            ValueMap     mValues;
            size_t       mCount;
        };

        lxorderedmap::lxorderedmap()
            : mCount (0)
        {
        }

        lxvalue*    
        lxorderedmap::clone (void) const
        {
            auto* pClone = new lxorderedmap;
            pClone->mCount = 0;
            for (auto it = mOrder.begin(); it != mOrder.end(); ++it)
            {
                auto& value = mValues.find(it->second);
                pClone->mOrder.insert(std::make_pair(pClone->mCount++, it->second));
                pClone->mValues.insert(std::make_pair(it->second, value->second));
            }
            return pClone;
        }

        lxvar*
        lxorderedmap::find (const char* key) const
        {
            auto it = mValues.find(key);
            if (it != mValues.end())
                return const_cast<lxvar*>(&it->second);
            else
                return nullptr;
        }

        void 
        lxorderedmap::insert (const char* key, lxvar& value) 
        {
            auto it = mValues.find(key);
            if (it == mValues.end())
            {
                mOrder.insert(std::make_pair(mCount++, key));
                mValues.insert(std::make_pair(key, value));
            }
            else
            {
                // Retain the original ordering on replace operations
                mValues.erase(it);
                mValues.insert(std::make_pair(key, value));
            }
        }

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
        : mValue( new lxbool(b) )
    {
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

    /*!
        Creates a map that retains the original insertion order upon iteration of the map.

        This implementation requires overhead than an unordered map.  
     */
    lxvar
    lxvar::ordered_map (void)
    {
        lxvar v;
        v._castTo<lxorderedmap>();
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
        lxarray* pArray = _castTo<lxarray>();
        pArray->mValue.push_back(v);
    }

    bool
    lxvar::isMap (void) const
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

    /*!
        Converts the undefined variable to an empty map.
     */
    void       
    lxvar::toMap (void)
    {
        _castTo<lxstringmap>();
    }

    bool
    lxvar::has (const char* key) const
    {
         auto map = _castTo<lxstringmap>()->mValue;
         return map.find(key) != map.end();
    }

    lxvar&
    lxvar::operator[] (int i)
    {
        if (!isDefined()) 
            *this = array();

        return *mValue->at(i);
    }

    lxvar&
    lxvar::operator[] (const char* s)
    {
        if (!isDefined()) 
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
        if (!isDefined())
            return !that.isDefined();
        if (isInt())
            return that.isInt() && as<int>() == that.as<int>();
        if (isFloat())
            return that.isFloat() && as<float>() == that.as<float>();
        if (isString())
            return that.isString() && as<std::string>() == that.as<std::string>();
        
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
            if (v.isMap())
            {
                v = v.find(*it);
            }
            else if (v.isArray())
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
        if (!isDefined())
            *this = map();

        mValue->insert(key, const_cast<lxvar&>(value));
    }

    lxvar    
    lxvar::parse (const char* s)
    {
        lx0::core::detail::LxsonParser parser;
        return parser.parse(s);
    }

    //===========================================================================//
    
    void 
    insert (lxvar& v, const char* path, lxvar value)
    {
        lx_error("Not yet implemented!");
    }

    /*!
        A pretty-print function that prints the lxvar as name value pairs in a tabbed
        fashion.
     */
    std::string 
    format_tabbed (detail::lxvar& v)
    {
        std::string buffer;
        std::function<void (lxvar, std::string)> fmt = [&fmt, &buffer](lxvar v, std::string indent) 
        {
            if (v.isArray())
            {
                for (auto it = v.begin(); it != v.end(); ++it)
                {
                    fmt(*it, indent);
                }
            }
            else if (v.isMap())
            {
                for (auto it = v.begin(); it != v.end(); ++it)
                {
                    buffer += boost::str( boost::format("%s%s : ") % indent % it.key() );
                    if ((*it).isArray() || (*it).isMap())
                    {
                        buffer += "\n" + indent;
                        fmt(*it, indent + "    ");
                    }
                    else
                        fmt(*it, indent);
                        
                }
            }
            else if (v.isInt())
                buffer += boost::str( boost::format("%d\n") % v.as<int>() );
            else if (v.isFloat())
                buffer += boost::str( boost::format("%f\n") % v.as<float>() );
            else if (v.isString())
                buffer += boost::str( boost::format("%s\n") % v.as<std::string>().c_str() );
            else
                buffer += "<unknown>";
        };
        fmt(v, "");

        return buffer;
    }

}}}
