<p align="center">
   <img src="https://raw.githubusercontent.com/NeighTools/UnityDoorstop/master/docs/logo_sm.png"/>
</p>

![Github All Releases](https://img.shields.io/github/downloads/NeighTools/UnityDoorstop/total.svg)
![GitHub release](https://img.shields.io/github/release/NeighTools/UnityDoorstop.svg)
![license](https://img.shields.io/github/license/NeighTools/UnityDoorstop.svg)

*Run managed code before Unity does!*

# Unity Doorstop

Doorstop is a tool to execute managed assemblies inside Unity as early as possible!

This repository is indented mainly for developers!  
Developers should package Doorstop into their applications for the end-users.

## Features

* **Runs first**: Doorstop runs its own code before Unity has an ability to do so.
* **Configurable**: A very basic configuration file allows to specify your own assembly to execute!
* **Public domain!** You are fully free to embed Doorstop into your application!

## Guides for users and developers on [the wiki](https://github.com/NeighTools/UnityDoorstop/wiki)

## Building

To build, you will need:

* Visual Studio 2017 (2015 might work as well with some modifications)
* Visual C++ Toolset v140
* .NET Framework 3.5 or newer (only for the example, not used by the proxy)
* Python (only to generate proxy functions; not needed to build)

Clone, open in Visual Studio, select the platform (x86/x64) and build.

#### Custom proxy functions

Doorstop's proxy is flexible and allows to be load as different DLLs.
You can modify which functions you want to proxy by adding/removing function names in `Proxy/proxydefs.txt` and running `proxygen/proxy_gen.py ../Proxy/proxydefs.txt` to generate an appropriate proxy functions.

The current set up allows to use the proxy for the following DLLs:

* `winhttp.dll` (All exports)
* `iphlpapi.dll` (Only `GetIpAddrTable`)

(WIP: Currently all build results are placed in separate folders; will be changed later)
