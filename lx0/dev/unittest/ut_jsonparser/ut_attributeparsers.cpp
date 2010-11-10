#include <iostream>
#include <string>
#include <vector>
#include <map>

#include "main.hpp"
#include "ut_attributeparsers.hpp"

#include <lx0/v8bind.hpp>
#include <lx0/util.hpp>
#include <v8/v8.h>

using namespace v8;
using namespace lx0::core;
using v8::Object;

namespace 
{
    std::map<std::string, std::vector<Persistent<Function>>> mParsers;

    void addEngine(Handle<Context>& context)
    {
        struct L
        {  
            static v8::Handle<v8::Value> 
            addAttributeParser (const v8::Arguments& args)
            {
                std::string attr = *v8::String::AsciiValue(args[0]); 
                Handle<Function> func = Handle<Function>::Cast(args[1]); 
                
                mParsers[attr].push_back( Persistent<Function>::New(func) );
                return v8::Undefined();
            }

            static v8::Handle<v8::Value> 
            debug (const v8::Arguments& args)
            {
                std::string msg = *v8::String::AsciiValue(args[0]);                 
                std::cout << "JS: " << msg << std::endl;
                return v8::Undefined();
            }
        };

        Handle<FunctionTemplate> templ( FunctionTemplate::New() );
        
        Handle<ObjectTemplate> objInst( templ->InstanceTemplate() );

        Handle<Template> proto_t( templ->PrototypeTemplate() );
        proto_t->Set("debug",               FunctionTemplate::New(L::debug));
        proto_t->Set("addAttributeParser",  FunctionTemplate::New(L::addAttributeParser));

        Handle<Function> ctor( templ->GetFunction() );
        Handle<v8::Object> obj( ctor->NewInstance() );

        context->Global()->Set(String::New("engine"), obj);
    }


    lxvar
    callParser (Persistent<Context>& context, std::string attr, std::string value)
    {
        HandleScope handle_scope;
        Handle<v8::Object> recv = context->Global();
        Handle<Value> callArgs[1];
                
        Handle<Value> ret;
        auto group = mParsers[attr];
        for (auto it = group.begin(); it != group.end(); ++it)
        {
            callArgs[0] = String::New(value.c_str());
            ret = (*it)->Call(recv, 1, callArgs);

            if (!ret->IsUndefined())
                break;
        }
        return lx0::core::v8bind::_marshal(ret);
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
    HandleScope handle_scope;
    Handle<ObjectTemplate> global_templ = ObjectTemplate::New(); 
    {
        Persistent<Context> context = Context::New(0, global_templ);
        Context::Scope context_scope(context);

        addEngine(context);
        {
            ///@todo Loop over all files in attribute_parsers directory and load them
            std::string text = lx0::util::lx_file_to_string("media/scripts/engine/attribute_parsers/color.js");
            Handle<String> source = String::New(text.c_str());
            Handle<Script> script = Script::Compile(source);
 
            Handle<Value> result = script->Run();

            {
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
                CHECK( kBlack == callParser(context, "color", "black").convert() );
 	 	        CHECK( kSilver == callParser(context, "color", "silver").convert() );
 	 	        CHECK( kGray == callParser(context, "color", "gray").convert() );
 	 	        CHECK( kWhite == callParser(context, "color", "white").convert() );
 	 	        CHECK( kMaroon == callParser(context, "color", "maroon").convert() );
 	 	        CHECK( kRed == callParser(context, "color", "red").convert() );
 	 	        CHECK( kPurple == callParser(context, "color", "purple").convert() );
 	 	        CHECK( kFuchsia == callParser(context, "color", "fuchsia").convert() );
 	 	        CHECK( kGreen == callParser(context, "color", "green").convert() );
 	 	        CHECK( kLime == callParser(context, "color", "lime").convert() );
 	 	        CHECK( kOlive == callParser(context, "color", "olive").convert() );
 	 	        CHECK( kYellow == callParser(context, "color", "yellow").convert() );
 	 	        CHECK( kNavy == callParser(context, "color", "navy").convert() );
 	 	        CHECK( kBlue == callParser(context, "color", "blue").convert() );
 	 	        CHECK( kTeal == callParser(context, "color", "teal").convert() );
 	 	        CHECK( kAqua == callParser(context, "color", "aqua").convert() );

                // Ensure captialization has not effect
                CHECK( kBlack == callParser(context, "color", "BLACK").convert() );
 	 	        CHECK( kSilver == callParser(context, "color", "SILVER").convert() );
 	 	        CHECK( kGray == callParser(context, "color", "GRAY").convert() );
 	 	        CHECK( kWhite == callParser(context, "color", "WHITE").convert() );
 	 	        CHECK( kMaroon == callParser(context, "color", "MAROON").convert() );
 	 	        CHECK( kRed == callParser(context, "color", "RED").convert() );
 	 	        CHECK( kPurple == callParser(context, "color", "PURPLE").convert() );
 	 	        CHECK( kFuchsia == callParser(context, "color", "FUCHSIA").convert() );
 	 	        CHECK( kGreen == callParser(context, "color", "GREEN").convert() );
 	 	        CHECK( kLime == callParser(context, "color", "LIME").convert() );
 	 	        CHECK( kOlive == callParser(context, "color", "OLIVE").convert() );
 	 	        CHECK( kYellow == callParser(context, "color", "YELLOW").convert() );
 	 	        CHECK( kNavy == callParser(context, "color", "NAVY").convert() );
 	 	        CHECK( kBlue == callParser(context, "color", "BLUE").convert() );
 	 	        CHECK( kTeal == callParser(context, "color", "TEAL").convert() );
 	 	        CHECK( kAqua == callParser(context, "color", "AQUA").convert() );

                CHECK( kBlack == callParser(context, "color", "Black").convert() );
 	 	        CHECK( kSilver == callParser(context, "color", "Silver").convert() );
 	 	        CHECK( kGray == callParser(context, "color", "Gray").convert() );
 	 	        CHECK( kWhite == callParser(context, "color", "White").convert() );
 	 	        CHECK( kMaroon == callParser(context, "color", "Maroon").convert() );
 	 	        CHECK( kRed == callParser(context, "color", "Red").convert() );
 	 	        CHECK( kPurple == callParser(context, "color", "Purple").convert() );
 	 	        CHECK( kFuchsia == callParser(context, "color", "Fuchsia").convert() );
 	 	        CHECK( kGreen == callParser(context, "color", "Green").convert() );
 	 	        CHECK( kLime == callParser(context, "color", "Lime").convert() );
 	 	        CHECK( kOlive == callParser(context, "color", "Olive").convert() );
 	 	        CHECK( kYellow == callParser(context, "color", "Yellow").convert() );
 	 	        CHECK( kNavy == callParser(context, "color", "Navy").convert() );
 	 	        CHECK( kBlue == callParser(context, "color", "Blue").convert() );
 	 	        CHECK( kTeal == callParser(context, "color", "Teal").convert() );
 	 	        CHECK( kAqua == callParser(context, "color", "Aqua").convert() );

                // Ensure some invalid cases do in fact fail
                CHECK( callParser(context, "color", "bla ck").isDefined() == false);
                CHECK( callParser(context, "color", "blakc").isDefined() == false);
                CHECK( callParser(context, "color", "WhiteBlack").isDefined() == false);

                // Check hex notation
                auto parse = [&] (std::string s) {
                    return callParser(context, "color", s);
                };
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
            }

        }
 
        context.Dispose();
    }
}



