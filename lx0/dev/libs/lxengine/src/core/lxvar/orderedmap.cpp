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

        class lxorderedmap : public lxvalue
        {
        public:
            typedef std::map<std::string, lxvar> ValueMap;
            typedef std::map<size_t, std::string> OrderMap;

            class iterator_imp : public lxvalue_iterator
            {
            public:
                iterator_imp (lxorderedmap* pMap, OrderMap::iterator it) : mpMap(pMap), mIter(it) {}
                virtual lxvalue_iterator* clone     (void)                         { return new iterator_imp(mpMap, mIter); }

                virtual bool    equal               (const lxvalue_iterator& that) { return mIter == dynamic_cast<const iterator_imp&>(that).mIter; }
                virtual void    inc                 (void)                         { mIter++; }
                virtual std::string key             (void)                         { return mIter->second; }
                virtual lxvar   dereference         (void)                         { return mpMap->mValues.find(key())->second; }

            protected:
                lxorderedmap*       mpMap;
                OrderMap::iterator  mIter;
            };

            lxorderedmap();


            virtual bool        sharedType  (void) const { return true; }
            virtual lxvalue*    clone       (void) const;

            virtual bool        is_map      (void) const    { return true; }

            virtual int         size        (void) const { return int( mValues.size() ); }

            lxvar::iterator     begin           (void) { return lxvar::iterator(new iterator_imp(this, mOrder.begin())); }
            lxvar::iterator     end             (void) { return lxvar::iterator(new iterator_imp(this, mOrder.end())); }

            virtual bool        has         (const char* key) const { return mValues.find(key) != mValues.end(); }
            virtual lxvar*      find        (const char* key) const;
            virtual void        insert      (const char* key, lxvar& value);

            OrderMap     mOrder;
            ValueMap     mValues;
            size_t       mCount;
        };

        lxorderedmap::lxorderedmap()
            : mCount (0)
        {
        }

        lxvalue*    
        lxorderedmap::clone (void) const
        {
            auto* pClone = new lxorderedmap;
            pClone->mCount = 0;
            for (auto it = mOrder.begin(); it != mOrder.end(); ++it)
            {
                auto& value = mValues.find(it->second);
                pClone->mOrder.insert(std::make_pair(pClone->mCount++, it->second));
                pClone->mValues.insert(std::make_pair(it->second, value->second));
            }
            return pClone;
        }

        lxvar*
        lxorderedmap::find (const char* key) const
        {
            auto it = mValues.find(key);
            if (it != mValues.end())
                return const_cast<lxvar*>(&it->second);
            else
                return nullptr;
        }

        void 
        lxorderedmap::insert (const char* key, lxvar& value) 
        {
            auto it = mValues.find(key);
            if (it == mValues.end())
            {
                mOrder.insert(std::make_pair(mCount++, key));
                mValues.insert(std::make_pair(key, value));
            }
            else
            {
                // Retain the original ordering on replace operations
                mValues.erase(it);
                mValues.insert(std::make_pair(key, value));
            }
        }

        lxvalue* create_lxorderedmap    (void) { return new lxorderedmap; }

            }
        }
    }
}
