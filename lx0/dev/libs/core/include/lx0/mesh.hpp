//===========================================================================//
/*
                                   LxEngine

    LICENSE
    * MIT License (http://www.opensource.org/licenses/mit-license.php)

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

#pragma once

#include <lx0/detail/forward_decls.hpp>
#include <lx0/lxvar.hpp>
#include <lx0/object.hpp>
#include <lx0/point3.hpp>

namespace lx0 { namespace core {

    //===========================================================================//
    //!
    /*!
     */   
    class Mesh : public lx0::core::detail::lxvalue
    {
    public:
        Mesh (lxvar& src);


        virtual lxvalue*    clone       (void) const { return nullptr; }

        float           boundingRadius  (void);
        vector3         boundingVector  (void);


    //protected:
        struct Quad
        {
            int index[4];
        };

        std::vector<point3> mVertices;
        std::vector<Quad>   mFaces;

    };
    typedef lx0::core::detail::lxshared_ptr<lx0::core::detail::lxvalue> LxValuePtr;
    typedef lx0::core::detail::lxshared_ptr<Mesh> MeshPtr;

}}

