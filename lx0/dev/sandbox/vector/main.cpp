#include <iostream>
#include <functional>
#include <string>

#include <Ogre/OgreVector3.h>

namespace lx { namespace core {

    namespace detail
    {
        template <typename To, typename From>
        struct cast_is_safe
        {
            enum { value = 0 };
        };
         
        template <typename To, typename From, bool Valid>
        struct cast_imp
        {
        };
        
        template <typename To, typename From>
        struct cast_imp<To,From,false>
        {
            static To cast (From& f)
            {
                static_assert(false, "Not a valid lx::core::cast()");
                throw std::exception();
            }
        };
        
        template <typename To, typename From>
        struct cast_imp<To,From,true>
        {
            static To cast (From& f)
            {
                static_assert(sizeof(To) == sizeof(From), "lx::core::cast between different sized types is not allowed");
                return reinterpret_cast<To>(f);
            }
        };
    }
    
    template <typename To,typename From>
    To cast (From& f)
    {
        return detail::cast_imp<To,From, 
            detail::cast_is_safe<To,From>::value >::cast(f);
    } 


    namespace detail
    {
        /*!
            Base class used to consolidate common code in
            3-tuple classes.
         */
        class base_tuple3
        {
        public:
            base_tuple3() : x(0.0f), y(0.0f), z(0.0f) {}
            base_tuple3(float x0, float y0, float z0) : x(x0), y(y0), z(z0) {}
        
            union
            {
                struct 
                {
                    float x, y, z;
                };
                
                float elem[3];
                
                // The anonymous struct allows a class with a non-trivial ctor
                // to be used inside the union.  A compiler with full C++x0 
                // support should not require this.
                struct 
                { 
                    Ogre::Vector3 ogreVec; 
                };
            };    
        };
    }
    

    class tuple3 
        : public detail::base_tuple3
    {
    public: 
    };

    namespace detail
    {
        template <> struct cast_is_safe<tuple3&,        Ogre::Vector3>         { enum { value = 1 }; };
        template <> struct cast_is_safe<const tuple3&,  Ogre::Vector3>         { enum { value = 1 }; };
        template <> struct cast_is_safe<const tuple3&,  const Ogre::Vector3>   { enum { value = 1 }; }; 
    }
    
    inline tuple3   add             (const tuple3& a, const tuple3& b) { return cast<tuple3&>(a.ogreVec + b.ogreVec); }
    inline tuple3   sub             (const tuple3& a, const tuple3& b) { return cast<tuple3&>(a.ogreVec - b.ogreVec); }
    
    class vector3 
        : public detail::base_tuple3
    {
    };

    namespace detail
    {
        template <> struct cast_is_safe<vector3&,       Ogre::Vector3>         { enum { value = 1 }; };
        template <> struct cast_is_safe<const vector3&, Ogre::Vector3>         { enum { value = 1 }; };
        template <> struct cast_is_safe<const vector3&, const Ogre::Vector3>   { enum { value = 1 }; }; 
    }

    inline float    dot             (const vector3& a, const vector3& b) { return a.ogreVec.dotProduct(b.ogreVec); }
    inline float    abs_dot         (const vector3& a, const vector3& b) { return abs(dot(a, b)); }
    inline vector3  cross           (const vector3& a, const vector3& b) { return cast<vector3&>(a.ogreVec.crossProduct(b.ogreVec)); }
    inline vector3  normalize       (const vector3& a)                   { vector3 t = a; t.ogreVec.normalise(); return t; }
    inline float    length          (const vector3& a)                   { return a.ogreVec.length(); }

    inline float    angle_between   (const vector3& a, const vector3& b) 
    {
        // Work around a bug in OGRE 1.7.1: angleBetween should be const
        // See http://ogre3d.org/forums/viewtopic.php?f=3&t=56750&p=402888#p402888
        return const_cast<Ogre::Vector3&>(a.ogreVec).angleBetween(b.ogreVec).valueRadians() ; 
    }
    
    //=======================================================================//
    //! A single-precision, 3-space point class
    /*!
     */
    class point3 
        : public detail::base_tuple3
    {
    public:

    };

