using SharedInterface;
using System;
using System.IO;
using System.Linq;
using System.Reflection;
using Microsoft.CodeAnalysis;
using Microsoft.CodeAnalysis.CSharp;
using System.Runtime.Loader;

namespace ScriptHost
{
    public class CollectibleAssemblyContext : AssemblyLoadContext
    {
        public CollectibleAssemblyContext() : base(isCollectible: true) { }

        protected override Assembly? Load(AssemblyName assemblyName)
        {
            // Look for loaded assemblies (like hostfxr loaded ScriptHandler)
            foreach (var loadedAssembly in AppDomain.CurrentDomain.GetAssemblies())
            {
                if (loadedAssembly.GetName().Name == assemblyName.Name)
                {
                    return loadedAssembly;
                }
            }

            return null;
        }
    }

    public class ScriptHotReloadEngine
    {
        private readonly string _projectPath;
        private CollectibleAssemblyContext? _loadContext;
        private Dictionary<string, Type> _scriptTypes = new Dictionary<string, Type>();
        public Dictionary<IntPtr, IScriptBehavior?>? CurrentInstances = new Dictionary<IntPtr, IScriptBehavior?>();

        FileSystemWatcher watcher;
        private static Timer _debounceTimer;
        private static readonly TimeSpan DebounceDelay = TimeSpan.FromMilliseconds(500);

        public ScriptHotReloadEngine(string projectPath)
        {
            _projectPath = projectPath;
            SetupFileWatcher();
            Reload();
        }

        private void SetupFileWatcher()
        {
            watcher = new FileSystemWatcher(_projectPath)
            {
                Filter = "*.cs",
                EnableRaisingEvents = true,
                IncludeSubdirectories = true
            };

            watcher.Changed += (s, e) =>
            {
                _debounceTimer?.Dispose();
                _debounceTimer = new Timer(TimerCallback, e.FullPath, DebounceDelay, Timeout.InfiniteTimeSpan);
            };
        }

        private void TimerCallback(object state)
        {
            string filepath = (string)state;

            if (IsFileReady(filepath))
            {
                Console.WriteLine("Change detected! Hot reloading...");
                Reload();
            }
        }
        private static bool IsFileReady(string filename)
        {
            try
            {
                using var inputStream = File.Open(filename, FileMode.Open, FileAccess.Read, FileShare.None);
                return inputStream.Length > 0;
            }
            catch (IOException)
            {
                return false; // File is still locked by the OS / writing process
            }
        }

        public void Reload()
        {
            try
            {

                List<SyntaxTree> syntaxTrees = new List<SyntaxTree>();
                foreach (var file in Directory.GetFiles(Path.GetDirectoryName(_projectPath)!, "*.cs", SearchOption.AllDirectories))
                {
                    Console.WriteLine($"Compiling file: {file}");
                    string code = File.ReadAllText(file);

                    // Setup syntax tree and references
                    syntaxTrees.Add(CSharpSyntaxTree.ParseText(code));
                }

                var references = AppDomain.CurrentDomain.GetAssemblies()
                    .Where(a => !a.IsDynamic && !string.IsNullOrEmpty(a.Location))
                    .Select(a => MetadataReference.CreateFromFile(a.Location))
                    .Cast<MetadataReference>()
                    .ToList();

                // Rosyln Compilation
                var compilation = CSharpCompilation.Create(
                    $"ScriptAssembly_{Guid.NewGuid():N}",
                    syntaxTrees,
                    references,
                    new CSharpCompilationOptions(OutputKind.DynamicallyLinkedLibrary));

                using var peStream = new MemoryStream();
                var result = compilation.Emit(peStream);

                if (!result.Success)
                {
                    Console.WriteLine("Compilation Failed:");
                    foreach (var diagnostic in result.Diagnostics.Where(d => d.Severity == DiagnosticSeverity.Error))
                    {
                        Console.WriteLine($"   {diagnostic.GetMessage()}");
                    }
                    return;
                }

                peStream.Seek(0, SeekOrigin.Begin);

                // Free memory
                Dictionary<IntPtr, string> oldInstances = new Dictionary<IntPtr, string>();
                if (_loadContext != null)
                {
                    foreach (IScriptBehavior? script in CurrentInstances?.Values ?? Enumerable.Empty<IScriptBehavior?>())
                    {
                        if (script != null)
                        {
                            oldInstances[script.ScriptInstancePtr] = script.GetType().Name;
                        }
                    }
                    CurrentInstances = null;
                    _loadContext.Unload();
                    GC.Collect();
                    GC.WaitForPendingFinalizers();
                }

                // Load new AssemblyLoadContext
                _loadContext = new CollectibleAssemblyContext();
                Assembly assembly = _loadContext.LoadFromStream(peStream);

                _scriptTypes.Clear();
                // Look for usage of IScriptBehavior
                var scriptBehaviors = assembly.GetTypes().Where(t => typeof(IScriptBehavior).IsAssignableFrom(t));
                foreach (var type in scriptBehaviors)
                {
                    if (type != null)
                    {
                        _scriptTypes[type.Name] = type;
                    }
                }
                ReinitializeInstances(oldInstances);
            }
            catch (Exception ex)
            {
                Console.WriteLine($"Error during hot reload: {ex.Message}");
            }
        }

        public void ReinitializeInstances(Dictionary<IntPtr, string> oldInstances)
        {
            CurrentInstances = new Dictionary<IntPtr, IScriptBehavior?>();
            int count = 0;
            foreach ((IntPtr EntityID, string typename) in oldInstances)
            {
                if (_scriptTypes.TryGetValue(typename, out Type? type))
                {
                    CurrentInstances[EntityID] = (IScriptBehavior?)Activator.CreateInstance(type);
                    CurrentInstances[EntityID]!.ScriptInstancePtr = EntityID;
                    count += 1;
                }
            }
            Console.WriteLine($"Hot Reload successful! Reloaded {count} script instances.");
        }

        public void InstantiateScript(IntPtr EntityID, string scriptName)
        {
            if (CurrentInstances != null && _scriptTypes.TryGetValue(scriptName, out Type? type))
            {
                CurrentInstances[EntityID] = (IScriptBehavior?)Activator.CreateInstance(type);
                CurrentInstances[EntityID]!.ScriptInstancePtr = EntityID;
                Console.WriteLine("[C# Host] Instantiated script: " + scriptName + " with EntityID: " + EntityID);
            }
        }

        public void DecimateScript(IntPtr EntityID)
        {
            if (CurrentInstances != null && CurrentInstances.TryGetValue(EntityID, out IScriptBehavior? script))
            {
                CurrentInstances.Remove(EntityID);
            }
        }
    }
}