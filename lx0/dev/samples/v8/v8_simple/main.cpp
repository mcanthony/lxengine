// Adapted directly from: http://code.google.com/apis/v8/get_started.html

#include <iostream>
#include <v8/v8.h>

using namespace v8;

int 
main (int argc, char** argv)
{
    HandleScope handle_scope;
    Persistent<Context> context = Context::New();
  
    Context::Scope context_scope(context);
    Handle<String> source = String::New("'Hello World!'");
    Handle<Script> script = Script::Compile(source);

    Handle<Value> result = script->Run();
  
    context.Dispose();

    // Convert the result to an ASCII string and print it.
    String::AsciiValue ascii(result);
    std::cout << *ascii << std::endl;

    return 0;
}
