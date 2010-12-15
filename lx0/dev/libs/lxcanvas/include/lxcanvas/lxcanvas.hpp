//===========================================================================//
/*
                                   LxEngine

    LICENSE
    * MIT License (http://www.opensource.org/licenses/mit-license.php)

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

/*!
   \mainpage

   The LxCanvas project is a lightweight, minimal library for producing a 
   cross-platform window for hardware accelerated rendering. 

   LxCanvas does not intend to be a full fledged GUI toolkit, but rather an
   easy-to-use alternative when only OpenGL rendering is required.

   In its current form, the class is Windows-only.
 */

#pragma once

#include <lx0/slot.hpp>
#include <map>
#include <string>

namespace lx0 { namespace canvas {

    namespace platform {

        class Win32WindowClass;

        /// WIP class
        class KeyModifiers
        {
        public:
            KeyModifiers() : packed (0) {}
            union
            {
                struct 
                {
                    unsigned    ctrl    : 1;
                    unsigned    rctrl   : 1;
                    unsigned    lctrl   : 1;
                    unsigned    alt     : 1;
                    unsigned    ralt    : 1;
                    unsigned    lalt    : 1;
                    unsigned    shift   : 1;
                    unsigned    rshift  : 1;
                    unsigned    lshift  : 1;
                };
                unsigned int    packed;
            };
        };

        /// WIP class
        class MouseState
        {
        public:
            MouseState()
                : x (0)
                , y (0)
                , previousX(0)
                , previousY (0)
            {
            }

            int             deltaX      (void)      const { return x - previousX; }
            int             deltaY      (void)      const { return y - previousY; }

            int x, y;
            int previousX, previousY;
        };

        /// WIP class
        class ButtonState
        {
        public:
            ButtonState()
                : bDown     (false)
                , startX    (0)
                , startY    (0)
            {
            }

            bool            bDown;         
            int             startX, startY;         // drag starting point
        };

        //===========================================================================//
        //! Base class for a Lx Canvas Window
        /*!
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
            void        show                (void);
            void        invalidate          (void);
            void        destroy             (void);
            ///@} 

            ///@name Window State
            ///@{
            bool        isActive            (void) const;
            bool        isVisible           (void) const;
            ///@}

            ///@name Event Signals
            ///@{
            lx0::core::slot<void()>             slotCreate;
            lx0::core::slot<void()>             slotDestroy;
            lx0::core::slot<void(bool)>         slotActivate;
            lx0::core::slot<void()>             slotClose;
            lx0::core::slot<void(int, int)>     slotResize;
            lx0::core::slot<void()>             slotRedraw;

            lx0::core::slot<void(unsigned int)> slotKeyDown;
            lx0::core::slot<void(unsigned int)> slotKeyUp;


            lx0::core::slot<void(int, int)>                                            slotMouseMove;
            lx0::core::slot<void(const MouseState&, int)>                              slotMouseWheel;
            lx0::core::slot<void(const MouseState&, const ButtonState&, KeyModifiers)> slotLMouseDrag;
            lx0::core::slot<void(const MouseState&, const ButtonState&, KeyModifiers)> slotMMouseDrag;
            lx0::core::slot<void(const MouseState&, const ButtonState&, KeyModifiers)> slotRMouseDrag;
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
            virtual bool    impLMouseDrag   (const MouseState&, const ButtonState&, KeyModifiers)     { return true; }
            ///@}

            //@name Low-level override of the WNDPROC
            //@{
            void            overrideParentWndProc   (void* wndProc);
            //@}

            static int __stdcall windowProc(void* hWnd, unsigned int uMsg, unsigned int* wParam, long* lParam );
            static Win32WindowClass s_windowClass;

            void*         m_opaque_hWnd;

            MouseState  m_mouse;
            ButtonState m_lButton;
            ButtonState m_mButton;
            ButtonState m_rButton;
        };



        //===========================================================================//
        //! Canvas window that create an OpenGL context
        /*!
            Further documentation TBD.
         */
        class CanvasGL : public CanvasBase
        {
        public:
    
            //@name Constructor / Destructor
            //@{
            CanvasGL  (const char* pszTitle, int w, int h, bool bResizeable);
            ~CanvasGL (void);
            //@}

            //@name Window Functions
            //@{
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
            void    createGlContext     (void);
            void    destroyGlContext    (void);
    
            void*   m_opaque_hDC;   //!< Handle to Device Context
            void*   m_opaque_hRC;   //!< Handle to OpenGL Rendering Context
        };


        //===========================================================================//
        //!
        /*!
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

    } // namespace platform


}} // namespace lx0::canvas
