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

        class lxstringmap : public lxvalue
        {
        public:
            class iterator_imp : public lxvalue_iterator
            {
            public:
                iterator_imp (std::map<std::string, lxvar>::iterator it) : mIter(it) {}
                virtual lxvalue_iterator* clone     (void)                         { return new iterator_imp(mIter); }

                virtual bool    equal               (const lxvalue_iterator& that) { return mIter == dynamic_cast<const iterator_imp&>(that).mIter; }
                virtual void    inc                 (void)                         { mIter++; }
                virtual std::string key             (void)                         { return mIter->first; }
                virtual lxvar   dereference         (void)                         { return mIter->second; }

            protected:
                std::map<std::string, lxvar>::iterator    mIter;
            };


            virtual bool        sharedType  (void) const { return true; }
            virtual lxvalue*    clone       (void) const;

            virtual bool        is_map      (void) const    { return true; }

            virtual int         size        (void) const { return int( mValue.size() ); }

            lxvar::iterator     begin           (void) { return lxvar::iterator(new iterator_imp(mValue.begin())); }
            lxvar::iterator     end             (void) { return lxvar::iterator(new iterator_imp(mValue.end())); }

            virtual bool        has         (const char* key) const { return mValue.find(key) != mValue.end(); }
            virtual lxvar*      find        (const char* key) const;
            virtual void        insert      (const char* key, lxvar& value);

            typedef std::map<std::string, lxvar> Map;
            Map mValue;
        };

        lxvalue*    
        lxstringmap::clone (void) const
        {
            lxstringmap* pClone = new lxstringmap;
            for (auto it = mValue.begin(); it != mValue.end(); ++it)
                pClone->mValue.insert( std::make_pair(it->first, it->second) );
            return pClone;
        }

        lxvar*
        lxstringmap::find (const char* key) const
        {
            auto it = mValue.find(key);
            if (it != mValue.end())
                return const_cast<lxvar*>(&it->second);
            else
                return nullptr;
        }

        void 
        lxstringmap::insert (const char* key, lxvar& value) 
        {
            mValue.erase(key);
            mValue.insert(std::make_pair(key, value));
        }

        lxvalue* create_lxstringmap     (void) { return new lxstringmap; }

            }
        }
    }
}
