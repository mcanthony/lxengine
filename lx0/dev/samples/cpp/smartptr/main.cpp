#include <iostream>
#include <string>
#include <memory> // std::shared_ptr, std::weak_ptr defined here
#include <functional>

class Object
{
public:
    Object(const char* name) 
        : mName (name)
    {
        std::cout << "ctor '" << mName << "'" << std::endl;
    }
    ~Object()
    {
        std::cout << "dtor '" << mName << "'" << std::endl;
    }

    std::string mName;
};


int 
main (int argc, char** argv)
{
    // weak_ptr are references to shared_ptrs that do not increment the 
    // reference count; thus allowing them to be freed.
    std::weak_ptr<Object> wp;
    std::weak_ptr<Object> wq;

    std::function<void()> g;
    std::function<void()> h;
    {
        std::function<void()> f;
        {
            // shared_ptr can be used with any new heap object
            std::shared_ptr<Object> p(new Object("shared p"));
            std::shared_ptr<Object> q(new Object("shared q"));

            // But don't assign the existing heap object directly to two different
            // shared_ptrs: only assign new heap objects or other shared_ptrs,
            // otherwise the reference count will fall out of sync.
            //
            // Object* x = new Object("shared x");
            // std::shared_ptr<Object> p(x);
            // std::shared_ptr<Object> q(x);

            // assign the weak_ptr references
            wp = p;
            wq = q;

            // lamdba functions properly copy the shared_ptr arguments, thus
            // a shared_ptr exist to p and q will exist for the scope of the
            // lambda function as well.
            f = [p]() { std::cout << "lambda: '" << p->mName << "'" << std::endl; };
            h = [q]() { std::cout << "lambda: '" << q->mName << "'" << std::endl; };
        }

        // These are both valid calls: p and q are out of scope, but the 
        // f and h implicitly each have a shared_ptr to those objects.
        f();
        h();
        
        // g creates a new shared_ptr to what was once p when copying f
        // h ends the scope of the lambda function, thereby releasing what was once q
        g = f;      // wp.expired() == false
        h = 0;      // wq.expired() == true
    }

    // This call is valid since g implicitly has a shared_ptr to the p object
    g();

    // Check that the weak_ptrs: p == false, q == true
    std::cout << "wp expired: " << (wp.expired() ? "true" : "false") << std::endl;
    std::cout << "wq expired: " << (wq.expired() ? "true" : "false") << std::endl;

    return 0;
}
