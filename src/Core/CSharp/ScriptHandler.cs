using System;
using System.Runtime.InteropServices;

namespace ScriptEngine
{
    public class ScriptHandler
    {
        // Struct to safely pass initialization configuration from C++
        [StructLayout(LayoutKind.Sequential, CharSet = CharSet.Unicode)]
        public struct HostConfig
        {
            public string ScriptDirectory;
        }

        [UnmanagedCallersOnly(EntryPoint = "InitializeHandler")]
        public static int InitializeHandler(IntPtr arg, int argLength)
        {
            if (argLength < Marshal.SizeOf<HostConfig>()) return -1;

            // Marshal configuration out of native memory
            HostConfig config = Marshal.PtrToStructure<HostConfig>(arg);
            Console.WriteLine($"[C# Handler] Script Engine initialized. Watching: {config.ScriptDirectory}");

            return 0; // Success
        }

        [UnmanagedCallersOnly(EntryPoint = "RunScript")]
        public static int RunScript(IntPtr arg, int argLength)
        {
            string scriptPath = Marshal.PtrToStringUni(arg);
            Console.WriteLine($"[C# Handler] Compiling and running script: {scriptPath}");

            // Integrate scripting APIs like Microsoft.CodeAnalysis.CSharp.Scripting here

            return 100; // Return execution status code back to C++
        }
    }
}
