`FaustPrefix` must be provided, find ready-to-use standalone Faust installations here, extract them somewhere and provide the path: https://nightly.link/Simon-L/faust/workflows/libfaust/mfx-gh-actions?preview

This library is intended for use with Luajit through the FFI, `DspFaust.cpp` contains code at the end of the file that exposes functions in C namespace to easily call from Luajit, but plain C will work just as well!

Standalone Luajit prefixes are also provided here: https://github.com/Simon-L/hererocks/releases/tag/Nightly

### Building and testing on Linux
```
rm -rf build
cmake -Bbuild -DFaustPrefix=/tmp/faust-ubuntu-57a32ab -DLuaPrefix=/tmp/ubuntu-latest
cmake --build build --verbose --target LuajitTest
```

### Cross-compiling and testing for Windows on linux using msvc-wine
```
rm -rf build
CC=cl CXX=cl cmake -Bbuild --toolchain msvc-wine.cmake -DFaustPrefix=/tmp/faust-windows-57a32ab -DLuaPrefix=/tmp/windows-latest
cmake --build build --verbose --target LuajitTest
```
