//===========================================================================//
/*
                                   LxEngine

    LICENSE
    * MIT License (http://www.opensource.org/licenses/mit-license.php)

    Copyright (c) 2010-2011 athile@athile.net (http://www.athile.net)

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

//===========================================================================//
//   H E A D E R S
//===========================================================================//

#include <cassert>
#include <string>

#include <lx0/lxengine.hpp>

namespace {

    struct Profile
    {
        Profile() { ::memset(this, 0, sizeof(*this)); }

        int _inited;
        int update;
        
        void initialize()
        {
            if (!_inited)
            {
                _inited = 1;
                auto spEngine = lx0::Engine::acquire();
                spEngine->registerProfileCounter("Document update", &update);
            }
        }
    } profile;
}

namespace lx0 { namespace engine_ns {

    Document::Document()
        : m_spRoot     ( new Element )
        , m_documentId (0)
    {
        auto spEngine = Engine::acquire();
        profile.initialize();
        spEngine->incObjectCount("Document");
        m_documentId = spEngine->generateId();

        lx_log("Constructed Document %1%", m_documentId);

        m_spRoot->notifyAdded(this);
    }

    /*!
        @todo The shutdown sequence here is not well-defined. Given the use of
        shared_ptrs throughout, it is hard to definitely avoid cycles and 
        out-of-order resource releases without a better shutdown design.
    */
    Document::~Document()
    {       
        this->iterateElements2([](ElementPtr spElem){
            spElem->clearComponents();
        });

        mViews.clear();
        m_spRoot->removeAll();
        _clearComponents();
        m_spRoot->notifyRemoved(this);

        lx_log("Destructed Document %1%", m_documentId);

        Engine::acquire()->decObjectCount("Document");
    }

    //! Reserved for future use.  Do not use.
    /*!
     */
    TransactionPtr 
    Document::transaction ()
    {
        assert(this);

        TransactionPtr sp(new Transaction);
        m_openTransactions.push_back(sp);
        return sp;
    }

    ViewPtr
    Document::createView (std::string type, std::string name)
    {
        ViewPtr spView( new View(type, this) );

        mViews.insert(std::make_pair(name, spView));

        // Forward the events along so they can be caught at any level
        spView->slotKeyDown += [&](KeyEvent& e) { this->slotKeyDown(e); };

        return spView;
    }

    //!
    /*!
     */
    ViewPtr 
    Document::createView (std::string type, std::string name, lx0::View::Component* pRenderer)
    {
        auto spView = createView(type, name);
        spView->attachComponent(pRenderer);
        return spView;
    }

    ViewPtr 
    Document::createView (std::string type, std::string name, std::string rendererName)
    {
        auto pRenderer = Engine::acquire()->createViewComponent(rendererName);
        return createView(type, name, pRenderer);
    }
    
    void
    Document::destroyView (std::string name)
    {
        auto it = mViews.find(name);
        if (it != mViews.end())
        {
            mViews.erase(it);
        }
        else
            throw lx_error_exception("Could name find view '%s' on document.", name.c_str());
    }
    
    //---------------------------------------------------------------------------//
    /*!
     */
    ViewPtr
    Document::view (int index)
    {
        auto it = mViews.begin();
        while (index--)
            it++;

        return it->second;
    }

    //---------------------------------------------------------------------------//
    //! Sets the root Element of the Document
    /*!
     */
    void
    Document::root (ElementPtr spRoot) 
    {
        ElementPtr spOldRoot = m_spRoot;
        m_spRoot = spRoot; 

        spOldRoot->notifyRemoved(this);
        m_spRoot->notifyAdded(this);
    }

    //---------------------------------------------------------------------------//
    //! Create a new Element that can be added to the Document
    /*!
     */
    ElementPtr     
    Document::createElement (std::string tagName)
    {
        // At the moment, there's no need for this to be a method on Document - 
        // but eventually Document may want to track the elements it creates.
        
        ElementPtr spElem(new Element);
        spElem->tagName(tagName);

        slotElementCreated(spElem);

        return spElem;
    }

    static void _iterateElements2Imp (ElementPtr spElem, std::function<void (ElementPtr)> f)
    {
        f(spElem);
        for (int i = 0; i < spElem->childCount(); ++i)
            _iterateElements2Imp(spElem->child(i), f);
    }

    void          
    Document::iterateElements2     (std::function<void (ElementPtr)> f)
    {
        _iterateElements2Imp(root(), f);
    }

    static
    bool 
    _walkElementsImp (std::function<bool (ElementPtr)> f, ElementPtr spElem)
    {
        if (f(spElem))
            return true;

        const auto count = spElem->childCount();
        auto pElem = spElem.get();
                
        for (int i = 0; i < count; ++i)
        {
            if (_walkElementsImp(f, pElem->child(i)))
                return true;
        }
                
        return false;
    }

    /*!
        Helper method that traverses the Elements in the Document from root down
        (depth-first) and calls the function f on each Element.  The traversal
        will stop if f() returns true or all elements have been visited.
     */
    bool
    Document::_walkElements (std::function<bool (ElementPtr)> f)
    {
        return _walkElementsImp(f, root());
    }

    bool
    Document::_containsElement (ElementPtr spElem)
    {
        bool bFound = _walkElements([&](ElementPtr spCurrent) {
            return spCurrent.get() == spElem.get();
        });
        return bFound;
    }

    /*!
        @todo This eventually needs to be cached, but for simplicity prior to v1.0,
        just naively walk the whole document and return the first matching
        element.  The caching of course will need to respond to Element additions,
        removals, and modifications...
     */
    ElementPtr
    Document::getElementById (std::string id)
    {
        ElementPtr spMatch;
        _walkElements([&](ElementPtr spElem) -> bool {
            lxvar v = spElem->attr("id");
            if (v.is_string())
            {
                std::string elemId = v.as<std::string>();
                if (elemId == id)
                {
                    spMatch = spElem;
                    return true;
                }
            }
            return false;
        });
        return spMatch;
    }

    std::vector<ElementPtr> 
    Document::getElementsByTagName (std::string name)
    {
        lx_check_error(this != nullptr);

        std::vector<ElementPtr> matches;
        _walkElements([&](ElementPtr spElem) -> bool {
            if (spElem->tagName() == name)
                matches.push_back(spElem);
            return false;
        });
        return matches;
    }

    /*!
        The current implementation is not efficient.  Depending on how often
        this API gets used, it should either cache the list of Elements or the
        API should be deprecated.
     */
    std::vector<ElementPtr> 
    Document::getElements (void)
    {
        lx_check_error(this != nullptr);

        std::vector<ElementPtr> matches;
        _walkElements([&](ElementPtr spElem) -> bool {
            matches.push_back(spElem);
            return false;
        });
        return matches;
    }

    void            
    Document::beginRun ()
    {
        for (auto it = mViews.begin(); it != mViews.end(); ++it)
            it->second->runBegin();
    }

    void            
    Document::endRun ()
    {
        for (auto it = mViews.begin(); it != mViews.end(); ++it)
            it->second->runEnd();
    }

    void            
    Document::update ()
    {
        lx0::ProfileSection section(profile.update);

        _foreach ([&](ComponentPtr it) {                        
            it->onUpdate(shared_from_this());
        });          
        
        slotUpdateRun();

        for (auto it = mElementsWithUpdate.begin(); it != mElementsWithUpdate.end(); ++it)
        {
            auto& pElem = *it;
            pElem->notifyUpdate(this);
        }
        

        for (auto it = mViews.begin(); it != mViews.end(); ++it)
            it->second->update();
    }

    void
    Document::updateFrame (void)
    {
        if (!mViews.empty())
        {
            auto spThis = shared_from_this();
            for (auto it = mViews.begin(); it != mViews.end(); ++it)
                it->second->updateFrame(spThis);
        }
    }

    void 
    Document::notifyElementAdded (ElementPtr spElem)
    {
        // Automatically attach all registered Element components for the given tag
        //
        auto comps = Engine::acquire()->elementComponents();

        auto jt = comps.find( spElem->tagName() );
        if (jt != comps.end())
        {
            for (auto it = jt->second.begin(); it != jt->second.end(); ++it)
            {
                // Don't add it twice.  This theoretically could happen if the element is added
                // to the document, then removed, and re-added.  
                const auto& name = it->first;
                auto& ctor = it->second;
                if (spElem->getComponent<Element::Component>(name).get() == nullptr)
                {
                    spElem->attachComponent((ctor)(spElem));
                }
            }
        }

        // Pass on notification to attached components and slots
        //
        _foreach ([&](ComponentPtr it) {
            it->onElementAdded(shared_from_this(), spElem);
        });   
        slotElementAdded(spElem);

        notifyFlagsModified(spElem.get());
    }

    void 
    Document::notifyElementRemoved (ElementPtr spElem)
    {
        //
        // Remove from the cached list
        //
        mElementsWithUpdate.erase(spElem.get());

        _foreach ([&](ComponentPtr it) {
            it->onElementRemoved(this, spElem);
        });  
        slotElementRemoved(spElem);
    }

    void
    Document::notifyFlagsModified (Element* pElem)
    {
        //
        // Update any cached information regarding this Element's flags
        //
        if (pElem->flagNeedsUpdate())
            mElementsWithUpdate.insert(pElem);
        else
            mElementsWithUpdate.erase(pElem);
    }

    void        
    Document::sendEvent (std::string evt, lxvar params)
    {
        for (auto it = mControllers.begin(); it != mControllers.end(); ++it)
            (*it)->handleEvent(evt, params);

        for (auto it = mViews.begin(); it != mViews.end(); ++it)
            (*it).second->sendEvent(evt, params);
    }

    void        
    Document::addController (Controller* pController)
    {
        mControllers.push_back( ControllerPtr(pController) );
    }

}}
