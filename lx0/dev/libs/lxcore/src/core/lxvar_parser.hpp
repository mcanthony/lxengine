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

#pragma once

#include <lx0/core/base/slot.hpp>
#include <lx0/core/data/lxvar.hpp>

namespace lx0 { namespace core { namespace detail {

    class BaseParser
    {
    protected:
                        BaseParser();

        void            _reset              (const char* pStream);

        char            _peek               (void);
        char            _advance            (void);
        void            _consume            (char c);
        bool            _consumeConditional (char c);
        void            _skipWhitespace     (void);

        void            _pushState          (void);
        void            _popState           (void);

        int             _lineNumber         (void) const { return state().mLineNumber; }
        int             _column             (void) const { return state().mColumn; }
        std::string     _currentLine        (void) const;

    private:
        struct State
        {
            const char*     mpStartLine;
            const char*     mpStream;
            int             mLineNumber;
            int             mColumn;
        };

        const State&    state               (void) const     { return mState.back(); }
        State&          state               (void)           { return mState.back(); }

        const char*         mpStartText;
        std::vector<State>  mState;
    };

    /*!
        @todo Split into a callback parser and an lxvar builder
        */
    class LxsonParser : public detail::BaseParser
    {
    public:
        typedef lx0::core::lxvar        lxvar;

        lxvar           parse (const char* s);

    protected:
        std::string     _readToEnd          (void);
        lxvar           _readObject         (void);
        lxvar           _readArray          (void);
        std::string     _readString         (void);
        std::string     _readUnquotedString (void);
        std::string     _readKey            (void);
        lxvar           _readNumber         (void);
        lxvar           _readValue          (void);

        bool            _peekLxNamedMap     (void);
        lxvar           _readLxNamedMap     (void);
    };

}}}

