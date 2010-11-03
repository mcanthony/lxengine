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

// Standard headers
#include <cassert>
#include <memory>

// Public headers
#include <lx0/view.hpp>
#include <lx0/core.hpp>
#include <lx0/document.hpp>
#include <lx0/element.hpp>
#include <lx0/mesh.hpp>
#include <lx0/point3.hpp>
#include <lx0/engine.hpp>
#include <lx0/util.hpp>

// Internal headers
#include "view_input.hpp"

#include <OGRE/OgreRoot.h>
#include <OGRE/OgreRenderWindow.h>
#include <OGRE/OgreWindowEventUtilities.h>
#include <OGRE/OgreManualObject.h>
#include <OGRE/OgreMaterial.h>
#include <OGRE/OgreMaterialManager.h>
#include <OGRE/OgreOverlayManager.h>
#include <OGRE/OgreEntity.h>
#include <OGRE/OgreQuaternion.h>
#include <OGRE/OgreMeshManager.h>

_ENABLE_LX_CAST(lx0::core::point3, Ogre::Vector3)

using namespace lx0::util;

//===========================================================================//
//   Detail
//===========================================================================//

namespace lx0 { namespace core { namespace detail {

    void _convert(lxvar& v, Ogre::Vector3& u)
    {
        lx_check_error(v.size() == 3);
        u = Ogre::Vector3(v.at(0).asFloat(), v.at(1).asFloat(), v.at(2).asFloat());
    }

    void _convert(lxvar& value, Ogre::Quaternion& q)
    {
        lx_check_error(value.size() == 4);
        q.x = value.at(0).asFloat();
        q.y = value.at(1).asFloat();
        q.z = value.at(2).asFloat();
        q.w = value.at(3).asFloat();
    }
}}}

namespace lx0 { namespace core {

    namespace detail {

        class LxOgre
        {
        public:
            template <typename T> friend std::shared_ptr<T> acquireSingleton (std::weak_ptr<T>&);
            static LxOgrePtr acquire() { return acquireSingleton<LxOgre>(s_wpLxOgre); }

            Ogre::Root* root() { return mspRoot.get(); }


        protected:
            ~LxOgre() 
            {
            }

            LxOgre()
            {
                // Initialize OGRE
                try 
                {
                    // Create a LogManager manually before creating the root to intercept the
                    // OGRE logging during initialization.
                    // OGRE tracks these pointers internally - no need for the client to delete them.
                    Ogre::LogManager* pLogManager = new Ogre::LogManager;
                    Ogre::Log* pLog = pLogManager->createLog("ogre.log");
                    pLog->setLogDetail(Ogre::LL_BOREME);
                    pLog->setDebugOutputEnabled(false);

#ifndef _DEBUG
                    std::string pluginsCfgFile("plugins.cfg");
#else
                    std::string pluginsCfgFile("plugins_d.cfg");
#endif
                    mspRoot.reset(new Ogre::Root(pluginsCfgFile.c_str()));
                    mspRoot->showConfigDialog();
                }
                catch (std::exception& e)
                {
                    lx_fatal("OGRE exception caught during initialization");
                    throw e;
                }
            }

            static LxOgreWPtr s_wpLxOgre;

            std::auto_ptr<Ogre::Root>   mspRoot;
        };

        LxOgreWPtr LxOgre::s_wpLxOgre;


        class LxWindowEventListener : public Ogre::WindowEventListener
        {
        public:
            virtual void windowClosed(Ogre::RenderWindow* pRenderWindow)
            {
                Engine::acquire()->sendMessage("quit");
            }
        };

        class LxFrameEventListener : public Ogre::FrameListener
        {
        public:
            LxFrameEventListener(View* pView) : mpView(pView) {}
            View* mpView;

            virtual bool frameStarted(const Ogre::FrameEvent& evt) { return true; }
            virtual bool frameRenderingQueued(const Ogre::FrameEvent& evt) { mpView->_updateFrameRenderingQueued(); return true; }
            virtual bool frameEnded(const Ogre::FrameEvent& evt) { return true; }
        };
    }

    using namespace detail;