    namespace detail
    {
        template <> struct cast_is_safe<point3&,       Ogre::Vector3>         { enum { value = 1 }; };
        template <> struct cast_is_safe<const point3&, Ogre::Vector3>         { enum { value = 1 }; };
        template <> struct cast_is_safe<const point3&, const Ogre::Vector3>   { enum { value = 1 }; }; 
    }

    inline point3   add             (const point3& p, const vector3& v) { return cast<point3&>(p.ogreVec + v.ogreVec); }
    inline point3   sub             (const point3& p, const vector3& v) { return cast<point3&>(p.ogreVec - v.ogreVec); }

    inline float    distance        (const point3& a, const point3& b)  { return a.ogreVec.distance(b.ogreVec); }
    inline point3   mid_point       (const point3& a, const point3& b)  { return cast<point3&>(a.ogreVec.midPoint(b.ogreVec)); }
    
    namespace detail
    {
        template <> struct cast_is_safe<tuple3&,        point3>         { enum { value = 1 }; };
        template <> struct cast_is_safe<const tuple3&,  point3>         { enum { value = 1 }; };
        template <> struct cast_is_safe<const tuple3&,  const point3>   { enum { value = 1 }; }; 
        template <> struct cast_is_safe<tuple3&,        vector3>        { enum { value = 1 }; };
        template <> struct cast_is_safe<const tuple3&,  vector3>        { enum { value = 1 }; };
        template <> struct cast_is_safe<const tuple3&,  const vector3>  { enum { value = 1 }; };
        
        template <> struct cast_is_safe<point3&,        tuple3>         { enum { value = 1 }; };
        template <> struct cast_is_safe<const point3&,  tuple3>         { enum { value = 1 }; };
        template <> struct cast_is_safe<const point3&,  const tuple3>   { enum { value = 1 }; };
        template <> struct cast_is_safe<point3&,        vector3>        { enum { value = 1 }; };
        template <> struct cast_is_safe<const point3&,  vector3>        { enum { value = 1 }; };
        template <> struct cast_is_safe<const point3&,  const vector3>  { enum { value = 1 }; };
        
        template <> struct cast_is_safe<vector3&,       tuple3>         { enum { value = 1 }; };
        template <> struct cast_is_safe<const vector3&, tuple3>         { enum { value = 1 }; };
        template <> struct cast_is_safe<const vector3&, const tuple3>   { enum { value = 1 }; };
        template <> struct cast_is_safe<vector3&,       point3>         { enum { value = 1 }; };
        template <> struct cast_is_safe<const vector3&, point3>         { enum { value = 1 }; };
        template <> struct cast_is_safe<const vector3&, const point3>   { enum { value = 1 }; };      
    }

}};

class UnitTest
{
public:
    void add_group (std::string s, std::function<void()> f)
    {
        std::cout << "Running group '" << s << "'..." << std::endl;
        f();
        std::cout << "Done." << std::endl;
    }
    
    void check (bool b)
    {
        std::cout << "\t";
        if (b)
            std::cout << "PASS";
        else
            std::cout << "FAIL";
        std::cout << std::endl;
    }
};



using namespace lx::core;


int 
main (int argc, char** argv)
{
    

    std::cout << "lx::vector sandbox" << std::endl;
    
    UnitTest test;
    
    test.add_group("type conversions", [test]() 
    {
        tuple3 a;
        point3 b;
        vector3 c;
        
        //
        // Implicit conversions are illegal
        //
        //a = b;
        //a = c;
        //b = a;
        //b = c;
        //c = a;
        //c = b;
        
        //
        // Explicit casts are legal
        //
        a = cast<tuple3&>(b);
        a = cast<tuple3&>(c);
        b = cast<point3&>(a);
        b = cast<point3&>(c);
        c = cast<vector3&>(a);
        c = cast<vector3&>(b);
    });
    
    test.add_group("constructors", [&test]() {
        tuple3 a;
        test.check(a.x == 0.0f);
        test.check(a.y == 0.0f);
        test.check(a.z == 0.0f);
        
        point3 b;
        test.check(b.x == 0.0f);
        test.check(b.y == 0.0f);
        test.check(b.z == 0.0f);
        
        vector3 c;
        test.check(c.x == 0.0f);
        test.check(c.y == 0.0f);
        test.check(c.z == 0.0f);
    });
    
    
	return 0;
}