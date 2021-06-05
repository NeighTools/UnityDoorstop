Welcome to the Doorstop wiki!

Here you can find information about Unity Doorstop.

## About Unity Doorstop

Unity Doorstop (just Doorstop from now on) is a minimal tool to *execute a managed assembly* within Unity. Doorstop executes the assembly *before* Unity has the ability to modify its application domain, which means only `mscorlib.dll` is loaded when Doorstop executes the assembly.

Doorstop is a tool that can be included with other tools that want to execute its managed code within Unity Root Domain before Unity. That is, all patchers, patcher loaders, assembly modifiers and debuggers will benefit from using the tool

Doorstop is released to the public domain under CC0 and can be distributed as-is with any available tools.

## Installation

1. Put `winhttp.dll` into the game's root directory
2. In game's root directory, create a file `doorstop_config.ini` with the following contents:

```ini
[UnityDoorstop]
# Specifies whether assembly executing is enabled
enabled=true
# Specifies the path (absolute, or relative to the game's exe) to the DLL/EXE that should be executed by Doorstop
targetAssembly=Doorstop.dll
# Specifies whether Unity's output log should be redirected to <current folder>\output_log.txt
redirectOutputLog=false
```

3. Configure Doorstop to point to your DLL/EXE.
    * *Alternatively*, extract `Example\Doorstop.dll` into the game's root folder and use the above configuration as-is.
4. Run the game
    * If run with the example, a file named `doorstop_is_alive.txt` should appear in the game's root with the success message.

## Using Doorstop as `version.dll` proxy

The official releases allow you to use Doorstop as `version.dll` proxy as well.  
While `winhttp.dll` has been tested to have better support running on Wine, `version.dll` 
appears more functional with Unity 3 games. As such, you can choose between `winhttp` and `version` proxies 
by simply renaming the Doorstop DLL.

## Using Doorstop to execute assemblies

Doorstop will execute any managed .NET assembly (version depends on version of Mono used by the game) with a valid [entry point](https://docs.microsoft.com/en-us/dotnet/csharp/programming-guide/main-and-command-args/). In nutshell any of the following signatures are considered a valid entry point:

```csharp
public static void Main();
public static void Main(string[] args);
public static int Main();
public static int Main(string[] args);
```

If Doorstop encounters multiple entry points, the result is undefined (in a way that Doorstop will try to invoke the very first valid entry point it finds).

### Entry point arguments

> NOTE! Breaking change in 3.0.0.0
> Starting 3.0.0.0, no values are passed to the array. This is to increase compatibility with older Unity versions.
> Instead, use `DOORSTOP_PROCESS_PATH` (Or `Process.GetCurrentProcess().MainModule.FileName` is `System.Core` is included with the game).

If the entry point provides `string[]` as parameter, Doorstop will provide an empty array to simulate running with no arguments.
Use `Environment.CommandLine` to handle application's command line arguments.

## Environment variables

In addition to command line arguments, Doorstop passes some important paths via environment variables that you can access using `Environment.GetEnvironmentVariable`. Doorstop adds the following variables:

| Variable | Description |
|----------|------------ |
| `DOORSTOP_DISABLE` | A helper environment variable that disabled Doorstop execution. Can be set to `TRUE` by the user to disable Doorstop (in addition to other ways to disable it discussed below). |
| `DOORSTOP_INVOKE_DLL_PATH` | Path to the assembly that was invoked via Doorstop. Contains the same value as in `targetAssembly` configuration option in the config file. |
| `DOORSTOP_MANAGED_FOLDER_DIR` | Full path to the game's `Managed` folder that contains all the game's managed assemblies |
| `DOORSTOP_PROCESS_PATH` | Full path to the game's executable. Useful when game doesn't ship with `System.Core` or you if you want to patch it. |
| `DOORSTOP_DLL_SEARCH_DIRS` | List of paths where mono searches DLLs for before resorting to managed assembly resolvers. Paths are separated with `;` |

### Assembly references

Doorstop executes the assembly with the *minimal number of assemblies* loaded. At the minimum, only `mscorlib.dll` and only the necessary dependencies will be loaded *as needed* (as per CLR's specification).

Doorstop does not install any additional assembly resolve hooks, which means that by default only assemblies found in `<Game>_Data\Managed` will be resolved automatically. It is thus the developer's job to resolve any external assemblies.

## Example executable class

An example of an executable assembly can be found in the [project source](https://github.com/NeighTools/UnityDoorstop/tree/master/DoorstopTest).

## Configuration file

Doorstop will load the the configuration file named `doorstop_config.ini` from the game's root directory.  
The structure of the configuration file is as follows:

```ini
[UnityDoorstop]
# Specifies whether assembly executing is enabled
enabled=true
# Specifies the path (absolute, or relative to the game's exe) to the DLL/EXE that should be executed by Doorstop
targetAssembly=Doorstop.dll
# Specifies whether Unity's output log should be redirected to <current folder>\output_log.txt
redirectOutputLog=false
# If enabled, DOORSTOP_DISABLE env var value is ignored
# USE THIS ONLY WHEN ASKED TO OR YOU KNOW WHAT THIS MEANS
ignoreDisableSwitch=false
# Overrides default Mono DLL search path
# Sometimes it is needed to instruct Mono to seek its assemblies from a different path
# (e.g. mscorlib is stripped in original game)
# This option causes Mono to seek mscorlib and core libraries from a different folder before Managed
# Original Managed folder is added as a secondary folder in the search path
dllSearchPathOverride=


# Settings related to bootstrapping a custom Mono runtime
# Do not use this in managed games!
# These options are intended running custom mono in IL2CPP games!
[MonoBackend]
# Path to main mono.dll runtime library
runtimeLib=
# Path to mono config/etc directory
configDir=
# Path to core managed assemblies (mscorlib et al.) directory
corlibDir=
# Specifies whether the mono soft debugger is enabled (listening on port 55555)
debug=false
```

## Configuration via command-line arguments

Alternatively, Doorstop 2.3+ supports supplying configuration options via command-line arguments.  
To use the feature, run the game with the following command-line arguments:

* `--doorstop-enable`: enable or disable Doorstop. Allowed values are `true` and `false`.
* `--doorstop-target`: Target DLL to execute. Allows a path to the DLL.
* `--redirect-output-log`: enable or disable redirecting output_log.txt to the game's root folder. Only for Unity 5+. Allowed values are `true` and `false`.
* `--doorstop-dll-search-override`: Overrides default Mono DLL search path, see `dllSearchPathOverride` in configuration file for more details.

If both configuration file and command-line arguments are present **command-line arguments take precedence*.*
