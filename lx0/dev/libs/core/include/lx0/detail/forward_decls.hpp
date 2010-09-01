#pragma once

#include <memory>

namespace lx0 { namespace core {

#define _LX_FORWARD_DECL_PTRS(Klass) \
    class Klass; \
    typedef std::shared_ptr<Klass> Klass ## Ptr; \
    typedef std::shared_ptr<const Klass> Klass ## CPtr; \
    typedef std::weak_ptr<Klass> Klass ## WPtr; \
    typedef std::weak_ptr<const Klass> Klass ## CWPtr; 

    _LX_FORWARD_DECL_PTRS(Object);
    _LX_FORWARD_DECL_PTRS(Element);
    _LX_FORWARD_DECL_PTRS(Transaction);
    _LX_FORWARD_DECL_PTRS(Document);
    _LX_FORWARD_DECL_PTRS(Space);
    _LX_FORWARD_DECL_PTRS(Engine);
    _LX_FORWARD_DECL_PTRS(View);
    _LX_FORWARD_DECL_PTRS(Controller);

#undef _LX_FORWARD_DECL_PTRS

}}