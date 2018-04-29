using System;
using System.Collections.Generic;
using System.IO;
using System.Reflection;
using Mono.Cecil;

namespace PatchLoader
{
    public static class Loader
    {
        private static Dictionary<string, List<MethodInfo>> patchersDictionary;

        public static void LoadPatchers()
        {
            patchersDictionary = new Dictionary<string, List<MethodInfo>>();

            Logger.Log(LogLevel.Info, "Loading patchers");

            foreach (string dll in Directory.GetFiles(Utils.PatchesDir, "*.Patcher.dll"))
            {
                Assembly assembly;

                try
                {
                    assembly = Assembly.LoadFile(dll);
                }
                catch (Exception e)
                {
                    Logger.Log(LogLevel.Error, $"Failed to load {dll}: {e.Message}");
                    if (e.InnerException != null)
                        Logger.Log(LogLevel.Error, $"Inner: {e.InnerException}");
                    continue;
                }

                foreach (Type type in assembly.GetTypes())
                {
                    if (type.IsInterface)
                        continue;

                    FieldInfo targetAssemblyNamesField =
                            type.GetField("TargetAssemblyNames", BindingFlags.Static | BindingFlags.Public);

                    if (targetAssemblyNamesField == null || targetAssemblyNamesField.FieldType != typeof(string[]))
                        continue;

                    MethodInfo patchMethod = type.GetMethod("Patch", BindingFlags.Static | BindingFlags.Public);

                    if (patchMethod == null || patchMethod.ReturnType != typeof(void))
                        continue;

                    ParameterInfo[] parameters = patchMethod.GetParameters();

                    if (parameters.Length != 1 || parameters[0].ParameterType != typeof(AssemblyDefinition))
                        continue;

                    string[] requestedAssemblies = targetAssemblyNamesField.GetValue(null) as string[];

                    if (requestedAssemblies == null || requestedAssemblies.Length == 0)
                        continue;

                    Logger.Log(LogLevel.Info, $"Adding {type.FullName}");

                    foreach (string requestedAssembly in requestedAssemblies)
                    {
                        if (!patchersDictionary.TryGetValue(requestedAssembly, out List<MethodInfo> list))
                        {
                            list = new List<MethodInfo>();
                            patchersDictionary.Add(requestedAssembly, list);
                        }

                        list.Add(patchMethod);
                    }
                }
            }
        }

        public static void Patch()
        {
            Logger.Log(LogLevel.Info, "Patching assemblies:");

            foreach (KeyValuePair<string, List<MethodInfo>> patchJob in patchersDictionary)
            {
                string assembly = patchJob.Key;
                List<MethodInfo> patchers = patchJob.Value;

                string assemblyPath = Path.Combine(Utils.GameAssembliesDir, assembly);

                if (!File.Exists(assemblyPath))
                {
                    Logger.Log(LogLevel.Warning, $"{assemblyPath} does not exist. Skipping...");
                    continue;
                }

                AssemblyDefinition assemblyDefinition;

                try
                {
                    assemblyDefinition = AssemblyDefinition.ReadAssembly(assemblyPath);
                }
                catch (Exception e)
                {
                    Logger.Log(LogLevel.Error, $"Failed to open {assemblyPath}: {e.Message}");
                    continue;
                }

                foreach (MethodInfo patcher in patchers)
                {
                    Logger.Log(LogLevel.Info, $"Running {patcher.DeclaringType.FullName}");
                    try
                    {
                        patcher.Invoke(null, new object[] {assemblyDefinition});
                    }
                    catch (Exception e)
                    {
                        Logger.Log(LogLevel.Error, $"Failed to patch because: {e.Message}");
                        Logger.Log(LogLevel.Error, $"Stack trace:\n{e.StackTrace}");
                    }
                }

                MemoryStream ms = new MemoryStream();

                assemblyDefinition.Write(ms);
                assemblyDefinition.Dispose();

                Assembly.Load(ms.ToArray());

                ms.Dispose();
            }
        }

        public static void Run()
        {
            if (!Directory.Exists(Utils.LogsDir))
                Directory.CreateDirectory(Utils.LogsDir);

            Logger.Enabled = true;
            Logger.RerouteStandardIO();

            Logger.Log("===Unity PrePatcher Loader===");
            Logger.Log($"Started on {DateTime.Now:R}");

            if (!Directory.Exists(Utils.PatchesDir))
            {
                Directory.CreateDirectory(Utils.PatchesDir);
                Logger.Log(LogLevel.Info, "No patches directory found! Created an empty one!");
                Logger.Dispose();
                return;
            }

            Logger.Log(LogLevel.Info, "Adding ResolveAssembly Handler");

            AppDomain.CurrentDomain.AssemblyResolve += ResolvePatchers;

            LoadPatchers();

            if (patchersDictionary.Count == 0)
            {
                Logger.Log(LogLevel.Info, "No valid patchers found! Quiting...");
                Logger.Dispose();
                return;
            }

            Patch();

            Logger.Log(LogLevel.Info, "Patching complete! Disposing of logger!");
            Logger.Dispose();
        }

        private static Assembly ResolvePatchers(object sender, ResolveEventArgs args)
        {
            if (Utils.TryResolveAssembly(args.Name, Utils.PatchesDir, out Assembly patchAssembly))
                return patchAssembly;
            if (Utils.TryResolveAssembly(args.Name, Utils.BinariesDir, out Assembly binaryAssembly))
                return binaryAssembly;
            return null;
        }
    }
}