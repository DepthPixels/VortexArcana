using ScriptHost;
using System;
using System.Reflection;
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
            public int physics2Doffset;
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

            BridgeAPI._Engine_Physics2DOffset = config.physics2Doffset;

            _engine = new ScriptHotReloadEngine(config.ScriptDirectory);

            NativeLibrary.SetDllImportResolver(typeof(BridgeAPI).Assembly, ResolveHostProcess);

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

        [UnmanagedCallersOnly(EntryPoint = "RunPhysUpdate")]
        public static int RunPhysUpdate(float deltaTime)
        {
            Console.WriteLine($"[C# Handler] Running Phys Update() on all scripts");

            // Update all script behaviors
            foreach (BaseEntity? script in _engine?.CurrentInstances?.Values ?? Enumerable.Empty<BaseEntity?>())
            {
                script?.PhysUpdate(deltaTime);
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

        private static IntPtr ResolveHostProcess(string libraryName, Assembly assembly, DllImportSearchPath? searchPath)
        {
            // If .NET is looking for LibraryImport string
            if (libraryName == "HostProcess")
            {
                // Return the memory handle of the main C++ executable
                return NativeLibrary.GetMainProgramHandle();
            }

            return IntPtr.Zero;
        }
    }        

    public static partial class BridgeAPI
    {
        public static int _Engine_Physics2DOffset;

        // Entity
        [LibraryImport("HostProcess", EntryPoint = "Entity_GetComponentBridge")]
        internal static partial IntPtr Entity_GetComponentBridge(IntPtr entityID, int componentID);

        [LibraryImport("HostProcess", EntryPoint = "Entity_GetComponentsBridge")]
        internal static partial IntPtr Entity_GetComponentsBridge(IntPtr entityID, int componentID, out int length);


        // SpriteRenderer2D
        [LibraryImport("HostProcess", StringMarshalling = StringMarshalling.Utf8, EntryPoint = "SpriteRenderer2D_LoadSpriteBridge")]
        internal static partial void SpriteRenderer2D_LoadSpriteBridge(IntPtr componentPtr, string location, [MarshalAs(UnmanagedType.U1)] bool alpha);

        [LibraryImport("HostProcess", EntryPoint = "Physics2D_UpdateBridge")]
        internal static partial void Physics2D_UpdateBridge(IntPtr componentPtr, float deltaTime);
    }

    internal static class ComponentRegistryGateway
    {
        // A delegate that takes a Type and returns the Integer ID
        public static Func<Type, int> LookupId;
    }

    internal static class ComponentIdCache<T> where T : BaseComponent
    {
        public static readonly int Id;

        static ComponentIdCache()
        {
            if (ComponentRegistryGateway.LookupId == null)
            {
                throw new InvalidOperationException("Component Gateway hasn't been init.");
            }

            Id = ComponentRegistryGateway.LookupId(typeof(T));
        }
    }
}
