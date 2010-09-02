#pragma once

#include <memory>
#include <string>
#include <vector>

#include <lx0/detail/forward_decls.hpp>

namespace lx0 { namespace core {

    class Transaction
    {
    public:

        void        add (std::string path, ElementPtr spElem);

        void  openForRead   (std::string name);
        void  openForWrite  (std::string name);

        bool submit();
        void revert();

    protected:
        typedef std::vector<ObjectPtr>  WrList;
        typedef std::vector<ObjectCPtr> RdList;

        WrList  m_writeList;
        RdList  m_readList;
    };
}}
