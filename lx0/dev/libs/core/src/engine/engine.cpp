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

#include <iostream>
#include <string>

#include <v8/v8.h>
#include "../src/extern/tinyxml/tinyxml.h"

#include <lx0/core.hpp>
#include <lx0/engine.hpp>
#include <lx0/document.hpp>
#include <lx0/element.hpp>
#include <lx0/mesh.hpp>
#include <lx0/util.hpp>
#include <lx0/v8bind.hpp>

#include <OGRE/OgreRoot.h>
#include <OGRE/OgreSceneManager.h>

namespace lx0 { namespace core {

    namespace detail
    {
        ObjectCount::ObjectCount (size_t current)
            : mCurrent (current)
            , mTotal   (current)
        {
        }

        void   
        ObjectCount::inc (void)
        {
            mCurrent++;
            mTotal++;
        }

        void   
        ObjectCount::dec (void)
        {
            mCurrent--;
        }
    }

    using namespace detail;

    std::weak_ptr<Engine> Engine::s_wpEngine;

    Engine::Engine()
    {
        // Define a helper lambda function that returns a function (this effectively 
        // acts as runtime template function).
        auto prefix_print = [](std::string prefix) -> std::function<void(const char*)> {
            return [prefix](const char* s) { std::cout << prefix << s << std::endl; };
        };
        slotDebug   = prefix_print("DBG: ");
        slotLog     = prefix_print("LOG: ");
        slotWarn    = prefix_print("WARN: ");
        slotError   = prefix_print("ERROR: ");
        slotFatal   = prefix_print("FATAL: ");

        lx_log("lx::core::Engine ctor");
    }

    /*!
        Subject to future change.

        An explicit shutdown method, in addition to the normal destructor, is currently required
        to ensure a proper order of events for object destruction.
     */
    void
    Engine::shutdown()
    {
        // Explicitly free all references to shared objects so that memory leak checks will work
       m_documents.clear();
    }

    Engine::~Engine()
    {
       lx_log("lx::core::Engine dtor");

       // Check for memory leaks of Engine-related objects
       for (auto it = m_objectCounts.begin(); it != m_objectCounts.end(); ++it)
       {
           if (it->second.current() != 0)
               lx_warn("Leaked %u %s objects", it->second.current(), it->first.c_str());
           else
               lx_debug("Allocated %u %s objects.  0 leaked.", it->second.total(), it->first.c_str()); 
       }
    }

    /*!
        @todo This method is inefficient; but it is simple.  Until 1.0 is complete, simplicity
            is favored over efficiency.
     */
    void
    Engine::incObjectCount  (std::string name)
    {
        auto it = m_objectCounts.find(name);
        if (it == m_objectCounts.end())
            m_objectCounts.insert(std::make_pair(name, ObjectCount(1)));
        else
            it->second.inc();
    }

    void 
    Engine::decObjectCount  (std::string name)
    {
        auto it = m_objectCounts.find(name);
        lx_check_error (it != m_objectCounts.end());
        lx_check_fatal(it->second.current() >= 1);

        it->second.dec();
    }

    ElementPtr  
    Engine::_loadDocumentRoot (std::string filename)
    {
        //
        // Define a local structure within which the recursive loading function can be set
        //
        struct L
        {
            static ElementPtr build (TiXmlNode* pParent, int depth)
            {
                ElementPtr spElem (new Element);
         
                std::string value = pParent->Value();
                for (int i = 0; i < depth; i++)
                    std::cout << "    ";
                std::cout << value << std::endl;

                spElem->type(value);

                if (TiXmlElement* pTiElement = pParent->ToElement())
                {
                    for (TiXmlAttribute* pAttrib= pTiElement->FirstAttribute(); pAttrib; pAttrib = pAttrib->Next())
                    {
                        std::string name = pAttrib->Name();
                        std::string value = pAttrib->Value();
                        spElem->attr(name, lxvar(value.c_str()));
                    }
                }

                // This should be controlled in a more dynamic, pluggable fashion
                if (spElem->type() == "Mesh") 
                {
                    MeshPtr spMesh (new Mesh);

                    lxvar src = spElem->attr("src");
                    if (src.isDefined())
                    {
                        lxvar value = lx0::util::lx_file_to_json(src.asString().c_str());
                        spMesh->deserialize(value);
                        spElem->value(spMesh);
                    }
                }

                for (TiXmlNode* pChild = pParent->FirstChild(); pChild != 0; pChild = pChild->NextSibling())
                {
                    ElementPtr spLxElem = build(pChild, depth + 1);
                    spElem->append(spLxElem);
                }

                return spElem;
            }
        };

        ElementPtr spRoot(new Element);

        TiXmlDocument doc(filename.c_str());
        if (doc.LoadFile())
        {
            spRoot = L::build(doc.RootElement(), 0);
        }
        else
            spRoot.reset();

        return spRoot;
    }

    
    DocumentPtr
    Engine::loadDocument (std::string filename)
    {
        DocumentPtr spDocument(new Document);
        ElementPtr spRoot = _loadDocumentRoot(filename);
        spDocument->root(spRoot);
        
        // API Design question: does this belong here?  Is a load an implicit connection?
        // What is an "unconnected" document good for?
        this->connect(spDocument);

        return spDocument;
    }

