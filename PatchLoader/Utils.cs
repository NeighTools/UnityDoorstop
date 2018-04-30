using System;
using System.Diagnostics;
using System.IO;
using System.Reflection;

namespace PatchLoader
{
    public static class Utils
    {
        static Utils()
        {
            // Removed in order to not depend on System.dll (in order to make it patchable)
            //GameName = Path.GetFileNameWithoutExtension(Process.GetCurrentProcess().ProcessName);
            //GameRootDir = Path.GetDirectoryName(Process.GetCurrentProcess().MainModule.FileName);
            //GameAssembliesDir = Path.Combine(GameRootDir, Path.Combine($"{GameName}_Data", "Managed"));

            GameAssembliesDir = Path.GetDirectoryName(typeof(Assembly).Assembly.Location);
            BinariesDir = Path.GetDirectoryName(Assembly.GetExecutingAssembly().Location);
            UnityPrePatcherDir = Path.Combine(BinariesDir, "..");
            LogsDir = Path.Combine(UnityPrePatcherDir, "logs");
            PatchesDir = Path.Combine(UnityPrePatcherDir, "patches");
        }

        /// <summary>
        ///     Binaries directory for UnityPrePatcher.
        /// </summary>
        public static string BinariesDir { get; }

        /// <summary>
        ///     Game's Managed directory. Takes in account game's executable name.
        /// </summary>
        public static string GameAssembliesDir { get; }

        ///// <summary>
        /////     The game's name (read from the process name)
        ///// </summary>
        //public static string GameName { get; }

        ///// <summary>
        /////     Game's root directory (deduced from current process info).
        ///// </summary>
        //public static string GameRootDir { get; }

        /// <summary>
        ///     Logs directory for UnityPrePatcher.
        /// </summary>
        public static string LogsDir { get; }

        /// <summary>
        ///     Patches directory for UnityPrePatcher.
        /// </summary>
        public static string PatchesDir { get; }

        /// <summary>
        ///     Base UnityPrePatcher directory.
        /// </summary>
        public static string UnityPrePatcherDir { get; }

        /// <summary>
        ///     Try to resolve and load the given assembly DLL.
        /// </summary>
        /// <param name="name">Name of the assembly. Follows the format of <see cref="AssemblyName" />.</param>
        /// <param name="directory">Directory to search the assembly from.</param>
        /// <param name="assembly">The loaded assembly.</param>
        /// <returns>True, if the assembly was found and loaded. Otherwise, false.</returns>
        public static bool TryResolveDllAssembly(string name, string directory, out Assembly assembly)
        {
            assembly = null;
            string path = Path.Combine(directory, $"{new AssemblyName(name).Name}.dll");

            if (!File.Exists(path))
                return false;

            try
            {
                assembly = Assembly.LoadFile(path);
            }
            catch (Exception)
            {
                return false;
            }

            return true;
        }
    }
}