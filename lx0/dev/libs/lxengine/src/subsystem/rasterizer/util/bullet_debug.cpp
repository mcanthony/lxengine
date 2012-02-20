

//////////////////////////////////////////////////////////////////////////////////////////

#include <Windows.h>
#include <gl/GL.h>

#include <GL3/gl3.h>
extern "C" {
extern PFNGLUSEPROGRAMPROC gl3wUseProgram;
#define glUseProgram		gl3wUseProgram
}

#include <bullet/btBulletCollisionCommon.h>
#include <bullet/LinearMath/btIDebugDraw.h>

#include <lx0/lxengine.hpp>
#include <lx0/plugins/bulletphysics.hpp>

class DebugDraw : public btIDebugDraw
{
public:
                    DebugDraw();
            
    virtual void    drawLine            (const btVector3& from,const btVector3& to,const btVector3& color);
    virtual void    drawContactPoint    (const btVector3& PointOnB,const btVector3& normalOnB,btScalar distance,int lifeTime,const btVector3& color);
    virtual void    reportErrorWarning  (const char* warningString);
    virtual void    draw3dText          (const btVector3& location,const char* textString);
    virtual void    setDebugMode        (int debugMode);
    virtual int     getDebugMode        (void) const { return m_debugMode;}

protected:
         int m_debugMode;     
};

DebugDraw::DebugDraw()
    : m_debugMode   (btIDebugDraw::DBG_DrawWireframe)
{
   
}

void    
DebugDraw::drawLine(const btVector3& from,const btVector3& to,const btVector3& color)
{
    glBegin(GL_LINES);
    glColor3f(color.getX(), color.getY(), color.getZ());
    glVertex3f(from.getX(), from.getY(), from.getZ());
    glVertex3f(to.getX(), to.getY(), to.getZ());
    glEnd();
}

void    
DebugDraw::setDebugMode(int debugMode)
{
   m_debugMode = debugMode;
}

void    
DebugDraw::draw3dText(const btVector3& location,const char* textString)
{
}

void    
DebugDraw::reportErrorWarning (const char* warningString)
{
}

void    
DebugDraw::drawContactPoint(const btVector3& pointOnB,const btVector3& normalOnB,btScalar distance,int lifeTime,const btVector3& color)
{
}


//////////////////////////////////////////////////////////////////////////////////////////

using namespace lx0;

void enable_bullet_debug()
{
    auto spEngine = Engine::acquire();
    auto vDocs = spEngine->documents();

    for (auto it = vDocs.begin(); it != vDocs.end(); ++it)
    {
        auto spPhysicsDoc = (*it)->getComponent<lx0::IPhysicsDoc>();
        if (spPhysicsDoc)
        {
            auto pWorld = spPhysicsDoc->getWorld();
            pWorld->setDebugDrawer(new DebugDraw);
        }
    }
}

void draw_bullet_debug(const glm::mat4& projMatrix, const glm::mat4& viewMatrix)
{
    auto spEngine = Engine::acquire();
    auto vDocs = spEngine->documents();

    for (auto it = vDocs.begin(); it != vDocs.end(); ++it)
    {
        auto spPhysicsDoc = (*it)->getComponent<lx0::IPhysicsDoc>();
        if (spPhysicsDoc)
        {
            glUseProgram(0);
            glDepthRange (0.0f, 0.999999f);

            glMatrixMode(GL_PROJECTION);
            glLoadMatrixf(glm::value_ptr(projMatrix));

            glMatrixMode(GL_MODELVIEW);
            glLoadMatrixf(glm::value_ptr(viewMatrix));

            auto pWorld = spPhysicsDoc->getWorld();
            pWorld->debugDrawWorld();

            glDepthRange (0.0f, 1.0f);
        }
    }
}
