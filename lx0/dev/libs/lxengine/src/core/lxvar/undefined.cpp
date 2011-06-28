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

        class lxundefined : public lxvalue
        {
        public:
            static lxvalue*     acquire() { return &s_singleton; }

            virtual bool        sharedType() const { return true; }
            virtual lxvalue*    clone() const { return const_cast<lxundefined*>(this); }

            virtual bool        is_undefined(void) const            { return true; }

        private:
            lxundefined() { _incRef(); }
            ~lxundefined() {}
            static lxundefined s_singleton;
        };

        lxundefined lxundefined::s_singleton;

        lxvalue* create_lxundefined     (void) { return lxundefined::acquire(); }

            }
        }
    }
}
