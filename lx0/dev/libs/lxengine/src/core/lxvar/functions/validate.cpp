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
#include <lx0/core/log/log.hpp>
#include <lx0/util/misc/util.hpp>
#include <boost/format.hpp>

namespace lx0 { namespace core {  namespace lxvar_ns {      

    /*!
        \ingroup lx0_core_lxvar

        Returns a validation function that always fails, effectively
        making the variable read-only and unassignable.
     */
    ModifyCallback validate_readonly (void)
    {
        return [] (lxvar&) -> bool { return false; };
    }

    /*!
        \ingroup lx0_core_lxvar

        Returns a validation function that will only succeed if the
        input variable is of boolean type.
     */
    ModifyCallback validate_bool (void)
    {
        return [] (lxvar& v) -> bool { return v.is_bool(); };
    }

    /*!
        \ingroup lx0_core_lxvar

        Returns a validation function that will only succeed if the
        input variable is of string type.
     */
    ModifyCallback validate_string (void)
    {
        return [] (lxvar& v) -> bool { return v.is_string(); };
    }

    ModifyCallback validate_filename (void)
    {
        return [] (lxvar& v) -> bool { 
            if (v.is_string())
            {
                if (lx0::file_exists(v))
                    return true;
            }
            return false;
        };
    }

    /*!
        \ingroup lx0_core_lxvar

        Returns a validation function that will only succeed if the
        input variable is of integer type and within the given range
        (inclusive).
     */
    ModifyCallback validate_int_range (int min, int max)
    {
        return [min,max](lxvar& v) -> bool {
            if (v.is_int() && v.as<int>() >= min && v.as<int>() <= max)
                return true;
            else
                return false;
        };
    }

}}}
