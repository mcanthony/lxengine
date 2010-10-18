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

#include <lx0/detail/forward_decls.hpp>
#include <vector>
#include <memory>
#include <map>
#include <string>

namespace lx0 { namespace core {

    namespace detail 
    {
        class lxvalue;
    }
        
    class lxvar
    {
    public:
        lxvar   (void);
        lxvar   (const lxvar& that);
        lxvar   (int i);
        lxvar   (int a, int b, int c, int d);
        lxvar   (float a);
        lxvar   (float a, float b, float c);
        lxvar   (const char* s);

        lxvar   clone  () const;        //!< Create a deep clone of the lxvar

        void    push   (const lxvar& e);

        void    insert (const char* key, const lxvar& value);

    protected:

        template <typename T>   T*    _castTo ();

        std::shared_ptr<detail::lxvalue> mValue;
    };

    namespace detail
    {
        class lxvalue
        {
        public:
            virtual ~lxvalue() {}
            virtual bool sharedType() const = 0;
            virtual lxvalue* clone() const = 0;
        };

        class lxundefined : public lxvalue
        {
        public:
            static std::shared_ptr<lxvalue> acquire() { return acquireSingleton<lxundefined>(uwpSingleton); }

            virtual bool sharedType() const { return true; }
            virtual lxvalue* clone() const { return const_cast<lxundefined*>(this); }

        private:
            lxundefined() {}
            ~lxundefined() {}
            template <typename T> friend std::shared_ptr<T> detail::acquireSingleton (std::weak_ptr<T>&);
            static std::weak_ptr<lxundefined> uwpSingleton;
        };

        class lxint : public lxvalue
        {
        public:
            lxint() : mValue (0) {}
            lxint(int i) : mValue (i) {}

            virtual bool        sharedType  (void) const { return false; }
            virtual lxvalue*    clone       (void) const { return new lxint(mValue); }

        protected:
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

        class lxarray : public lxvalue
        {
        public:
            virtual bool        sharedType  (void) const { return true; }
            virtual lxvalue*    clone       (void) const;

            std::vector<lxvar> mValues;
        };

        class lxstringmap : public lxvalue
        {
        public:
            virtual bool        sharedType  (void) const { return true; }
            virtual lxvalue*    clone       (void) const;

            typedef std::map<std::string, lxvar> Map;
            Map mValue;
        };
    }


}}