//===========================================================================//
/*
                                   LxEngine

    LICENSE
    * MIT License (http://www.opensource.org/licenses/mit-license.php)

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

#pragma once

#include <functional>
#include <deque>
#include <tuple>

namespace lx0 { 
    
    namespace core {
        
        namespace slot_ns
        {

            //===========================================================================//
            //! A Qt-like slot
            /*
                \ingroup lx0_core_slot

                TODO:
                - Use PIMPL pattern to reduce overhead of common cases (empty slot, 1 call slot, etc.)
                - When variadic templates are supported by VS2010, simplify invokation
                - Make mIdCounter into a global (no need for it to be a member)
             */
            template <typename Signature>
            class slot
            {
            public:
                typedef std::function<Signature> Function;
                typedef std::deque<std::pair<int,Function>> FunctionList;

                slot() : mIdCounter (0) {}

                //@name Invokation
                //@{
                void operator() () { invoke(std::make_tuple()); }
                template <typename T0>              
                void operator()  (T0 t0) { invoke(std::make_tuple(t0) ); }
                template <typename T0, typename T1> 
                void operator()  (T0 t0, T1 t1) { invoke(std::make_tuple(t0, t1) ); }
                template <typename T0, typename T1, typename T2> 
                void operator()  (T0 t0, T1 t1, T2 t2) { invoke(std::make_tuple(t0, t1, t2) ); }
                template <typename T0, typename T1, typename T2, typename T3> 
                void operator()  (T0 t0, T1 t1, T2 t2, T3 t3) { invoke(std::make_tuple(t0, t1, t2, t3) ); }
                template <typename T0, typename T1, typename T2, typename T3, typename T4> 
                void operator()  (T0 t0, T1 t1, T2 t2, T3 t3, T4 t4) { invoke(std::make_tuple(t0, t1, t2, t3, t4) ); }
                template <typename T0, typename T1, typename T2, typename T3, typename T4, typename T5> 
                void operator()  (T0 t0, T1 t1, T2 t2, T3 t3, T4 t4, T5 t5) { invoke(std::make_tuple(t0, t1, t2, t3, t4, t5) ); }
                //@}

                int  operator=  (Function f) { mBody.resize(1); mBody[0] = std::make_pair(++mIdCounter, f); return mIdCounter; }
                int  operator+= (Function f) { mBody.push_back(std::make_pair(++mIdCounter, f)); return mIdCounter; }
                void operator-= (int id) { 
                    for (auto it = mBody.begin(); it != mBody.end(); ++it) { 
                        if (it->first == id) { 
                            mBody.erase(it); 
                            return; 
                        } 
                    } 
                }

            protected:
                int                  mIdCounter;
                FunctionList         mBody;

            private:
                    // Expand out a tuple of arguments into a list for a function call
                template <typename F, typename T, int N>
                struct invokeN { };
                template <typename F, typename T>
                struct invokeN <F,T,0> { static void invoke(F f, T t) { f(); } };
                template <typename F, typename T>
                struct invokeN <F,T,1> { static void invoke(F f, T t) { f(std::get<0>(t)); } };
                template <typename F, typename T>
                struct invokeN <F,T,2> { static void invoke(F f, T t) { f(std::get<0>(t), std::get<1>(t)); } };
                template <typename F, typename T>
                struct invokeN <F,T,3> { static void invoke(F f, T t) { f(std::get<0>(t), std::get<1>(t), std::get<2>(t)); } };
                template <typename F, typename T>
                struct invokeN <F,T,4> { static void invoke(F f, T t) { f(std::get<0>(t), std::get<1>(t), std::get<2>(t), std::get<3>(t)); } };
                template <typename F, typename T>
                struct invokeN <F,T,5> { static void invoke(F f, T t) { f(std::get<0>(t), std::get<1>(t), std::get<2>(t), std::get<3>(t), std::get<4>(t)); } };
                template <typename F, typename T>
                struct invokeN <F,T,6> { static void invoke(F f, T t) { f(std::get<0>(t), std::get<1>(t), std::get<2>(t), std::get<3>(t), std::get<4>(t), std::get<5>(t)); } };

                template <typename Func, typename Tuple>
                void invokeFunc (Func f, Tuple t)
                {
                    invokeN<Func, Tuple, std::tuple_size<Tuple>::value >::invoke(f, t);
                }

                template <typename Tuple>
                void invokeList (FunctionList& list, Tuple& args)
                {
                    for (auto it = list.begin(); it != list.end(); it++ )
                        invokeFunc(it->second, args);
                }

                template <typename Tuple>
                void invoke(Tuple& t)
                {
                    invokeList(mBody, t);
                }
            };
        }
    }
    using namespace lx0::core::slot_ns;
}
