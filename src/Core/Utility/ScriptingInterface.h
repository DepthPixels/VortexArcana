#pragma once

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <chrono>
#include <iostream>
#include <thread>
#include <vector>

#include <nethost.h>
#include <coreclr_delegates.h>
#include <hostfxr.h>

#define WINDOWS

// Pathing and stuff
#ifdef WINDOWS
#include <Windows.h>

#define STR(s) L ## s
#define CH(c) L ## c
#define DIR_SEPARATOR L'\\'

#define string_compare wcscmp

#else
#include <dlfcn.h>
#include <limits.h>

#define STR(s) s
#define CH(c) c
#define DIR_SEPARATOR '/'
#define MAX_PATH PATH_MAX

#define string_compare strcmp

#endif

using string_t = std::basic_string<char_t>;

struct HostConfig {
    const char_t* ScriptDirectory;
};

namespace
{
    // Globals to hold hostfxr exports
    hostfxr_initialize_for_dotnet_command_line_fn init_for_cmd_line_fptr;
    hostfxr_initialize_for_runtime_config_fn init_for_config_fptr;
    hostfxr_get_runtime_delegate_fn get_delegate_fptr;
    hostfxr_close_fn close_fptr;

    // Forward declarations
    bool load_hostfxr(const char_t* app);
    load_assembly_and_get_function_pointer_fn get_dotnet_load_assembly(const char_t* assembly);

    int run_app_example(const string_t& root_path);
}

namespace
{
    int run_app_example(const string_t& root_path)
    {
        const string_t app_path = root_path + STR("ScriptHandler.runtimeconfig.json");

        if (!load_hostfxr(app_path.c_str()))
        {
            assert(false && "Failure: load_hostfxr()");
            return EXIT_FAILURE;
        }

        // Load .NET Core
        hostfxr_handle cxt = nullptr;
		int rc = init_for_config_fptr(app_path.c_str(), nullptr, &cxt);
        if (rc != 0 || cxt == nullptr)
        {
            std::cerr << "Init failed: " << std::hex << std::showbase << rc << std::endl;
            close_fptr(cxt);
            return EXIT_FAILURE;
        }

        // Get the function pointer to get function pointers
		load_assembly_and_get_function_pointer_fn load_assembly_and_get_function_pointer = nullptr;
        rc = get_delegate_fptr(
            cxt,
            hdt_load_assembly_and_get_function_pointer,
            (void**)&load_assembly_and_get_function_pointer);
        if (rc != 0 || load_assembly_and_get_function_pointer == nullptr)
        {
            std::cerr << "Get delegate failed: " << std::hex << std::showbase << rc << std::endl;
            close_fptr(cxt);
            return EXIT_FAILURE;
        }

		const string_t dllPath = root_path + STR("ScriptHandler.dll");

        // Function pointer to ScriptHandler.InitializeHandler
        typedef int (CORECLR_DELEGATE_CALLTYPE* initialize_handler_fn)(void* args, int sizeBytes);
        initialize_handler_fn initialize_handler;
        rc = load_assembly_and_get_function_pointer(
            dllPath.c_str(),
            STR("ScriptEngine.ScriptHandler, ScriptHandler"),
            STR("InitializeHandler"),
            UNMANAGEDCALLERSONLY_METHOD,
            nullptr, (void**)&initialize_handler);
        assert(rc == 0 && initialize_handler != nullptr && "Failure: get_function_pointer()");

        // Function pointer to ScriptHandler.RunScript
        typedef int (CORECLR_DELEGATE_CALLTYPE* run_script_fn)(void* args, int sizeBytes);
        run_script_fn run_script;
        rc = load_assembly_and_get_function_pointer(
            dllPath.c_str(),
            STR("ScriptEngine.ScriptHandler, ScriptHandler"),
            STR("RunScript"),
            UNMANAGEDCALLERSONLY_METHOD,
            nullptr, (void**)&run_script);
        assert(rc == 0 && run_script != nullptr && "Failure: get_function_pointer()");

        // Function pointer to ScriptHandler.RunUpdate
        typedef int (CORECLR_DELEGATE_CALLTYPE* run_update_fn)();
        run_update_fn run_update;
        rc = load_assembly_and_get_function_pointer(
            dllPath.c_str(),
            STR("ScriptEngine.ScriptHandler, ScriptHandler"),
            STR("RunUpdate"),
            UNMANAGEDCALLERSONLY_METHOD,
            nullptr, (void**)&run_update);
        assert(rc == 0 && run_update != nullptr && "Failure: get_function_pointer()");

        // Execution
		string_t scriptDir = root_path + STR("DebugScripts\\");
        HostConfig config{ scriptDir.c_str() };
		initialize_handler(&config, sizeof(config));
		int scriptResult = run_update();

        close_fptr(cxt);
        return EXIT_SUCCESS;
    }
}


