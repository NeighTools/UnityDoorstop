# UnityPrePatcher

UnityPrePatcher (or **UPP** for short) is a tool to patch managed Mono assemblies in Unity games with the use of Cecil.Inject.

Currently the tool is WIP and thus does not yet have prebuilt binaries.


## Building

First, clone the repository with submodules.

To build, you'll need Visual Studio (preferably 2017) with Visual C++ compiler toolset with Windows XP support (needed for minhook to compile).
Alternatively, you can try to use minhook from NuGet or download prebuilt libraries.

Open in Visual Studio, and build for your platform.


## Using the tool

> TODO: Update with proper binaries and guide

1. Compile x86 or x64 version depending on the game you want to patch.
2. Put compiled `opengl32.dll` into the game's root folder
3. Create folder called `UnityPrePatcher` inside the game's root folder
4. Put `Mono.Cecil.dll` and `PatcherLoader.dll` into `UnityPrePatcher\bin` folder
5. Put Sybaris patchers into `UnityPrePatcher\patches`
6. Run the game
7. Read the logs (`opengl_proxy.log` in the game's root and `UnityPrePatcher\logs` folder).