    View::View()
        : mspLxOgre         ( LxOgre::acquire() )
        , mpRenderWindow    (0)
        , mpSceneMgr        (0)
        , mpDocument        (nullptr)
    {
        Engine::acquire()->incObjectCount("View");
    }

    View::~View()
    {
        // These pointers are owned by OGRE.  Do not delete.  Set to NULL only for
        // documentation purposes.
        mpRenderWindow = 0;
        mpSceneMgr  = 0;

        Engine::acquire()->decObjectCount("View");
    }

    void  
    View::attach (Document* pDocument)
    {
        lx_check_error(mpDocument == nullptr);
        mpDocument = pDocument;

        mpDocument->slotElementRemoved += [&](ElementPtr spElem) { 
            _onElementRemoved(spElem);
        };
        mpDocument->slotElementAdded += [&](ElementPtr spElem) { 
            _onElementAdded(spElem);
        };
    }

    void  
    View::detach (Document* pDocument)
    {
        lx_check_error(mpDocument == pDocument);
        mpDocument = nullptr;
    }


    void
    View::_addMesh (std::string name, MeshPtr spMesh)
    {
        // The mesh pointer should not be null
        lx_check_error(spMesh);

        // See http://www.ogre3d.org/tikiwiki/ManualObject
        // See http://www.ogre3d.org/tikiwiki/tutorial+manual+object+to+mesh
        
        Ogre::ManualObject* pObject = mpSceneMgr->createManualObject((name + "-manual").c_str());

        int index = 0;
        auto add_tri = [&index, pObject] (Ogre::Vector3& v0, Ogre::Vector3& v1, Ogre::Vector3& v2) -> void {
            Ogre::Vector3 normal = (v1 - v0).crossProduct(v2 - v1);
            normal.normalise();

            pObject->position(v0); 
            pObject->normal(normal);
            pObject->position(v1);
            pObject->normal(normal);
            pObject->position(v2);
            pObject->normal(normal);
            pObject->triangle(index, index + 1, index + 2);
            index += 3;
        };

        auto add_quad = [&index, pObject] (Ogre::Vector3& v0, Ogre::Vector3& v1, Ogre::Vector3& v2, Ogre::Vector3& v3) -> void {
            Ogre::Vector3 normal = (v1 - v0).crossProduct(v2 - v1);
            normal.normalise();

            pObject->position(v0); 
            pObject->normal(normal);
            pObject->position(v1);
            pObject->normal(normal);
            pObject->position(v2);
            pObject->normal(normal);
            pObject->triangle(index, index + 1, index + 2);
            index += 3;

            pObject->position(v2); 
            pObject->normal(normal);
            pObject->position(v3);
            pObject->normal(normal);
            pObject->position(v0);
            pObject->normal(normal);
            pObject->triangle(index, index + 1, index + 2);
            index += 3;
        };

        pObject->begin("LxMaterial", Ogre::RenderOperation::OT_TRIANGLE_LIST);
        for (auto fi = spMesh->mFaces.begin(); fi != spMesh->mFaces.end(); ++fi)
        {
            Ogre::Vector3& v0 = reinterpret_cast<Ogre::Vector3&>(spMesh->mVertices[fi->index[0]]);
            Ogre::Vector3& v1 = reinterpret_cast<Ogre::Vector3&>(spMesh->mVertices[fi->index[1]]);
            Ogre::Vector3& v2 = reinterpret_cast<Ogre::Vector3&>(spMesh->mVertices[fi->index[2]]);
            
            // A final index of -1 is a special value indicating that this is a triangle,
            // not a quad.
            if (fi->index[3] >= 0)
            {
                Ogre::Vector3& v3 = reinterpret_cast<Ogre::Vector3&>(spMesh->mVertices[fi->index[3]]);
                add_quad(v0, v1, v2, v3);
            }
            else
            {
                add_tri(v0, v1, v2);
            }
        }
        pObject->end();

        pObject->convertToMesh(name.c_str());
    }

    class SceneElem : public Element::Component
    {
    public:
        SceneElem   (ElementPtr spElem);

        virtual void onAttributeChange(ElementPtr spElem, std::string name, lxvar value);

