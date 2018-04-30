using System;
using System.IO;

namespace PatchLoader
{
    public enum LogLevel
    {
        Message,
        Info,
        Warning,
        Error
    }

    /// <summary>
    ///     A simple logger for patch loader.
    /// </summary>
    public static class Logger
    {
        private static bool enabled;
        private static bool initialized;
        private static TextWriter standardWriter;

        private static TextWriter textWriter;

        /// <summary>
        ///     Enable or disable the logger.
        /// </summary>
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

        /// <summary>
        ///     Dispose the logger.
        /// </summary>
        public static void Dispose()
        {
            if (!initialized)
                return;

            if (standardWriter != null)
                Console.SetOut(standardWriter);

            textWriter.Dispose();

            initialized = false;
        }

        /// <summary>
        ///     Reroutes the standard IO to the logger file.
        /// </summary>
        public static void RerouteStandardIO()
        {
            standardWriter = Console.Out;
            Console.SetOut(textWriter);
        }

        /// <summary>
        ///     Add a log entry.
        /// </summary>
        /// <param name="level">Log level.</param>
        /// <param name="message">Log message.</param>
        public static void Log(LogLevel level, string message)
        {
            if (!initialized || !enabled)
                return;

            textWriter.WriteLine($"[{level}] {message}");
        }

        /// <summary>
        ///     Log a message.
        /// </summary>
        /// <param name="message">Message to log.</param>
        public static void Log(string message)
        {
            Log(LogLevel.Message, message);
        }

        private static void Init()
        {
            if (initialized)
                return;

            StreamWriter writer =
                    File.CreateText(Path.Combine(Utils.LogsDir,
                                                 $"{DateTime.Now:yyyyMMdd_HHmmss_fff}_patcherloader.log"));
            writer.AutoFlush = true;

            textWriter = writer;

            initialized = true;
        }
    }
}