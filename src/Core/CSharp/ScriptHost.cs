using VortexArcana;
using System;
using System.IO;
using System.Linq;
using System.Reflection;
using Microsoft.CodeAnalysis;
using Microsoft.CodeAnalysis.CSharp;
using System.Runtime.Loader;
using Microsoft.CodeAnalysis.CSharp.Syntax;
using System.Runtime.CompilerServices;

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
        public Dictionary<IntPtr, BaseEntity?>? CurrentInstances = new Dictionary<IntPtr, BaseEntity?>();
        public Dictionary<IntPtr, Dictionary<string, object>>? SavedStates = new Dictionary<IntPtr, Dictionary<string, object>>();

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

                    syntaxTrees.Add(InjectSaveReloadState(CSharpSyntaxTree.ParseText(code)));
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
                    foreach (BaseEntity? script in CurrentInstances?.Values ?? Enumerable.Empty<BaseEntity?>())
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
                // Look for usage of BaseEntity
                var scriptBehaviors = assembly.GetTypes().Where(t => typeof(BaseEntity).IsAssignableFrom(t));
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

        public SyntaxTree InjectSaveReloadState(SyntaxTree syntaxTree)
        {
            CompilationUnitSyntax root = syntaxTree.GetRoot() as CompilationUnitSyntax;
            // Add Using Directive for System.Collections.Generic if not present
            UsingDirectiveSyntax usingDirective = SyntaxFactory.UsingDirective(SyntaxFactory.ParseName("System.Collections.Generic"));
            SyntaxList<UsingDirectiveSyntax> alreadyUsing = root.Usings;
            if (!alreadyUsing.Any(m => m.Name.ToString() == "System.Collections.Generic"))
            {
                root = root.AddUsings(usingDirective);
            }

            // Get class declaration
            ClassDeclarationSyntax classDec = root.DescendantNodes().OfType<ClassDeclarationSyntax>().First();
            // Get all public variables
            List<FieldDeclarationSyntax> publicFields = classDec.DescendantNodes().OfType<FieldDeclarationSyntax>()
                .Where(f => f.Modifiers.Any(m => m.IsKind(SyntaxKind.PublicKeyword))).ToList();

            // Save State
            // Test Print Statement
            ExpressionStatementSyntax printSaveStatement = SyntaxFactory.ExpressionStatement(
                SyntaxFactory.InvocationExpression(
                    SyntaxFactory.MemberAccessExpression(
                        SyntaxKind.SimpleMemberAccessExpression,
                        SyntaxFactory.IdentifierName("Console"),
                        SyntaxFactory.IdentifierName("WriteLine")
                    ),
                    SyntaxFactory.ArgumentList(
                        SyntaxFactory.SingletonSeparatedList<ArgumentSyntax>(
                            SyntaxFactory.Argument(
                                SyntaxFactory.LiteralExpression(
                                    SyntaxKind.StringLiteralExpression,
                                    SyntaxFactory.Literal("SaveState called!")
                                )
                            )
                        )
                    )
                )
            );

            MethodDeclarationSyntax saveState = SyntaxFactory.MethodDeclaration(
                SyntaxFactory.PredefinedType(SyntaxFactory.Token(SyntaxKind.VoidKeyword)),
                "SaveState"
            ).WithParameterList(SyntaxFactory.ParameterList(
                SyntaxFactory.SeparatedList<ParameterSyntax>(new[]
                {
                    SyntaxFactory.Parameter(SyntaxFactory.Identifier("SaveMap")).WithType(SyntaxFactory.ParseTypeName("Dictionary<string, object>"))
                })
            ))
            .WithBody(SyntaxFactory.Block(printSaveStatement))
            .AddModifiers(SyntaxFactory.Token(SyntaxKind.PublicKeyword), SyntaxFactory.Token(SyntaxKind.OverrideKeyword));

            // Reload State
            // Test Print Statement
            ExpressionStatementSyntax printReloadStatement = SyntaxFactory.ExpressionStatement(
                SyntaxFactory.InvocationExpression(
                    SyntaxFactory.MemberAccessExpression(
                        SyntaxKind.SimpleMemberAccessExpression,
                        SyntaxFactory.IdentifierName("Console"),
                        SyntaxFactory.IdentifierName("WriteLine")
                    ),
                    SyntaxFactory.ArgumentList(
                        SyntaxFactory.SingletonSeparatedList<ArgumentSyntax>(
                            SyntaxFactory.Argument(
                                SyntaxFactory.LiteralExpression(
                                    SyntaxKind.StringLiteralExpression,
                                    SyntaxFactory.Literal("ReloadState called!")
                                )
                            )
                        )
                    )
                )
            );
            MethodDeclarationSyntax reloadState = SyntaxFactory.MethodDeclaration(
                SyntaxFactory.PredefinedType(SyntaxFactory.Token(SyntaxKind.VoidKeyword)),
                "ReloadState"
            ).WithParameterList(SyntaxFactory.ParameterList(
                SyntaxFactory.SeparatedList<ParameterSyntax>(new[]
                {
                    SyntaxFactory.Parameter(SyntaxFactory.Identifier("ReloadMap")).WithType(SyntaxFactory.ParseTypeName("Dictionary<string, object>"))
                })
            ))
            .WithBody(SyntaxFactory.Block(printReloadStatement))
            .AddModifiers(SyntaxFactory.Token(SyntaxKind.PublicKeyword), SyntaxFactory.Token(SyntaxKind.OverrideKeyword));

            ClassDeclarationSyntax modifiedClass = classDec.AddMembers(new[] { saveState, reloadState });
            root = root.ReplaceNode(classDec, modifiedClass);
            return SyntaxFactory.SyntaxTree(root);
        }

        public void ReinitializeInstances(Dictionary<IntPtr, string> oldInstances)
        {
            CurrentInstances = new Dictionary<IntPtr, BaseEntity?>();
            int count = 0;
            foreach ((IntPtr EntityID, string typename) in oldInstances)
            {
                if (_scriptTypes.TryGetValue(typename, out Type? type))
                {
                    CurrentInstances[EntityID] = (BaseEntity?)Activator.CreateInstance(type);
                    CurrentInstances[EntityID]!.ScriptInstancePtr = EntityID;
                    SavedStates[EntityID] = new Dictionary<string, object>();
                    count += 1;
                }
            }
            Console.WriteLine($"Hot Reload successful! Reloaded {count} script instances.");
        }

        public void InstantiateScript(IntPtr EntityID, string scriptName)
        {
            if (CurrentInstances != null && _scriptTypes.TryGetValue(scriptName, out Type? type))
            {
                CurrentInstances[EntityID] = (BaseEntity?)Activator.CreateInstance(type);
                CurrentInstances[EntityID]!.ScriptInstancePtr = EntityID;
                SavedStates[EntityID] = new Dictionary<string, object>();
                Console.WriteLine("[C# Host] Instantiated script: " + scriptName + " with EntityID: " + EntityID);
            }
        }

        public void DecimateScript(IntPtr EntityID)
        {
            if (CurrentInstances != null && CurrentInstances.TryGetValue(EntityID, out BaseEntity? script))
            {
                CurrentInstances.Remove(EntityID);
            }
        }
    }
}