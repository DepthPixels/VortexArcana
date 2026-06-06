using Microsoft.CodeAnalysis;
using Microsoft.CodeAnalysis.CSharp;
using Microsoft.CodeAnalysis.CSharp.Syntax;
using Microsoft.CodeAnalysis.Scripting;
using System;
using System.IO;
using System.Linq;
using System.Reflection;
using System.Runtime.CompilerServices;
using System.Runtime.Loader;
using VortexArcana;

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
                            script.SaveState(SavedStates![script.ScriptInstancePtr]);
                            Console.WriteLine($"Saved state for EntityID: {script.ScriptInstancePtr} of type {script.GetType().Name}");
                            Console.WriteLine("Saved States Status:");
                            foreach ((IntPtr id, Dictionary<string, object> states) in SavedStates)
                            {
                                foreach ((string varname, object value) in states)
                                {
                                    Console.WriteLine($"   {varname}: {value}");
                                }
                            }
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

            Console.WriteLine($"Found {publicFields.Count} public fields to inject into Save/ReloadState methods.");

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
            SyntaxList<StatementSyntax> saveStatements = SyntaxFactory.List<StatementSyntax>();
            foreach (FieldDeclarationSyntax field in publicFields) {
                ElementAccessExpressionSyntax mapAccess = SyntaxFactory.ElementAccessExpression(
                    SyntaxFactory.IdentifierName("SaveMap"),
                    SyntaxFactory.BracketedArgumentList(
                        SyntaxFactory.SingletonSeparatedList<ArgumentSyntax>(
                            SyntaxFactory.Argument(
                                SyntaxFactory.LiteralExpression(
                                    SyntaxKind.StringLiteralExpression,
                                    SyntaxFactory.Literal(field.Declaration.Variables.First().Identifier.ValueText)
                                )
                            )
                        )
                    )
                );
                ExpressionSyntax assignmentExpr = SyntaxFactory.AssignmentExpression(
                    SyntaxKind.SimpleAssignmentExpression,
                    mapAccess,
                    SyntaxFactory.IdentifierName(field.Declaration.Variables.First().Identifier)
                );
                saveStatements = saveStatements.Add(SyntaxFactory.ExpressionStatement(assignmentExpr));
            }
            saveStatements = saveStatements.Add(printSaveStatement);

            MethodDeclarationSyntax saveState = SyntaxFactory.MethodDeclaration(
                SyntaxFactory.PredefinedType(SyntaxFactory.Token(SyntaxKind.VoidKeyword)),
                "SaveState"
            ).WithParameterList(SyntaxFactory.ParameterList(
                SyntaxFactory.SeparatedList<ParameterSyntax>(new[]
                {
                    SyntaxFactory.Parameter(SyntaxFactory.Identifier("SaveMap")).WithType(SyntaxFactory.ParseTypeName("Dictionary<string, object>"))
                })
            ))
            .WithBody(SyntaxFactory.Block(saveStatements))
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
            SyntaxList<StatementSyntax> reloadStatements = SyntaxFactory.List<StatementSyntax>();
            foreach (FieldDeclarationSyntax field in publicFields)
            {
                ElementAccessExpressionSyntax mapAccess = SyntaxFactory.ElementAccessExpression(
                    SyntaxFactory.IdentifierName("ReloadMap"),
                    SyntaxFactory.BracketedArgumentList(
                        SyntaxFactory.SingletonSeparatedList<ArgumentSyntax>(
                            SyntaxFactory.Argument(
                                SyntaxFactory.LiteralExpression(
                                    SyntaxKind.StringLiteralExpression,
                                    SyntaxFactory.Literal(field.Declaration.Variables.First().Identifier.ValueText)
                                )
                            )
                        )
                    )
                );
                ExpressionSyntax assignmentExpr = SyntaxFactory.AssignmentExpression(
                    SyntaxKind.SimpleAssignmentExpression,
                    SyntaxFactory.IdentifierName(field.Declaration.Variables.First().Identifier),
                    SyntaxFactory.CastExpression(SyntaxFactory.ParseTypeName(field.Declaration.Type.ToString()), mapAccess)
                );

                ExpressionSyntax tryGetValueCall = SyntaxFactory.InvocationExpression(
                    SyntaxFactory.MemberAccessExpression(
                        SyntaxKind.SimpleMemberAccessExpression,
                        SyntaxFactory.IdentifierName("ReloadMap"),
                        SyntaxFactory.IdentifierName("TryGetValue")
                    )
                )
                .WithArgumentList(
                    SyntaxFactory.ArgumentList(
                        SyntaxFactory.SeparatedList<ArgumentSyntax>(
                            new SyntaxNodeOrToken[]
                            {
                                // First argument: key
                                SyntaxFactory.Argument(SyntaxFactory.LiteralExpression(
                                    SyntaxKind.StringLiteralExpression,
                                    SyntaxFactory.Literal(field.Declaration.Variables.First().Identifier.ValueText)
                                )),
                                SyntaxFactory.Token(SyntaxKind.CommaToken),
                                // Second argument: out var value
                                SyntaxFactory.Argument(
                                    SyntaxFactory.DeclarationExpression(
                                        SyntaxFactory.IdentifierName(
                                            SyntaxFactory.Identifier(
                                                SyntaxFactory.TriviaList(),
                                                SyntaxKind.VarKeyword,
                                                "var",
                                                "var",
                                                SyntaxFactory.TriviaList()
                                            )
                                        ),
                                        SyntaxFactory.SingleVariableDesignation(SyntaxFactory.Identifier("value"))
                                    )
                                )
                                .WithRefOrOutKeyword(SyntaxFactory.Token(SyntaxKind.OutKeyword))
                            }
                        )
                    )
                );
                ExpressionStatementSyntax printVarStatement = SyntaxFactory.ExpressionStatement(
                    SyntaxFactory.InvocationExpression(
                        SyntaxFactory.MemberAccessExpression(
                            SyntaxKind.SimpleMemberAccessExpression,
                            SyntaxFactory.IdentifierName("Console"),
                            SyntaxFactory.IdentifierName("WriteLine")
                        ),
                        SyntaxFactory.ArgumentList(
                            SyntaxFactory.SingletonSeparatedList<ArgumentSyntax>(
                                SyntaxFactory.Argument(SyntaxFactory.IdentifierName(field.Declaration.Variables.First().Identifier))
                            )
                        )
                    )
                );
                IfStatementSyntax ifStatement = SyntaxFactory.IfStatement(
                    tryGetValueCall, // The expression block generated above
                    SyntaxFactory.Block(
                        SyntaxFactory.SingletonList<StatementSyntax>(
                            SyntaxFactory.ExpressionStatement(assignmentExpr)
                        ).Add(printVarStatement)
                    )
                );

                
                reloadStatements = reloadStatements.Add(ifStatement);
            }
            reloadStatements = reloadStatements.Add(printReloadStatement);

            /* Test Print
            Console.WriteLine("Generated ReloadState method with the following statements:");
            foreach (StatementSyntax statement in reloadStatements)
            {
                Console.WriteLine(statement.ToString());
            }
            */

            MethodDeclarationSyntax reloadState = SyntaxFactory.MethodDeclaration(
                SyntaxFactory.PredefinedType(SyntaxFactory.Token(SyntaxKind.VoidKeyword)),
                "ReloadState"
            ).WithParameterList(SyntaxFactory.ParameterList(
                SyntaxFactory.SeparatedList<ParameterSyntax>(new[]
                {
                    SyntaxFactory.Parameter(SyntaxFactory.Identifier("ReloadMap")).WithType(SyntaxFactory.ParseTypeName("Dictionary<string, object>"))
                })
            ))
            .WithBody(SyntaxFactory.Block(reloadStatements))
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
                    CurrentInstances[EntityID]!.ReloadState(SavedStates![EntityID]);
                    Console.WriteLine($"Reloaded state for EntityID: {CurrentInstances[EntityID]!.ScriptInstancePtr} of type {CurrentInstances[EntityID]!.GetType().Name}");
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
                SavedStates![EntityID] = new Dictionary<string, object>();
                Console.WriteLine("[C# Host] Instantiated script: " + scriptName + " with EntityID: " + EntityID);
            }
        }

        public void DecimateScript(IntPtr EntityID)
        {
            if (CurrentInstances != null && CurrentInstances.TryGetValue(EntityID, out BaseEntity? script))
            {
                CurrentInstances.Remove(EntityID);
                SavedStates!.Remove(EntityID);
            }
        }
    }
}