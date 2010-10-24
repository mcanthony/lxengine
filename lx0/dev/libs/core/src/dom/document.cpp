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
#include <string>

#include <lx0/document.hpp>
#include <lx0/transaction.hpp>
#include <lx0/element.hpp>
#include <lx0/view.hpp>
#include <lx0/core.hpp>
#include <lx0/engine.hpp>
#include <lx0/lxvar.hpp>

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

    ElementPtr     
    Document::createElement (std::string type)
    {
        // At the moment, there's no need for this to be a method on Document - 
        // but eventually Document may want to track the elements it creates.
        
        ElementPtr spElem(new Element);
        spElem->type(type);
        return spElem;
    }

    /*
        This eventually needs to be cached, but for simplicity prior to v1.0,
        just naively walk the whole document and return the first matching
        element.
     */
    ElementPtr
    Document::getElementById (std::string id)
    {
        struct L
        {
            static ElementPtr findId (std::string id, ElementPtr spElem)
            {
                lxvar v = spElem->attr("id");
                if (v.isString() && v.asString() == id)
                    return spElem;

                for (int i = 0; i < spElem->childCount(); ++i)
                {
                    ElementPtr spMatch = findId(id, spElem->child(i));
                    if (spMatch)
                        return spMatch;
                }
                
                return ElementPtr();
            }
        };

        return L::findId(id, root());
    }

    void            
    Document::updateBegin()
    {
        for (auto it = m_views.begin(); it != m_views.end(); ++it)
            it->second->updateBegin();
    }

    void            
    Document::updateEnd()
    {
        for (auto it = m_views.begin(); it != m_views.end(); ++it)
            it->second->updateEnd();
    }

    void            
    Document::updateFrame ()
    {
        for (auto it = m_views.begin(); it != m_views.end(); ++it)
            it->second->updateFrame();
    }

}}
