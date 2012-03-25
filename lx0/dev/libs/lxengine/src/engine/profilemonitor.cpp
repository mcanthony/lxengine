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

//===========================================================================//
//   H E A D E R S
//===========================================================================//

#include <cassert>
#include <fstream>

#include <lx0/lxengine.hpp>
#include <lx0/engine/profilemonitor.hpp>

namespace lx0 { namespace engine { namespace profilemonitor_ns {

    //===========================================================================//

    ProfileCounter::ProfileCounter()
    {
        ::memset(this, 0, sizeof(*this));
    }

    //===========================================================================//

    __declspec(thread) ProfileCounter* _profileCounterTable;
    __declspec(thread) ProfileCounter* _activeCounter = nullptr;

    ProfileMonitor::ProfileMonitor() 
        : mSize          (0)
    {
        mNameMap.push_back("<invalid id>");
    }

    void 
    ProfileMonitor::registerCounter (const char* name, int* id)
    {
        //
        // Multiple threads may be registering counters; throw a lock around
        // this non-performance critical method to ensure simultaneous, data-structure
        // corrupting registrations do not occur.
        //
        boost::lock_guard<boost::mutex> lock(mMutex);

        if (!ProfileSection::pMonitor)
            ProfileSection::pMonitor = this;

        auto it = mNameMap2.find(name);
        if (it == mNameMap2.end())
        {
            *id = ++mSize;
            mNameMap.push_back(name);
            mNameMap2.insert(std::make_pair(std::string(name), *id));
        }
        else
            *id = it->second;
    }

    void
    ProfileMonitor::addRelation (const char* parentName, const char* childName)
    {
        boost::lock_guard<boost::mutex> lock(mMutex);
        mRelations.push_back( std::make_pair(std::string(parentName), std::string(childName)) );
    }

    ProfileCounter* 
    ProfileMonitor::enter (int counterId)
    {
        if (!_profileCounterTable)
        {
            auto it = mThreadMap.find(lx0::lx_current_thread_id());
            if (it != mThreadMap.end())
            {
                _profileCounterTable = it->second;
            }
            else
            {
                _profileCounterTable = new ProfileCounter[2048];
                mThreadMap.insert( std::make_pair(lx0::lx_current_thread_id(), _profileCounterTable) );
            }
        }
        ProfileCounter* pCounter = &_profileCounterTable[counterId];
               
        auto now = lx0::lx_ticks();

        pCounter->calls++;

        if (++pCounter->depth == 1)
            pCounter->inclusiveStart = now;
        
        if (_activeCounter)
            _activeCounter->exclusive += (now - _activeCounter->exclusiveStart);
        pCounter->pPrevious = _activeCounter;

        pCounter->exclusiveStart = now;        
        _activeCounter = pCounter;
        
        return pCounter;
    }

    void 
    ProfileMonitor::leave (ProfileCounter* pCounter)
    {
        auto now = lx0::lx_ticks();

        pCounter->exclusive += (now - pCounter->exclusiveStart);
        if (--pCounter->depth == 0)
            pCounter->inclusive += (now - pCounter->inclusiveStart);

        _activeCounter = pCounter->pPrevious;
        if (_activeCounter)
            _activeCounter->exclusiveStart = now;
    }

    void 
    ProfileMonitor::logCounters()
    {
        std::ofstream file;
        file.open("lxprofile.log");
        auto out = [&](std::string& s) {
            lx_log(s.c_str());
            file << s << std::endl;            
        };

        out( _lx_format("====== Profile Data ======"));

        double div = double( lx0::lx_ticks_per_second() ) / 1000.0;
        auto toMs = [div](lx0::int64 ticks) {
            return int(double(ticks) / div);
        };
        
        for (auto jt = mThreadMap.begin(); jt != mThreadMap.end(); ++jt)
        {           
            out( _lx_format("Thread %1% -----------------------", jt->first) );

            std::map<std::string,int> nameMap2;
            auto pTable = jt->second;

            for (auto i = 1; i <= mSize; ++i)
            {
                auto& name = mNameMap[i];
                auto& prof = pTable[i];

                if (prof.calls)
                {
                    nameMap2.insert(std::make_pair(name, i));

                    double avgTicks = double(prof.inclusive) / double(prof.calls);
                    double avgMs = avgTicks / div;

                    out( _lx_format("  %-30s :: %6d calls  %8dms inc %8dms ex %10.3f avg", 
                        name.c_str(),
                        prof.calls, 
                        toMs(prof.inclusive),
                        toMs(prof.exclusive),
                        avgMs
                    ));
                }
            }

            for (auto it = mRelations.begin(); it != mRelations.end(); ++it)
            {
                auto getId = [&](std::string& name) -> int {
                    auto it = nameMap2.find(name);
                    if (it != nameMap2.end())
                        return it->second;
                    else
                        return 0;
                };

                int parentId = getId(it->first);
                int childId = getId(it->second);

                if (parentId && childId)
                {
                    auto& parentName = mNameMap[parentId];
                    auto& childName = mNameMap[childId];
                    auto& parent = pTable[parentId];
                    auto& child = pTable[childId];

                    double percent = 100.0 * double(child.inclusive) / double(parent.inclusive);

                    out( _lx_format("  %-30s >     %7.3f%% of %-30s", childName, percent, parentName) );
                }
            }
        }

        file.close();
    }

    //===========================================================================//

    ProfileMonitor* ProfileSection::pMonitor = 0;

}}}
