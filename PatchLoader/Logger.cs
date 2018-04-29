using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Text;

namespace PatchLoader
{
    public enum LogLevel
    {
        Message,
        Info,
        Warning,
        Error
    }

    public static class Logger
    {
        private static bool initialized;

        private static bool enabled;
        public static bool Enabled
        {
            get => enabled;
            set
            {
                enabled = value;

                if (enabled)
                    Init();
                else
                    Dispose();
            }
        }

        private static TextWriter textWriter;
        private static TextWriter standardWriter;

        private static void Init()
        {
            if (initialized)
                return;

            StreamWriter writer = File.CreateText(Path.Combine(Utils.LogsDir, $"{DateTime.Now:yyyyMMdd_HHmmss_fff}_patcherloader.log"));
            writer.AutoFlush = true;

            textWriter = writer;
            
            initialized = true;
        }

        public static void Dispose()
        {
            if (!initialized)
                return;

            if(standardWriter != null)
                Console.SetOut(standardWriter);

            textWriter.Dispose();

            initialized = false;
        }

        public static void RerouteStandardIO()
        {
            standardWriter = Console.Out;
            Console.SetOut(textWriter);
        }

        public static void Log(LogLevel level, string message)
        {
            if (!initialized || !enabled)
                return;

            textWriter.WriteLine($"[{level}] {message}");
        }

        public static void Log(string message)
        {
            Log(LogLevel.Message, message);
        }
    }
}
