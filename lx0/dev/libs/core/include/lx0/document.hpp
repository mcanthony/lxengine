#pragma once

#include <memory>
#include <vector>

#include <lx0/detail/forward_decls.hpp>

namespace lx0 { namespace core {

    class Document
    {
    public:
        Document();
        TransactionPtr  transaction ();

        ElementCPtr     root() { return m_spRoot; }

    protected:

        typedef std::vector< TransactionWPtr > TrWList;

        TrWList m_openTransactions;

        ElementPtr  m_spRoot;
    };
}}
