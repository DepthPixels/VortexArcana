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
        public List<IScriptBehavior?>? CurrentInstances { get; private set; } = new List<IScriptBehavior?>();

        public ScriptHotReloadEngine(string projectPath)
        {
            _projectPath = projectPath;
            SetupFileWatcher();
            Reload();
        }

        private void SetupFileWatcher()
        {
            var watcher = new FileSystemWatcher(Path.GetDirectoryName(_projectPath)!)
            {
                Filter = "*.cs",
                EnableRaisingEvents = true,
                IncludeSubdirectories = true
            };

            watcher.Changed += (s, e) =>
            {
                Console.WriteLine("Change detected! Hot reloading...");
                Reload();
            };
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
                if (_loadContext != null)
                {
                    CurrentInstances = null;
                    _loadContext.Unload();
                    GC.Collect();
                    GC.WaitForPendingFinalizers();
                }

                // Load new AssemblyLoadContext
                _loadContext = new CollectibleAssemblyContext();
                Assembly assembly = _loadContext.LoadFromStream(peStream);

                CurrentInstances = new List<IScriptBehavior?>();
                // Look for usage of IScriptBehavior
                int count = 0;
                var scriptBehaviors = assembly.GetTypes().Where(t => typeof(IScriptBehavior).IsAssignableFrom(t));
                foreach (var type in scriptBehaviors)
                {
                    if (type != null)
                    {
                        CurrentInstances.Add((IScriptBehavior?)Activator.CreateInstance(type));
                        count += 1;
                    }
                }
                Console.WriteLine($"Hot Reload successful! Reloaded {count} script instances.");
            }
            catch (Exception ex)
            {
                Console.WriteLine($"Error during hot reload: {ex.Message}");
            }
        }
    }
}