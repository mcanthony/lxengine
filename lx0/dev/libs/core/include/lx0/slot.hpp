#pragma once

#include <functional>
#include <deque>
#include <tuple>

namespace lx0 { namespace core {

    /*
        TODO:
        - Use PIMPL pattern to reduce overhead of common cases (empty slot, 1 call slot, etc.)
        - When variadic templates are supported by VS2010, simplify invokation
     */
    template <typename Signature>
    class slot
    {
    public:
        typedef std::function<Signature> Function;
        typedef std::deque<Function> FunctionList;

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

        void operator=  (Function f) { mBody.resize(1); mBody[0] = f; }
        void operator+= (Function f) { mBody.push_back(f); }

    protected:

        FunctionList         mPrologue;
        FunctionList         mBody;
        FunctionList         mEpilogue;

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
            for (auto f = list.begin(); f != list.end(); f++ )
                invokeFunc(*f, args);
        }

        template <typename Tuple>
        void invoke(Tuple& t)
        {
            invokeList(mPrologue, t);
            invokeList(mBody, t);
            invokeList(mEpilogue, t);
        }
    };
}}
