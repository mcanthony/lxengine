//===========================================================================//
/*
                                   LxEngine

    LICENSE

    Copyright (c) 2011 athile@athile.net (http://www.athile.net)

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

//===========================================================================//
//   H E A D E R S   &   D E C L A R A T I O N S 
//===========================================================================//

#include <iostream>
#include <lx0/lxengine.hpp>
#include <lx0/subsystem/javascript.hpp>
#include "renderer.hpp"

using namespace lx0;

//===========================================================================//
//   E N T R Y - P O I N T
//===========================================================================//

#include <glgeom/prototype/image.hpp>
#include <lx0/prototype/misc.hpp>
#include <glgeom/ext/patterns.hpp>
#include <glgeom/ext/mappers.hpp>
#include <boost/random/mersenne_twister.hpp>
#include <boost/random/uniform_int.hpp>
#include <boost/random/uniform_real.hpp>
#include <boost/random/uniform_on_sphere.hpp>
#include <boost/random/variate_generator.hpp>

static 
void
generateNoiseCubeMap ()
{
    boost::mt19937 generator;         
    boost::uniform_int<> range(0,1023);
    boost::variate_generator<boost::mt19937&, boost::uniform_int<> > die(generator, range);

    glm::vec3 offset(223.33f, 3125.32138f, 2734.234221f);

    std::vector<glgeom::image3f> imageSet;
    glgeom::generate_cube_map(imageSet, 256, 256, [&] (const glm::vec3& v) -> glm::vec3 {

        glm::vec3 water;
        if (1) {
            glm::vec3 sum;
            for (int i = 1; i <= 8; ++i)
            {
                glm::vec2 uv;
                uv = glgeom::mapper_spherical(v, glm::vec2(4, 4));
                uv.x += i * 4.0f * lx0::noise3d_perlin(v * 1.2f + glm::vec3(234, i * 122, 621));
                uv.y += i * 4.0f * lx0::noise3d_perlin(v * 1.2f + glm::vec3(i * 532, i * 7732, 23462));

                glgeom::color3f c;
                sum += glgeom::pattern_spot_dimmed(
                        glm::vec3(13/255.0f, 128/255.0f, 255/255.0f),
                        glm::vec3(237/255.0f, 241/255.0f, 244/255.0f),
                        .45f,
                        uv
                    );
            }

            water = sum / 8.0f;
        }

        glm::vec3 grass;
        {
            glm::vec3 sum;
            for (int i = 1; i <= 3; ++i)
            {
                glm::vec2 uv;
                uv = glgeom::mapper_spherical(v, glm::vec2(4, 4));
                uv.x += i * 4.0f * lx0::noise3d_perlin(v * 5.2f + glm::vec3(234, i * 122, 621));
                uv.y += i * 4.0f * lx0::noise3d_perlin(v * 4.2f + glm::vec3(i * 532, i * 7732, 23462));

                glgeom::color3f c;
                sum += glgeom::pattern_checker(
                        glm::vec3(174/255.0f, 165/255.0f, 117/255.0f),
                        glm::vec3(90/255.0f, 47/255.0f, 12/255.0f),
                        uv
                    );
            }

            grass = sum / 8.0f;
        }

        float b =  lx0::noise3d_perlin(v * 3.2f + glm::vec3(234, 3112, 621));
        b += lx0::noise3d_perlin(v * 0.62f + glm::vec3(234, 3112, 621));
        b /= 2.0f;
        b = pow(.45f + b, 3);
        b = glm::clamp(b, 0.0f, 1.0f);
        if (b < .25f) b = powf(b, 1.25f);
        b = .55f + b * .45f;

        glm::vec3 sum;
        sum = glm::mix(water, grass, b);

        return sum;
    });

    const char* filename[] = 
    {
        "xpos.png",
        "xneg.png",
        "ypos.png",
        "yneg.png",
        "zpos.png",
        "zneg.png",
    };

    for (int i = 0; i < 6; ++i)
    {
        std::string file = "media2/textures/cubemaps/noise000/";
        file += filename[i];
        lx0::save_png(imageSet[i], file.c_str());
    }
}


static
glm::vec3
uniform_on_sphere ()
{
    static boost::mt19937 outer_rng;
    static boost::uniform_on_sphere<float> s(3);
    static boost::variate_generator<boost::mt19937, boost::uniform_on_sphere<float>> die(outer_rng, s);
    
    auto p = die();
    return glm::vec3( p[0], p[1], p[2] );
}

static
float 
uniform_unit()
{
    static boost::mt19937 gen;
    static boost::uniform_real<float> r( 0.0f, 1.0f );
    static boost::variate_generator<boost::mt19937, boost::uniform_real<float> > die(gen, r);
    return die();
}

static
float 
uniform_unit_exclusive()
{
    static boost::mt19937 gen;
    static boost::uniform_real<float> r( 0.0f, 1.0f );
    static boost::variate_generator<boost::mt19937, boost::uniform_real<float> > die(gen, r);
    
    float d;
    do 
    {
        d = die();
    } while (!(d < 1.0f));
    return d;
}

static 
void
set (glgeom::cubemap3f& cubemap, const glm::vec3& p, const glgeom::color3f& c)
{
    //
    // Map the 3-space point to a cube map tile + texel location
    //

    // Set by coverage where the sample point is considered the
    // center of a unit square on the texture

    int tile;
    glm::vec3 ap( glm::abs(p) );
    if (ap.x > ap.z)
    {
        if (ap.x > ap.y)
            tile = (p.x > 0) ? 0 : 1;
        else 
            tile = (p.y > 0) ? 2 : 3;
    }
    else
    {
        if (ap.y > ap.z)
            tile = (p.y > 0) ? 2 : 3;
        else 
            tile = (p.z > 0) ? 4 : 5;
    }

    glm::vec2 uv;
    auto set = [](float sc, float tc, float ma) -> glm::vec2
    {
        return glm::vec2(
            (sc / ma + 1.0f) / 2.0f,
            (tc / ma + 1.0f) / 2.0f
        );
    };

    switch (tile)
    {
    default:    lx_assert(0);
    case 0:     uv = set(-p.z, -p.y, ap.x);     break;
    case 1:     uv = set( p.z, -p.y, ap.x);     break;
    case 2:     uv = set( p.x,  p.z, ap.y);     break;
    case 3:     uv = set( p.x, -p.z, ap.y);     break;
    case 4:     uv = set( p.x, -p.y, ap.z);     break;
    case 5:     uv = set(-p.x, -p.y, ap.z);     break;
    };
    uv.x *= cubemap.width();
    uv.y *= cubemap.height();

    glm::ivec2 xy( glm::floor(uv) );

    glm::ivec2 g( glm::floor(uv - glm::vec2(.5f, .5f)) );
    glm::vec2  f( glm::vec2(1.0f, 1.0f) - glm::fract(uv - glm::vec2(.5f, .5f)) );

    if (g.x >= 0 && g.y >= 0 && g.x < cubemap.width() && g.y < cubemap.height()) 
        cubemap.mImage[tile].set(g.x, g.y, c * (f.x * f.y));
    if (g.x + 1 >= 0 && g.y >= 0 && g.x + 1 < cubemap.width() && g.y < cubemap.height()) 
        cubemap.mImage[tile].set(g.x + 1, g.y, c * ((1.0f - f.x) * f.y));
    if (g.x >= 0 && g.y + 1 >= 0 && g.x < cubemap.width() && g.y + 1 < cubemap.height()) 
        cubemap.mImage[tile].set(g.x, g.y + 1, c * (f.x * (1.0f - f.y)));
    if (g.x + 1 >= 0 && g.y + 1 >= 0 && g.x + 1 < cubemap.width() && g.y + 1 < cubemap.height()) 
        cubemap.mImage[tile].set(g.x + 1, g.y + 1, c * ((1.0f - f.x) * (1.0f - f.y)));
    
    // .6 - .5 = .1
    //cubemap.mImage[tile].set(xy.x, xy.y, c);
}


static
void
generateSkyMap()
{
    glgeom::cubemap3f cubemap(512, 512);
    glgeom::clear(cubemap, glgeom::color3f(0.02f, 0.05f, .10f) );


    // Color gradient choice, radius of circle, (diamond stretch)
    // Clustering
    // Gradient maps - named? Loaded from disk?
    // Draw circle to image map

    auto rollf = [&](glm::fvec2 range) -> float
    {
        boost::mt19937 gen;
        boost::uniform_real<float> r( range[0], range[1] );
        boost::variate_generator<boost::mt19937, boost::uniform_real<float> > die(gen, r);
        return die();
    };

    auto rolli = [&](glm::ivec2 range) -> int
    {
        boost::mt19937 gen;
        boost::uniform_int<> r( range[0], range[1] );
        boost::variate_generator<boost::mt19937, boost::uniform_int<> > die(gen, r);
        return die();
    };

    /*
        clusters, count_range, gradient, size_range
     */
    struct Batch
    {
        glm::ivec2      clusterCount;
        glm::vec2       clusterRadius;
        glm::ivec2      starCount;
        glm::vec2       starRadius;
        std::vector<std::pair<glgeom::color3f,glgeom::color3f>> color;
    };

    Batch b;
    b.clusterCount  = glm::ivec2(20, 40);
    b.clusterRadius = glm::vec2(glgeom::pi().value / 100.0f, glgeom::pi().value / 3.0f);
    b.starCount     = glm::ivec2(14, 320);
    b.starRadius    = glm::vec2(1, 1);
    b.color.push_back( std::make_pair( glgeom::color3f(0.9f, 0.9f, 1.0f), glgeom::color3f(0.5f, 0.4f, 0.4f) ) );
    b.color.push_back( std::make_pair( glgeom::color3f(0.6f, 0.55f, 0.40f), glgeom::color3f(0.1f, 0.1f, 0.1f) ) );
    b.color.push_back( std::make_pair( glgeom::color3f(1.0f, 1.0f, 0.90f), glgeom::color3f(0.4f, 0.4f, 0.4f) ) );


    const int clusters = rolli(b.clusterCount);
    for (int i = 0; i < clusters; ++i)
    {
        const float clusterRadius = rollf(b.clusterRadius);
        const int   stars = rolli(b.starCount);
        for (int j = 0; j < stars; ++j)
        {
            const int   gradient = int(uniform_unit_exclusive() * b.color.size());
            const float blend = uniform_unit();
            const glgeom::color3f color = glm::mix(b.color[gradient].first.vec, b.color[gradient].second.vec, blend);
            
            const float radius = rollf(b.starRadius);

            // Eventually, we want this to generate a center point of the cluster
            // and create variations from there.  For now, it's much simpler...
            glm::vec3 pt = uniform_on_sphere();

            set(cubemap, pt, color);
        }
    }

    const char* filename[] = 
    {
        "xpos.png",
        "xneg.png",
        "ypos.png",
        "yneg.png",
        "zpos.png",
        "zneg.png",
    };

    for (int i = 0; i < 6; ++i)
    {
        std::string file = "temp/";
        file += filename[i];
        lx0::save_png(cubemap.mImage[i], file.c_str());
    }
}

