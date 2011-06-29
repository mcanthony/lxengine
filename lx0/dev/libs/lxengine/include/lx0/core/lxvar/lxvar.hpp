//===========================================================================//
/*
                                   LxEngine

    LICENSE
    * MIT License (http://www.opensource.org/licenses/mit-license.php)

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

#pragma once

#include <lx0/_detail/forward_decls.hpp>
#include <vector>
#include <memory>
#include <map>
#include <string>
#include <functional>

namespace lx0 
{ 
    namespace core 
    { 
        namespace lxvar_ns
        {
            namespace detail 
            {
                class lxvar;
                class lxvalue;
                class lxvalue_iterator;
        
                lxvalue* create_lxundefined     (void);
                lxvalue* create_lxbool          (bool b);
                lxvalue* create_lxint           (int i);
                lxvalue* create_lxstring        (const char* s);
                lxvalue* create_lxstring        (const std::string& s);
                lxvalue* create_lxfloat         (float f);
                lxvalue* create_lxdouble        (double d);
                lxvalue* create_lxarray         (void);
                lxvalue* create_lxstringmap     (void);
                lxvalue* create_lxorderedmap    (void);
                lxvalue* create_lxdecoratedmap  (void);

                typedef std::function<bool (lxvar, lxvar&)> ValidateFunction;
            
                using lx0::lxshared_ptr;
        
                //===========================================================================//
                /*!
                    \ingroup lx0_core_lxvar

                    Dev Notes:

                    A major design decision, that requires consistency across the class is
                    type strictness.  For example, should querying the string value of a undefined lxvar
                    result in an empty string or a thrown exception?  The lxvar is intended to be a flexible,
                    scripting-like data type which would favor the idea of implicit conversions, however
                    a strict definition often leads to a more robust and well-defined class.   Another
                    example is comparisons: should an epsilon be implicit in floating point comparisons?
        
                    The current design goal is to provide both strict and loose comparisons and conversions
                    under different, consistently-named methods.  

                    @todo Review for a good const-ness strategy for this class.  Non-trivial 
                        given the reference-counted nature of the underlying objects.
                    @todo Iterators should be use the pImpl approach so a begin() method can return either
                        an array or a map iterator.
                    @todo Move lxvar to its own library
                */
                class lxvar
                {
                public:
                    struct auto_cast2
                    {
                        auto_cast2 (lxvar& v) : mValue (v) {}
                        lxvar& mValue;

                        template <typename T>
                        operator T ()           { T t; _convert(mValue, t); return t; }
                    };

                    class iterator
                    {
                    public:
                        iterator (void);
                        iterator (detail::lxvalue_iterator* pIter) : mValue(pIter) {}
                        iterator (const iterator& that);

                        void        operator=   (const iterator& that);

                        bool        operator== (iterator& that);
                        bool        operator!= (iterator& that);
                        void        operator++ (void);
                        void        operator++ (int);
                        lxvar       operator-> (void);
                        lxvar       operator*  (void);

                        std::string key        (void);

                    protected:
                        mutable std::shared_ptr<detail::lxvalue_iterator> mValue;
                    };

                                    lxvar           (void);
                                    lxvar           (const lxvar& that);

                                    lxvar           (detail::lxvalue* imp);

                                    lxvar           (bool b);
                                    lxvar           (int i);
                                    lxvar           (int a, int b);
                                    lxvar           (int a, int b, int c);
                                    lxvar           (int a, int b, int c, int d);
                                    lxvar           (float a);
                                    lxvar           (float a, float b, float c);
                                    lxvar           (float a, float b, float c, float d);
                                    lxvar           (const char* s);
                                    lxvar           (std::string s);

                                    /*template <typename T>
                                    lxvar           (const T& t)
                                    {
                                        *this = detail::lxvar_from(t);
                                    }*/

                    static lxvar    undefined       (void);                 //!< Return an undefined lxvar
                    static lxvar    map             (void);                 //!< Return an empty map
                    static lxvar    ordered_map     (void);                 //!< Return an empty ordered map
                    static lxvar    decorated_map   (void);                 //!< Return an empty decorated map
                    static lxvar    array           (void);                 //!< Return an empty array

                    static lxvar    parse           (const char* s);

                    template <typename T>
                    detail::lxshared_ptr<T>
                                    imp             (void)                  { return detail::lxshared_ptr<T>( dynamic_cast<T*>(mValue.get()) ); }

        
                    bool            isShared        (void) const;
                    bool            isSharedType    (void) const;
                    lxvar           clone           (void) const;           //!< Create a deep clone of the lxvar

                    auto_cast2      convert         (void)                  { return auto_cast2(*this); }
                    
                    template <typename T>
                    T               convert         (const T& t)            { return is_undefined() ? t : (T)auto_cast2(*this); }

                    bool            equal           (int i) const           { return (is_int() && as<int>() == i); } //!< Is strictly equal: same type and same value
                    bool            equal           (std::string s) const   { return (is_string() && as<std::string>() == s);}

                    bool            equiv           (const char* s) const;  //!< Is equal, or is equal after a type conversion

                    //@name Type checks
                    //@{
                    bool            is_defined       (void) const            { return !is_undefined(); }
                    bool            is_undefined     (void) const;
                    bool            is_bool          (void) const;
                    bool            is_int           (void) const;
                    bool            is_float         (void) const;
                    bool            is_string        (void) const;
                    bool            is_array         (void) const;
                    bool            is_map           (void) const;
        
                    bool            isHandle        (void) const;
                    std::string     handleType      (void) const;
                    void*           unwrap          (void);

                    bool            __isBinary      (void) const;           //!< Reserved for future binary blob support (specialization of an array)
                    //@}

                    template <typename T>
                    T               as              (void) const;

                    iterator        begin           (void);
                    iterator        end             (void);

                    int             size            (void) const;
                    lxvar           at              (int index) const;
                    void            at              (int index, lxvar value);
                    void            push            (const lxvar& e);

                    bool            has             (const char* key) const;
                    lxvar           find            (const char* key) const;
                    lxvar           find            (const std::string& s) const;
                    void            insert          (const char* key, const lxvar& value);

                    void            add             (const char* key, lx0::uint32 flags, ValidateFunction validate, lxvar def = lxvar());
                    lx0::uint32     flags           (const char* key);

  
                    template <typename T>
                    operator T () { T t; _convert(*this, t); return t; }

                    const lxvar&    operator=       (const lxvar& that);

                    lxvar&          operator[]      (int i);
                    lxvar&          operator[]      (const char* s);
                    lxvar&          operator[]      (const std::string& s) { return (*this)[s.c_str()]; }

                    bool            operator==      (const lxvar& that) const;
                    bool            operator==      (int i) const { return equal(i); }
                    

                protected:
                    template <typename T>   bool    _isType (void) const;
                    template <typename T>   T*      _castTo (void) const;

                    mutable detail::lxshared_ptr<detail::lxvalue> mValue;
                };

                template <typename T>   
                bool    
                lxvar::_isType (void) const
                {
                    // A bit quicker than a dynamic_cast<> since the check is for an
                    // exact type match - not a match with regards to the inheritance
                    // tree.
                    //
                    return typeid(*mValue.get()) == typeid(T);
                }

                //===========================================================================//
                //!
                /*!
                 */
                class lxvalue_iterator
                {
                public:
                    virtual         ~lxvalue_iterator   () {}
            
                    virtual lxvalue_iterator* clone     (void)                         { return new lxvalue_iterator(); }

                    virtual bool    equal               (const lxvalue_iterator& that) { _invalid(); return false; }
                    virtual void    inc                 (void)                         { _invalid(); }
                    virtual std::string key             (void)                         { _invalid(); return std::string(); }
                    virtual lxvar   dereference         (void)                         { _invalid(); return lxvar(); }

                protected:
                    void            _invalid            (void) const;
                };

                //===========================================================================//
                //!
                /*!
                 */
                class lxvalue
                {
                public:
                                        lxvalue() : mRefCount (0) {}
                    virtual             ~lxvalue() {}

                    void                _incRef     (void)          { mRefCount++; }
                    void                _decRef     (void)          { if (--mRefCount == 0) delete this; }
                    unsigned int        _refCount   (void) const    { return mRefCount; }

                    virtual bool        sharedType  (void) const    { return true; }    //!< On a set operation, is the r-value referenced or copied?
                    virtual lxvalue*    clone       (void) const = 0;                   //!< Deep clone of the value
            
                    virtual bool        isHandle    (void) const                { return false; }
                    virtual std::string handleType  (void) const                { _invalid(); return ""; }
                    virtual void*       unwrap      (void)                      { return nullptr; }


                    virtual bool        is_undefined(void) const            { return false; }
                    virtual bool        is_bool     (void) const            { return false; }
                    virtual bool        is_int      (void) const            { return false; }
                    virtual bool        is_float    (void) const            { return false; }
                    virtual bool        is_string   (void) const            { return false; }
                    virtual bool        is_array    (void) const            { return false; }
                    virtual bool        is_map      (void) const            { return false; }

                    virtual void        as          (bool&)        const        { _invalid(); }
                    virtual void        as          (int&)         const        { _invalid(); }
                    virtual void        as          (float&)       const        { _invalid(); }
                    virtual void        as          (double&)      const        { _invalid(); }
                    virtual void        as          (std::string&) const        { _invalid(); }

                    virtual int         size        (void) const                { _invalid(); return 0; }

                    virtual lxvar*      at          (int i)                     { _invalid(); return nullptr; }
                    virtual void        at          (int index, lxvar value)    { _invalid(); }
                    virtual void        push        (lxvar value)               { _invalid(); }

                    virtual bool        has         (const char* key) const     { _invalid(); return false; }
                    virtual lxvar*      find        (const char* key) const     { _invalid(); return nullptr; }
                    virtual void        insert      (const char* key, lxvar& value) { _invalid(); }

                    virtual void        add         (const char* key, lx0::uint32 flags, ValidateFunction validate) { _invalid(); }
                    virtual lx0::uint32 flags       (const char* key)           { _invalid(); return 0; }

                    virtual lxvar::iterator begin  (void)                       { _invalid(); return lxvar::iterator(); }
                    virtual lxvar::iterator end    (void)                       { _invalid(); return lxvar::iterator(); }

                protected:
                    void                _invalid    (void) const;

                    unsigned int        mRefCount;
                };

                template <typename T>
                typename T lxvar::as (void) const
                { 
                    T t; 
                    mValue->as(t); 
                    return t; 
                }
            }
    
            namespace detail
            {
                inline void    _convert    (lxvar& v, bool& b)         { b = v.as<bool>(); }
                inline void    _convert    (lxvar& v, int& i)          { i = v.as<int>(); }
                inline void    _convert    (lxvar& v, float& f)        { f = v.as<float>(); }
                inline void    _convert    (lxvar& v, double& d)       { d = v.as<double>(); }
                inline void    _convert    (lxvar& v, std::string& s)  { s = v.as<std::string>(); }
            }

            using detail::lxvar;
            using detail::ValidateFunction;

            lxvar       find            (lxvar& v, const char* path);
            void        insert          (lxvar& v, const char* path, lxvar value);
            
            /*!
             */
            template <typename T>
            T           query           (lxvar& v, const T& def)
            {
                return v.is_defined() ? static_cast<T>(v) : def;
            }

            inline
            std::string query           (lxvar& v, const char* def)
            {
                return v.is_string() ? v.as<std::string>() : std::string(def);
            }

            /*!
                Searchs a map along a given path; if a value exists at that path
                then it is returned as the queried type, otherwise the specified
                default value is returned.
             */
            template <typename T>
            T           query_path           (lxvar& v, const char* path, const T& def)
            {
                lxvar u = find(v, path);
                return query(u, def);
            }

            inline
            std::string query_path           (lxvar& v, const char* path, const char* def)
            {
                lxvar u = find(v, path);
                return query(u, def);
            }
                    

            std::string         format_tabbed       (lxvar& v);

            ValidateFunction    validate_readonly   (void);
            ValidateFunction    validate_bool       (void);
            ValidateFunction    validate_int_range  (int imin, int imax);

        }   // end namespace lxvar

    }   // end namespace core

    using namespace lx0::core::lxvar_ns;

} // end namespace lx0
