#pragma once

#include <memory>

namespace lx0 { namespace core {

    class Object
    {
    public:
        typedef std::shared_ptr<Object> ObjectPtr;

        virtual ObjectPtr clone() const { return ObjectPtr(); }
    
    protected:

    };

    typedef std::shared_ptr<Object> ObjectPtr;

}}
