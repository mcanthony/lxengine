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

#pragma once


    class WrapperBase : public lx0::core::lxvar_ns::detail::lxvalue
    {
    public:
        virtual bool        sharedType  (void) const    { return true; }
        virtual bool        isHandle    (void) const    { return true; }
        virtual void*       as2         (const type_info& type)          { return getData(type); }

    protected:
        class Data
        {
        public:
            virtual ~Data() {}
        };

        virtual void* getData (const type_info& type) const = 0;
    };

    template <typename T>
    class OpaqueWrapper : public WrapperBase
    {
    public:
        OpaqueWrapper (const T& t)
        {
            mpData.reset( new DataT<T>(t) );
        }

        virtual lx0::core::lxvar_ns::detail::lxvalue* clone  (void) const    { return new OpaqueWrapper<T>(*(T *)getData(typeid(T))); } 

    protected:
        template <typename T>
        class DataT : public Data
        {
        public:
            DataT(const T& t) : data(t) {}
            T data;
        };

        virtual void* getData (const type_info& type) const
        {  
            if (typeid(T) == type)
            {
                DataT<T>* pData = dynamic_cast<DataT<T>*>(mpData.get());
                return &pData->data;
            }
            else
                return nullptr;
        }

        std::unique_ptr<Data> mpData;
    };


template <typename T>
lx0::lxvar lxvar_wrap (const T& t)
{
    auto pValue = new OpaqueWrapper<T>(t);
    return lx0::lxvar(pValue);
}

template <typename T>
T& lxvar_unwrap (lx0::lxvar& v)
{
    auto pBase = dynamic_cast<WrapperBase*>( v.imp<WrapperBase>().get() );
    return *reinterpret_cast<T*>(pBase->as2(typeid(T)));
}
