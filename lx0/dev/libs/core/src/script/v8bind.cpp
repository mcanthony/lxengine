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

using namespace lx0::core;

namespace lx0 { namespace v8_bind {

    V8Bind gV8Bind;

    /*!
    @todo This leaks memory.  The native object is allocated here, but there's no
        notification mechanism to let the engine know that the script no longer
        is using the native object.
     */
    v8::Handle<v8::Object>
    V8Bind::newObject (std::string name)
    {
        Class& klass = mClasses.find(name)->second;
        return wrapObject(name, klass.mCtor());
    }

    v8::Handle<v8::Object>
    V8Bind::wrapObject (std::string name, void* pObject)
    {
        auto it = mClasses.find(name);
        lx_check_error(it != mClasses.end());
        
        Class& klass = it->second;
        v8::Handle<v8::Function> ctor = klass.mTemplate->GetFunction();
        v8::Handle<v8::Object> obj = ctor->NewInstance();
        obj->SetInternalField(0, v8::External::New(pObject));

        return obj;
    }
}}