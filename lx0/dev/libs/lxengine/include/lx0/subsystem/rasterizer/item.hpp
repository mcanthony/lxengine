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

namespace lx0
{
    namespace subsystem
    {
        namespace rasterizer_ns
        {
            namespace detail
            {
                /*
                    Provides a opaque "data" member for the class, but with some degree runtime of type
                    safety.  The data member gets wrapped in a class with a vtable, which
                    guarentees that dynamic_cast<> can be used when accessing the member.  The 
                    cost is an extra indirection between the data member and the class.
                */
                class WithUserData
                {
                public:
                    class Data
                    {
                    public:
                        virtual ~Data() {}
                    };

                    template <typename T>
                    class DataT : public Data
                    {
                    public:
                        DataT(const T& t) : data(t) {}
                        T data;
                    };

                    template <typename T>
                    void setData (const T& data)
                    {
                        mpData.reset( new DataT<T>(data) );
                    }

                    template <typename T>
                    T getData () 
                    {  
                        DataT<T>* pData = dynamic_cast<DataT<T>*>(mpData.get());
                        if (pData)
                            return pData->data;
                        else
                            return T();
                    }

                protected:
                    std::unique_ptr<Data> mpData;
                };
            }

            //===========================================================================//
            //! \ingroup lx0_subsystem_rasterizer
            class Instance
                : public detail::WithUserData
            {
            public:
                Instance() {}
        
                //weak_ptr<Target> wpTarget;  - probably should be a member of the RenderList layer or RenderAlgorithm?
                CameraPtr           spCamera;
                TransformPtr        spTransform;
                MaterialInstancePtr spMaterial2;
                GeometryPtr         spGeometry;
                LightSetPtr         spLightSet;

                glgeom::bsphere3f   bsphere;
            };

        }
    }
}
