//===========================================================================//
/*
                                   LxEngine

    LICENSE
    * MIT License (http://www.opensource.org/licenses/mit-license.php)

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

//===========================================================================//
//   H E A D E R S   &   D E C L A R A T I O N S 
//===========================================================================//

#include <lx0/lxengine.hpp>
#include <lx0/subsystem/javascript.hpp>
#include <lx0/elements/core.hpp>


using namespace lx0;

namespace lec = lx0::elements::core_ns;

void lec::processIncludeDocument (DocumentPtr spDocument)
{
    EnginePtr spEngine = Engine::acquire();

    auto includes = spDocument->getElementsByTagName("IncludeDocument");
    for (auto it = includes.begin(); it != includes.end(); ++it)
    {
        std::string filename = (*it)->attr("src").as<std::string>();
        auto spParent = (*it)->parent();

        auto spDoc2 = spEngine->loadDocument(filename);
        spDoc2->iterateElements([&](ElementPtr spElem) -> bool {
            spParent->append( spElem->cloneDeep() );
            return false;
        });
        spEngine->closeDocument(spDoc2);
    }
}

static 
void
_runIfScriptElement (ElementPtr spElem)
{
    if (spElem->tagName() == "Script")
    {
        std::string language = spElem->attr("language").as<std::string>();
        std::string content;
        if (spElem->value().is_defined())
        {
            content = spElem->value().as<std::string>();
        }
        else
        {
            std::string filename = spElem->attr("src").as<std::string>();
            content = lx0::string_from_file(filename);
        }

        lx_check_error(language.empty() || language == "javascript");

        spElem->document()->getComponent<lx0::IJavascriptDoc>()->run(content);
    }
}

void 
lec::processHeaderScript (DocumentPtr spDocument)
{
    std::vector<std::string> scripts;
    ElementPtr spRoot = spDocument->root();
    for (int i = 0; i < spRoot->childCount(); ++i)
    {
        ElementPtr spChild = spRoot->child(i);
        if (spChild->tagName() == "Header")
        {
            for (int j = 0; j < spChild->childCount(); ++j)
            {
                ElementPtr spElem = spChild->child(j);
                _runIfScriptElement( spElem );
            }
        }
    }
}



Engine::Component*
lec::createProcessScriptElement (void)
{
    class DocComp : public Document::Component
    {
    public:
        virtual const char* name() const { return "ScriptHandler"; }

        virtual void    onElementAdded (DocumentPtr spDocument, ElementPtr spElem) 
        {
            _runIfScriptElement(spElem);
        }
    };

    class EngComp : public Engine::Component
    {
    public:
        virtual const char* name() const { return "ScriptHandler"; }
        virtual void onDocumentCreated (EnginePtr spEngine, DocumentPtr spDocument)
        {
            spDocument->attachComponent(new DocComp);
        }
    };

    return new EngComp;
}



//===========================================================================//
/*!
    */
class Scripting : public Document::Component
{
public: 
    virtual const char* name() const { return "scriptHandler2"; }

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

            spElem->document()->getComponent<lx0::IJavascriptDoc>()->run(source);
        }
    }
};

class JavascriptPlugin : public Engine::Component
{
public:
    virtual const char* name() const { return "scriptHandler"; }
    virtual void    onDocumentCreated   (EnginePtr spEngine, DocumentPtr spDocument);
};

void JavascriptPlugin::onDocumentCreated (EnginePtr spEngine, DocumentPtr spDocument)
{
    lx_log("Attaching <Script/> component");
    spDocument->attachComponent(new Scripting);
}

Engine::Component* lec::createScriptHandler()
{
    return new JavascriptPlugin;
}