int 
main (int argc, char** argv)
{
    int exitCode = -1;
    try
    {
        EnginePtr spEngine = Engine::acquire();
        
        spEngine->globals().add("file",      lx0::eAcceptsString,  lx0::validate_filename());
        spEngine->globals().add("output",    lx0::eAcceptsString,  lx0::validate_string());
        spEngine->globals().add("width",     lx0::eAcceptsInt,     lx0::validate_int_range(32, 4096), 512);
        spEngine->globals().add("height",    lx0::eAcceptsInt,     lx0::validate_int_range(32, 4096), 512);

        if (spEngine->parseCommandLine(argc, argv, "file"))
        {
            // Temp
            {
                generateSkyMap();
                //generateNoiseCubeMap();
            }
            //


            spEngine->attachComponent(lx0::createJavascriptSubsystem());
            spEngine->attachComponent(createScriptHandler());
        
            DocumentPtr spDocument = spEngine->loadDocument(spEngine->globals().find("file"));
            spDocument->addController( create_controller(spDocument) );

            ViewPtr spView = spDocument->createView("Canvas", "view", create_renderer() );
            spView->addUIBinding( create_uibinding() );

            lxvar options;
            options.insert("title",  "LxEngine Rasterizer Sample");
            options.insert("width",  spEngine->globals().find("width"));
            options.insert("height", spEngine->globals().find("height"));
            spView->show(options);

            lxvar outputVar = spEngine->globals().find("output");
            if (outputVar.is_defined())
            {
                spView->sendEvent("screenshot", outputVar);
                spEngine->sendEvent("quit");
            }

            exitCode = spEngine->run();
        }
        spEngine->shutdown();
    }
    catch (lx0::error_exception& e)
    {
        std::cerr << "Error: " << e.details().c_str() << std::endl
                    << "Code: " << e.type() << std::endl
                    << std::endl;
    }
    catch (std::exception& e)
    {
        std::cerr << "Fatal: unhandled std::exception.\n"
            << "Exception: " << e.what();
    }

    return exitCode;
}
