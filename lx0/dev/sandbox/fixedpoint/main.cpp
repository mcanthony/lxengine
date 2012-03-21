//===========================================================================//
/*
                                   LxEngine

    LICENSE

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

//===========================================================================//
//   H E A D E R S
//===========================================================================//

// Standard headers
#include <iostream>
#include <string>
#include <sstream>
#include <memory> 
#include <functional>
#include <vector>
#include <map>
#include <deque>

// Lx0 headers
#include <lx0/lxengine.hpp>
#include <lx0/core/lxvar/lxvar.hpp>
#include <lx0/util/misc/util.hpp>

#include <glgeom/core/_detail/swizzle_members.hpp>

#include <windows.h>

using namespace lx0::core;

class fixed8
{
public:
                fixed8() {}
    explicit    fixed8(int value);
    explicit    fixed8(float v);
    explicit    fixed8(double v);

    operator float () { return float(mValue) / kScale; }
    operator double() { return double(mValue) / kScale; }

    friend fixed8 operator+ (fixed8, fixed8);
    friend fixed8 operator- (fixed8, fixed8);

protected:
    enum { 
        kShift = 8,
        kScale = (1 << kShift),
    };

    static fixed8   _raw    (int value)    { fixed8 t; t.mValue = value; return t; }

    int mValue;
};



fixed8::fixed8 (int value)
    : mValue (value * kScale)
{
}

fixed8::fixed8 (float v)
    : mValue (int(v * kScale))
{
}

fixed8::fixed8 (double v)
    : mValue (int(v * kScale))
{
}


inline fixed8 operator+ (fixed8 a, fixed8 b) { return fixed8::_raw(a.mValue + b.mValue); }
inline fixed8 operator- (fixed8 a, fixed8 b) { return fixed8::_raw(a.mValue - b.mValue); }

//===========================================================================//

#include <boost/thread.hpp>
#include <boost/thread/mutex.hpp>

typedef std::function<void()>  YieldFunc;

struct InterleavedThreads
{
    InterleavedThreads (std::function<void(YieldFunc)> f0, std::function<void(YieldFunc)> f1)
    {
        volatile int start0 = 0;
        volatile int start1 = 0;

        auto yield = [&]() 
        {
            active.unlock();
            waiting.lock();
            active.lock();
            waiting.unlock();
        };

        boost::thread_group group;
        group.create_thread( [&]() {
            
            active.lock();
            start0 = 1;
            while (!start1) {}

            f0(yield);

            active.unlock();
        });
        group.create_thread(  [&]() {
            
            while (!start0) {}
            
            waiting.lock();
            start1 = 1;

            active.lock();
            waiting.unlock();

            f1(yield);

            active.unlock();
        });
        group.join_all();
    }

    volatile int    started;
    boost::mutex    active;
    boost::mutex    waiting;
};

void count_odds (YieldFunc yield)
{
    for (int i = 1; i < 100; i += 2)
    {
        std::cout << i << ", ";
        std::cout.flush();
        yield();
    }
}

void count_evens (YieldFunc yield)
{
    for (int i = 0; i < 100; i += 2)
    {
        std::cout << i << ", ";
        std::cout.flush();
        yield();
    }
}

void interleaved()
{
    InterleavedThreads(count_evens, count_odds);
}

//===========================================================================//



struct triple
{
    triple () : x(0), y(0), z(0) {}
    triple (int a, int b, int c) : x(a), y(b), z(c) {}  
    
    void operator= (const triple& t)
    {
        x = t.x;
        y = t.y;
        z = t.z;
    }
    int operator[] (int i) const { return e[i]; }

    union 
    {
        struct
        {
            int x, y, z;
        };
        int e[3];

        _GLGEOM_SWIZZLE3_MEMBERS(int,triple,x,y,z);
    };
    
};

//===========================================================================//

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

class ProfileMonitor
{
public:
    ProfileMonitor();

    void            registerCounter     (const char* name, int* id);

    ProfileCounter* enter               (int counterId);
    void            leave               (ProfileCounter* pCounter);

    void            logCounters         (void);

protected:
    typedef std::map<lx0::uint32, ProfileCounter*> ThreadMap;

    ProfileCounter*             mpActiveCounter;
    ThreadMap                   mThreadMap;  
    std::vector<std::string>    mNameMap;
    int                         mSize;
};

class ProfileSection
{
public:
    ProfileSection (int id) { pCounter = pMonitor->enter(id); }
    ~ProfileSection () { pMonitor->leave(pCounter); }

    static ProfileMonitor* pMonitor;
    ProfileCounter* pCounter;
};

//===========================================================================//

ProfileCounter::ProfileCounter()
{
    ::memset(this, 0, sizeof(*this));
}

//===========================================================================//

__declspec(thread) ProfileCounter* _profileCounterTable;

ProfileMonitor::ProfileMonitor() 
    : mpActiveCounter(nullptr) 
    , mSize          (0)
{
    mNameMap.push_back("<invalid id>");
}

void ProfileMonitor::registerCounter (const char* name, int* id)
{
    if (!ProfileSection::pMonitor)
        ProfileSection::pMonitor = this;

    *id = ++mSize;
    mNameMap.push_back(name);
}

ProfileCounter* ProfileMonitor::enter (int counterId)
{
    if (!_profileCounterTable)
    {
        _profileCounterTable = new ProfileCounter[2048];
        mThreadMap.insert(std::make_pair(lx0::lx_current_thread_id(), _profileCounterTable));
    }
    ProfileCounter* pCounter = &_profileCounterTable[counterId];
               
    auto now = lx0::lx_ticks();

    pCounter->calls++;

    if (++pCounter->depth == 1)
        pCounter->inclusiveStart = now;

    pCounter->pPrevious = mpActiveCounter;
    if (pCounter->pPrevious)
        pCounter->pPrevious->exclusive += (now - pCounter->pPrevious->exclusiveStart);
        
    pCounter->exclusiveStart = now;        
    mpActiveCounter = pCounter;
        
    return pCounter;
}

void ProfileMonitor::leave (ProfileCounter* pCounter)
{
    auto now = lx0::lx_ticks();

    pCounter->exclusive += (now - pCounter->exclusiveStart);
    if (--pCounter->depth == 0)
        pCounter->inclusive += (now - pCounter->inclusiveStart);

    if (pCounter->pPrevious)
        pCounter->pPrevious->exclusiveStart = now;
    mpActiveCounter = pCounter->pPrevious;
}

void ProfileMonitor::logCounters()
{
    double div = double( lx0::lx_ticks_per_second() );

    for (auto jt = mThreadMap.begin(); jt != mThreadMap.end(); ++jt)
    {
        lx_log("Thread %1% -----------------------", jt->first);

        auto pTable = jt->second;
        for (auto i = 1; i <= mSize; ++i)
        {
            auto& name = mNameMap[i];
            auto& prof = pTable[i];

            lx_log("(%5% :: calls=%1% inc=%2%ms ex=%3%ms ex2=%4%)", 
                prof.calls, 
                double(prof.inclusive) / div,
                double(prof.exclusive) / div,
                prof.exclusive,
                name
            );
        }
    }
}

//===========================================================================//

ProfileMonitor* ProfileSection::pMonitor = 0;

//===========================================================================//

ProfileMonitor gProfileMonitor;

struct
{
    int total;
    int func1;
    int func2;
    int func3;
} counters;

//===========================================================================//
//   E N T R Y - P O I N T
//===========================================================================//

void func3 ()
{
    ProfileSection _section(counters.func3);

    ::Sleep(750);
}

void func2 (int depth)
{
    ProfileSection _section(counters.func2);

    lx_log("Time %2% = %1%", lx0::lx_milliseconds(), depth);
    ::Sleep(50);
    if (depth < 9)
    {
        if (depth == 3)
            func3();
        func2(depth + 1);
    }
}

void func1()
{
    ProfileSection _section(counters.func1);

    ::Sleep(1000);
    func2(0);
    ::Sleep(1000);
}

int 
main (int argc, char** argv)
{
    gProfileMonitor.registerCounter("total", &counters.total);
    gProfileMonitor.registerCounter("func1", &counters.func1);
    gProfileMonitor.registerCounter("func2", &counters.func2);
    gProfileMonitor.registerCounter("func3", &counters.func3);

    {
        ProfileSection _section(counters.total);
        func1();
    }

    gProfileMonitor.logCounters();

    return 0;
}
