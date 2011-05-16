#include <iostream>
#include <string>
#include <vector>
#include <map>

#include "main.hpp"
#include "ut_attributeparsers.hpp"

#include <lx0/engine/engine.hpp>
#include <lx0/engine/document.hpp>
#include <lx0/core/util/util.hpp>
#include <lx0/core/base/base.hpp>
#include <lx0/subsystem/javascript.hpp>
#include <lx0/core/data/lxvar_convert.hpp>
#include <glgeom/glgeom.hpp>

using glgeom::color3f;

using namespace lx0::core;
using namespace lx0;
using namespace lx0::util;


void test_attributeparsers()
{
    using namespace lx0::core;
    using namespace lx0::util;


    EnginePtr spEngine = Engine::acquire();
    DocumentPtr spDocument = spEngine->createDocument();
    spDocument->attachComponent("javascript", lx0::createIJavascript() );

    //spDocument->getComponent<IJavascript>("javascript")->run( lx_file_to_string("media2/scripts/engine/attribute_parsers/color.js") );
    {
        auto parse = [&] (std::string s) {
            return spEngine->parseAttribute("color", s);
        };

        const color3f kBlack (0/255.0f,0/255.0f,0/255.0f);
 	 	const color3f kSilver (192/255.0f,192/255.0f,192/255.0f);
 	 	const color3f kGray (128/255.0f,128/255.0f,128/255.0f);
 	 	const color3f kWhite (255/255.0f,255/255.0f,255/255.0f);
 	 	const color3f kMaroon (128/255.0f,0/255.0f,0/255.0f);
 	 	const color3f kRed (255/255.0f,0/255.0f,0/255.0f);
 	 	const color3f kPurple (128/255.0f,0/255.0f,128/255.0f);
 	 	const color3f kFuchsia (255/255.0f,0/255.0f,255/255.0f);
 	 	const color3f kGreen (0/255.0f,128/255.0f,0/255.0f);
 	 	const color3f kLime (0/255.0f,255/255.0f,0/255.0f);
 	 	const color3f kOlive (128/255.0f,128/255.0f,0/255.0f);
 	 	const color3f kYellow (255/255.0f,255/255.0f,0/255.0f);
 	 	const color3f kNavy (0/255.0f,0/255.0f,128/255.0f);
 	 	const color3f kBlue (0/255.0f,0/255.0f,255/255.0f);
 	 	const color3f kTeal (0/255.0f,128/255.0f,128/255.0f);
 	 	const color3f kAqua (0/255.0f,255/255.0f,255/255.0f);

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
        CHECK( parse("bla ck").isString() == true);
        CHECK( parse("blakc").isString() == true);
        CHECK( parse("WhiteBlack").isString() == true);

        // Check hex notation
        CHECK( kWhite             == parse("#fff").convert() );
        CHECK( kBlack             == parse("#000").convert() );
        CHECK( color3f(255/255.0f, 0, 0)   == parse("#F00").convert() );

        CHECK( color3f(240/255.0f, 15/255.0f,  0) == parse("#F00F00").convert() );
        CHECK( color3f(  1/255.0f,  1/255.0f,  1/255.0f) == parse("#010101").convert() );
        CHECK( color3f(  1/255.0f,  1/255.0f,  2/255.0f) == parse("#010102").convert() );
        CHECK( color3f(  1/255.0f,  3/255.0f,  4/255.0f) == parse("#010304").convert() );
        CHECK( color3f(  5/255.0f,  6/255.0f,  7/255.0f) == parse("#050607").convert() );
        CHECK( color3f(133/255.0f,150/255.0f,167/255.0f) == parse("#8596A7").convert() );

        // Check some malformed hex
        CHECK( parse("000").isInt() );
        CHECK( parse("#00112").isString() );


        // Check rgb notation
        CHECK( color3f(255/255.0f, 255/255.0f, 255/255.0f) == parse("rgb(255, 255, 255)").convert() );
        CHECK( color3f(  0,   0,   0) == parse("rgb(0,0,0)").convert() );
        CHECK( color3f( 12/255.0f,  34/255.0f, 5/255.0f) == parse("rgb(12, 34 , 5  )").convert() );

        // Check rgb percentage notation
        CHECK( kWhite == parse("rgb(100%, 100%, 100%)").convert() );
    }

    spDocument.reset();
    spEngine->shutdown();
}



