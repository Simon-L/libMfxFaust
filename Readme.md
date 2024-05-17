### Building and testing on Linux
`cmake -Bbuild -DFaustPrefix=/tmp/faust-ubuntu-57a32ab && cmake --build build --verbose`

### Cross-compiling and testing for Windows on linux using msvc-wine
`CC=cl CXX=cl cmake -Bbuild --toolchain msvc-wine.cmake -DFaustPrefix=/home/xox/faust && cmake --build build --verbose`
