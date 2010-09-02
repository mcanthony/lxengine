#include <lx0/core.hpp>
#include <lx0/element.hpp>

namespace lx0 { namespace core {

    void    
    Element::prepend (ElementPtr spElem)
    {
    }

    void
    Element::append (ElementPtr spElem)
    {
        if (spElem->parent().get())
        {
            error("Cannot append element.  The element to be appended already has "
                "a parent");
            return;
        }
    }

}}