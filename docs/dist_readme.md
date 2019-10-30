Welcome to the Doorstop wiki!

Here you can find information about Unity Doorstop.

## About Unity Doorstop

Unity Doorstop (just Doorstop from now on) is a minimal tool to *execute a managed assembly* within Unity. Doorstop executes the assembly *before* Unity has the ability to modify its application domain, which means only `mscorlib.dll` is loaded when Doorstop executes the assembly.

Doorstop is a tool that can be included with other tools that want to execute its managed code within Unity Root Domain before Unity. That is, all patchers, patcher loaders, assembly modifiers and debuggers will benefit from using the tool

Doorstop is released to the public domain under CC0 and can be distributed as-is with any available tools.

## Installation

1. Put `version.dll` into the game's root directory
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

If the entry point provides `string[]` as parameter, Doorstop will provide the following array:

```csharp
[0] => Process.GetCurrentProcess().MainModule.FileName
[1] => "--doorstop-invoke"
```

thus, the first parameter is the *path to the game's executable*, because the assembly will be run within the game's process. This frees the developer from importing `System.dll` and doing the lookup manually.

The second argument is as a hint for the assembly that it was invoked by Doorstop and that it is running within the Unity Root Domain.

## Environment variables

In addition to command line arguments, Doorstop passes some important paths via environment variables that you can access using `Environment.GetEnvironmentVariable`. Doorstop adds the following variables:

| Variable | Description |
|----------|------------ |
| `DOORSTOP_DISABLE` | A helper environment variable that disabled Doorstop execution. Can be set to `TRUE` by the user to disable Doorstop (in addition to other ways to disable it discussed below). |
| `DOORSTOP_INVOKE_DLL_PATH` | Path to the assembly that was invoked via Doorstop. Contains the same value as in `targetAssembly` configuration option in the config file. |
| `DOORSTOP_MANAGED_FOLDER_DIR` | Full path to the game's `Managed` folder that contains all the game's managed assemblies |

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
```

## Configuration via command-line arguments

Alternatively, Doorstop 2.3+ supports supplying configuration options via command-line arguments.  
To use the feature, run the game with the following command-line arguments:

* `--doorstop-enable`: enable or disable Doorstop. Allowed values are `true` and `false`.
* `--doorstop-target`: Target DLL to execute. Allows a path to the DLL.
* `----redirect-output-log`: enable or disable redirecting output_log.txt to the game's root folder. Only for Unity 5+. Allowed values are `true` and `false`.

If both configuration file and command-line arguments are present **command-line arguments take precedence*.*
