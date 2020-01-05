#!/bin/sh

FILES=`ls osx`
for f in $FILES
do
    # OS X's Dynamic Linker looks for an "install path" inside of
    # a given dynamic library. It will then try to find the library
    # at that location. This usually defaults to somewhere in the
    # system folders (e.g. /Library/Frameworks/... or /usr/lib/...)
    #
    # Instead, we want to set @rpath in the executable, then fix the paths in
    # the libraries to use @rpath for their link paths.
    install_name_tool -id @rpath/`basename $f` osx/$f
    install_name_tool -change /usr/local/lib/libSDL2-2.0.0.dylib @rpath/libSDL2-2.0.0.dylib osx/$f

    # You should see @rpath/LIBNAME here
    otool -L osx/$f
done
