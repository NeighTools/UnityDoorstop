# Unity Doorstop

> ⚠️ **Note for UPP users**
>
> The project has been divided! Namely,
> * UPP is now known as Doorstop, and it does not contain any patching tools.
> * PatcherLoader now exists as a legacy [SybarisLoader](https://github.com/NeighTools/SybarisLoader)

Doorstop is a tool to sideload and execute managed code in Unity before any of the assemblies are loaded*!

*For the exception of `mscorlib.dll`.

## Features

* **Runs first**: Doorstop's bootstrapper runs its own code before Unity has an ability to do so.
* **Modular**: Add your own loaders, and the bootstrapper will execute them!
* **Open source!**

## Guides for users and developers on [the wiki](https://github.com/denikson/UnityDoorstop/wiki)

## Building

To build, you will need:

* Visual Studio 2017 (2015 might work as well with some modifications)
* Visual C++ Toolset v140
* .NET Framework 3.5 or newer

First, clone the repository with submodules:

```bash
git clone --recurse-submodules https://github.com/denikson/UnityPrePatcher.git
```

Open in Visual Studio, select the platform (x86/x64) and build.

(WIP: Currently all build results are placed in separate folders; will be changed later)
