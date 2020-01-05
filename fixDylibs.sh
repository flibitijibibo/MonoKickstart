#!/bin/sh

FILES=`ls osx`
for f in $FILES
do
    # OS X's Dynamic Linker looks for an "install path" inside of
    # a given dynamic library. It will then try to find the library
    # at that location. This usually defaults to somewhere in the
    # system folders (e.g. /Library/Frameworks/... or /usr/lib/...)
    # 
    # However, since we're packaging 3rd party libraries into the
    # app bundle, we want to tell OS X's dynamic linker to look
    # in the executable's path + osx subfolder (where we've told
    # XCode to copy our dynamic libraries.)
    install_name_tool -id @executable_path/osx/`basename $f` osx/$f
    install_name_tool -change /usr/local/lib/libSDL2-2.0.0.dylib @executable_path/osx/libSDL2-2.0.0.dylib osx/$f

    # You should see @executable_path/osx/LIBNAME here
    otool -L osx/$f
done
