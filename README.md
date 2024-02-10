# Foster.Audio
A small C# game audio library.

_★ very work in progress! likely to have frequent, breaking changes! please use at your own risk! ★_

### Dependencies
 - [dotnet 8.0](https://dotnet.microsoft.com/en-us/download/dotnet/8.0) and [C# 12](https://learn.microsoft.com/en-us/dotnet/csharp/whats-new/csharp-12)

### Platform Library
 - The [Platform library](https://github.com/MrBrixican/Foster.Audio/tree/main/Platform) is a C library that implements native methods required to play audio.
 - By default, it is currently being built for 64-bit Linux, MacOS, and Windows through [Github Actions](https://github.com/MrBrixican/Foster.Audio/blob/main/.github/workflows/build-libs.yml).
 - To add support for more platforms, you need to build the [Platform library](https://github.com/MrBrixican/Foster.Audio/tree/main/Platform) and then include it in [Foster.Audio.csproj](https://github.com/MrBrixican/Foster.Audio/blob/main/Foster.Audio/Foster.Audio.csproj)

### Audio
 - Implemented via [miniaudio](https://github.com/mackron/miniaudio) for Linux/Mac/Windows.
 - Supported features
   - Loading WAV, MP3, QOA, OGG, and raw PCM data
   - Essential operations/settings: Play, Pause, Stop, Seek, Volume, Pitch, Pan, Looping, Spatialization
   - Sound groups to manage multiple sound instances (useful for sound category volume management)
   - Garbage free managed sound instances

### Notes
 - Contributions are welcome! However, anything that adds external dependencies or complicates the build process will not be accepted.
 - This library is made to work alongside the [Foster](https://github.com/FosterFramework/Foster) game framework, but is perfectly capable of working standalone as long as lifecycle methods (`Audio.Startup`, `Audio.Shutdown`, `Audio.Update`) are called.
 - This is intended to be a relatively simple library to load and play sounds. If you need more functionality, it may be best to look into more complete solutions like FMOD or Wwise.
