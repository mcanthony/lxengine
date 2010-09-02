#pragma once

#include <map>
#include <deque>
#include <memory>

#include <lx0/detail/forward_decls.hpp>
#include <lx0/lxvar.hpp>


namespace lx0 { namespace core {

    class Object;

    class Element
    {
    public:
        ElementCPtr     parent() const  { return m_spParent; }

        void    prepend (ElementPtr spElem);
        void    append  (ElementPtr spElem);

    protected:
        typedef std::shared_ptr<Object> ObjectPtr;
        typedef std::map<std::string, lxvar> AttrList;
        typedef std::deque< std::shared_ptr<Element> > ElemList;

        AttrList    m_attributes;
        ElementPtr  m_spParent;
        ElemList    m_children;
        ObjectPtr   m_spValue;
    };

    typedef std::shared_ptr<Element> ElementPtr;

}}

