#include <iostream>
#include <string>
#include <vector>
#include <map>

#include "main.hpp"
#include "ut_attributeparsers.hpp"

#include <lx0/v8bind.hpp>
#include <lx0/core/util/util.hpp>
#include <v8/v8.h>

using namespace v8;
using namespace lx0::core;
using v8::Object;

namespace attribute_parsers
{

    class Engine
    {
    public:
        lxvar parse(std::string attr, std::string value)
        {
            auto group = mParsers[attr];
            for (auto it = group.begin(); it != group.end(); ++it)
            {
                lxvar v = (*it)(value);
                if (v.isDefined())
                    return v;
            }
            return lxvar::undefined();
        }

        void addAttributeParser (std::string attr, std::function<lxvar (std::string)> func)
        {
            mParsers[attr].push_back(func);
        }

    protected:
        std::map<std::string, std::vector<std::function<lxvar (std::string s)>>> mParsers;
    };

    class TestContext : public lx0::core::v8bind::_V8Context
    {
    public:
        TestContext(Engine* pEngine) : mpEngine(pEngine) {  _addEngine(); }
    protected:
        

        void            _addEngine();

    public:
        Engine*          mpEngine;
    };

    namespace wrappers_engine
    {
        static v8::Handle<v8::Value> 
        addAttributeParser (const v8::Arguments& args)
        {
            auto                 pThis = lx0::core::v8bind::_nativeThis<TestContext>(args);
            std::string          attr  = lx0::core::v8bind::_marshal(args[0]); 
            Persistent<Function> func  = Persistent<Function>::New( Handle<Function>::Cast(args[1]) ); 
                
            auto wrapper = [pThis, func] (std::string value) -> lxvar { 
                Context::Scope context_scope(pThis->context);

                HandleScope handle_scope;
                Handle<v8::Object> recv = pThis->context->Global();
                Handle<Value> callArgs[1];
                
                Handle<Value> ret;
                callArgs[0] = String::New(value.c_str());
                ret = func->Call(recv, 1, callArgs);

                return lx0::core::v8bind::_marshal(ret);
            };
            pThis->mpEngine->addAttributeParser(attr, wrapper);
            return v8::Undefined();
        }

        static v8::Handle<v8::Value> 
        debug (const v8::Arguments& args)
        {
            std::string msg = *v8::String::AsciiValue(args[0]);                 
            std::cout << "JS: " << msg << std::endl;
            return v8::Undefined();
        }
    }

    void 
    TestContext::_addEngine()
    {
        namespace W = wrappers_engine;

        Context::Scope context_scope(context);
        HandleScope handle_scope;

        Handle<FunctionTemplate> templ( FunctionTemplate::New() );
        
        Handle<ObjectTemplate> objInst( templ->InstanceTemplate() );
        objInst->SetInternalFieldCount(1);

        Handle<Template> proto_t( templ->PrototypeTemplate() );
        proto_t->Set("debug",               FunctionTemplate::New(W::debug));
        proto_t->Set("addAttributeParser",  FunctionTemplate::New(W::addAttributeParser));

        Handle<Function> ctor( templ->GetFunction() );
        Handle<v8::Object> obj( ctor->NewInstance() );
        obj->SetInternalField(0, External::New(this));

        context->Global()->Set(String::New("engine"), obj);
    }
}

struct color
{
    color() : r(0), g(0), b(0) {}
    color(int r_, int g_, int b_) : r((unsigned char)r_), g((unsigned char)g_), b((unsigned char)b_) {}

    bool operator== (const color& that) const
    {
        return r == that.r && g == that.g && b == that.b;
    }
    unsigned char r, g, b;
};

namespace lx0 { namespace core { namespace detail {

    void _convert (lxvar& v, color& c)
    {
        lx_check_error(v.isArray(), "Cannot convert lxvar to color that is not an array!");
        lx_check_error(v.size() == 3);
        c = color(
                int (v.at(0).asFloat() * 255.0f),
                int (v.at(1).asFloat() * 255.0f),
                int (v.at(2).asFloat() * 255.0f)
            );
    }
}}}

