The Platform library is a small C99 library that implements the native methods required to play audio.

The repo already contains prebuilt binaries for 64-bit Windows/Linux/macOS, but you can also build it yourself using CMake:
```sh
mkdir build
cd build
cmake ../
make
```
If built successfully, the library should appear in `libs/{yourPlatform}`, which is then used & copied from `Foster.Audio/Foster.Audio.csproj`.