    protected:
        void    _reset  (ElementPtr spElem);

        Ogre::Overlay*              mpOverlay;
        Ogre::TextureUnitState*     mpTexUnit;
    };

    SceneElem::SceneElem(ElementPtr spElem)
        : mpOverlay (nullptr) 
        , mpTexUnit (nullptr)
    {
        // Credit to: http://www.ogre3d.org/tikiwiki/FadeEffectOverlay&structure=Cookbook
        
        Ogre::ResourceGroupManager::getSingleton().addResourceLocation("data/sm_lx_cube_rain", "FileSystem");
        Ogre::ResourceGroupManager::getSingleton().initialiseAllResourceGroups();

        Ogre::ResourcePtr spResource = Ogre::MaterialManager::getSingleton().getByName("Materials/OverlayMaterial");
        Ogre::Material* pMat = dynamic_cast<Ogre::Material*>(spResource.getPointer());
        lx_check_error(pMat != nullptr, "Could not load OGRE material");

        Ogre::Technique* pTech = pMat->getTechnique(0);
        mpTexUnit = pTech->getPass(0)->getTextureUnitState(0);

        mpOverlay = Ogre::OverlayManager::getSingleton().getByName("Overlays/FadeInOut");

        _reset(spElem);
    }

    void 
    SceneElem::onAttributeChange(ElementPtr spElem, std::string name, lxvar value)
    {
        _reset(spElem);
    }

    void
    SceneElem::_reset  (ElementPtr spElem)
    {
        float fade = spElem->attr("fade").query(0.0f);

        if (fade < 0.01f)
        {
            mpTexUnit->setAlphaOperation(Ogre::LBX_MODULATE, Ogre::LBS_MANUAL, Ogre::LBS_TEXTURE, 0.0f);
            mpOverlay->hide();
        }
        else
        {
            mpOverlay->show();
            mpTexUnit->setAlphaOperation(Ogre::LBX_MODULATE, Ogre::LBS_MANUAL, Ogre::LBS_TEXTURE, fade);
        }
    }

    static int refCount = 0;

    class OgreNodeLink : public Element::Component
    {
    public:
        OgreNodeLink (Ogre::SceneManager* mpSceneMgr, ElementPtr spElem) 
            : mpEntity(nullptr) 
        {
            std::string name("anonymousRef");
            name += lx_itoa(refCount++);
            std::string ref = spElem->attr("ref").asString();
                   
            Ogre::Entity* pEntity = mpSceneMgr->createEntity(ref);
            pEntity->setCastShadows(true);

            Ogre::SceneNode* pNode = mpSceneMgr->getRootSceneNode()->createChildSceneNode(name);
            pNode->attachObject(pEntity);

            mpEntity = pEntity;

            _setMaterial(spElem);
            _setTranslation( spElem->attr("translation") );
        }
        OgreNodeLink::~OgreNodeLink()
        {
            _node()->getParent()->removeChild(_node());
        }

        void _setTranslation (lxvar& v)
        {
            if (v.isDefined())
                _node()->setPosition(v.convert());
        }

        void _setMaterial(ElementPtr spElem)
        {
            Ogre::Vector3 diffuse (1, 1, 1);
            Ogre::Vector3 specular (0, 0, 0);
            float shininess = 1;

            lxvar elemDiffuse = spElem->attr("color");
            lxvar elemSpecular = spElem->attr("specular");
            lxvar elemShininess = spElem->attr("shininess");

            if (elemDiffuse.isDefined())
                diffuse = (Ogre::Vector3)elemDiffuse.convert();
            if (elemSpecular.isDefined())
                specular = (Ogre::Vector3)elemSpecular.convert();
            if (elemShininess.isDefined())
                shininess = *elemShininess;
               
            Ogre::MaterialPtr spMat = Ogre::MaterialManager::getSingleton().create("LxMaterial", Ogre::ResourceGroupManager::INTERNAL_RESOURCE_GROUP_NAME);
            spMat->setLightingEnabled(true);
            spMat->setAmbient(.1f, .1f, .1f);
            spMat->setDiffuse(diffuse.x, diffuse.y, diffuse.z, 1);
            spMat->setSpecular(specular.x, specular.y, specular.z, 0);
            spMat->setShininess(shininess);
            mpEntity->setMaterial(spMat);
        }

