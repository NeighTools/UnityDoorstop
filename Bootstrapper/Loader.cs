using System;
using System.IO;
using System.Linq;
using System.Reflection;

namespace UnityDoorstop.Bootstrap
{
    public static class Loader
    {
        private static readonly string BinDir = Path.GetDirectoryName(Assembly.GetExecutingAssembly().Location);

        public static void Main(string[] args)
        {
            using (TextWriter tw = File.CreateText("test.txt"))
            {
                tw.WriteLine("Hello, world!");
                tw.WriteLine($"Got {args.Length} params!");

                for (var i = 0; i < args.Length; i++)
                {
                    tw.WriteLine($"{i} => {args[i]}");
                }

                tw.Flush();
            }
        }
    }
}