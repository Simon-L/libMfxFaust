`FaustPrefix` must be provided, find ready-to-use standalon Faust installations here, extract them somewhere and provide the path: https://nightly.link/Simon-L/faust/workflows/libfaust/mfx-gh-actions?preview

### Building and testing on Linux
`cmake -Bbuild -DFaustPrefix=/tmp/faust-ubuntu-57a32ab && cmake --build build --verbose`

### Cross-compiling and testing for Windows on linux using msvc-wine
`CC=cl CXX=cl cmake -Bbuild --toolchain msvc-wine.cmake -DFaustPrefix=/tmp/faust-windows-57a32ab && cmake --build build --verbose`
