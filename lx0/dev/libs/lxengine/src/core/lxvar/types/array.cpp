//===========================================================================//
/*
                                   LxEngine

    LICENSE

    Copyright (c) 2011 athile@athile.net (http://www.athile.net)

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

#include <lx0/core/log/log.hpp>
#include <lx0/core/lxvar/lxvar.hpp>

namespace lx0 
{ 
    namespace core 
    { 
        namespace lxvar_ns 
        {
            namespace detail
            {

        //===========================================================================//
        /*!
            Implementation for a generic array of lxvars.
            */
        class lxarray : public lxvalue
        {
        public:
            class iterator_imp : public lxvalue_iterator
            {
            public:
                iterator_imp (std::vector<lxvar>::iterator it) : mIter(it) {}
                virtual lxvalue_iterator* clone     (void)                         { return new iterator_imp(mIter); }

                virtual bool    equal               (const lxvalue_iterator& that) { return mIter == dynamic_cast<const iterator_imp&>(that).mIter; }
                virtual void    inc                 (void)                         { mIter++; }
                virtual lxvar   dereference         (void)                         { return *mIter; }

            protected:
                std::vector<lxvar>::iterator    mIter;

            };

            virtual bool        sharedType  (void) const { return true; }
            virtual lxvalue*    clone       (void) const;

            virtual int         size        (void) const { return int( mValue.size() ); }
            virtual lxvar*      at          (int i);
            virtual void        at          (int index, lxvar value);

            virtual void        push        (lxvar value) { mValue.push_back(value); }

            lxvar::iterator     begin           (void) { return lxvar::iterator(new iterator_imp(mValue.begin())); }
            lxvar::iterator     end             (void) { return lxvar::iterator(new iterator_imp(mValue.end())); }

            std::vector<lxvar> mValue;
        };

        lxvalue*    
        lxarray::clone (void) const
        {
            lxarray* pClone = new lxarray;
            pClone->mValue.reserve(mValue.size());
            for (auto it = mValue.begin(); it != mValue.end(); ++it)
                pClone->mValue.push_back(it->clone());
            return pClone;
        }

        lxvar*
        lxarray::at (int i)      
        {
            lx_check_error(i >= 0 && i < int(mValue.size()));
            return &mValue[i]; 
        }

        void 
        lxarray::at (int i, lxvar value)      
        {
            lx_check_error(i >= 0 && i < int(mValue.size()));
            mValue[i] = value;; 
        }

        lxvalue* create_lxarray() { return new lxarray; }

            }
        }
    }
}
