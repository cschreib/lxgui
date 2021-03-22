# Windows dependencies

See `get-windows-dependencies.sh`. This is not a fully automated script, and more of an outline. All the dependencies which must be compiled should be done manually (cmake ..; then open Visual Studio solution, select Release configuration, and build).


# WebAssembly

Clone the lua repository, checkout version 5.3.x, copy the `LuaWASMCMakeLists.txt` in the lua repository, rename it to `CMakeLists.txt`. Then

```
mkdir build
cd build
emcmake cmake ..
emmake make
```