        virtual void onAttributeChange(ElementPtr spElem, std::string name, lxvar value)
        {
            if (name == "translation")
                _setTranslation(value);
            else if (name == "rotation")
                _node()->setOrientation(value.convert());
            else if (name == "display")
            {
                if (value.equal("block"))
                    mpEntity->setVisible(true);
                else if (value.equal("none"))
                    mpEntity->setVisible(false);
                else
                    lx_error("Unexpected value for display attribute");
            }
            else if (name == "color" || name == "specular")
                _setMaterial(spElem);
        }

        Ogre::SceneNode* _node() { return mpEntity->getParentSceneNode(); }

        Ogre::Entity*    mpEntity;
    };

    void
    View::_onElementAdded (ElementPtr spElem)
    {
        if (spElem->tagName() == "Ref")
            _processRef(spElem);
        else if (spElem->tagName() == "Scene")
            _processScene(spElem);
    }

    void
    View::_onElementRemoved (ElementPtr spElem)
    {
        if (spElem->tagName() == "Ref")
            spElem->removeComponent("OgreLink");
    }

    void
    View::_processScene (ElementPtr spElem)
    {
        spElem->attachComponent("OgreSceneElem", new SceneElem(spElem));
    }

    void
    View::_processRef (ElementPtr spElem)
    {
        spElem->attachComponent("OgreLink", new OgreNodeLink(mpSceneMgr, spElem));
    }

    void        
    View::_processGroup (ElementPtr spElem)
    {
        for (int j = 0; j < spElem->childCount(); ++j)
        {
            ElementPtr spChild = spElem->child(j);

            if (spChild->tagName() == "Ref")
                _processRef(spChild);
            else if (spChild->tagName() == "Group")
            {
                _processGroup(spChild);
            }
        }
    }

