# UnityPrePatcher

UnityPrePatcher (or **UPP** for short) is a tool to patch managed Mono assemblies in Unity games with the use of Cecil.Inject.

## Features

* **Uses Mono.Cecil** to patch assemblies: get all the benefits of direct assembly manipulation
* **In-memory patching**: does not patch any files!
* **Very simple loader**: loading done through an OpenGL proxy. Works well with locale emulators!
* **Compatible with Sybaris patchers** (with some minor differences)
* **Open source!**

## Guides for users and developers on [the wiki](https://github.com/denikson/UnityPrePatcher/wiki)

## Building

To build, you will need:

* Visual Studio 2017 (2015 might work as well with some modifications)
* Visual C++ Toolset v140_xp (WIP: _xp only needed to compile minhook; might replace it with prebuilt library)
* .NET Framework 3.5 or newer
* Mono.Cecil (installed via NuGet on build)

First, clone the repository with submodules:

```bash
git clone --recurse-submodules https://github.com/denikson/UnityPrePatcher.git
```

Open in Visual Studio, select the platform (x86/x64) and build.

(WIP: Currently all build results are placed in separate folders; will be changed later)