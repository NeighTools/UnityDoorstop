# Breaking changes from UnityDoorstop 3.x

This version of Doorstop is almost a complete rewrite of previous versions.
Because of this, some breaking changes were introduced to streamline the syntax and behaviour.

This document outlines the breaking changes from UnityDoorstop 3.x and their rationale.

## `doorstop_config.ini` options and command-line argument names were renamed

The configuration file sections were renamed, and options were converted to snake_case:

| UnityDoorstop 3.x                     | Doorstop 4                           |
| ------------------------------------- | ------------------------------------ |
| `UnityDoorstop.enabled`               | `General.enabled`                    |
| `UnityDoorstop.targetAssembly`        | `General.target_assembly`            |
| `UnityDoorstop.redirectOutputLog`     | `General.redirect_output_log`        |
| `UnityDoorstop.ignoreDisableSwitch`   | `General.ignore_disable_switch`      |
| `UnityDoorstop.dllSearchPathOverride` | `UnityMono.dll_search_path_override` |
| `MonoBackend.runtimeLib`              | removed, see next section            |
| `MonoBackend.configDir`               | removed, see next section            |
| `MonoBackend.corlibDir`               | removed, see next section            |
| `MonoBackend.debugEnabled`            | `UnityMono.debug_enabled`            |
| `MonoBackend.debugSuspend`            | `UnityMono.debug_suspend`            |
| `MonoBackend.debugAddress`            | `UnityMono.debug_address`            |


In addition, command-line arguments were renamed to follow the internal naming scheme:

| UnityDoorstop 3.x                | Doorstop 4                                 |
| -------------------------------- | ------------------------------------------ |
| `--doorstop-enable`              | `--doorstop-enabled`                       |
| `--redirect-output-log`          | `--doorstop-redirect-output-log`           |
| `--doorstop-target`              | `--doorstop-target-assembly`               |
| `--doorstop-dll-search-override` | `--doorstop-mono-dll-search-path-override` |
| `--mono-runtime-lib`             | removed, see next section                  |
| `--mono-config-dir`              | removed, see next section                  |
| `--mono-corlib-dir`              | removed, see next section                  |
| `--mono-debug-enabled`           | `--doorstop-mono-debug-enabled`            |
| `--mono-debug-suspend`           | `--doorstop-mono-debug-suspend`            |
| `--mono-debug-address`           | `--doorstop-mono-debug-address`            |


## CoreCLR is now used with Il2Cpp runtime instead of mono

With CoreCLR receiving better support for embedding it and mono being slowly merged with CoreCLR, it is more reasonable to start using it for Il2Cpp games.  
As such, Doorstop will now execute managed assemblies using the CoreCLR runtime in Il2Cpp games.

With this change, all `MonoBackend` configuration options are removed.
Instead, two new options are added under `Il2Cpp`:

* `Il2Cpp.coreclr_path` - Path to `coreclr.dll` that contains the CoreCLR runtime. All other CoreCLR-related DLLs (`clrjit.dll`, `clrgc.dll`, and others) should be in the same folder as `coreclr.dll`.
* `Il2Cpp.corlib_dir` - Path to the directory containing the managed core libraries for CoreCLR (`mscorlib`, `System`, and others)

## Managed entry point is now always `static void Doorstop.Entrypoint.Start()`

Instead of looking for an entry point automatically, Doorstop will now always look for the same entry point:

```cs
namespace Doorstop;

class Entrypoint
{
    public static void Start()
    {
        // Your custom code
    }
}
```

Specifically, `Main(string[] args)` is no longer a good entry point.


The change was made for two reasons:

* Doorstop does not need to pass any arguments anymore. Instead, Doorstop passes all the information via environment variables.
* CoreCLR does not support easy method searching using wildcards yet.