    void    
    Engine::connect (DocumentPtr spDocument)
    {
        m_documents.push_back(spDocument);


        //
        // Not sure this is exactly the right place for the scripts to be run...
        //
        ElementPtr spRoot = spDocument->root();
        for (int i = 0; i < spRoot->childCount(); ++i)
        {
            ElementCPtr spChild = spRoot->child(i);
            if (spChild->type() == "Header")
            {
                for (int j = 0; j < spChild->childCount(); ++j)
                {
                    ElementCPtr spElem = spChild->child(j);
                    if (spElem->type() == "Script")
                    {
                        std::string language = spElem->attr("language").asString();
                        std::string src      = spElem->attr("src").asString();

                        std::string content = lx0::util::lx_file_to_string(src);
                        _runJavascript(spDocument, content);
                    }
                }
            }
        }
    }

	void   
	Engine::sendMessage (const char* message)
    {
        m_messageQueue.push_back(message);
    }

	int
	Engine::run()
	{
        while (!m_messageQueue.empty())
        {
            std::string msg = m_messageQueue.front();
            m_messageQueue.pop_front();
        }

        ///@todo Devise a better way to hand time-slices from the main loop to the individual documents
        /// for updates.  Also consider multi-threading.
        for(auto it = m_documents.begin(); it != m_documents.end(); ++it)
            (*it)->run();

		return 0;
	}

    /*
        This class is used to convert from V8 values to primitive types,
        including lxvar.  Better naming, unit testing, and completeness are
        all lacking from the current code.
     */
    struct Any
    {
        Any() : mValue( v8::Undefined() ) {}
        Any(v8::Handle<v8::Value>&v) : mValue(v) {}
        Any(v8::Handle<v8::Object>&v) : mValue(v) {}
        Any(int i) { mValue = v8::Integer::New(i); }
        Any(std::string s) 
        {
            mValue = v8::String::New(s.c_str());
        }

        v8::Handle<v8::Value> mValue;

        v8::Handle<v8::Value> handle() { return mValue; }

        operator std::string () 
        {
            v8::String::AsciiValue value (mValue);
            return *value;
        }
        operator int ()
        {
            return mValue->Int32Value();
        }
        operator lxvar ()
        {
            using namespace v8;

            if (mValue->IsUndefined())
            {
                return lxvar();
            }
            else if (mValue->IsString())
            {
                return lxvar( std::string( *this ).c_str() );
            }
            else if (mValue->IsArray())
            {
                Local<Array> arr = Array::Cast(*mValue);

                lxvar v;
                for (int i = 0; i < arr->Length(); ++i)
                {
                    Local<Value> e = arr->Get(i);
                    v.push( Any(e) );
                }
                return v;
            }
            else if (mValue->IsObject())
            {
                lx_error("Not valid");
            }
            else if (mValue->IsInt32())
            {
                return lxvar( int(*this) );
            }
            else if (mValue->IsNumber())
            {
                return lxvar( float( mValue->NumberValue() ) );
            }
            else if (mValue->IsExternal())
            {
                lx_error("Not valid");
            }
            else if (mValue->IsFunction())
            {
                lx_error("Not valid");
            }
            else
                lx_error("Cannot convert Javascript value to lxvar.");

            lx_error("Unreachable code.");
            return lxvar();
        }
    };

