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

#include <deque>
#include <map>
#include <functional>
#include <boost/thread.hpp>

namespace lx0 { namespace engine_ns { namespace detail { 


    class Event
    {
    public:
        enum State {
            kPending,
            kActive,
            kDone,
        };

        State                               state;

        std::string                         message;
        std::function<void()>               task;
        std::function<int()>                func;
        std::weak_ptr<std::function<int()>> wpFunc;
    };

    /*
        The event queue is intended to store time-based, repeatable events.
     */
    class EventQueue
    {
    public:
        int         run             (unsigned int realTime, unsigned int frameTime);
        void        enqueue         (int time, Event& evt);   

    protected:
        boost::mutex                        mMutex;
        std::deque<Event>                   mRealtimeQueue;
        std::map<unsigned int,Event>        mRealtimeDelayed;
        std::deque<Event>                   mFrametimeQueue;
        std::map<unsigned int,Event>        mFrametimeDelayed;
    };


} } }
