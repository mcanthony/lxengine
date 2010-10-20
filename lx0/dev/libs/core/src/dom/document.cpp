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

#include <cassert>
#include <lx0/document.hpp>
#include <lx0/transaction.hpp>
#include <lx0/element.hpp>
#include <lx0/view.hpp>
#include <lx0/core.hpp>
#include <lx0/engine.hpp>

namespace lx0 { namespace core {

    Document::Document()
        : m_spRoot (new Element)
    {
        Engine::acquire()->incObjectCount("Document");
    }

    Document::~Document()
    {
        Engine::acquire()->decObjectCount("Document");
    }


    TransactionPtr 
    Document::transaction ()
    {
        assert(this);

        TransactionPtr sp(new Transaction);
        m_openTransactions.push_back(sp);
        return sp;
    }

    void            
    Document::connect (std::string name, ViewPtr spView)
    {
        m_views.insert(std::make_pair(name, spView));
        spView->attach(this);
    }

    void
    Document::disconnect (std::string name)
    {
        auto it = m_views.find(name);
        if (it != m_views.end())
        {
            it->second->detach(this);
            m_views.erase(it);
        }
        else
            lx_error("Could name find view '%s' on document.", name.c_str());
    }

    void            
    Document::run()
    {
        for (auto it = m_views.begin(); it != m_views.end(); ++it)
            it->second->run();
    }

}}