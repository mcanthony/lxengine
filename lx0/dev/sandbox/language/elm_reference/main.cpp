#include <iostream>
#include <vector>
#include <string>
#include <map>

class Object
{
public:
    void incRef() { ++mRefCount; }
    void decRef() { if (--mRefCount == 0) delete this; }
    
    virtual Object* clone() = 0;

    // Invokes the object as a function; returns a new object
    // as its return value, which can be null if there is no
    // return value    
    virtual Object* invoke() { return NULL; }
    
protected:
    virtual ~Object() {}
   
private:
    int mRefCount;
};

class Object2 : public Object
{
public:
    virtual Object*  clone();    
    virtual void     addProperty (bool bShared, Object* pValue);
    
protected:
    struct Property
    {
        bool        bShared;
        Object*     pValue;
    };
    typedef std::vector<Property> PropertyList;       
    PropertyList mProperties;
};

Object*
Object2::clone()
{
    Object2* pClone = new Object2;
    for (PropertyList::iterator it = mProperties.begin(); it != mProperties.end(); ++it)
    {
        bool bShared = it->bShared;   
        Object* pValue = (bShared) ? it->pValue : it->pValue->clone();
        pClone->addProperty(bShared, pValue);
    }
    return pClone;
}

void
Object2::addProperty (bool bShared, Object* pValue)
{
    pValue->incRef();
    Property p = { bShared, pValue };
    mProperties.push_back(p);
}

class ObjectPtr
{
public:
    ObjectPtr() : mPtr(NULL) {}
    ~ObjectPtr() { reset(); }
    
    ObjectPtr(Object* p) : mPtr(p) { if(p) p->incRef(); }
    ObjectPtr(const ObjectPtr& that) : mPtr(that.mPtr) { if (mPtr) mPtr->incRef(); }
    
    void reset() { if (mPtr) mPtr->decRef(); mPtr = NULL; }
protected:
    Object* mPtr;
};

namespace BuiltIns 
{
    class Array 
        : public Object
    {
    public:
        virtual Object* clone();
        
        
        // built-ins
        void push (Object* p) { p->incRef(); mData.push_back(p); }
        ObjectPtr pop() 
        {        
            ObjectPtr p( mData.back() ); 
            return p; 
        }
    protected:
        typedef std::vector<Object*> List;
        List mData;
    };
    
    Object* 
    Array::clone()
    {
        Array* pClone = new Array;
        for (List::iterator it = mData.begin(); it != mData.end(); ++it)
        {
            // Currently, always clone the contents of an array
            // This is temporary, as it does not make sense in all
            // cases
            Object* pItem = (*it)->clone();
            pItem->incRef();
            pClone->mData.push_back(pItem);
        }
        return pClone;
    }
}

int 
main (int argc, char** argv)
{
    Object2* pGlobal = new Object2;
    pGlobal->incRef();
    {
        {
            // Array class
            pGlobal->addProperty(true, new BuiltIns::Array);
        }
        {
            // String class
            Object2* pS = new Object2;
            pGlobal->addProperty(true, pS);
        }
    }
    pGlobal->decRef();
	return 0;
}