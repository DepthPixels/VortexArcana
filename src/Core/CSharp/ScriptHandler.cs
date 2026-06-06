using System;
using ScriptHost;
using System.Runtime.InteropServices;
using SharedInterface;

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
            foreach (IScriptBehavior? script in _engine?.CurrentInstances ?? Enumerable.Empty<IScriptBehavior?>())
            {
                script?.Update();
            }

            return 100; // Return execution status code back to C++
        }
    }
}