    /*
        Workaround until there's a better understanding of exposing C++ objects
        in V8 works.  For now, expose the object via an int handle and do a 
        mapping on every transition between C++/JS.  
     */
    class MappingTable
    {
    public:
        int add (ElementPtr spElem)
        {
            lx_check_error(spElem);

            int handle = int( mElems.size() );
            mElems.push_back(spElem);
            mHandles.insert(std::make_pair(spElem.get(), handle));
            return handle;
        }
        ElementPtr find (int i)
        {
            return mElems[i];
        }

        int find (ElementPtr spElem)
        {
            auto it = mHandles.find(spElem.get());
            lx_check_error(it != mHandles.end());
            return it->second;
        }

        int findOrAdd (ElementPtr spElem)
        {
            lx_check_error(spElem);

            auto it = mHandles.find(spElem.get());
            if (it != mHandles.end())
                return it->second;
            else
                return add(spElem);
        }

        void clear()
        {
            mHandles.clear();
            mElems.clear();
        }

        std::vector<ElementPtr> mElems;
        std::map<Element*,int> mHandles;
    };

    static MappingTable s_mappingTable;
    static DocumentPtr  s_spDocument;

    static v8::Handle<v8::Value> createElement (const v8::Arguments& args)
    {
        using namespace v8;

        std::string name = Any(args[0]);
        ElementPtr spElem( new Element );
        spElem->type(name);
        int handle = s_mappingTable.add(spElem);

        return Integer::New(handle);
    }

    static v8::Handle<v8::Value> appendElement (const v8::Arguments& args)
    {
        using namespace v8;

        int hParent = Any(args[0]);
        int hChild = Any(args[1]);
        ElementPtr spParent = s_mappingTable.find(hParent);
        ElementPtr spChild = s_mappingTable.find(hChild);
        
        spParent->append(spChild);
        return Undefined();
    }

    static v8::Handle<v8::Value> getElementById (const v8::Arguments& args)
    {
        using namespace v8;
        lx_check_error(args.Length() == 1);

        std::string id = Any(args[0]);
        ElementPtr spElem = s_spDocument->getElementById(id);

        // The code does not yet gracefully handle a failed search
        if (!spElem)
            lx_error("Could not find element with id '%s' in the document.", id.c_str());

        int hElem = s_mappingTable.findOrAdd(spElem);
        return Integer::New(hElem);
    }

    static v8::Handle<v8::Value> setAttribute (const v8::Arguments& args)
    {
        using namespace v8;

        int hElem = Any(args[0]);
        std::string name = Any(args[1]);
        lxvar value = Any(args[2]);

        ElementPtr spElem = s_mappingTable.find(hElem);
        spElem->attr(name, value);
        return Undefined();
    }

    static v8::Handle<v8::Value> print (const v8::Arguments& args)
    {
        using namespace v8;

        std::string name = Any(args[0]);
        std::cout << "JS print: " << name << std::endl;

        return Undefined();
    }

    void        
    Engine::_runJavascript (DocumentPtr spDocument, std::string sourceText)
    {
        using namespace v8;
        using namespace lx0::v8_bind;

        HandleScope handle_scope;
        Handle<ObjectTemplate> global = ObjectTemplate::New(); 

        // Stand-alone DOM functions.  These are place-holders which eventually should be
        // replaced with a "document" object in the global context.   The functions on
        // that object should mirror those on the HTML DOM.
        //
        global->Set(String::New("document_createElement"), FunctionTemplate::New(createElement));
        global->Set(String::New("document_getElementById"), FunctionTemplate::New(getElementById));
        global->Set(String::New("document_setAttribute"), FunctionTemplate::New(setAttribute));
        global->Set(String::New("document_append"), FunctionTemplate::New(appendElement));
        
        // Internal debugging methods to make development a little easier.
        //
        global->Set(String::New("__lx_print"), FunctionTemplate::New(print));
        
        // For the duration of the script, keep a mapping table of all the
        // referenced elements as well as the current document.  These are
        // stored in global variables since the invokation functions in V8
        // require statics.  It would be better to wrap this in some sort of
        // execution context object.
        //
        s_mappingTable.clear();
        s_spDocument = spDocument;

        {
            Persistent<Context> context = Context::New(0, global);
            Context::Scope context_scope(context);

            Handle<String> source = String::New(sourceText.c_str());
            Handle<Script> script = Script::Compile(source);

            Handle<Value> result = script->Run();

            context.Dispose();
        }

        s_spDocument.reset();
        s_mappingTable.clear();
    }

}}