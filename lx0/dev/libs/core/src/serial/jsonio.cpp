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

    lxvar
    JsonParser::parse (const char* pStream)
    {
        mpStream = pStream;
        mLineNumber = 1;

        return _readObject();
    }

    void
    JsonParser::_consume (char c)
    {
        if (*mpStream == c)
            mpStream ++;
        else
            lx_error("Did not find expected character '%c'.  Found '%c' instead.  Around line %d.",
                     c, *mpStream, mLineNumber);
    }

    bool
    JsonParser::_consumeConditional (char c)
    {
        if (*mpStream == c)
        {
            mpStream ++;
            return true;
        }
        else
            return false;
    }

    void            
    JsonParser::_skipWhitespace (void)
    {
        while( isspace(*mpStream) ) 
        {
            if (*mpStream == '\n')
                mLineNumber++;
            mpStream++;
        }
    }

    lxvar 
    JsonParser::_readNumber ()
    {
        _skipWhitespace();

        lxvar w;
        int i = 0;
        while(isdigit(*mpStream))
        {
            int t = *mpStream - '0';
            i = 10 * i + t;
            mpStream++;
        }
        w = i;

    
        if (*mpStream == '.')
        {
            mpStream++;
            double f = i;
            int divisor = 10;
            while (isdigit(*mpStream))
            {
                int t = *mpStream - '0';
                f += double(t) / divisor;
                mpStream++;
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
        while (*mpStream != '\"')
        {
            t += *mpStream++;
        }
        lx_check_error(*mpStream == '\"');

        mpStream++;
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
        switch (*mpStream)
        {
        case '\"'   : return lxvar( _readString().c_str() );
        case '{'    : return _readObject();
        case '['    : lx_error("Arrays not yet supported"); break;
        case 't'    : break;
        case 'f'    : break;
        case 'n'    : break;
        default:
            if (isdigit(*mpStream)) 
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
