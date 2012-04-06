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

#include <lx0/engine/detail/eventqueue.hpp>

namespace lx0 { namespace engine_ns { namespace detail { 
   
    static inline
    void _extractDelayed (unsigned int time, std::map<unsigned int, Event>& delayed, std::deque<Event>& queue)
    {
        auto it = delayed.begin();
        while (it != delayed.end())
        {
            // Note the subtle but important use of post-increment on the
            // iterator.  We need to advance before invalidating via erase
            // but still pass in the prior value to ensure the right entry
            // is erased.
            if (it->first <= time)
            {
                queue.push_back(it->second);
                delayed.erase(it++);
            }
            else
                break;
        }
    }

    static bool
    _runQueue (unsigned int realTime, 
               unsigned int frameTime,
               std::deque<Event>& queue, 
               std::deque<Event>& requeueRt,
               std::deque<Event>& requeueFt,
               std::map<unsigned int,Event>& requeueRtDelayed, 
               std::map<unsigned int,Event>& requeueFtDelayed)
    {
        bool bDone = false;

        while (!queue.empty())
        {
            Event& evt = queue.front();
            evt.state = Event::kActive;

            int delay = 0;

            std::shared_ptr<std::function<int()>> spFunc;

            if (evt.message == "quit")
            {
                bDone = true;
            }
            else if (evt.task)
            {
                evt.task();
            }
            else if (evt.func)
            {
                delay = evt.func();
            }
            else if (spFunc = evt.wpFunc.lock())
            {
                //
                // Cancel-able events use a weak_ptr to allow the event to expire:
                // the caller needs to maintain the shared_ptr as long as they want
                // the event to exist; if the reference to that event is released,
                // then the weak_ptr will expire and the event will not be processed
                // by the engine.
                //
                delay = (*spFunc)();
            }

            //
            // Requeue or discard the event
            //
            if (delay != 0)
            {
                bool bFrameTime = (delay < 0);
                std::deque<Event>* pRequeue = bFrameTime ? &requeueFt : &requeueRt;
                std::map<unsigned int,Event>* pDelayed  = bFrameTime ? &requeueFtDelayed : &requeueRtDelayed;

                unsigned int time;
                if (bFrameTime) 
                    time = frameTime + -delay;
                else
                    time = realTime + delay;

                evt.state = Event::kPending;
                if (delay == 1)
                    pRequeue->push_back(evt);
                else
                    pDelayed->insert(std::make_pair(time, evt));
            }
            else 
                evt.state = Event::kDone;

            queue.pop_front();
        }

        return bDone;
    }

    /*
        Since this is an internal function we pack the return value:
        
        * -1 implies a "quit" message of some sort has been processed
        * 0 implies to events were processed (i.e. "idle")
        * > 0 is the number of events processed
     */
    int
    EventQueue::run (unsigned int realTime, unsigned int frameTime)
    {
        std::deque<Event> q1;
        std::deque<Event> q2;

        //
        // Lock the queues since their contents will be modified here.
        //
        // Move the data to be processed to local containers to limit
        // the time the lock is needed.
        //
        {
            boost::lock_guard<boost::mutex> lock(mMutex);

            //
            // Everything in the regular queues are supposed to
            // run on the next cycle, so add them automatically.
            //
            // For efficiency, use swap() rather than adding both to
            // a single queue.
            //
            q1.swap( mRealtimeQueue );
            q2.swap( mFrametimeQueue );

            _extractDelayed(realTime, mRealtimeDelayed, q1);
            _extractDelayed(frameTime, mFrametimeDelayed, q2);
        }

        //
        // Run the "current" events and track what needs to be requeued in local
        // containers.
        //
        std::deque<Event>            requeueRt, requeueFt;
        std::map<unsigned int,Event> requeueRtDelayed, requeueFtDelayed;

        int  eventCount = int(q1.size() + q2.size());
        bool bDone = false;
        bDone |= _runQueue(realTime, frameTime, q1, requeueRt, requeueFt, requeueRtDelayed, requeueFtDelayed);
        bDone |= _runQueue(realTime, frameTime, q2, requeueRt, requeueFt, requeueRtDelayed, requeueFtDelayed);

        //
        // Lock the queues and move the local data into those queues
        //
        {
            boost::lock_guard<boost::mutex> lock(mMutex);

            mRealtimeQueue.insert(mRealtimeQueue.end(), requeueRt.begin(), requeueRt.end());
            mFrametimeQueue.insert(mFrametimeQueue.end(), requeueFt.begin(), requeueFt.end());

            mRealtimeDelayed.insert(requeueRtDelayed.begin(), requeueRtDelayed.end());
            mFrametimeDelayed.insert(requeueFtDelayed.begin(), requeueFtDelayed.end());
        }

        return bDone ? -1 : eventCount;
    }


    void
    EventQueue::enqueue (int time, Event& evt)
    {
        boost::lock_guard<boost::mutex> lock(mMutex);
        if (time < 0)
        {
            if (time == -1)
                mFrametimeQueue.push_back(evt);
            else 
                mFrametimeDelayed.insert(std::make_pair(-time,evt));
        }
        else
        {
            if (time <= 1)
                mRealtimeQueue.push_back(evt);
            else
                mRealtimeDelayed.insert(std::make_pair(time,evt));
        }
    }

} } }
