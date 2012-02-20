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

#include <bullet/btBulletDynamicsCommon.h>

_LX_FORWARD_DECL_PTRS(btCollisionShape);

namespace lx0
{
    namespace subsystem
    {
        namespace physics_ns
        {
            namespace detail
            {

                //-----------------------------------------------------------------------//
                //! Base class for the Shape cache keys.
                /*!
                    The shape caches are used to ensure that if two objects with the same
                    size bounding box (or bounding sphere, for example) will reuse the
                    same btCollisionShape object.  

                    The ShapeKey is responsible for:

                    (1) Generating a key from specific shape parameters: for example, it
                        ensures that an object with a bounding sphere of size 1.00001
                        and a second sphere of size 1.00002 will share the same collision
                        shape - rather than needlessly creating two very similar objects.

                    (2) Knowing how to generate a new collision shape when a desired 
                        shape does not already exist in the cache.

                    (3) Providing a comparison operator so the key can be used with 
                        std::map<>.

                 */
                struct ShapeKeyBase
                {
                    const float granularity() const 
                    {
                        return 0.05f;
                    }
                };

                //-----------------------------------------------------------------------//
                //! Key used for caching btBoxShape
                /*!
                    Determines (a) which cache element to use for a given bounding box, (b) how to 
                    create new a cache element for the bounds if one does not exist.
                 */
                struct BoxKey : public ShapeKeyBase
                {
                    BoxKey (const glgeom::vector3f& halfBounds)
                    {
                        // Create an integer-based key.  This rounds similar decimal 
                        // values such that they will generate the same integer key - and 
                        // thus reuse the same collision shape.
                        //
                        for (int i = 0; i < 3; ++i)
                            xyz[i] = int( ceil(halfBounds[i] / granularity() ) );
                    }

                    btCollisionShape* createShape() const
                    {
                        // Multiply back rather than using the original halfBounds so that
                        // the same key always generates the same collision same.  Otherwise,
                        // the collision shape size might vary slightly depending on which
                        // object first generated the key.
                        //
                        btVector3 bulletVec;
                        bulletVec.setX( xyz[0] * granularity() );
                        bulletVec.setY( xyz[1] * granularity() );
                        bulletVec.setZ( xyz[2] * granularity() );

                        return new btBoxShape(bulletVec);
                    }

                    bool operator< (const BoxKey& that) const
                    {
                        for (int i = 0; i < 3; ++i)
                        {
                            int cmp = xyz[0] - that.xyz[0];
                            if (cmp < 0)
                                return true;
                            else if (cmp > 0)
                                return false;
                        }
                        return false;
                    }
                    int xyz[3];
                };

                //-----------------------------------------------------------------------//
                //! Key used for caching btSphereShape
                /*!
                    Determines (a) which cache element to use for a given bounding sphere, (b) how to 
                    create new a cache element for the bounds if one does not exist.
                 */
                struct SphereKey : public ShapeKeyBase
                {
                    SphereKey (float radius)
                    {
                        lx_check_error(radius >= 0.0f);

                        // Only respect a limited granularity so that similar shapes will simply be
                        // reused
                        key = int( ceil(radius /  granularity()) );
                    }

                    bool operator< (const SphereKey& that) const
                    {
                        int cmp = key - that.key;
                        return (cmp < 0) ? true : false;
                    }

                    btCollisionShape* createShape() const
                    {
                        const float fCachedRadius = key * granularity();
                        return new btSphereShape(fCachedRadius);
                    }

                    int key;
                };

                //-----------------------------------------------------------------------//
                //! Key used for caching btCapsuleShape
                /*!
                    Determines (a) which cache element to use for a given bounds, (b) how to 
                    create new a cache element for the bounds if one does not exist.
                 */
                struct CapsuleKey : public ShapeKeyBase
                {
                    CapsuleKey (float width, float height)
                    {
                        lx_check_error(width >= 0.0f);
                        lx_check_error(height >= 0.0f);

                        // Only respect a limited granularity so that similar shapes will simply be
                        // reused.
                        //
                        // Note: this sets a limit of about 0.05 to 3,250 for the sizes...
                        //
                        auto key0 = unsigned short( ceil(width /  granularity()) );
                        auto key1 = unsigned short( ceil(height /  granularity()) );
                        key = (key0 << 16) | key1;
                    }

                    bool operator< (const CapsuleKey& that) const
                    {
                        int cmp = key - that.key;
                        return (cmp < 0) ? true : false;
                    }

                    btCollisionShape* createShape() const
                    {
                        const float width = float((key >> 16) & 0xFFFF) * granularity();
                        const float height = float(key & 0xFFFF) * granularity();
                        return new btCapsuleShape(width, height);
                    }

                    int key;
                };

                //-----------------------------------------------------------------------//
                //! Wrapper on std::map<> to manage cached btCollisionShapes
                /*!
                 */
                template <typename KeyType>
                class ShapeCache
                {
                public:
                    typedef         KeyType     Key;

                    btCollisionShapePtr acquire (const Key& key)
                    {  
                        auto it = mCache.find(key);
                        if (it == mCache.end())
                            it = mCache.insert(std::make_pair(key, btCollisionShapeWPtr() )).first;

                        btCollisionShapeWPtr& wpShape = it->second;
                        btCollisionShapePtr spShape = wpShape.lock();
                        if (!spShape)
                        {
                            spShape.reset( key.createShape() );
                            wpShape = spShape;
                        }

                        lx_check_error( spShape.get() != nullptr );
                        return spShape;
                    }

                protected:
                    std::map<Key, btCollisionShapeWPtr>     mCache;
                };

            }
        }
    }
}
