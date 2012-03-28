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

#include <memory>

namespace lx0
{
    namespace subsystem 
    {
        namespace rasterizer_ns
        {

            //===========================================================================//
            //! 
            /*!
                \ingroup lx0_subsystem_rasterizer
                
                A full cache intended to eventually support:
                - Resource finding
                - Resource codec plug-ins
                - Automatic resource sharing
                - Resource usage tracking
                - Paging
                - Auto-reload
                - etc.
             */
            class MeshCache
            {
            public:
                            MeshCache   (std::shared_ptr<RasterizerGL> spRasterizer);
                            ~MeshCache  ();
    
                GeometryPtr acquire     (const char* filename);

                //@name Statistics
                //@{
                size_t      acquireCount    (void) const;
                size_t      cacheSize       (void) const;
                //@}

            protected:
                std::shared_ptr<RasterizerGL>       mspRasterizer;
                std::map<std::string,GeometryPtr>   mCache;

                struct
                {
                    size_t                          acquireCount;
                } mStats;
            };

            typedef std::shared_ptr<MeshCache> MeshCachePtr;

        }
    }
}
