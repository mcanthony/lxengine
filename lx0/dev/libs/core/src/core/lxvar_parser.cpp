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

#include <lx0/core.hpp>
#include "lxvar_parser.hpp"

using namespace lx0::core;

namespace lx0 { namespace core {

    namespace detail {

        BaseParser::BaseParser()
        {
            _reset(nullptr);
        }

        void
        BaseParser::_reset (const char* pStream)
        {
            mpStartText = pStream;
            mState.resize(1);
            mState.back().mpStartLine = pStream;
            mState.back().mpStream = pStream;
            mState.back().mLineNumber = 1;
            mState.back().mColumn = 0;
        }

        void
        BaseParser::_pushState (void)
        {
            mState.push_back(mState.back());
        }

        void
        BaseParser::_popState (void)
        {
            mState.pop_back();
        }

        char
        BaseParser::_peek (void)
        {
            return *state().mpStream;
        }

        char            
        BaseParser::_advance (void)
        {
            if (*state().mpStream == '\n')
            {
                state().mLineNumber++;
                state().mpStartLine = state().mpStream + 1;
                state().mColumn = 0;
            }
            else
                ++state().mColumn;

            return *state().mpStream++;
        }

        void
        BaseParser::_consume (char c)
        {
            if (_peek() == c)
                _advance();
            else
            {
                std::string carrot;
                carrot.reserve(_column());
                for (int i = 0; i < _column() - 1; ++i)
                    carrot += " ";
                carrot += "^";

                lx_error(
                    "JSON Parse Error, line %d.\n%s\n%s\n"
                    "Did not find expected character '%c'.  Found '%c' instead.",
                         state().mLineNumber, _currentLine().c_str(), carrot.c_str(), c, *state().mpStream);
            }
        }

        bool
        BaseParser::_consumeConditional (char c)
        {
            if (_peek() == c)
            {
                _advance();
                return true;
            }
            else
                return false;
        }

        
        void            
        BaseParser::_skipWhitespace (void)
        {
            while( isspace(_peek()) ) 
                _advance();
        }

        std::string     
        BaseParser::_currentLine (void) const
        {
            const char* p = state().mpStartLine;
            std::string line;
            while (*p != '\n' && *p)
                line += *p++;
            return line;
        }
    }

    using namespace detail;

    lxvar
    LxsonParser::parse (const char* pStream)
    {
        _reset(pStream);
        return _readValue();
    }

    lxvar 
    LxsonParser::_readNumber ()
    {
        _skipWhitespace();

        lxvar w;
        int i = 0;

        int sign = 1;
        if (_consumeConditional('-'))
        {
            sign = -1;
            _skipWhitespace();
        }

        while(isdigit(_peek()))
        {
            int t = _peek() - '0';
            i = 10 * i + t;
            _advance();
        }
        w = sign * i;

    
        if (_peek() == '.')
        {
            _advance();
            double f = i;
            int divisor = 10;
            while (isdigit(_peek()))
            {
                int t = _peek() - '0';
                f += double(t) / divisor;
                _advance();
                divisor *= 10;
            }
            w = float(sign * f);
        }

        _skipWhitespace();

        return w;
    }

    /*!
        @todo Escape character handling.
     */
    std::string 
    LxsonParser::_readString (void)
    {
        std::string t;

        _skipWhitespace();

        const char delimiter = (_peek() == '\'') ? '\'' : '\"';

        _consume(delimiter);
        while (_peek() != delimiter)
        {
            t += _advance();
        }
        _consume(delimiter);

        return t;
    }

    std::string 
    LxsonParser::_readToEnd (void)
    {
        std::string t;
        while (_peek())
            t += _advance();
        return t;
    }

    /*!
        @todo Escape character handling.
     */
    std::string 
    LxsonParser::_readUnquotedString (void)
    {
        std::string t;

        lx_check_error(isalpha(_peek()) != 0);

        while (isalnum(_peek()) || _peek() == '_') 
        {
            t += _advance();
        }

        return t;
    }

    lxvar 
    LxsonParser::_readArray (void)
    {
        lxvar obj;
        obj.toArray();

        _skipWhitespace();
        _consume('[');

        do
        {
            _skipWhitespace();
            if (_peek() == ']')
                break;

            lxvar value = _readValue();
            obj.push(value);

        } while ( _consumeConditional(',') );

        _skipWhitespace();
        _consume(']');

        return obj;
    }

    std::string
    LxsonParser::_readKey (void)
    {
        if (isalpha(_peek()))
            return _readUnquotedString();
        else
            return _readString();
    }

    lxvar 
    LxsonParser::_readObject (void)
    {
        lxvar obj;
        obj.toMap();
    
        _skipWhitespace();
        _consume('{');

        do 
        {
            _skipWhitespace();
            if (_peek() == '}')
                break;

            ///@todo Limitation: currently assumes keys are always strings
            _skipWhitespace();
            std::string key = _readKey();
            _skipWhitespace();

            _consume(':');

            _skipWhitespace();
            lxvar value = _readValue();
            _skipWhitespace();

            obj.insert(key.c_str(), value);

        } while ( _consumeConditional(',') );

        _skipWhitespace();

        _consume('}');

        return obj;
    }

    /*!
        Returns true is the upcoming characters appear to be parse-able as an
        lxson 'named map'.  See _readLxNamedMap.
     */
    bool
    LxsonParser::_peekLxNamedMap (void)
    {
        bool isLxNamedMap = false;

        _pushState();
        int count = 0;
        while (isalpha(_peek()))
        {
            _advance();
            count++;
        }
        if (count > 0)
        {
            _skipWhitespace();
            if (_peek() == '{')
                isLxNamedMap = true;
        }
        _popState();

        return isLxNamedMap;
    }

    /*!
        Extension of the JSON syntax that allows for a short-hand for creating
        a name-object pair.

        Example:
        phong { } -> [ "phong", { } ]
     */
    lxvar
    LxsonParser::_readLxNamedMap (void)
    {
        lxvar r = lxvar::array();
        r.push( _readUnquotedString() );
        _skipWhitespace();
        r.push( _readObject() );
        return r;
    }

    lxvar
    LxsonParser::_readValue (void)
    {
        lxvar r;

        _skipWhitespace();
        switch (_peek())
        {
        case '\''   : // fall through
        case '\"'   : return lxvar( _readString().c_str() );

        case '{'    : return _readObject();
        case '['    : return _readArray();
        
        default:
            {
                if (strchr("0123456789.-", _peek()))
                    r = _readNumber();
                else if (_peekLxNamedMap())
                    r = _readLxNamedMap();
                else
                    r = _readToEnd();
            }
        };

        return r;
    }

}}