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

#include <cassert>

#include <lx0/engine/transaction.hpp>
#include <lx0/engine/element.hpp>

namespace lx0 { namespace engine_ns {

    ElementPtr  
    Transaction::write (ElementCPtr spElement)
    {
        // clone it
        // on submit, each element time-stamped
        // swap each written item with the new clone
        // mark each swapped out item as out-of-date (shared_ptr will discard
        // when no one else is using the old item, but the flag will indicate
        // that open for write / submit operation will succeed on the stale
        /// object).

        assert(spElement.get());

        ElementPtr spWritable = spElement->_clone();

        m_validations.push_back([spElement]() -> bool {
            // if submit time > spElement time, ok : else fail
            return true;
        });
        m_operations.push_back([spElement, spWritable]() -> bool {
            // spElement->_swap(spWriteable);
            return true;
        });

        return spWritable;
    }

    bool 
    Transaction::submit()
    {
        // document.lock();
        bool bValid = true;
        for (auto it = m_validations.begin(); it != m_validations.end(); ++it)
        {
            bValid &= (*it)();
        }

        // TODO: Need to check that there aren't any open references to the object
        // after the submit?  Otherwise, the Element opened for write could simply
        // be used again after the submit - but this would be directly modifying
        // the contents of the document outside the transaction mechanism.
        // This might have to be handled via a run-time flag "open" on the Element.

        if (bValid)
        {
            for (auto it = m_operations.begin(); it != m_operations.end(); ++it)
            {
                bool bOk = (*it)();

                if (!bOk)
                {
                    // stop loop
                    // attempt rollback if possible
                    // throw exception
                }
            }
        }
        
        
        // document.unlock();
        return bValid;
    }
}}
