#pragma once

#include <memory>
#include <vector>

#include <lx0/detail/forward_decls.hpp>

namespace lx0 { namespace core {

    class Document
    {
    public:
        TransactionPtr  transaction ();

    protected:

        typedef std::vector< TransactionWPtr > TrWList;

        TrWList m_openTransactions;
    };
}}
