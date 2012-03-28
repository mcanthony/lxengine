//===========================================================================//
/*
                                   LxEngine

    LICENSE
    * MIT License (http://www.opensource.org/licenses/mit-license.php)

    Copyright (c) 2010-2011 athile@athile.net (http://www.athile.net)

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

#include <lx0/core/slot/slot.hpp>
#include <map>
#include <string>

#include <lx0/engine/view.hpp>
#include <lx0/util/gl/glinterface.hpp>

namespace lx0 { 
    
    namespace subsystem {

        /*!
           \defgroup lx0_subsystem_canvas lx0_subsystem_canvas
           \ingroup Subsystem

           The LxCanvas project is a lightweight, minimal library for producing a 
           cross-platform window for hardware accelerated rendering. 

           LxCanvas does not intend to be a full fledged GUI toolkit, but rather an
           easy-to-use alternative when only OpenGL rendering is required.

           In its current form, the class is Windows-only.
         */
        namespace canvas_ns 
        {
            namespace detail 
            {

                class Win32WindowClass;

                //===========================================================================//
                //! Base class for a Lx Canvas Window
                /*!
                    \ingroup lx0_subsystem_canvas

                    Base window class intended to provide an easy to use wrapper on the Win32 windowing
                    APIs.  This is not intended to provide a full GUI system but rather a host window for
                    graphical windows such an OpenGL, DirectX, Direct2D, or other direct rendering
                    Win32 technologies.

                    This is primarily done via translating the "Window Procedure" (WNDPROC) into a 
                    series of virtual method calls that derived class can implement along with a 
                    slot/signal event mechanism for external code to hook into.   The general pattern is
                    this:
    
                    - CanvasBase's WNDPROC translates the Win32 data into more convenient form
                    - The "imp" method for the message is called.  If it return false, processing
                      stops here.
                    - The signal is emitted to any connected slots

                    Derived classes can also use overrideParentWndProc() to handle message that the
                    base class does not provide wrappers for.

                    The chain of events is very simple:
                    - The WNDPROC is called
                    - The message handler in the WNDPROC
                        - Calls the imp() virtual function
                            - If the imp() function return true, the message processing continues
                            - The slot() for the given message is invoked

                    Therefore:
                    - To respond to an event, attach a slot handler
                    - To modify an event in a way that requires pre-processing before the handlers,
                      the derive a class and override the imp() method.  Be sure to call the parent
                      imp() as well.
                    - To respond to an entirely new type of event, derive a new class, 
                      register a new WNDPROC, implement the above pattern for that message by introducing
                      a new imp() and slot for the new message.  The new WNDPROC should call the base
                      WNDPROC to handle all the usual messages.
                 */
                class CanvasBase
                {
                public:
                    ///@name Constructor / Destructor
                    ///@{
                                CanvasBase     (void);
                                ~CanvasBase    (void);
                    ///@}
                
                    ///@name Window Functions
                    ///@{
                    size_t      handle              (void);
                    void        show                (void);
                    void        invalidate          (void);
                    void        destroy             (void);
                    ///@} 

                    ///@name Window State
                    ///@{
                    bool                    isActive    (void) const;
                    bool                    isVisible   (void) const;

                    const KeyboardState&    keyboard    (void) const;    
                    int                     width       (void) const;
                    int                     height      (void) const;
                    ///@}

                    ///@name Event Signals
                    ///@{
                    lx0::slot<void()>             slotCreate;
                    lx0::slot<void()>             slotDestroy;
                    lx0::slot<void(bool)>         slotActivate;
                    lx0::slot<void()>             slotClose;
                    lx0::slot<void(int, int)>     slotResize;
                    lx0::slot<void()>             slotRedraw;

                    lx0::slot<void(unsigned int)> slotKeyDown;
                    lx0::slot<void(unsigned int)> slotKeyUp;


                    lx0::slot<void(int, int)>                                            slotMouseMove;
                    lx0::slot<void(const MouseState&, const ButtonState&, KeyModifiers)> slotLMouseClick;
                    lx0::slot<void(const MouseState&, const ButtonState&, KeyModifiers)> slotMMouseClick;
                    lx0::slot<void(const MouseState&, const ButtonState&, KeyModifiers)> slotRMouseClick;
                    lx0::slot<void(const MouseState&, int)>                              slotMouseWheel;
                    lx0::slot<void(const MouseState&, const ButtonState&, KeyModifiers)> slotLMouseDrag;
                    lx0::slot<void(const MouseState&, const ButtonState&, KeyModifiers)> slotMMouseDrag;
                    lx0::slot<void(const MouseState&, const ButtonState&, KeyModifiers)> slotRMouseDrag;
                    ///@}

                    ///@name OpenGL Interface
                    ///@{
                    std::shared_ptr<lx0::OpenGlApi3_2>  getOpenGlApi   () { return gl; }
                    ///@}

                
                protected:
                    ///@name Event handlers for derived classes
                    ///
                    /// Guarentees the derived class a first chance at handling the event before the
                    /// public signal is sent out.
                    ///
                    ///@{
                    virtual void    impCreate   (void)                      {}
                    virtual void    impDestroy  (void)                      {}
                    virtual bool    impResize   (int iWidth, int iHeight)   { return true; }
                    virtual bool    impClose    (void)                      { return true; }
                    virtual bool    impRedraw   (void)                      { return true; }
                    virtual bool    impKeyDown  (unsigned int keyCode)      { return true; }
                    virtual bool    impKeyUp    (unsigned int keyCode)      { return true; }
    
                    virtual bool    impMouseWheel   (const MouseState&, int delta)                            { return true; }
                    virtual bool    impLMouseClick  (const MouseState&, const ButtonState&, KeyModifiers)     { return true; }
                    virtual bool    impMMouseClick  (const MouseState&, const ButtonState&, KeyModifiers)     { return true; }
                    virtual bool    impRMouseClick  (const MouseState&, const ButtonState&, KeyModifiers)     { return true; }
                    virtual bool    impLMouseDrag   (const MouseState&, const ButtonState&, KeyModifiers)     { return true; }
                    virtual bool    impMMouseDrag   (const MouseState&, const ButtonState&, KeyModifiers)     { return true; }
                    virtual bool    impRMouseDrag   (const MouseState&, const ButtonState&, KeyModifiers)     { return true; }
                    ///@}

                    //@name Low-level override of the WNDPROC
                    //@{
                    void            overrideParentWndProc   (void* wndProc);
                    //@}

                    static int __stdcall windowProc(void* hWnd, unsigned int uMsg, unsigned int* wParam, long* lParam );
                    static Win32WindowClass s_windowClass;

                    void*           mOpaqueHwnd;;

                    KeyboardState   mKeyboard;
                    MouseState      mMouse;
                    ButtonState     mLButton;
                    ButtonState     mMButton;
                    ButtonState     mRButton;

                    std::shared_ptr<lx0::OpenGlApi3_2>  gl;
                };



                //===========================================================================//
                //! Canvas window that create an OpenGL context
                /*!
                    \ingroup lx0_subsystem_canvas
                    
                    Further documentation TBD.
                 */
                class CanvasGL : public CanvasBase
                {
                public:
    
                    //@name Constructor / Destructor
                    //@{
                    CanvasGL  (const char* pszTitle, int x, int y, int w, int h, bool bResizeable);
                    ~CanvasGL (void);
                    //@}

                    //@name Window Functions
                    //@{
                    void swapBuffers        (void);
                    //@}

                protected:

                    //! Custom WNDPROC
                    static int __stdcall windowProc(void* hWnd, unsigned int uMsg, unsigned int* wParam, long* lParam );
   
                    //@name Event Handlers
                    //@{
                    virtual void impCreate();
                    virtual void impDestroy();   
                    virtual bool impRedraw();
                    virtual bool impResize(int width, int height);
                    //@}


                private:
                    void    _createWindow       (const char* pszTitle, int x, int y, int w, int h, bool bResizeable);

                    void    createGlContext     (void);
                    void    destroyGlContext    (void);

                    unsigned int mPixelFormat;
                    void*   m_opaque_hDC;   //!< Handle to Device Context
                    void*   m_opaque_hRC;   //!< Handle to OpenGL Rendering Context
                    

                    bool    mRedrawActive;
                };


                //===========================================================================//
                //!
                /*!
                    \ingroup lx0_subsystem_canvas
                 */
                class CanvasHost
                {
                public:
                    void                create          (CanvasBase* pWin, const char* id, bool bVisible);
                    bool                processEvents   (void);
                    void                shutdown        (void);

                protected:
                    void                destroyWindows  (void);

                    typedef std::map<std::string, CanvasBase*> WindowMap;
                    WindowMap   m_windows;
                };

            } // namespace detail

            using detail::CanvasGL;
            using detail::CanvasHost;
        }
    }

    using namespace lx0::subsystem::canvas_ns;
} 
