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
#include <lx0/jsonio.hpp>

using namespace lx0::core;

namespace lx0 { namespace serial {

    namespace detail {

        BaseParser::BaseParser()
        {
            _reset(nullptr);
        }

        void
        BaseParser::_reset (const char* pStream)
        {
            mpStartText = pStream;
            mpStartLine = pStream;
            mpStream = pStream;
            mLineNumber = 1;
            mColumn = 0;
        }

        char
        BaseParser::_peek (void)
        {
            return *mpStream;
        }

        char            
        BaseParser::_advance (void)
        {
            if (*mpStream == '\n')
            {
                mLineNumber++;
                mpStartLine = mpStream + 1;
                mColumn = 0;
            }
            else
                ++mColumn;

            return *mpStream++;
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
                         mLineNumber, _currentLine().c_str(), carrot.c_str(), c, *mpStream);
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
            const char* p = mpStartLine;
            std::string line;
            while (*p != '\n' && *p)
                line += *p++;
            return line;
        }
    }

    using namespace detail;

    lxvar
    JsonParser::parse (const char* pStream)
    {
        _reset(pStream);
        return _readValue();
    }

    lxvar 
    JsonParser::_readNumber ()
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
    JsonParser::_readString (void)
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

    /*!
        @todo Escape character handling.
     */
    std::string 
    JsonParser::_readUnquotedString (void)
    {
        std::string t;

        lx_check_error(isalpha(_peek()));

        while (isalnum(_peek()) || _peek() == '_') 
        {
            t += _advance();
        }

        return t;
    }

    lxvar 
    JsonParser::_readArray (void)
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
    JsonParser::_readKey (void)
    {
        if (isalpha(_peek()))
            return _readUnquotedString();
        else
            return _readString();
    }

    lxvar 
    JsonParser::_readObject (void)
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

    lxvar
    JsonParser::_readValue (void)
    {
        lxvar r;

        _skipWhitespace();
        switch (_peek())
        {
        case '\''   : // fall through
        case '\"'   : return lxvar( _readString().c_str() );

        case '{'    : return _readObject();
        case '['    : return _readArray();
        
        case 't'    : break;
        case 'f'    : break;
        case 'n'    : break;
        
        default:
            if (strchr("0123456789.-", _peek()))
                r = _readNumber();
            else
                lx_error("Unknown parse error.  Lead character '%c'.", _peek());
        };

        return r;
    }

}}
