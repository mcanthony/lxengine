//===========================================================================//
/*
                                   LxEngine

    LICENSE
    * MIT License (http://www.opensource.org/licenses/mit-license.php)

    Copyright (c) 2012 athile@athile.net (http://www.athile.net)

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

namespace lx0 
{ 
    namespace engine 
    {
        namespace profilemonitor_ns
        {
            //===========================================================================//
            //! 
            /*!
             */
            class ProfileCounter
            {
            public:
                ProfileCounter();
    
                lx0::uint32     calls;
                lx0::int64      inclusive;
                lx0::int64      exclusive;

                lx0::uint32     depth;
                lx0::int64      inclusiveStart;
                lx0::int64      exclusiveStart;
                ProfileCounter* pPrevious;
            };

            //===========================================================================//
            //! 
            /*!
             */
            class ProfileMonitor
            {
            public:
                ProfileMonitor();

                void            registerCounter     (const char* name, int* id);
                void            addRelation         (const char* parentName, const char* childName);

                ProfileCounter* enter               (int counterId);
                void            leave               (ProfileCounter* pCounter);

                void            logCounters         (void);

            protected:
                typedef std::map<lx0::uint32, ProfileCounter*> ThreadMap;

                ProfileCounter*             mpActiveCounter;
                ThreadMap                   mThreadMap;  
                std::vector<std::string>    mNameMap;
                int                         mSize;

                std::vector<std::pair<std::string,std::string>> mRelations;
            };

            //===========================================================================//
            //! 
            /*!
             */
            class ProfileSection
            {
            public:
                ProfileSection (int id) { pCounter = pMonitor->enter(id); }
                ~ProfileSection () { pMonitor->leave(pCounter); }

                static ProfileCounter* enter (int id)                   { return pMonitor->enter(id); }
                static void            leave (ProfileCounter* pCounter) { pMonitor->leave(pCounter); }

                static ProfileMonitor* pMonitor;
                ProfileCounter* pCounter;
            };
        }
    }

    using namespace lx0::engine::profilemonitor_ns;
}