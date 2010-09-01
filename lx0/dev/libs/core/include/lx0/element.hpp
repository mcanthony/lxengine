#pragma once

#include <map>
#include <deque>
#include <memory>

#include <lx0/lxvar.hpp>


namespace lx0 { namespace core {

    class Object;

    class Element
    {
    public:
    protected:
        typedef std::shared_ptr<Object> ObjectPtr;
        typedef std::map<std::string, lxvar> AttrList;
        typedef std::deque< std::shared_ptr<Element> > ElemList;

        AttrList    attributes;
        ElemList    children;
        ObjectPtr   value;
    };

    typedef std::shared_ptr<Element> ElementPtr;

}}

