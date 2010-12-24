//===========================================================================//
/*
                                   LxEngine

    LICENSE
    * MIT License (http://www.opensource.org/licenses/mit-license.php)

    Copyright (c) 2010 athile@athile.net (http://www.athile.net)

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

#include <lx0/core/detail/forward_decls.hpp>
#include <vector>
#include <memory>
#include <map>
#include <string>

namespace lx0 { namespace core {

    namespace detail 
    {
        class lxvalue;
        class lxvalue_iterator;
        
        class lxundefined;
        class lxint;
        class lxfloat;
        class lxstring;
        class lxarray;
        class lxstringmap;
    }
        
    /*!
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
     */
    class lxvar
    {
    public:

        struct auto_cast
        {
            auto_cast (lxvar& v) : mValue (v) {}
            lxvar& mValue;

            operator int ()         { return mValue.asInt(); }
            operator float ()       { return mValue.asFloat(); }
            operator std::string () { return mValue.asString(); }
        };

        struct auto_cast2
        {
            auto_cast2 (lxvar& v) : mValue (v) {}
            lxvar& mValue;

            template <typename T>
            operator T ()           { T t; detail::_convert(mValue, t); return t; }
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

                        lxvar           (int i);
                        lxvar           (int a, int b, int c, int d);
                        lxvar           (float a);
                        lxvar           (float a, float b, float c);
                        lxvar           (float a, float b, float c, float d);
                        lxvar           (const char* s);
                        lxvar           (std::string s);

        static lxvar    undefined       (void);                 //!< Return an undefined lxvar
        static lxvar    map             (void);                 //!< Return an empty map
        static lxvar    array           (void);                 //!< Return an empty array

        static lxvar    parse           (const char* s);

        template <typename T>
        detail::lxshared_ptr<T>
                        imp             (void)                  { return detail::lxshared_ptr<T>( dynamic_cast<T*>(mValue.get()) ); }

        
        bool            isShared        (void) const;
        bool            isSharedType    (void) const;
        lxvar           clone           (void) const;           //!< Create a deep clone of the lxvar

        auto_cast2      convert         (void)                  { return auto_cast2(*this); }

        bool            equal           (int i) const           { return (isInt() && asInt() == i); } //!< Is strictly equal: same type and same value
        bool            equal           (std::string s) const   { return (isString() && asString() == s);}

        bool            equiv           (const char* s) const;  //!< Is equal, or is equal after a type conversion

        int             query           (int def) const         { return isInt() ? asInt() : def; }
        std::string     query           (std::string def) const { return isString() ? asString() : def; }
        float           query           (float def) const       { return isFloat() ? asFloat() : (isInt() ? asInt() : def); }

        //@name Type checks
        //@{
        bool            isDefined       (void) const            { return !isUndefined(); }
        bool            isUndefined     (void) const            { return _isType<detail::lxundefined>(); }
        bool            isInt           (void) const            { return _isType<detail::lxint>(); }
        bool            isFloat         (void) const            { return _isType<detail::lxfloat>(); }
        bool            isString        (void) const            { return _isType<detail::lxstring>(); }
        bool            isArray         (void) const            { return _isType<detail::lxarray>(); }
        bool            isMap           (void) const;
        
        bool            isHandle        (void) const;
        std::string     handleType      (void) const;
        void*           unwrap          (void);

        bool            __isBoolean     (void) const;
        bool            __isBinary      (void) const;           //!< Reserved for future binary blob support (specialization of an array)
        //@}

        int             asInt           (void) const;
        float           asFloat         (void) const;
        std::string     asString        (void) const;

        void            toArray         (void);
        void            toMap           (void);

        iterator        begin           (void);
        iterator        end             (void);

        int             size            (void) const;
        lxvar           at              (int index) const;
        void            at              (int index, lxvar value);
        void            push            (const lxvar& e);

        bool            containsKey     (const char* key) const;
        lxvar           find            (const char* key) const;
        lxvar           find            (const std::string& s) const;
        void            insert          (const char* key, const lxvar& value);

        auto_cast       operator*       (void) { return auto_cast(*this); }

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

    namespace detail
    {
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
            
            virtual bool        isHandle    (void) const    { return false; }
            virtual std::string handleType  (void) const    { _invalid(); return ""; }
            virtual void*       unwrap      (void)          { return nullptr; }

            virtual int         size        (void) const                { _invalid(); return 0; }
            virtual lxvar       at          (int i)                     { _invalid(); return lxvar(); }
            virtual void        at          (int index, lxvar value)    { _invalid(); }

            virtual lxvar       find        (const char* key) const     { _invalid(); return lxvar::undefined(); }

            virtual lxvar::iterator begin  (void)                       { _invalid(); return lxvar::iterator(); }
            virtual lxvar::iterator end    (void)                       { _invalid(); return lxvar::iterator(); }

        protected:
            void                _invalid    (void) const;

            unsigned int        mRefCount;
        };



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

        class lxint : public lxvalue
        {
        public:
            lxint() : mValue (0) {}
            lxint(int i) : mValue (i) {}

            virtual bool        sharedType  (void) const { return false; }
            virtual lxvalue*    clone       (void) const { return new lxint(mValue); }

            int mValue;
        };

        class lxfloat : public lxvalue
        {
        public:
            lxfloat() : mValue (0.0f) {}
            lxfloat(float f) : mValue(f) {}

            virtual bool        sharedType  (void) const { return false; }
            virtual lxvalue*    clone       (void) const { return new lxfloat(mValue); }

            float mValue;
        };

        class lxstring : public lxvalue
        {
        public:
            lxstring() {}
            lxstring(std::string s) : mValue(s) {}
            lxstring(const char* s) : mValue(s) {}

            virtual bool        sharedType  (void) const { return false; }
            virtual lxvalue*    clone       (void) const { return new lxstring(mValue); }

            std::string mValue;
        };

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
            virtual lxvar       at          (int i);
            virtual void        at          (int index, lxvar value);

            lxvar::iterator     begin           (void) { return lxvar::iterator(new iterator_imp(mValue.begin())); }
            lxvar::iterator     end             (void) { return lxvar::iterator(new iterator_imp(mValue.end())); }

            std::vector<lxvar> mValue;
        };

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

            virtual int         size        (void) const { return int( mValue.size() ); }

            lxvar::iterator     begin           (void) { return lxvar::iterator(new iterator_imp(mValue.begin())); }
            lxvar::iterator     end             (void) { return lxvar::iterator(new iterator_imp(mValue.end())); }

            virtual lxvar       find        (const char* key) const;

            typedef std::map<std::string, lxvar> Map;
            Map mValue;
        };
    }

    
    namespace detail
    {
        inline void    _convert    (lxvar& v, int& i)          { i = v.asInt(); }
        inline void    _convert    (lxvar& v, float& f)        { f = v.asFloat(); }
        inline void    _convert    (lxvar& v, std::string& s)  { s = v.asString(); }
    }

}}
