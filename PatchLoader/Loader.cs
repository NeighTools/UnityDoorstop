using System;
using System.IO;

namespace PatchLoader
{
    public static class Loader
    {
        public static void Run()
        {
            if (!Directory.Exists(Utils.LogsDir))
                Directory.CreateDirectory(Utils.LogsDir);

            Logger.Enabled = true;

            Logger.Log("===Unity PrePatcher Loader===");
            Logger.Log($"Started on {DateTime.Now:R}");

            Logger.Dispose();
        }
    }
}