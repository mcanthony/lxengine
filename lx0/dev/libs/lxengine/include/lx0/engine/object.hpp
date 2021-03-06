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

#include <memory>
#include <lx0/core/lxvar/lxvar.hpp>

namespace lx0 { namespace engine_ns {

    //===========================================================================//
    //! 
    /*!
        Developer Notes:

        Currently unused, but may be useful for some features such as 
        serialization.  For example, a generalized serializer could simply
        walk all the properties (treating the lxvar any old lxvar), but 
        a custom serialization method would be more efficient.  That would
        also provide an immediate solution for the notion of transient 
        properties that are useful during runtime but do not need to be
        saved.  Another example might be compression: the object might know
        how to store itself in a different format than the run-time format.

        In any case, this class may eventually serve a useful purpose.
     */   
    class Object : public lx0::core::lxvar_ns::detail::lxvalue
    {
    public:
    };


}}