    /*!
        Makes the view or window visible.
     */
    void 
    View::show()
    {
        lx_check_error(mpRenderWindow == NULL);

        Ogre::Root& root = *mspLxOgre->root();

        // The render window creation also creates many internal OGRE data objects; therefore,
        // create it first.  Otherwise objects like the Camera won't even work.
        mpRenderWindow = root.initialise(true, "View" ); 

        // Create the input manager for the window, now that the window has been created
        {
            size_t hWindowHandle;
            mpRenderWindow->getCustomAttribute("WINDOW", &hWindowHandle);
            
            unsigned int width, height, depth;
            int top, left;
            mpRenderWindow->getMetrics(width, height, depth, left, top);

            mspLxInputManager.reset( new LxInputManager(hWindowHandle, width, height) );

            mspLxInputManager->slotKeyDown += [&] (KeyEvent& e) { this->slotKeyDown(e); };
        }

        mpSceneMgr = root.createSceneManager(Ogre::ST_GENERIC, "generic");

        // Note: SHADOWTYPE_STENCIL_ADDITIVE requires that meshes have index buffers
        // (can't be pure vertex lists).
        //
        mpSceneMgr->setShadowTechnique(Ogre::SHADOWTYPE_STENCIL_ADDITIVE);

        Ogre::Camera* mCamera = mpSceneMgr->createCamera("Camera");

        // Make Z-up on the OGRE camera
        // See http://www.gamedev.net/community/forums/topic.asp?topic_id=452424
        mCamera->roll(Ogre::Radian(1.57079633f));
        mCamera->setFixedYawAxis(true, Ogre::Vector3::UNIT_Z);

        mCamera->setPosition(Ogre::Vector3(9.0f,8.0f,10.0f));
        mCamera->lookAt(Ogre::Vector3(0.0f,0.0f,0.0f));
        mCamera->setNearClipDistance(0.1f);
        mCamera->setFarClipDistance(100.0f);

        Ogre::Viewport* mViewport = mpRenderWindow->addViewport(mCamera);
        mViewport->setBackgroundColour(Ogre::ColourValue(0.1f, 0.1f, 0.16f));

        mCamera->setAspectRatio(Ogre::Real(mViewport->getActualWidth()) / Ogre::Real(mViewport->getActualHeight()));

        // Add ambient light since there currently are not standard lights
        mpSceneMgr->setAmbientLight(Ogre::ColourValue(0.2f, 0.2f, 0.1f));

        {
            Ogre::Light* light = mpSceneMgr->createLight("Light");
            light->setType(Ogre::Light::LT_POINT);
            light->setDiffuseColour(Ogre::ColourValue(1.0f, 1.0f, 1.0f));
            light->setSpecularColour(Ogre::ColourValue(1.0f, 1.0f, 1.0f));
            light->setPosition(10, 13, 18);
            light->setAttenuation(1000, 1, 0, 0);
        }

        {
            Ogre::MaterialPtr spMat = Ogre::MaterialManager::getSingleton().create("LxMaterial", Ogre::ResourceGroupManager::INTERNAL_RESOURCE_GROUP_NAME);
            spMat->setLightingEnabled(true);
            spMat->setAmbient(.1f, .1f, .1f);
            spMat->setDiffuse(1, 1, 1, 1);
            spMat->setSpecular(0, 0, 0, 0);
        }

        // Add a fixed ground plane.  This is going to have to go away...
        {
            Ogre::Plane plane(Ogre::Vector3::UNIT_Z, 0);
            Ogre::MeshManager::getSingleton().createPlane("ground", Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME,
                plane, 150, 150, 20, 20, true, 1, 5, 5, Ogre::Vector3::UNIT_Y);
 
            Ogre::Entity* pGround = mpSceneMgr->createEntity("GroundEntity", "ground");
            pGround->setCastShadows(false);

            mpSceneMgr->getRootSceneNode()->createChildSceneNode()->attachObject(pGround);
        }

        ElementPtr spRoot = mpDocument->root();
        for (int i = 0; i < spRoot->childCount(); ++i)
        {
            ElementPtr spChild = spRoot->child(i);
            if (spChild->tagName() == "Library")
            {
                lx_debug("View found Library element in Document");

                for (int j = 0; j < spChild->childCount(); ++j)
                {
                    ElementCPtr spMeshElem = spChild->child(j);
                    if (spMeshElem->tagName() == "Mesh")
                    {
                        MeshPtr spMesh = std::dynamic_pointer_cast<Mesh>(spMeshElem->value());
                        _addMesh(spMeshElem->attr("id").asString(), spMesh);
                    }
                }
            }
            else if (spChild->tagName() == "Scene")
            {
                lx_debug("View found Scene element in Document");
                _processScene(spChild);
                _processGroup(spChild);
            }
        }
    }

    void
    View::updateBegin()
    {
        lx_check_error(mspWindowEventListener.get() == nullptr, "Window Event Listener already allocated: has updateBegin() been incorrectly called twice?");

        mspWindowEventListener.reset(new LxWindowEventListener);
        Ogre::WindowEventUtilities::addWindowEventListener(mpRenderWindow, mspWindowEventListener.get());

        Ogre::Root* pRoot = mspLxOgre->root();

        mspFrameEventListener.reset(new LxFrameEventListener(this));
        pRoot->addFrameListener(mspFrameEventListener.get());

        lx_check_error(pRoot->getRenderSystem() != 0);
        pRoot->getRenderSystem()->_initRenderTargets();

        // Clear event times
		pRoot->clearEventTimes();
    }

    void
    View::updateEnd()
    {
        mspLxOgre->root()->removeFrameListener(mspFrameEventListener.get());
        Ogre::WindowEventUtilities::removeWindowEventListener(mpRenderWindow, mspWindowEventListener.get());
    }

    /*
        Called by OGRE between queuing up call the render calls and the GPU actually
        blitting the frame: i.e. time when the CPU might potentially be idle waiting
        for the GPU.
     */
    void
    View::_updateFrameRenderingQueued()
    {
        mspLxInputManager->update();
    }

    void
    View::updateFrame()
    {
		// Pump messages in all registered RenderWindow windows
		Ogre::WindowEventUtilities::messagePump();

        Ogre::Root* pRoot = mspLxOgre->root();
        pRoot->renderOneFrame();
    }
}}
