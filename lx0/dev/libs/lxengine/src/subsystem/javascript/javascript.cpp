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

#include <lx0/lxengine.hpp>
#include <lx0/engine/engine.hpp>
#include <lx0/subsystem/javascript.hpp>

using namespace lx0::core;

namespace lx0 { namespace subsystem { namespace javascript_ns {

    class DocImp : public IJavascript
    {
    public:
        virtual void    onAttached          (DocumentPtr spDocument) 
        {
            mpDocument = spDocument.get();
        }

        virtual void run  (const std::string& source)
        {
            Engine::acquire()->workaround_runJavascript(mpDocument->shared_from_this(), source);
        }

        Document* mpDocument;
    };

    IJavascript* createIJavascript()
    {
        return new DocImp;
    }

    //===========================================================================//
    /*!
     */
    class Scripting : public Document::Component
    {
    public: 
        virtual void onAttached (DocumentPtr spDocument) 
        {
            spDocument->iterateElements([&](ElementPtr spElem) -> bool { 
                _onElementAddRemove(spElem, true); return false; 
            });
        }

        virtual void onElementAdded (DocumentPtr spDocument, ElementPtr spElem) 
        {
            _onElementAddRemove(spElem, true);
        }

    protected:
        void _onElementAddRemove (ElementPtr spElem, bool bAdd)
        {
            if (spElem->tagName() == "Script") 
            {
                std::string source;
                if (spElem->attr("src").is_string())
                    source = lx0::string_from_file(spElem->attr("src").as<std::string>());
                else
                    source = spElem->value().as<std::string>();

                spElem->document()->getComponent<lx0::IJavascript>("javascript")->run(source);
            }
        }
    };

    class JavascriptPlugin : public Engine::Component
    {
    public:
        virtual void    onDocumentCreated   (EnginePtr spEngine, DocumentPtr spDocument);
    };

    void JavascriptPlugin::onDocumentCreated (EnginePtr spEngine, DocumentPtr spDocument)
    {
        lx_log("Attaching Javascript components");
        lx_log("Attaching <Script/> component");

        spDocument->attachComponent("javascript", new DocImp);
        spDocument->attachComponent("scripting", new Scripting);
    }

    Engine::Component* createScriptHandler()
    {
        return new JavascriptPlugin;
    }

}}}
