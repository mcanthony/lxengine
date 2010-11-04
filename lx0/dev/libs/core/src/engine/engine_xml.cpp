//===========================================================================//
/*
                                   LxEngine

    LICENSE

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

//===========================================================================//
//   H E A D E R S
//===========================================================================//

#include <iostream>
#include <string>

#include "../src/extern/tinyxml/tinyxml.h"

#include <lx0/core.hpp>
#include <lx0/engine.hpp>
#include <lx0/document.hpp>
#include <lx0/element.hpp>
#include <lx0/mesh.hpp>
#include <lx0/util.hpp>

using namespace lx0::util;

//===========================================================================//
//   I M P L E M E N T A T I O N 
//===========================================================================//

namespace lx0 { namespace core {

    
    ElementPtr  
    Engine::_loadDocumentRoot (DocumentPtr spDocument, std::string filename)
    {
        //
        // Define a local structure within which the recursive loading function can be set
        //
        struct L
        {
            static ElementPtr build (DocumentPtr spDocument, TiXmlElement* pTiElement, int depth)
            {
                
                ElementPtr spElem ( spDocument->createElement() );
         
                std::string tagName = pTiElement->Value();
                spElem->tagName(tagName);


                for (TiXmlAttribute* pAttrib= pTiElement->FirstAttribute(); pAttrib; pAttrib = pAttrib->Next())
                {
                    std::string name = pAttrib->Name();
                    std::string value = pAttrib->Value();
                    lxvar parsedValue = lxvar::parse(value.c_str());
                    spElem->attr(name, parsedValue);
                }

                size_t childElemCount = 0;
                size_t childTextCount = 0;
                std::string elemText;
                for (TiXmlNode* pChild = pTiElement->FirstChild(); pChild != 0; pChild = pChild->NextSibling())
                {
                    if (TiXmlElement* pElement = pChild->ToElement())
                    {
                        childElemCount++;

                        ElementPtr spLxElem = build(spDocument, pElement, depth + 1);
                        spElem->append(spLxElem);
                    }
                    else if (TiXmlText* pText = pChild->ToText())
                    {
                        childTextCount++;
                        elemText.append( pText->Value() );
                    }
                }

                lx_check_error(childElemCount == 0 || childTextCount == 0, "Unexpected XML element with both text and child nodes!");

                // This is a special-case that needs to be removed eventually.
                //
                lxvar elemValue;
                lxvar srcAttr = spElem->attr("src");
                if (srcAttr.isDefined() 
                    && (tagName == "Mesh" || tagName == "Camera"))
                {
                    if (!elemText.empty())
                        lx_warn("Element has both a 'src' attribute and an inline value!  "
                                "The src attribute overrides the value.");

                    elemValue = lx0::util::lx_file_to_json(srcAttr.asString().c_str());
                }
                else if (!elemText.empty())
                {
                    elemValue = lxvar::parse(elemText.c_str());
                }

                ///@todo This doesn't have a clean fit with the rest of the architecture...
                //
                // This should be controlled in a more dynamic, pluggable fashion
                if (tagName == "Mesh") 
                {
                    MeshPtr spMesh (new Mesh);
                    spMesh->deserialize(elemValue);
                    spElem->value(spMesh);
                }
                else
                {
                    LxVarObjectPtr spGeneric (new LxVarObject);
                    spGeneric->deserialize(elemValue);
                    spElem->value(spGeneric);
                }


                return spElem;
            }
        };

        ElementPtr spRoot = spDocument->createElement();

        TiXmlDocument doc(filename.c_str());
        if (doc.LoadFile())
        {
            spRoot = L::build(spDocument, doc.RootElement(), 0);
        }
        else
            spRoot.reset();

        return spRoot;
    }
 
}}

