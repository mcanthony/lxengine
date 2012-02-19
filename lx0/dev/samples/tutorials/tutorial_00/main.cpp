//===========================================================================//
/*
                                   LxEngine

    LICENSE

    Copyright (c) 2011-2012 athile@athile.net (http://www.athile.net)

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

//===========================================================================//
//   H E A D E R S   &   D E C L A R A T I O N S 
//===========================================================================//

#include <iostream>
#include <lx0/lxengine.hpp>

//===========================================================================//
//   E N T R Y - P O I N T
//===========================================================================//

int 
main (int argc, char** argv)
{
    int exitCode = -1;
    try
    {
        lx0::EnginePtr spEngine = lx0::Engine::acquire();
        spEngine->initialize();
        {
            //
            // Load up the Document containing the application data
            // 
            lx0::DocumentPtr spDocument = spEngine->loadDocument("media2/appdata/tutorial_00/document.xml");

            //
            // Find the element in the XML tree and extract details
            //
            lx0::ElementPtr spElement = spDocument->getElementById("my_phrase");
            const int repeat = spElement->getAttribute("repeat");
            lx0::lxvar value = spElement->value();
            std::string message = value["message"];
            std::string recepient = value["recepient"];

            //
            // Print the message
            //
            for (int i = 0; i < repeat; ++i)
                std::cout << message << ", " << recepient << "!" << std::endl;
        }
        spEngine->shutdown();
    }
    catch (std::exception& e)
    {
        throw lx_fatal_exception("Fatal: unhandled std::exception.\nException: %s\n", e.what());
    }

    return exitCode;
}
