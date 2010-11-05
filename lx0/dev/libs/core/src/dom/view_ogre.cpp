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

using namespace lx0::core;
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

    void _convert(lxvar& value, vector3& v)
    {
        lx_check_error(value.size() == 3);
        v.x = value.at(0).asFloat();
        v.y = value.at(1).asFloat();
        v.z = value.at(2).asFloat();
    }
}}}

//===========================================================================//
//   Components
//===========================================================================//

namespace {

    class OgreImp;

    //-----------------------------------------------------------------------//
    //! 
    /*!
     */
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

   
    //-----------------------------------------------------------------------//
    //! 
    /*!
     */
    class RefElem : public Element::Component
    {
    public:
        RefElem (Ogre::SceneManager* mpSceneMgr, ElementPtr spElem) 
            : mpEntity(nullptr) 
        {
            static int uniquePrefix = 0;

            std::string name("anonymousRef");
            name += lx_itoa(uniquePrefix++);
            std::string ref = spElem->attr("ref").asString();
                   
            Ogre::Entity* pEntity = mpSceneMgr->createEntity(ref);
            pEntity->setCastShadows(true);

            Ogre::SceneNode* pNode = mpSceneMgr->getRootSceneNode()->createChildSceneNode(name);
            pNode->attachObject(pEntity);

            mpEntity = pEntity;

            _setMaterial(spElem);
            _setTranslation( spElem->attr("translation") );
            _setMaxExtent(spElem->attr("max_extent"));
        }
        RefElem::~RefElem()
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

        void _setMaxExtent (lxvar value)
        {
            if (value.isUndefined())
            {
                // Empty value means use the original scale
                _node()->setScale(1, 1, 1);
            }
            else if (value.isFloat())
            {
                float maxExtent = *value;
                if (maxExtent > 0.0f)
                {
                    auto extents = _mesh()->getBounds().getSize();
                    float f = maxExtent / std::max(extents.x, std::max(extents.y, extents.z));
                    
                    _node()->setScale(f, f, f);
                }
                else
                    lx_warn("Invalid max_extent value %f", maxExtent);
            }
        }

        virtual void onAttributeChange(ElementPtr spElem, std::string name, lxvar value)
        {
            if (name == "translation")
                _setTranslation(value);
            else if (name == "rotation")
                _node()->setOrientation(value.convert());
            else if (name == "max_extent")
                _setMaxExtent(value);
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

        Ogre::Mesh*      _mesh()   { return mpEntity->getMesh().get(); }
        Ogre::Entity*    _entity() { return mpEntity; }
        Ogre::SceneNode* _node() { return mpEntity->getParentSceneNode(); }

        Ogre::Entity*    mpEntity;
    };

    //-----------------------------------------------------------------------//
    //! 
    /*!
        */
    class LxWindowEventListener : public Ogre::WindowEventListener
    {
    public:
        virtual void windowClosed(Ogre::RenderWindow* pRenderWindow)
        {
            Engine::acquire()->sendMessage("quit");
        }
    };

    //-----------------------------------------------------------------------//
    //! 
    /*!
        */
    class LxFrameEventListener : public Ogre::FrameListener
    {
    public:
        LxFrameEventListener(OgreImp* pView) : mpView(pView) {}
        OgreImp* mpView;

        virtual bool frameStarted(const Ogre::FrameEvent& evt) { return true; }
        virtual bool frameRenderingQueued(const Ogre::FrameEvent& evt);
        virtual bool frameEnded(const Ogre::FrameEvent& evt) { return true; }
    };


    //-----------------------------------------------------------------------//
    //! 
    /*!
     */
    class OgreImp : public ViewImp
    {
    public:
                       
                    OgreImp      (View* pHostView);
                    ~OgreImp     (void);

        Ogre::Root* root() { return mspRoot.get(); }

        void        createWindow    (View* pHostView, size_t& handle, unsigned int& width, unsigned int& height);
        void        show            (View* pHostView, Document* pDocument);


