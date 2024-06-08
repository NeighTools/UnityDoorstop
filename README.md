<p align="center">
  <img height="256" width="256" src="assets/img/icon.png">
</p>

<h1 align="center">Unity Doorstop</h1>

[![GitHub Workflow Status](https://img.shields.io/github/actions/workflow/status/NeighTools/UnityDoorstop/build-be.yml?branch=master)](https://github.com/NeighTools/UnityDoorstop/actions/workflows/build-be.yml)
[![nightly.link artifacts](https://img.shields.io/badge/Artifacts-nightly.link-blueviolet)](https://nightly.link/NeighTools/UnityDoorstop/workflows/build-be/master)

***

Doorstop is a tool to execute managed .NET assemblies inside Unity as early as possible.

**This is a total rewrite of UnityDoorstop 3. See [list of breaking changes](CHANGES.md) for more information.**

## Features

* **Runs first**: Doorstop runs its code before Unity can do so
* **Configurable**: An elementary configuration file allows you to specify your assembly to execute
* **Multiplatform**: Supports Windows, Linux, macOS
* **Debugger support**: Allows to debug managed assemblies in Visual Studio, Rider or dnSpy *without modifications to Mono*

## Unity runtime support

Doorstop supports executing .NET assemblies in both Unity Mono and Il2Cpp runtimes.
Depending on the runtime the game uses, Doorstop tries to run your assembly as follows:

* On Unity Mono, your assembly is executed in the same runtime. As a result
  * You don't need to include your custom Common Language Runtime (CLR); the one bundled with the game is used
  * Your assembly is run alongside other Unity code
  * You can access all Unity API directly
* On Il2Cpp, your assembly is executed in CoreCLR runtime because Il2Cpp cannot run managed assemblies. As a result:
  * You need to include .NET 6 or newer CoreCLR runtime with your managed assembly
  * Your assembly is run in a runtime that is isolated from Il2Cpp
  * You can access Il2Cpp runtime by interacting with its native `il2cpp_` API

## Building

Doorstop uses [xmake](https://xmake.io/) to build the project. To build, run `build.bat`, `build.ps1` or `build.sh`.

Available build options:

* `-with_logging`: build with logging enabled
* `-arch`: the architectures to build for, separated by commas (e.g. `-arch x86,x64`)
* `-debug`: build in debug mode (currently only for *nix)

> **Note:** Initial build times are usually slower because the build script automatically downloads and installs xmake.  
> On Unix, xmake is built directly from the source code.

## Minimal injection example

To have Doorstop inject your code, create `Entrypoint` class into `Doorstop` namespace.
Define a public static `Start` method in it:

```cs
using System.IO;

namespace Doorstop;

class Entrypoint
{
  public static void Start()
  {
      File.WriteAllText("doorstop_hello.log", "Hello from Unity!");
  }
}
```

You can then define any code you want in `Start`.

**NOTE:** On UnityMono, Doorstop bootstraps your assembly with a minimal number of assemblies and minimal configuration.
This early execution allows for some interesting tricks, like redirecting the loading of some game assemblies.
Bear also in mind that some of the Unity runtime is not initialized at such an early stage, limiting the code you can execute.
You might need to appropriately pause the execution of your code until the moment you want to modify the game.

### Doorstop environment variables

Doorstop sets some environment variables useful for code execution:

| Environment variable          | Description                                                                                                                      |
| ----------------------------- | -------------------------------------------------------------------------------------------------------------------------------- |
| `DOORSTOP_INITIALIZED`        | Always set to `TRUE`. Use to determine if your code is run via Doorstop.                                                         |
| `DOORSTOP_INVOKE_DLL_PATH`    | Path to the assembly executed by Doorstop relative to the current working directory.                                             |
| `DOORSTOP_PROCESS_PATH`       | Path to the application executable where the injected assembly is run.                                                           |
| `DOORSTOP_MANAGED_FOLDER_DIR` | *UnityMono*: Path to the game's `Managed` folder. *Il2Cpp*: Path to CoreCLR's base class library folder.                         |
| `DOORSTOP_DLL_SEARCH_DIRS`    | Paths where the runtime searchs assemblies from by default, separated by OS-specific separator (`;` on windows and `:` on *nix). |
| `DOORSTOP_MONO_LIB_PATH`      | *Only on UnityMono*: Full path to the mono runtime library.                                                                      |

### Debugging

Doorstop 4 supports debugging the assemblies in the runtime.

#### Debugging in UnityMono

To enable debugging, set `debug_enabled` to `true` and optionally change the debug server address via `debug_address` (see [configuration options](#doorstop-configuration)).  
After launching the game, you may connect to the debugger using the server address (default is `127.0.0.1:10000`).  
By default, the game won't wait for the debugger to connect; you may change the behaviour with the `debug_suspend` option.

> **If you use dnSpy**, you can use the `Debug > Start Debugging > Debug engine > Unity` option, automatically setting the correct debugging configuration.  
> Doorstop detects dnSpy and automatically enables debugging without any extra configuration.

#### Debugging in Il2Cpp

Debugging is automatically enabled in CoreCLR. 

To start debugging, compile your DLL in debug mode (with embedded or portable symbols) and start the game with the debugger of your choice.  
Alternatively, attach a debugger to the game once it is running. All standard CoreCLR debuggers should detect the CoreCLR runtime in the game.

Moreover, hot reloading is supported for Visual Studio, Rider and other debuggers with .NET 6 hot reloading feature enabled.

**Note that you can only debug managed code!** Because the game code is unmanaged (i.e. Il2Cpp), you cannot directly debug the actual game code.
Consider using native debuggers like GDB and visual debugging tools like IDA or Ghidra to debug actual game code.

## Doorstop configuration

Doorstop is highly configurable based on your needs and the environment you want to use.
There are two ways to configure Doorstop: via config and CLI arguments.

### Via configuration file

Refer to [`doorstop_config.ini`](assets/windows/doorstop_config.ini) (Windows) or [`run.sh`](assets/nix/run.sh) for all available configuration options.

### CLI arguments

The following CLI arguments are available on both *nix, and Windows builds:

All Doorstop arguments start with `--doorstop-` and always contain an argument. The arguments can be of the following type:

* `bool` = `true` or `false`
* `string` = any sequence of characters and numbers. Wrap into `"`s if the string contains spaces

| Argument                                          | Description                                                                                          |
| ------------------------------------------------- |------------------------------------------------------------------------------------------------------|
| `--doorstop-enabled bool`                         | Enable or disable Doorstop.                                                                          |
| `--doorstop-redirect-output-log bool`             | *Only on Windows*: If `true` Unity's output log is redirected to `<current folder>\output_log.txt`   |
| `--doorstop-target-assembly string`               | Path to the assembly to load and execute.                                                            |
| `--doorstop-boot-config-override string`          | Overrides the boot.config file path.                                                                 |
| `--doorstop-mono-dll-search-path-override string` | Overrides default Mono DLL search path                                                               |
| `--doorstop-mono-debug-enabled bool`              | If true, Mono debugger server will be enabled                                                        |
| `--doorstop-mono-debug-suspend bool`              | Whether to suspend the game execution until the debugger is attached.                                |
| `--doorstop-mono-debug-address string`            | The address to use for the Mono debugger server.                                                     |
| `--doorstop-clr-corlib-dir string`                | Path to coreclr library that contains the CoreCLR runtime                                            |
| `--doorstop-clr-runtime-coreclr-path string`      | Path to the directory containing the managed core libraries for CoreCLR (`mscorlib`, `System`, etc.) |


## License

Doorstop 4 is licensed under LGPLv2.1. You can view the entire license [here](LICENSE).  
You can still access the source code to the original UnityDoorstop 3 source (licensed under CC0) from [the legacy branch](https://github.com/NeighTools/UnityDoorstop/tree/legacy).
