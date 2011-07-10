#include <iostream>
#include <Ogre/Ogre.h>

class LxWindowEventListener : public Ogre::WindowEventListener
{
public:
    virtual void windowClosed(Ogre::RenderWindow* pRenderWindow)
    {
         Ogre::Root::getSingletonPtr()->queueEndRendering();
    }
};

int main (int argc, char** argv)
{
    try
    {
        // Use default arguments to Root
        std::unique_ptr<Ogre::Root> spRoot( new Ogre::Root );   

        if (spRoot->showConfigDialog())
        {
            std::unique_ptr<LxWindowEventListener> spWindowEventListener(new LxWindowEventListener);

            Ogre::RenderWindow* pRenderWindow = spRoot->initialise(true, "OGRE Example" );
            Ogre::WindowEventUtilities::addWindowEventListener(pRenderWindow, spWindowEventListener.get());

            spRoot->startRendering();

            Ogre::WindowEventUtilities::removeWindowEventListener(pRenderWindow, spWindowEventListener.get());
         }
    }
    catch (std::exception& e)
    {
        std::cerr << "Fatal exception.  Shutting down.\n"
                 << "Exception: " << e.what() << "\n";
    }
    return 0;
}