// Loading .NET Core and activating it.
namespace
{
    // Forward declarations
    void* load_library(const char_t*);
    void* get_export(void*, const char*);

#ifdef WINDOWS
    void* load_library(const char_t* path)
    {
        HMODULE h = ::LoadLibraryW(path);
        assert(h != nullptr);
        return (void*)h;
    }
    void* get_export(void* h, const char* name)
    {
        void* f = ::GetProcAddress((HMODULE)h, name);
        assert(f != nullptr);
        return f;
    }
#else
    void* load_library(const char_t* path)
    {
        void* h = dlopen(path, RTLD_LAZY | RTLD_LOCAL);
        assert(h != nullptr);
        return h;
    }
    void* get_export(void* h, const char* name)
    {
        void* f = dlsym(h, name);
        assert(f != nullptr);
        return f;
    }
#endif

    // Using the nethost library, discover the location of hostfxr and get exports
    bool load_hostfxr(const char_t* assembly_path)
    {
        get_hostfxr_parameters params{ sizeof(get_hostfxr_parameters), assembly_path, nullptr };
        // Pre-allocate a large buffer for the path to hostfxr
        char_t buffer[MAX_PATH];
        size_t buffer_size = sizeof(buffer) / sizeof(char_t);
        int rc = get_hostfxr_path(buffer, &buffer_size, &params);
        if (rc != 0)
            return false;

        // Load hostfxr and get desired exports
        // NOTE: The .NET Runtime does not support unloading any of its native libraries. Running
        // dlclose/FreeLibrary on any .NET libraries produces undefined behavior.
        void* lib = load_library(buffer);
        init_for_cmd_line_fptr = (hostfxr_initialize_for_dotnet_command_line_fn)get_export(lib, "hostfxr_initialize_for_dotnet_command_line");
        init_for_config_fptr = (hostfxr_initialize_for_runtime_config_fn)get_export(lib, "hostfxr_initialize_for_runtime_config");
        get_delegate_fptr = (hostfxr_get_runtime_delegate_fn)get_export(lib, "hostfxr_get_runtime_delegate");
        close_fptr = (hostfxr_close_fn)get_export(lib, "hostfxr_close");

        return (init_for_config_fptr && get_delegate_fptr && close_fptr);
    }

    // Load and initialize .NET Core and get desired function pointer for scenario
    load_assembly_and_get_function_pointer_fn get_dotnet_load_assembly(const char_t* config_path)
    {
        // Load .NET Core
        void* load_assembly_and_get_function_pointer = nullptr;
        hostfxr_handle cxt = nullptr;
        int rc = init_for_config_fptr(config_path, nullptr, &cxt);
        if (rc != 0 || cxt == nullptr)
        {
            std::cerr << "Init failed: " << std::hex << std::showbase << rc << std::endl;
            close_fptr(cxt);
            return nullptr;
        }

        // Get the load assembly function pointer
        rc = get_delegate_fptr(
            cxt,
            hdt_load_assembly_and_get_function_pointer,
            &load_assembly_and_get_function_pointer);
        if (rc != 0 || load_assembly_and_get_function_pointer == nullptr)
            std::cerr << "Get delegate failed: " << std::hex << std::showbase << rc << std::endl;

        close_fptr(cxt);
        return (load_assembly_and_get_function_pointer_fn)load_assembly_and_get_function_pointer;
    }
}