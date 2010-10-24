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
#include <vector>
#include <map>

#include <lx0/detail/forward_decls.hpp>

namespace lx0 { namespace core {

    class Document
    {
    public:
                        Document();
                        ~Document();

        TransactionPtr  transaction     (void);

        void            connect         (std::string name, ViewPtr spView);
        void            disconnect      (std::string name);

        ElementCPtr     root            (void) const        { return m_spRoot; }
        ElementPtr      root            (void)              { return m_spRoot; }
        void            root            (ElementPtr spRoot) { m_spRoot = spRoot; }

        ElementPtr      createElement   (std::string type);
        ElementPtr      getElementById  (std::string id);

        void            run             (void);

    protected:
        typedef std::vector< TransactionWPtr > TrWList;

        TrWList m_openTransactions;

        ElementPtr  m_spRoot;

        std::map<std::string, ViewPtr> m_views;
    };
}}
