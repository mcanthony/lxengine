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

#include <glgeom/glgeom.hpp>
#include <glgeom/extension/primitive_buffer.hpp>
#include <glgeom/prototype/std_lights.hpp>

#include "lxextensions/material_handle.hpp"

//===========================================================================//
//    C O M P O N E N T S
//===========================================================================//

class ITES3Loader : public lx0::Engine::Component
{
public:
    static ITES3Loader*     create();
    virtual const char*     name() const { return s_name(); }
    static const char*      s_name() { return "ITES3Loader"; }

    virtual         ~ITES3Loader    (void) {}

    virtual void    initialize      (const char* path) = 0;
    virtual void    cell            (const char* id, scene_group& group) = 0;

};