void test_attributeparsers()
{
    using namespace attribute_parsers;

    attribute_parsers::Engine engine;
    TestContext jsContext(&engine);

    Context::Scope context_scope(jsContext.context);
    
    jsContext.runFile("media/scripts/engine/attribute_parsers/color.js");
    {
        auto parse = [&] (std::string s) {
            return engine.parse("color", s);
        };

        const color kBlack (0,0,0);
 	 	const color kSilver (192,192,192);
 	 	const color kGray (128,128,128);
 	 	const color kWhite (255,255,255);
 	 	const color kMaroon (128,0,0);
 	 	const color kRed (255,0,0);
 	 	const color kPurple (128,0,128);
 	 	const color kFuchsia (255,0,255);
 	 	const color kGreen (0,128,0);
 	 	const color kLime (0,255,0);
 	 	const color kOlive (128,128,0);
 	 	const color kYellow (255,255,0);
 	 	const color kNavy (0,0,128);
 	 	const color kBlue (0,0,255);
 	 	const color kTeal (0,128,128);
 	 	const color kAqua (0,255,255);

        // Ensure the basic color keywords are working
        CHECK( kBlack   == parse("black").convert() );
 	 	CHECK( kSilver  == parse("silver").convert() );
 	 	CHECK( kGray    == parse("gray").convert() );
 	 	CHECK( kWhite   == parse("white").convert() );
 	 	CHECK( kMaroon  == parse("maroon").convert() );
 	 	CHECK( kRed     == parse("red").convert() );
 	 	CHECK( kPurple  == parse("purple").convert() );
 	 	CHECK( kFuchsia == parse("fuchsia").convert() );
 	 	CHECK( kGreen   == parse("green").convert() );
 	 	CHECK( kLime    == parse("lime").convert() );
 	 	CHECK( kOlive   == parse("olive").convert() );
 	 	CHECK( kYellow  == parse("yellow").convert() );
 	 	CHECK( kNavy    == parse("navy").convert() );
 	 	CHECK( kBlue    == parse("blue").convert() );
 	 	CHECK( kTeal    == parse("teal").convert() );
 	 	CHECK( kAqua    == parse("aqua").convert() );

        // Ensure captialization has not effect
        CHECK( kBlack   == parse("BLACK").convert() );
 	 	CHECK( kSilver  == parse("SILVER").convert() );
 	 	CHECK( kGray    == parse("GRAY").convert() );
 	 	CHECK( kWhite   == parse("WHITE").convert() );
 	 	CHECK( kMaroon  == parse("MAROON").convert() );
 	 	CHECK( kRed     == parse("RED").convert() );
 	 	CHECK( kPurple  == parse("PURPLE").convert() );
 	 	CHECK( kFuchsia == parse("FUCHSIA").convert() );
 	 	CHECK( kGreen   == parse("GREEN").convert() );
 	 	CHECK( kLime    == parse("LIME").convert() );
 	 	CHECK( kOlive   == parse("OLIVE").convert() );
 	 	CHECK( kYellow  == parse("YELLOW").convert() );
 	 	CHECK( kNavy    == parse("NAVY").convert() );
 	 	CHECK( kBlue    == parse("BLUE").convert() );
 	 	CHECK( kTeal    == parse("TEAL").convert() );
 	 	CHECK( kAqua    == parse("AQUA").convert() );

        CHECK( kBlack   == parse("Black").convert() );
 	 	CHECK( kSilver  == parse("Silver").convert() );
 	 	CHECK( kGray    == parse("Gray").convert() );
 	 	CHECK( kWhite   == parse("White").convert() );
 	 	CHECK( kMaroon  == parse("Maroon").convert() );
 	 	CHECK( kRed     == parse("Red").convert() );
 	 	CHECK( kPurple  == parse("Purple").convert() );
 	 	CHECK( kFuchsia == parse("Fuchsia").convert() );
 	 	CHECK( kGreen   == parse("Green").convert() );
 	 	CHECK( kLime    == parse("Lime").convert() );
 	 	CHECK( kOlive   == parse("Olive").convert() );
 	 	CHECK( kYellow  == parse("Yellow").convert() );
 	 	CHECK( kNavy    == parse("Navy").convert() );
 	 	CHECK( kBlue    == parse("Blue").convert() );
 	 	CHECK( kTeal    == parse("Teal").convert() );
 	 	CHECK( kAqua    == parse("Aqua").convert() );

        // Ensure some invalid cases do in fact fail
        CHECK( parse("bla ck").isDefined() == false);
        CHECK( parse("blakc").isDefined() == false);
        CHECK( parse("WhiteBlack").isDefined() == false);

        // Check hex notation
        CHECK( kWhite             == parse("#fff").convert() );
        CHECK( kBlack             == parse("#000").convert() );
        CHECK( color(255, 0, 0)   == parse("#F00").convert() );

        CHECK( color(240, 15,  0) == parse("#F00F00").convert() );
        CHECK( color(  1,  1,  1) == parse("#010101").convert() );
        CHECK( color(  1,  1,  2) == parse("#010102").convert() );
        CHECK( color(  1,  3,  4) == parse("#010304").convert() );
        CHECK( color(  5,  6,  7) == parse("#050607").convert() );
        CHECK( color(133,150,167) == parse("#8596A7").convert() );

        // Check some malformed hex
        CHECK( parse("000").isUndefined() );
        CHECK( parse("#00112").isUndefined() );


        // Check rgb notation
        CHECK( color(255, 255, 255) == parse("rgb(255, 255, 255)").convert() );
        CHECK( color(  0,   0,   0) == parse("rgb(0,0,0)").convert() );
        CHECK( color( 12,  34, 5) == parse("rgb(12, 34 , 5  )").convert() );

        // Check rgb percentage notation
        CHECK( kWhite == parse("rgb(100%, 100%, 100%)").convert() );
    }
}



