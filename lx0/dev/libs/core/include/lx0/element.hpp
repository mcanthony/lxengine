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

#include <map>
#include <deque>
#include <memory>
#include <string>

#include <lx0/detail/forward_decls.hpp>
#include <lx0/lxvar.hpp>


namespace lx0 { namespace core {

    class Object;

    class Element 
        : public std::enable_shared_from_this<Element>
    {
    public:
        std::string     type        (void) const { return mType; }
        void            type        (const char* s) { mType = s; }
        void            type        (const std::string& s) { type(s.c_str()); }

        const lxvar     attr        (std::string name) const;
        void            attr        (std::string name, lxvar value);      

        ElementCPtr     parent      () const;
        ElementCPtr     child       (int i) const;
        int             childCount  (void) const;

        const ObjectPtr value       (void) const    { return mspValue; }
        ObjectPtr       value       (void)          { return mspValue; }
        void            value       (ObjectPtr spValue);

        void            prepend     (ElementPtr spElem);
        void            append      (ElementPtr spElem);

        ElementPtr      _clone () const;

    protected:
        typedef std::map<std::string, lxvar> AttrMap;
        typedef std::deque<ElementPtr>       ElemList;

        std::string mType;
        AttrMap     mAttributes;
        ElementPtr  mspParent;
        ElemList    mChildren;
        ObjectPtr   mspValue;      // May be a proxy object for delay-loading
    };

    typedef std::shared_ptr<Element> ElementPtr;

}}

