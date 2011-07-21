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

//===========================================================================//
//! Simplified physics model used by LxMorrowind
/*!
    LxMorrowind uses a very simple physics model that is not physically accurate.
    As such, the Bullet physics simulation is disabled and this class handles
    the simplified "simulation" required to move objects around the world. 
    Bullet is used solely as a collision detection system with this class 
    handling the interaction.

    Why isn't Bullet simulation used?  It's actually more complex/accurate than 
    is desired.  For example, there is no desire for torsion, angle changes, etc.
    on the player; rather making Bullet behave in a physically inaccurate manner 
    by disabling all these attributes / setting very high damping values / 
    setting zero multipliers, it's literally easier to simply move the objects
    around via a simple physics model implemented here.
 */
class MwPhysicsDoc : public lx0::Document::Component
{
public:
    virtual const char*     name() const { return s_name(); }
    static  const char*     s_name()     { return "mwphysics"; }

                    MwPhysicsDoc        (void);

    virtual void    onUpdate            (lx0::DocumentPtr spDocument);


    void            enableGravity       (bool bEnable);
    bool            gravityEnabled      (void) const { return mbEnableGravity; }
    bool            movePlayer          (lx0::ElementPtr spPlayer, const glgeom::vector3f& step);

protected:
    bool        mbEnableGravity;
    lx0::uint32 mLastUpdate;
};