        void        _updateFrameRenderingQueued ();
        void        _onElementAdded             (ElementPtr spElem);
        void        _onElementRemoved           (ElementPtr spElem);

        void        updateBegin     (void);
        void        updateFrame     (void);
        void        updateEnd       (void);

    protected:
        void        _addLighting        (Ogre::SceneManager* pSceneMgr);
        void        _addMaterials       (void);
        void        _addGroundPlane     (Ogre::SceneManager* pSceneMgr);

        void        _processDocument    (View* pHostView, Document* pDocument);
        void        _processGroup       (ElementPtr spElem);
        void        _processRef         (ElementPtr spElem);
        void        _processScene       (ElementPtr spElem);
        void        _processMesh        (std::string name, MeshPtr spMesh);

        View*                       mpHostView;
        std::auto_ptr<Ogre::Root>   mspRoot;
        Ogre::SceneManager*         mpSceneMgr;     //! Non-owning pointer.  OGRE owns this pointer
        Ogre::RenderWindow*         mpRenderWindow; //! Non-owning pointer.  OGRE owns this pointer.
        std::unique_ptr<LxWindowEventListener> mspWindowEventListener;
        std::unique_ptr<LxFrameEventListener>  mspFrameEventListener;
    };

    //===========================================================================//
    // IMPLEMENTATION:
    // LxFrameEventListener
    //===========================================================================//

    bool 
    LxFrameEventListener::frameRenderingQueued(const Ogre::FrameEvent& evt) 
    { 
        mpView->_updateFrameRenderingQueued(); return true; 
    }
        
    //===========================================================================//
    // IMPLEMENTATION:
    // OgreImp
    //===========================================================================//

    OgreImp::~OgreImp() 
    {
    }

    OgreImp::OgreImp (View* pHostView)
        : mpHostView        (pHostView)
        , mpSceneMgr        (0)
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

    void
    OgreImp::createWindow (View* pHostView, size_t& handle, unsigned int& width, unsigned int& height)
    {
        // The render window creation also creates many internal OGRE data objects; therefore,
        // the client must create it first.  Otherwise, objects like the Camera won't even work.
        //
        mpRenderWindow = mspRoot->initialise(true, "View" ); 

        // Get the handle to the window
        mpRenderWindow->getCustomAttribute("WINDOW", &handle);
            
        // Get the dimensions of the window
        unsigned int depth;
        int top, left;
        mpRenderWindow->getMetrics(width, height, depth, left, top);
    }

    void
    OgreImp::show (View* pHostView, Document* pDocument)
    {
        mpSceneMgr = mspRoot->createSceneManager(Ogre::ST_GENERIC, "generic");

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

        _addLighting(mpSceneMgr);
        _addMaterials();
        _addGroundPlane(mpSceneMgr);

        _processDocument(pHostView, pDocument);
    }

    void
    OgreImp::_addLighting (Ogre::SceneManager* pSceneMgr)
    {
        // Add ambient light since there currently are not standard lights
        pSceneMgr->setAmbientLight(Ogre::ColourValue(0.2f, 0.2f, 0.1f));

        {
            Ogre::Light* light = pSceneMgr->createLight("Light");
            light->setType(Ogre::Light::LT_POINT);
            light->setDiffuseColour(Ogre::ColourValue(1.0f, 1.0f, 1.0f));
            light->setSpecularColour(Ogre::ColourValue(1.0f, 1.0f, 1.0f));
            light->setPosition(10, 13, 18);
            light->setAttenuation(1000, 1, 0, 0);
        }
    }

    void
    OgreImp::_addMaterials (void)
    {
        Ogre::MaterialPtr spMat = Ogre::MaterialManager::getSingleton().create("LxMaterial", Ogre::ResourceGroupManager::INTERNAL_RESOURCE_GROUP_NAME);
        spMat->setLightingEnabled(true);
        spMat->setAmbient(.1f, .1f, .1f);
        spMat->setDiffuse(1, 1, 1, 1);
        spMat->setSpecular(0, 0, 0, 0);
    }

