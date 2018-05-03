using System;
using System.IO;
using System.Linq;
using System.Reflection;

namespace UnityDoorstop.Bootstrap
{
    public static class Loader
    {
        private static readonly string BinDir = Path.GetDirectoryName(Assembly.GetExecutingAssembly().Location);

        /// <summary>
        ///     The entry point of the loader called by proxy.
        /// </summary>
        /// <remarks>
        ///     This is the entry point of the patch loader.
        ///     The method is invoked right after Unity Root Domain has been created,
        ///     which means only the minimal set of managed assemblies is loaded.
        ///     Since assemblies cannot be unloaded, you create hooks and event handlers.
        /// </remarks>
        public static void Run()
        {
            // Add a resolver for \bin directory for convenience
            AppDomain.CurrentDomain.AssemblyResolve += ResolveInBinDirectory;

            string rootDir = Path.Combine(BinDir, "..");
            string patchersDir = Path.Combine(rootDir, "loaders");

            if (!Directory.Exists(patchersDir))
            {
                Directory.CreateDirectory(patchersDir);
                return;
            }

            foreach (string dll in Directory.GetFiles(patchersDir, "*.dll"))
                try
                {
                    Assembly patcher = Assembly.LoadFile(dll);

                    const BindingFlags flags = BindingFlags.Public | BindingFlags.NonPublic | BindingFlags.Static;

                    Type entryType =
                            patcher.GetTypes().FirstOrDefault(t => t.GetMethods(flags).Any(m => m.Name == "Main"));

                    MethodInfo runMethod = entryType?.GetMethods(flags).FirstOrDefault(m => m.Name == "Main");

                    runMethod?.Invoke(null, new object[0]);
                }
                catch (Exception) { }
        }

        private static Assembly ResolveInBinDirectory(object sender, ResolveEventArgs args)
        {
            string path = Path.Combine(BinDir, $"{new AssemblyName(args.Name).Name}.dll");

            if (!File.Exists(path))
                return null;

            try
            {
                return Assembly.LoadFile(path);
            }
            catch (Exception)
            {
                return null;
            }
        }
    }
}