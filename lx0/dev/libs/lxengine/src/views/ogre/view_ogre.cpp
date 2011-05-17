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
#include <lx0/engine/view.hpp>
#include <lx0/lxengine.hpp>
#include <lx0/engine/document.hpp>
#include <lx0/engine/element.hpp>
#include <lx0/engine/mesh.hpp>
#include <lx0/engine/engine.hpp>
#include <lx0/util/misc/util.hpp>
#include <lx0/util/misc/lxvar_convert.hpp>
#include <lx0/lxengine.hpp>

// Internal headers
#include "../../engine/dom/view_input.hpp"

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
#include <OGRE/OgreSubEntity.h>

using namespace lx0::core;
using namespace lx0::util;


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
    class CameraElem : public Element::Component
    {
    public:
        CameraElem   (ElementPtr spElem, Ogre::Camera* pCamera);

        virtual void onValueChange(ElementPtr spElem, lxvar value);

    protected:
        void    _reset  (ElementPtr spElem);

        Ogre::Camera*   mpCamera;
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
            lx_check_error(spElem->tagName() == "Ref");

            static int uniquePrefix = 0;

            std::string name("anonymousRef");
            name += lx_itoa(uniquePrefix++);
            std::string ref = spElem->attr("ref").asString();
                   
            Ogre::Entity* pEntity = mpSceneMgr->createEntity(ref);
            pEntity->setCastShadows(true);

            Ogre::SceneNode* pNode = mpSceneMgr->getRootSceneNode()->createChildSceneNode(name);
            pNode->attachObject(pEntity);

            mpEntity = pEntity;

            _warnOnIncorrectAttributes(spElem);
            _setMaterial(spElem);
            _setTranslation( spElem->attr("translation") );
            _setMaxExtent(spElem, spElem->attr("max_extent"));
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
            lxvar varMaterial = spElem->attr("material");
            if (varMaterial.isArray() && varMaterial.size() == 2)
            {
                std::string base = varMaterial.at(0).asString();
                lxvar       params = varMaterial.at(1);

                if (base == "phong")
                {
                    Ogre::ColourValue ambient (0.0f, 0.0f, 0.2f);
                    Ogre::ColourValue diffuse (0.5f, 0.4f, 0.5f);
                    Ogre::ColourValue specular (0.9f, 0.8f, 0.5f);
                    float             shininess = 32.0f;
                    float             opacity = 1.0f;

                    if (params.find("ambient").isDefined()) 
                        ambient = params.find("ambient").convert();
                    if (params.find("diffuse").isDefined()) 
                        diffuse = params.find("diffuse").convert();
                    if (params.find("specular").isDefined()) 
                        specular = params.find("specular").convert();
                    if (params.find("shininess").isDefined()) 
                        shininess = params.find("shininess").convert();
                    if (params.find("opacity").isDefined()) 
                        opacity = params.find("opacity").convert();

                    Ogre::MaterialPtr spMat =  Ogre::MaterialManager::getSingleton().getByName("Material/PhongChecker_GLSL");
                    lx_check_error(!spMat.isNull());
                    spMat = spMat->clone("");
                    auto spConstants = spMat->getTechnique(0)->getPass(0)->getFragmentProgramParameters();
                    spConstants->setNamedConstant("unifLightCount", 1.0f);
                    spMat->setAmbient(ambient);
                    spConstants->setNamedConstant("unifAmbient", Ogre::Vector3(ambient.r, ambient.g, ambient.b));
                    spConstants->setNamedConstant("unifDiffuse", Ogre::Vector3(diffuse.r, diffuse.g, diffuse.b));
                    spConstants->setNamedConstant("unifSpecular", Ogre::Vector3(specular.r, specular.g, specular.b));
                    //spMat->setShininess(shininess);
                    mpEntity->setMaterial(spMat);
                }
            }
            else
            {
                Ogre::Vector3 diffuse (1, 1, 1);
                Ogre::Vector3 specular (0, 0, 0);
                float shininess = 1;

                std::string material = spElem->attr("material").query("standard");
                lxvar elemDiffuse = spElem->attr("color");
                lxvar elemSpecular = spElem->attr("specular");
                lxvar elemShininess = spElem->attr("shininess");

                if (elemDiffuse.isDefined())
                    diffuse = (Ogre::Vector3)elemDiffuse.convert();
                if (elemSpecular.isDefined())
                    specular = (Ogre::Vector3)elemSpecular.convert();
                if (elemShininess.isDefined())
                    shininess = *elemShininess;

                if (material == "solid")
                {
                    Ogre::MaterialPtr spMat =  Ogre::MaterialManager::getSingleton().getByName("Material/Minimal_GLSL");
                    lx_check_error(!spMat.isNull());
                    spMat = spMat->clone("anything");
                    spMat->setDiffuse(diffuse.x, diffuse.y, diffuse.z, 1);
                    mpEntity->setMaterial(spMat);
                }
                else if (material == "checker")
                {
                    Ogre::MaterialPtr spMat =  Ogre::MaterialManager::getSingleton().getByName("Material/Checker_GLSL");
                    lx_check_error(!spMat.isNull());
                    spMat = spMat->clone("");
                    spMat->getTechnique(0)->getPass(0)->getFragmentProgramParameters()->setNamedConstant("uniCheckerPrimaryColor", Ogre::Vector4(diffuse.x, diffuse.y, diffuse.z, 1));
                    mpEntity->setMaterial(spMat);
                }
                else 
                {
                    lx_check_error(material == "standard");

                    Ogre::MaterialPtr spMat = Ogre::MaterialManager::getSingleton().create("LxMaterial", Ogre::ResourceGroupManager::INTERNAL_RESOURCE_GROUP_NAME);
                    spMat->setLightingEnabled(true);
                    spMat->setAmbient(.1f, .1f, .1f);
                    spMat->setDiffuse(diffuse.x, diffuse.y, diffuse.z, 1);
                    spMat->setSpecular(specular.x, specular.y, specular.z, 0);
                    spMat->setShininess(shininess);
                    mpEntity->setMaterial(spMat);
                }
            }
        }

        void _setMaxExtent (ElementPtr spElem, lxvar value)
        {
            MeshPtr spMesh = spElem->document()->getElementById( spElem->attr("ref").asString() )->value().imp<Mesh>();
            const float f = spMesh->maxExtentScale(value);

            _node()->setScale(f, f, f);
        }

        virtual void onAttributeChange(ElementPtr spElem, std::string name, lxvar value)
        {
            _warnOnIncorrectAttributes(spElem);

            if (name == "translation")
                _setTranslation(value);
            else if (name == "rotation")
                _node()->setOrientation(value.convert());
            else if (name == "max_extent")
                _setMaxExtent(spElem, value);
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

        void _warnOnIncorrectAttributes(ElementPtr spElem)
        {
#ifdef _DEBUG
            if (spElem->attr("translate").isDefined())
                lx_warn_once("Found 'translate' attribute: was 'translation' intended?");
            if (spElem->attr("position").isDefined())
                lx_warn_once("Found 'position' attribute: was 'translation' intended?");
#endif
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

        virtual void    updateBegin     (void);
        virtual void    updateFrame     (DocumentPtr spDocument);
        virtual void    updateEnd       (void);

    protected:
        void        _addLighting        (Ogre::SceneManager* pSceneMgr);
        void        _addMaterials       (void);
        void        _addGroundPlane     (Ogre::SceneManager* pSceneMgr);

        void        _processDocument    (View* pHostView, Document* pDocument);
        void        _processGroup       (ElementPtr spElem);
        void        _processScene       (ElementPtr spElem);
        void        _processMesh        (std::string name, MeshPtr spMesh);

        View*                       mpHostView;
        std::auto_ptr<Ogre::Root>   mspRoot;
        Ogre::SceneManager*         mpSceneMgr;     //! Non-owning pointer.  OGRE owns this pointer
        Ogre::RenderWindow*         mpRenderWindow; //! Non-owning pointer.  OGRE owns this pointer.
        Ogre::Camera*               mpCamera;

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
        mpView->_updateFrameRenderingQueued(); 
        return true; 
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
        , mpRenderWindow    (0)
        , mpCamera          (0)
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

        mpCamera = mpSceneMgr->createCamera("Camera");

        // Make Z-up on the OGRE camera
        // See http://www.gamedev.net/community/forums/topic.asp?topic_id=452424
        mpCamera->roll(Ogre::Radian(1.57079633f));
        mpCamera->setFixedYawAxis(true, Ogre::Vector3::UNIT_Z);

        mpCamera->setPosition(Ogre::Vector3(9.0f,8.0f,10.0f));
        mpCamera->lookAt(Ogre::Vector3(0.0f,0.0f,0.0f));
        mpCamera->setNearClipDistance(0.1f);
        mpCamera->setFarClipDistance(100.0f);

        Ogre::Viewport* mViewport = mpRenderWindow->addViewport(mpCamera);
        mViewport->setBackgroundColour(Ogre::ColourValue(0.1f, 0.1f, 0.16f));

        mpCamera->setAspectRatio(Ogre::Real(mViewport->getActualWidth()) / Ogre::Real(mViewport->getActualHeight()));

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

        {
            Ogre::ResourceGroupManager::getSingleton().addResourceLocation("data/sm_lx_cube_rain", "FileSystem");

            Ogre::ResourceGroupManager::getSingleton().addResourceLocation("media/shaders/glsl/fragment", "FileSystem");
            Ogre::ResourceGroupManager::getSingleton().addResourceLocation("media/shaders/glsl/geometry", "FileSystem");
            Ogre::ResourceGroupManager::getSingleton().addResourceLocation("media/shaders/glsl/vertex", "FileSystem");
            Ogre::ResourceGroupManager::getSingleton().addResourceLocation("media/shaders/ogre", "FileSystem");
            Ogre::ResourceGroupManager::getSingleton().initialiseAllResourceGroups();
        }
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
    OgreImp::_processGroup (ElementPtr spParent)
    {
        for (int j = 0; j < spParent->childCount(); ++j)
        {
            ElementPtr spChild = spParent->child(j);

            if (spChild->tagName() == "Ref")
                spChild->attachComponent("OgreRefElem", new RefElem(mpSceneMgr, spChild));
            else if (spChild->tagName() == "Camera")
                spChild->attachComponent("OgreCameraElem", new CameraElem(spChild, mpCamera));
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
        lx_check_error(spMesh->mVertices.empty() == false);

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
            Mesh::Vertex& srcV0 = spMesh->mVertices[fi->index[0]];
            Mesh::Vertex& srcV1 = spMesh->mVertices[fi->index[1]];
            Mesh::Vertex& srcV2 = spMesh->mVertices[fi->index[2]];

            Ogre::Vector3& v0 = reinterpret_cast<Ogre::Vector3&>(srcV0.position);
            Ogre::Vector3& v1 = reinterpret_cast<Ogre::Vector3&>(srcV1.position);
            Ogre::Vector3& v2 = reinterpret_cast<Ogre::Vector3&>(srcV2.position);

            Ogre::Vector3& n0 = reinterpret_cast<Ogre::Vector3&>(srcV0.normal);
            Ogre::Vector3& n1 = reinterpret_cast<Ogre::Vector3&>(srcV1.normal);
            Ogre::Vector3& n2 = reinterpret_cast<Ogre::Vector3&>(srcV2.normal);
            
            if (!spMesh->mFlags.mVertexNormals)
            {
                // A final index of -1 is a special value indicating that this is a triangle,
                // not a quad.
                if (fi->index[3] >= 0)
                {
                    Ogre::Vector3& v3 = reinterpret_cast<Ogre::Vector3&>(spMesh->mVertices[fi->index[3]].position);
                    add_quad(v0, v1, v2, v3);
                }
                else
                {
                    add_tri(v0, v1, v2);
                }
            }
            else
            {
                pObject->position(v0); 
                pObject->normal(n0);
                pObject->position(v1);
                pObject->normal(n1);
                pObject->position(v2);
                pObject->normal(n2);
                pObject->triangle(index, index + 1, index + 2);
                index += 3;

                if (fi->index[3] >= 0)
                {
                    Ogre::Vector3& v3 = reinterpret_cast<Ogre::Vector3&>(spMesh->mVertices[fi->index[3]].position);
                    Ogre::Vector3& n3 = reinterpret_cast<Ogre::Vector3&>(spMesh->mVertices[fi->index[3]].normal);
                    pObject->position(v2); 
                    pObject->normal(n2);
                    pObject->position(v3);
                    pObject->normal(n3);
                    pObject->position(v0);
                    pObject->normal(n0);
                    pObject->triangle(index, index + 1, index + 2);
                    index += 3;
                }
            }
        }
        pObject->end();

        pObject->convertToMesh(name.c_str());
    }

    void
    OgreImp::_onElementAdded (ElementPtr spElem)
    {
        if (spElem->tagName() == "Ref")
            spElem->attachComponent("OgreRefElem", new RefElem(mpSceneMgr, spElem));
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
    OgreImp::updateFrame(DocumentPtr spDocument)
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


    //===========================================================================//
    // IMPLEMENTATION:
    // SceneElem
    //===========================================================================//

    CameraElem::CameraElem (ElementPtr spElem, Ogre::Camera* pCamera)
        : mpCamera (pCamera)
    {
        _reset(spElem);
    }

    void 
    CameraElem::onValueChange(ElementPtr spElem, lxvar value)
    {
        _reset(spElem);
    }

    void
    CameraElem::_reset (ElementPtr spElem)
    {
        lxvar val = spElem->value();

        Ogre::Vector3 pos = val.find("position").convert();
        Ogre::Vector3 lookAt = val.find("lookAt").convert();
        float nearDist = val.find("near").query(0.1f);
        float farDist = val.find("far").query(100.f);

        mpCamera->setPosition(pos);
        mpCamera->lookAt(lookAt);
        mpCamera->setNearClipDistance(nearDist);
        mpCamera->setFarClipDistance(farDist);
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
