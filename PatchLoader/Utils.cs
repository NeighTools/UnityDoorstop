using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.IO;
using System.Linq;
using System.Reflection;
using System.Text;

namespace PatchLoader
{
    public static class Utils
    {
        public static string GameName { get; } =
            Path.GetFileNameWithoutExtension(Process.GetCurrentProcess().ProcessName);

        public static string GameAssembliesDir { get; } = Path.Combine(".", Path.Combine($"{GameName}_Data", "Managed"));

        public static string BinariesDir { get; } = Path.GetDirectoryName(Assembly.GetExecutingAssembly().Location);

        public static string UnityPrePatcherDir { get; } = Path.Combine(BinariesDir, "..");

        public static string PatchesDir { get; } = Path.Combine(UnityPrePatcherDir, "patches");

        public static string LogsDir { get; } = Path.Combine(UnityPrePatcherDir, "logs");

        public static bool TryResolveAssembly(string name, string directory, out Assembly assembly)
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
