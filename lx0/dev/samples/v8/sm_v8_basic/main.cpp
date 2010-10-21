// Adapted directly from: http://code.google.com/apis/v8/get_started.html

#include <iostream>
#include <functional>
#include <string>
#include <v8/v8.h>

using namespace v8;

void print (std::string s)
{
    std::cout << s;
}

Handle<Value> 
func (const Arguments& args) 
{
    if (args.Length() != 1) 
    {
        return v8::Undefined();
    }
    else
    {
        HandleScope scope;
        Handle<Value> arg = args[0];
        String::AsciiValue value (arg);
        print(*value);
        return v8::Undefined();
    }
};


int 
main (int argc, char** argv)
{
    HandleScope handle_scope;
    
    Handle<ObjectTemplate> global = ObjectTemplate::New();
    global->Set(String::New("print"), FunctionTemplate::New(func));


    Persistent<Context> context = Context::New(0, global);
  
    Context::Scope context_scope(context);
    Handle<String> source = String::New("print('Hello '); 'World!'");
    Handle<Script> script = Script::Compile(source);

    Handle<Value> result = script->Run();
  
    context.Dispose();

    // Convert the result to an ASCII string and print it.
    String::AsciiValue ascii(result);
    std::cout << *ascii << std::endl;

    return 0;
}
