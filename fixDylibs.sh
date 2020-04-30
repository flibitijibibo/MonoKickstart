#!/bin/sh

UNAME=`uname`
if [ "$UNAME" == "Darwin" ]; then
    LIPO=lipo
    OTOOL=otool
    INSTALL_NAME_TOOL=install_name_tool
else
    LIPO=x86_64-apple-darwin18-lipo
    OTOOL=x86_64-apple-darwin18-otool
    INSTALL_NAME_TOOL=x86_64-apple-darwin18-install_name_tool
fi

FILES=`ls osx`
for f in $FILES
do
    # Rip out i386, you should never hit 32-bit anymore
    if $LIPO -archs osx/$f | grep i386; then
        cp osx/$f osx/$f.temp
        $LIPO osx/$f.temp -remove i386 -output osx/$f
        rm osx/$f.temp
        echo $f 32-bit code stripped
    else
        echo $f has no 32-bit code
    fi

    # OS X's Dynamic Linker looks for an "install path" inside of
    # a given dynamic library. It will then try to find the library
    # at that location. This usually defaults to somewhere in the
    # system folders (e.g. /Library/Frameworks/... or /usr/lib/...)
    #
    # Instead, we want to set @rpath in the executable, then fix the paths in
    # the libraries to use @rpath for their link paths.
    $INSTALL_NAME_TOOL -id @rpath/`basename $f` osx/$f
    $INSTALL_NAME_TOOL -change /usr/local/lib/libSDL2-2.0.0.dylib @rpath/libSDL2-2.0.0.dylib osx/$f
    $INSTALL_NAME_TOOL -change /usr/local/lib/libogg.0.dylib @rpath/libogg.0.dylib osx/$f
    $INSTALL_NAME_TOOL -change /usr/local/lib/libvorbis.0.dylib @rpath/libvorbis.0.dylib osx/$f
    $INSTALL_NAME_TOOL -change @loader_path/libsteam_api.dylib @rpath/libsteam_api.dylib osx/$f

    # You should see @rpath/LIBNAME here
    $OTOOL -L osx/$f
done
