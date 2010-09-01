#include <lx0/document.hpp>

namespace lx0 { namespace core {

    TransactionPtr 
    Document::transaction ()
    {
        TransactionPtr sp;
        m_openTransactions.push_back(sp);
        return sp;
    }

}}
