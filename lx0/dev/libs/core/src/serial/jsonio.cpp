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
            mpStream = pStream;
            mLineNumber = 1;
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
                mLineNumber++;

            return *mpStream++;
        }

        void
        BaseParser::_consume (char c)
        {
            if (_peek() == c)
                _advance();
            else
                lx_error("Did not find expected character '%c'.  Found '%c' instead.  Around line %d.",
                         c, *mpStream, mLineNumber);
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
    }

    using namespace detail;

    lxvar
    JsonParser::parse (const char* pStream)
    {
        _reset(pStream);
        return _readObject();
    }



    void            
    JsonParser::_skipWhitespace (void)
    {
        while( isspace(_peek()) ) 
            _advance();
    }

    lxvar 
    JsonParser::_readNumber ()
    {
        _skipWhitespace();

        lxvar w;
        int i = 0;
        while(isdigit(_peek()))
        {
            int t = _peek() - '0';
            i = 10 * i + t;
            _advance();
        }
        w = i;

    
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
            w = float(f);
        }

        _skipWhitespace();

        return w;
    }

    std::string 
    JsonParser::_readString (void)
    {
        std::string t;

        _skipWhitespace();
        _consume('\"');
        while (_peek() != '\"')
        {
            t += _advance();
        }
        lx_check_error(_peek() == '\"');

        _advance();
        _skipWhitespace();

        return t;
    }

    // Split this into a "well-formed" handler and a tolerant handler
    // so that
    //  key_name : "value"
    // is accepted even though it should be
    //  "key_name" : "value"
    // 
    lxvar
    JsonParser::_readValue (void)
    {
        lxvar r;

        _skipWhitespace();
        switch (_peek())
        {
        case '\"'   : return lxvar( _readString().c_str() );
        case '{'    : return _readObject();
        case '['    : lx_error("Arrays not yet supported"); break;
        case 't'    : break;
        case 'f'    : break;
        case 'n'    : break;
        default:
            if (isdigit(_peek())) 
                r = _readNumber();
            else
                lx_error("Unknown parse error");
        };
        _skipWhitespace();

        return r;
    }

    lxvar 
    JsonParser::_readObject (void)
    {
        lxvar obj;
    
        _skipWhitespace();
        _consume('{');
        _skipWhitespace();
        
        do 
        {
            ///@todo Limitation: currently assumes keys are always strings
            _skipWhitespace();
            std::string key = _readString();
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

}}
