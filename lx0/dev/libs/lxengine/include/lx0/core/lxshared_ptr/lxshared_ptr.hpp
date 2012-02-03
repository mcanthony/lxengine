//===========================================================================//
/*
                                   LxEngine

    LICENSE

    Copyright (c) 2010-2012 athile@athile.net (http://www.athile.net)

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

#pragma once

namespace lx0
{
    namespace core
    {
        namespace lxshared_ptr_ns
        {

            //===========================================================================//
            //! Less flexible, more efficient alternative to std::shared_ptr<> (use for performance reasons)
            /*!
                \ingroup lx0_core_types

                This is effectively a boost::intrusive_ptr<>.   The fact that it
                (a) does not support weak pointers and (b) stores the reference
                count on the object rather than on the pointer itself, makes this
                noticeably faster and half the size of std::shared_ptr<>.  For a
                small, performance sensitive object like lxvar, using a custom
                class is warranted.

                Expects the type used to have _incRef() and _decRef() methods to modify
                the object's internal reference count.

                In all other cases, use a std::shared_ptr.  Keep it simple, standard, and
                recognizable to other developers.
             */
            template <typename T>
            struct lxshared_ptr
            {
                lxshared_ptr (void)
                    : mPtr (nullptr)
                {
                }

                lxshared_ptr (T* p) 
                    : mPtr (p)
                {
                    if (mPtr)
                        mPtr->_incRef();                    
                }
                ~lxshared_ptr ()
                {
                    if (mPtr)
                        mPtr->_decRef();
                }
                lxshared_ptr(const lxshared_ptr& that)
                    : mPtr (that.mPtr)
                {
                    if (mPtr)
                        mPtr->_incRef();
                }
                lxshared_ptr (lxshared_ptr&& that)
                {
                    mPtr = that.mPtr;
                    that.mPtr = nullptr;
                }

                void operator= (const lxshared_ptr& that)
                {
                    if (that.mPtr)
                        that.mPtr->_incRef();
                    if (mPtr)
                        mPtr->_decRef();
                    mPtr = that.mPtr;
                }

                void operator= (lxshared_ptr&& that)
                {
                    if (that.mPtr != mPtr)
                    {
                        if (mPtr)
                            mPtr->_decRef();
                        mPtr = that.mPtr;
                        that.mPtr = nullptr;
                    }
                }

                T*      operator->  () { return mPtr; }
                T*      get         () { return mPtr; }
                void    reset       () { if (mPtr) mPtr->_decRef(); mPtr = 0; }
                void    reset       (T* p) { if (p) p->_incRef(); if (mPtr) mPtr->_decRef(); mPtr = p; }

            protected:
                T* mPtr;
            };
        }
    }

    using namespace lx0::core::lxshared_ptr_ns;
}

