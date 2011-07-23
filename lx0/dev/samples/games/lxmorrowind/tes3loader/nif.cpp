//===========================================================================//
/*
                                   LxEngine

    LICENSE

    Copyright (c) 2011 athile@athile.net (http://www.athile.net)

    MIT License: http://www.opensource.org/licenses/mit-license.php

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

#include <lx0/lxengine.hpp>
#include <lx0/util/misc.hpp>

#include <niflib/niflib.h>
#include <niflib/obj/NiObject.h>
#include <niflib/obj/NiNode.h>
#include <niflib/obj/NiGeometry.h>
#include <niflib/obj/NiTriShape.h>
#include <niflib/obj/NiTriShapeData.h>
#include <niflib/obj/NiMaterialProperty.h>
#include <niflib/obj/NiTexture.h>
#include <niflib/obj/NiTextureProperty.h>
#include <niflib/obj/NiTexturingProperty.h>
#include <niflib/obj/NiTextureModeProperty.h>
#include <niflib/obj/NiImage.h>
#include <niflib/obj/NiRawImageData.h>
#include <niflib/gen/TexDesc.h>
#include <niflib/gen/TexSource.h>
#include <niflib/obj/NiPixelData.h>
#include <niflib/obj/NiSourceTexture.h>

#include "nif.hpp"

//===========================================================================//
//   I M P L E M E N T A T I O N
//===========================================================================//

/*
    Process the Niflib object and covnert it into a generic scene_group
    structure.
 */
static
std::shared_ptr<scene_group>
processNifObject (Niflib::NiObjectRef spObject, std::function<std::string (std::string)> textureNameToId)
{
    std::shared_ptr<scene_group> spGroup( new scene_group );

    if (Niflib::NiNodeRef spNode = Niflib::DynamicCast<Niflib::NiNode>(spObject))
    {
        auto children = spNode->GetChildren();
        for (auto it = children.begin(); it != children.end(); ++it)
        {
            if (Niflib::NiTriShapeRef spTriShape = Niflib::DynamicCast<Niflib::NiTriShape>(*it))
            {
                // Skip hidden elements
                if (!spTriShape->GetVisibility())
                    continue;

                //int material = spTriShape->GetActiveMaterial();
                std::string textureFilename;

                auto properties = spTriShape->GetProperties();
                for (auto jt = properties.begin(); jt != properties.end(); ++jt)
                {
                    if (Niflib::NiMaterialPropertyRef spMaterial = Niflib::DynamicCast<Niflib::NiMaterialProperty>(*jt))
                    {
                    }
                    else if (Niflib::NiTexturingPropertyRef spInfo = Niflib::DynamicCast<Niflib::NiTexturingProperty>(*jt))
                    {
                        int count = spInfo->GetTextureCount();
                        for (int i = 0; i < count; ++i)
                        {
                            Niflib::TexDesc desc = spInfo->GetTexture(0);
                            Niflib::NiSourceTextureRef spSource = desc.source;

                            if (!spSource->IsTextureExternal())
                            {
                                Niflib::NiPixelDataRef spData = spSource->GetPixelData();
                                int width = spData->GetWidth();
                                int height = spData->GetHeight();
                                auto texels = spData->GetColors();
                            }
                            else
                            {
                                textureFilename = spSource->GetTextureFileName();
                            }
                        }
                    }
                    else if (Niflib::NiTexturePropertyRef spTexture = Niflib::DynamicCast<Niflib::NiTextureProperty>(*jt))
                    {
                        Niflib::NiImageRef spImage = spTexture->GetImage();
                        bool bExternal = spImage->IsTextureExternal();
                        if (!bExternal)
                        {
                            Niflib::NiRawImageDataRef spData = spImage->GetRawImageData();
                            spData->GetIDString();
                        }
                    }
                }
                
                if (Niflib::NiTriBasedGeomDataRef spData = Niflib::DynamicCast<Niflib::NiTriBasedGeomData>(spTriShape->GetData()))
                {
                    spGroup->instances.resize( spGroup->instances.size() + 1 );
                    auto& primitive = *spGroup->instances.back().spPrimitive;
                    auto& transform = *spGroup->instances.back().spTransform;

                    //
                    // A mesh is being processed, so record the texture filename found earlier.
                    //
                    texture_handle texture;
                    texture.name = textureFilename;
                    spGroup->textures.push_back(texture);

                    lx0::lxvar graph;
                    graph["_type"] = "phong";
                    graph["diffuse"]["_type"] = "texture2d";
                    graph["diffuse"]["texture"] = textureNameToId(textureFilename);
                    graph["diffuse"]["uv"] = "fragUV";
                    spGroup->instances.back().material = graph;
                
                    //
                    // Let NifLib compute the full transform up to the parent
                    //
                    auto world = spTriShape->GetWorldTransform();
                    for (int i = 0; i < 16; ++i)
                        transform[i%4][i/4] = world[i%4][i/4];

                    //
                    // Copy the data from Niflib form to GLGeom form
                    //
                    auto indices = spData->GetTriangles();
                    primitive.indices.reserve(indices.size() * 3);
                    for (auto it = indices.begin(); it != indices.end(); ++it)
                    {
                        primitive.indices.push_back( it->v1 );
                        primitive.indices.push_back( it->v2 );
                        primitive.indices.push_back( it->v3 );
                    }
                
                    auto vertices = spData->GetVertices();
                    primitive.vertex.positions.reserve(vertices.size());
                    for (auto it = vertices.begin(); it != vertices.end(); ++it)
                    {
                        glgeom::point3f p(it->x, it->y, it->z);
                        primitive.vertex.positions.push_back(p);
                        primitive.bbox.merge(p);
                    }

                    auto normals = spData->GetNormals();
                    primitive.vertex.normals.reserve(vertices.size());
                    for (auto it = normals.begin(); it != normals.end(); ++it)
                        primitive.vertex.normals.push_back( glgeom::vector3f(it->x, it->y, it->z) );

                    auto colors = spData->GetColors();
                    primitive.vertex.colors.reserve(colors.size());
                    for (auto it = colors.begin(); it != colors.end(); ++it)
                    {
                        // Ignore alpha for now
                        primitive.vertex.colors.push_back( glgeom::color3f(it->r, it->g, it->b) );
                    }

                    auto channels = spData->GetUVSetCount();
                    primitive.vertex.uv.resize(channels);
                    for (short i = 0; i < channels; ++i)
                    {
                        auto uv = spData->GetUVSet(i);
                        primitive.vertex.uv[i].reserve(uv.size());
                        for (auto it = uv.begin(); it != uv.end(); ++it)
                            primitive.vertex.uv[i].push_back( glgeom::point2f(it->u, it->v) );
                    }

                    auto center = spData->GetCenter();
                    primitive.bsphere.center = glgeom::point3f(center.x, center.y, center.z);
                    primitive.bsphere.radius = spData->GetRadius();
                }
            }
        }
    }
    return spGroup;
}


std::shared_ptr<scene_group> 
readNifObject (std::istream& in, std::function<std::string (std::string)> textureNameToId)
{
    //
    // Use Niflib to read the stream into a Niflib data structure
    //
    Niflib::NifInfo info;
    auto spNifRoot = Niflib::ReadNifTree(in, &info);
    
    //
    // Now convert the Niflib object into a scene_group object
    //
    auto spGroup = processNifObject(spNifRoot, textureNameToId);
    
    return spGroup;
}
