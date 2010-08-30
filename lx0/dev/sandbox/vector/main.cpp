#include <iostream>
#include <functional>
#include <string>

namespace lx { namespace core {

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
            };    
        };
    }
    

    class tuple3 
        : public detail::base_tuple3
    {
    public: 
    };
    
    inline tuple3
    add (const tuple3& a, const tuple3& b)
    {
        tuple3 r;
        r.x = a.x + b.x;
        r.y = a.y + b.y;
        r.z = a.z + b.z;
        return r;
    }
    
    inline tuple3
    sub (const tuple3& a, const tuple3& b)
    {
        tuple3 r;
        r.x = a.x - b.x;
        r.y = a.y - b.y;
        r.z = a.z - b.z;
        return r;
    }
    
    class vector3 
        : public detail::base_tuple3
    {
    };
    
    class point3 
        : public detail::base_tuple3
    {
    public:

    };
    
    namespace detail
    {
        template <typename To, typename From>
        struct cast_is_safe
        {
            enum { value = 0 };
        };
        
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

     
    inline point3
    add (const point3& p, const vector3& v)
    {
        add( cast<const tuple3&>(p), cast<const tuple3&>(v) );
        tuple3 r;
        return cast<const point3&>(r);
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