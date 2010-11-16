#!/bin/bash          
echo Retrieving source code

###############################################################################
# Functions
###############################################################################

function get_svn() {
    local sdkname=$1
    local directory=$2
    local url=$3
     
    echo
    echo "Updating" $sdkname...
    
    mkdir -p $directory && cd $directory
    if [ ! -f $directory/$directory ]; then
        svn co $url $directory
    fi
    
    cd $directory
    svn update
    cd ../..
}

function get_mercurial() {
    local sdkname=$1
    local directory=$2
    local url=$3
    local label=$4
     
    echo
    echo "Updating" $sdkname...
    
    mkdir -p $directory && cd $directory
    if [ ! -f $directory/$directory ]; then
        hg clone $url -u $label
    fi
    
    cd $directory
    hg pull --update
    cd ../..
}

function get_git() {
    local sdkname=$1
    local directory=$2
    local url=$3
    local label=$4
     
    echo
    echo "Updating" $sdkname...
    
    mkdir -p $directory && cd $directory
    if [ ! -f $directory/$label ]; then
        git clone $url $label
    fi
    cd ..
}

###############################################################################
# Main script
###############################################################################

get_git "OpenAL" openal git://repo.or.cz/openal-soft.git openal-soft
get_svn "Vorbis" vorbis http://svn.xiph.org/tags/vorbis/libvorbis-1.3.2/
get_svn "Ogg"    ogg    http://svn.xiph.org/tags/ogg/libogg-1.2.1/
get_svn "V8 Javascript Engine"  v8 http://v8.googlecode.com/svn/tags/1.3.18.22
get_svn "Bullet Physics SDK" bullet http://bullet.googlecode.com/svn/tags/bullet-2.77
get_mercurial "OGRE 3D (Object-oriented Graphics Rendering Engine)..." ogre http://bitbucket.org/sinbad/ogre/ v1-7
get_svn "OIS (Object-oriented Input System)" ois https://wgois.svn.sourceforge.net/svnroot/wgois/ois/branches/v1-3
get_git "FreeType2" freetype2 http://git.savannah.gnu.org/cgit/freetype/freetype2.git STABLE
get_svn "Boost"  boost  http://svn.boost.org/svn/boost/tags/release/Boost_1_44_0 
