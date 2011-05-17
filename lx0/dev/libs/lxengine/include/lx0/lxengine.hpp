//===========================================================================//
/*
                                   LxEngine

    LICENSE
    * MIT License (http://www.opensource.org/licenses/mit-license.php)

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

#ifndef LX0_LXENGINE_HPP
#define LX0_LXENGINE_HPP

//
// Version
//
enum 
{ 
    LXENGINE_VERSION_MAJOR = 0,
    LXENGINE_VERSION_MINOR = 0,
    LXENGINE_VERSION_REVISION = 1,
};

//
// Standard headers
//
#define NOMINMAX
#include <deque>
#include <vector>
#include <map>
#include <string>
#include <memory> 
#include <functional>


//
// Doxygen documentation
//
#include "lxengine_dox.hpp"


//
// LxEngine headers
//

#include <lx0/core/init/init.hpp>
#include <lx0/core/log/log.hpp>
#include <lx0/core/slot/slot.hpp>
#include <lx0/core/lxvar/lxvar.hpp>

#include <lx0/util/misc/util.hpp>

#include <lx0/engine/engine.hpp>
#include <lx0/engine/document.hpp>
#include <lx0/engine/element.hpp>
#include <lx0/engine/view.hpp>
#include <lx0/engine/controller.hpp>
#include <lx0/engine/transaction.hpp>

#include <lx0/util/math/noise.hpp>
#include <lx0/util/math/smooth_functions.hpp>
#include <lx0/util/misc/util.hpp>

using namespace lx0::core::log_ns;

#endif

