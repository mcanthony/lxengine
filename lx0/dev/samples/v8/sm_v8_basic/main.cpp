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

//===========================================================================//
//   H E A D E R S   &   D E C L A R A T I O N S 
//===========================================================================//


// Adapted directly from: http://code.google.com/apis/v8/get_started.html

#include <iostream>
#include <functional>
#include <string>
#include <memory>
#include <vector>
#include <map>
#include <v8/v8.h>

#include <lx0/util.hpp>
#include <lx0/v8bind.hpp>

using namespace v8;

//
// C++ code
//
int objectCount = 0;
class LxQuerySet
{
public:
    LxQuerySet() { objectCount++; }
    ~LxQuerySet() { objectCount--; }

    void append (std::string s)
    {
        std::cout << "append called: " << s << std::endl;
    }
};

LxQuerySet* LxQuery (std::string s)
{
    std::cout << "LxQuery called: " << s << std::endl;
    return new LxQuerySet;
}

using lx0::v8_bind::gV8Bind;

int 
main (int argc, char** argv)
{
    HandleScope handle_scope;
    Handle<ObjectTemplate> global = ObjectTemplate::New();

    struct InvokeLxQuery : public lx0::v8_bind::FunctionWrapper {
        void imp() { mReturn = wrap("LxQuerySet", LxQuery(mA[0])); }
    };
    gV8Bind.addFunction<InvokeLxQuery>(global, "$");

    gV8Bind.addClass<LxQuerySet>("LxQuerySet");
    struct InvokeAppend : public lx0::v8_bind::MethodWrapper<LxQuerySet> { 
        void imp () { mpObj->append(mA[0]); }
    };
    gV8Bind.addMethod<InvokeAppend>("LxQuerySet", "append");
    

    Persistent<Context> context = Context::New(0, global);
  
    Context::Scope context_scope(context);
    Handle<String> source = String::New(lx0::util::lx_file_to_string("script.js").c_str());
    Handle<Script> script = Script::Compile(source);

    Handle<Value> result = script->Run();

    context.Dispose();

    return 0;
}
