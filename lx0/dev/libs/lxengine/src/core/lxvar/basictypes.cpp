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

            virtual bool        is_bool     (void) const            { return true; }
        };

        lxvalue* create_lxbool(bool b) { return new lxbool(b); }

        //===========================================================================//

        class lxint : public lxvalue_basic<lxint, int>
        {
        public:
            lxint() : Base (0) {}
            lxint(int i) : Base (i) {}

            virtual bool        is_int    (void) const            { return true; }

            //@name Implicit up-casts
            //@{
            virtual void as (float& v)  const { v = float(mValue); }
            virtual void as (double& v) const { v = double(mValue); }
            //@}
        };

        lxvalue* create_lxint(int i) { return new lxint(i); }

        //===========================================================================//

        class lxfloat : public lxvalue_basic<lxfloat, float>
        {
        public:
            lxfloat() : Base (0.0f) {}
            lxfloat(float f) : Base(f) {}

            virtual bool        is_float     (void) const            { return true; }

            //@name Implicit up-casts
            //@{
            virtual void as (double& v) const { v = mValue; }
            //@}
        };

        lxvalue* create_lxfloat(float f) { return new lxfloat(f); }

        //===========================================================================//

        class lxdouble : public lxvalue_basic<lxdouble, double>
        {
        public:
            lxdouble() : Base (0.0) {}
            lxdouble(double f) : Base(f) {}

            virtual bool        is_double     (void) const            { return true; }
        };

        lxvalue* create_lxdouble(double d) { return new lxdouble(d); }

        //===========================================================================//

        class lxstring : public lxvalue_basic<lxstring, std::string>
        {
        public:
            lxstring() {}
            lxstring(std::string s) : Base(s) {}
            lxstring(const char* s) : Base(s) {}

            virtual bool        is_string     (void) const            { return true; }
        };

        lxvalue* create_lxstring(const char* s) { return new lxstring(s); }
        lxvalue* create_lxstring(const std::string& s) { return new lxstring(s); }

            }
        }
    }
}
