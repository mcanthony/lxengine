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

#include <lx0/core.hpp>
#include <lx0/v8bind.hpp>
#include <lx0/util.hpp>

using namespace lx0::core;
using namespace v8;
using v8::Object;

namespace lx0 { namespace core { namespace v8bind
{
    
    _V8Context::_V8Context ()
    {
        HandleScope handle_scope;
        Handle<ObjectTemplate> global_templ = ObjectTemplate::New(); 

        context = Context::New(0, global_templ);
    }

    _V8Context::~_V8Context ()
    {
        context.Dispose();
    }

    void
    _V8Context::runFile (const char* filename)
    {
        lx_debug("Running JS file '%s'", filename);

        try
        {
            std::string text = lx0::util::lx_file_to_string(filename);
        
            Context::Scope context_scope(context);
        
            HandleScope handle_scope;
            Handle<String> source = String::New(text.c_str());
            Handle<Script> script = Script::Compile(source);
 
            Handle<Value> result = script->Run();
        }
        catch (std::exception& e)
        {
            lx_error("Exception attempting run javascript script!");
            throw e;
        }
    }

}}}