    void
    OgreImp::_addGroundPlane (Ogre::SceneManager* pSceneMgr)
    {
        Ogre::Plane plane(Ogre::Vector3::UNIT_Z, 0);
        Ogre::MeshManager::getSingleton().createPlane("ground", Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME,
            plane, 150, 150, 20, 20, true, 1, 5, 5, Ogre::Vector3::UNIT_Y);
 
        Ogre::Entity* pGround = pSceneMgr->createEntity("GroundEntity", "ground");
        pGround->setCastShadows(false);

        pSceneMgr->getRootSceneNode()->createChildSceneNode()->attachObject(pGround);
    }

    void
    OgreImp::_processDocument (View* pHostView, Document* pDocument)
    {
        ElementPtr spRoot = pDocument->root();
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
                        MeshPtr spMesh = spMeshElem->value().imp<Mesh>();
                        _processMesh(spMeshElem->attr("id").asString(), spMesh);
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
    OgreImp::_processScene (ElementPtr spElem)
    {
        spElem->attachComponent("OgreSceneElem", new SceneElem(spElem));
    }

    void
    OgreImp::_processRef (ElementPtr spElem)
    {
        spElem->attachComponent("OgreRefElem", new RefElem(mpSceneMgr, spElem));
    }

    void        
    OgreImp::_processGroup (ElementPtr spElem)
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

    void
    OgreImp::_processMesh (std::string name, MeshPtr spMesh)
    {
        // The mesh pointer should not be null
        lx_check_error(spMesh.get() != nullptr);

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

    void
    OgreImp::_onElementAdded (ElementPtr spElem)
    {
        if (spElem->tagName() == "Ref")
            _processRef(spElem);
        else if (spElem->tagName() == "Scene")
            _processScene(spElem);
    }

    void
    OgreImp::_onElementRemoved (ElementPtr spElem)
    {
        if (spElem->tagName() == "Ref")
            spElem->removeComponent("OgreRefElem");
    }

    void
    OgreImp::_updateFrameRenderingQueued ()
    {
        mpHostView->notifyViewImpIdle();
    }

    void
    OgreImp::updateBegin()
    {
        lx_check_error(mspWindowEventListener.get() == nullptr, 
            "Window Event Listener already allocated: has updateBegin() been incorrectly called twice?");

        mspWindowEventListener.reset(new LxWindowEventListener);
        Ogre::WindowEventUtilities::addWindowEventListener(mpRenderWindow, mspWindowEventListener.get()); 

        mspFrameEventListener.reset(new LxFrameEventListener(this));
        mspRoot->addFrameListener(mspFrameEventListener.get());

        lx_check_error(mspRoot->getRenderSystem() != 0);
        mspRoot->getRenderSystem()->_initRenderTargets();

        // Clear event times
		mspRoot->clearEventTimes();
    }

    void
    OgreImp::updateEnd()
    {
        mspRoot->removeFrameListener(mspFrameEventListener.get());
        Ogre::WindowEventUtilities::removeWindowEventListener(mpRenderWindow, mspWindowEventListener.get());

        mspFrameEventListener.release();
        mspWindowEventListener.release();
    }

    void
    OgreImp::updateFrame()
    {
		// Pump messages in all registered RenderWindow windows
		Ogre::WindowEventUtilities::messagePump();

        mspRoot->renderOneFrame();
    }

    //===========================================================================//
    // IMPLEMENTATION:
    // SceneElem
    //===========================================================================//

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
    SceneElem::_reset (ElementPtr spElem)
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

}

//===========================================================================//
// 
//===========================================================================//

namespace lx0 { namespace core {

    ViewImp* 
    View::_createViewImpOgre  (View* pView)
    {
        return new OgreImp(pView);
    }
}}
