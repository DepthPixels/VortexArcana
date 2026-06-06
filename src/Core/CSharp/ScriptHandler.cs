using System;
using ScriptHost;
using System.Runtime.InteropServices;
using VortexArcana;

namespace ScriptEngine
{
    public class ScriptHandler
    {
        private static ScriptHotReloadEngine? _engine;

        // Struct to safely pass initialization configuration from C++
        [StructLayout(LayoutKind.Sequential, CharSet = CharSet.Unicode)]
        public struct HostConfig
        {
            public string ScriptDirectory;
        }

        [StructLayout(LayoutKind.Sequential, CharSet = CharSet.Unicode)]
        public struct InstantiateConfig
        {
            public IntPtr EntityID;
            public string ScriptName;
        }

        [UnmanagedCallersOnly(EntryPoint = "InitializeHandler")]
        public static int InitializeHandler(IntPtr arg, int argLength)
        {
            if (argLength < Marshal.SizeOf<HostConfig>()) return -1;

            // Marshal configuration out of native memory
            HostConfig config = Marshal.PtrToStructure<HostConfig>(arg);
            Console.WriteLine($"[C# Handler] Script Engine initialized. Initializing Engine for: {config.ScriptDirectory}");

            _engine = new ScriptHotReloadEngine(config.ScriptDirectory);

            return 0; // Success
        }

        [UnmanagedCallersOnly(EntryPoint = "RunScript")]
        public static int RunScript(IntPtr arg, int argLength)
        {
            if (argLength < Marshal.SizeOf<HostConfig>()) return -1;

            // Marshal configuration out of native memory
            HostConfig config = Marshal.PtrToStructure<HostConfig>(arg);
            Console.WriteLine($"[C# Handler] Running Update() on script: {config.ScriptDirectory}");

            // Testing

            return 100; // Return execution status code back to C++
        }

        [UnmanagedCallersOnly(EntryPoint = "RunUpdate")]
        public static int RunUpdate()
        {
            Console.WriteLine($"[C# Handler] Running Update() on all scripts");

            // Update all script behaviors
            foreach (BaseEntity? script in _engine?.CurrentInstances?.Values ?? Enumerable.Empty<BaseEntity?>())
            {
                script?.Update();
            }

            return 100; // Return execution status code back to C++
        }

        [UnmanagedCallersOnly(EntryPoint = "InstantiateScript")]
        public static int InstantiateScript(IntPtr arg, int argLength)
        {
            if (argLength < Marshal.SizeOf<InstantiateConfig>()) return -1;

            // Marshal configuration out of native memory
            InstantiateConfig config = Marshal.PtrToStructure<InstantiateConfig>(arg);

            Console.WriteLine($"[C# Handler] Instantiating script: {Path.GetFileNameWithoutExtension(config.ScriptName)}");

            _engine?.InstantiateScript(config.EntityID, Path.GetFileNameWithoutExtension(config.ScriptName));

            return 100; // Return execution status code back to C++
        }
    }
}
