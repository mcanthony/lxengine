//===========================================================================//
/*
                                   LxEngine

    LICENSE

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

namespace lx0 { namespace core {

#define _LX_FORWARD_DECL_PTRS(Klass) \
    class Klass; \
    typedef std::shared_ptr<Klass> Klass ## Ptr; \
    typedef std::shared_ptr<const Klass> Klass ## CPtr; \
    typedef std::weak_ptr<Klass> Klass ## WPtr; \
    typedef std::weak_ptr<const Klass> Klass ## CWPtr; 

    _LX_FORWARD_DECL_PTRS(Object);
    _LX_FORWARD_DECL_PTRS(Element);
    _LX_FORWARD_DECL_PTRS(Transaction);
    _LX_FORWARD_DECL_PTRS(Document);
    _LX_FORWARD_DECL_PTRS(Space);
    _LX_FORWARD_DECL_PTRS(Engine);
    _LX_FORWARD_DECL_PTRS(View);
    _LX_FORWARD_DECL_PTRS(Controller);



}}