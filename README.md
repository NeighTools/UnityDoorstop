<p align="center">
  <img height="256" width="256" src="assets/img/icon.png">
</p>

# Unity Doorstop

Doorstop is a tool to execute managed .NET assemblies inside Unity as early as possible.

**This is a full rewrite of UnityDoorstop 3.x. See [list of breaking changes](CHANGES.md) for more information.**

## Features

* **Runs first**: Doorstop runs its code before Unity can do so
* **Configurable**: An elementary configuration file allows you to specify your assembly to execute
* **Multiplatform**: Supports Windows, Linux, macOS

## Unity runtime support

Doorstop supports executing .NET assemblies in both Unity Mono and Il2Cpp runtimes.
Depending on the runtime the game uses, Doorstop tries to run your assembly as follows:

* On Unity Mono, your assembly is executed in the same Unity Mono runtime. As a result
  * You don't need to include your custom Common Language Runtime (CLR); the one bundled with the game is used
  * Your assembly is run alongside other Unity code
  * You can access all Unity API directly
* On Il2Cpp, your assembly is executed in CoreCLR runtime because Il2Cpp cannot run managed assemblies. As a result:
  * You need to include .NET 6 or newer CoreCLR runtime with your managed assembly
  * Your assembly is run in a runtime that is isolated from Il2Cpp
  * You can access Il2Cpp runtime by interacting with its native `il2cpp_` API

## WIP

This is a rewrite of the original Unity Doorstop. The goal of the rewrite is to clean up the code base and merge Unix version into the main build.

Current progress:

* [x] Restructure to allow multiple platform support
* [x] Port Windows support
* [x] Port mono support
* [x] Update Il2Cpp to use CoreCLR runtime 
* [x] Port Unix support
* [x] Add Unix build scripts
* [ ] Add example run/configure script for Unix
* [ ] Test that ports actually work
* [ ] Add automated CI builds

## Building

Doorstop uses [xmake](https://xmake.io/) to build the project. In order to build, run `build.bat`, `build.ps1` or `build.sh`.
Add `-with_logging` if you want to build with logging enabled.

> **Note:** Initial build times are usually slower because the build script automatically downloads and installs xmake.  
> On Unix, xmake is built directly from source.