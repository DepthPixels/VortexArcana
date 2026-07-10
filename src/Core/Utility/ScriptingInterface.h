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
#include <cstddef>

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
    int physics2DOffset;
};
struct InstantiateConfig {
	int* EntityID;
    const char_t* ScriptName;
};

namespace {
    // Globals to hold hostfxr exports
    hostfxr_initialize_for_dotnet_command_line_fn init_for_cmd_line_fptr;
    hostfxr_initialize_for_runtime_config_fn init_for_config_fptr;
    hostfxr_get_runtime_delegate_fn get_delegate_fptr;
    hostfxr_close_fn close_fptr;
    load_assembly_and_get_function_pointer_fn load_assembly_and_get_function_pointer;

    // HostFXR Context
    hostfxr_handle cxt;

    // Bridge Functions
    typedef int (CORECLR_DELEGATE_CALLTYPE* initialize_handler_fn)(HostConfig* args, int sizeBytes);
    initialize_handler_fn initialize_handler;

    typedef int (CORECLR_DELEGATE_CALLTYPE* run_script_fn)(HostConfig* args, int sizeBytes);
    run_script_fn run_script;

    typedef int (CORECLR_DELEGATE_CALLTYPE* run_update_fn)();
    run_update_fn run_update;

    typedef int (CORECLR_DELEGATE_CALLTYPE* instantiate_script_fn)(InstantiateConfig* args, int sizeBytes);
    instantiate_script_fn instantiate_script;

    // Forward declarations
    bool load_hostfxr(const char_t* app);
    load_assembly_and_get_function_pointer_fn get_dotnet_load_assembly(const char_t* assembly);

    int init_bridge(const string_t& root_path);
	void update_all_scripts_through_bridge();
	void instantiate_script_through_bridge(int* entityID, const string_t& scriptName);
	void exit_bridge();
}

namespace {
    int init_bridge(const string_t& root_path)
    {
        const string_t app_path = root_path + STR("ScriptHandler.runtimeconfig.json");

        if (!load_hostfxr(app_path.c_str()))
        {
            assert(false && "Failure: load_hostfxr()");
            return EXIT_FAILURE;
        }

        // Load .NET Core
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
        rc = load_assembly_and_get_function_pointer(
            dllPath.c_str(),
            STR("ScriptEngine.ScriptHandler, ScriptHandler"),
            STR("InitializeHandler"),
            UNMANAGEDCALLERSONLY_METHOD,
            nullptr, (void**)&initialize_handler);
        assert(rc == 0 && initialize_handler != nullptr && "Failure: get_function_pointer()");

        // Function pointer to ScriptHandler.RunScript
        rc = load_assembly_and_get_function_pointer(
            dllPath.c_str(),
            STR("ScriptEngine.ScriptHandler, ScriptHandler"),
            STR("RunScript"),
            UNMANAGEDCALLERSONLY_METHOD,
            nullptr, (void**)&run_script);
        assert(rc == 0 && run_script != nullptr && "Failure: get_function_pointer()");

        // Function pointer to ScriptHandler.RunUpdate
        rc = load_assembly_and_get_function_pointer(
            dllPath.c_str(),
            STR("ScriptEngine.ScriptHandler, ScriptHandler"),
            STR("RunUpdate"),
            UNMANAGEDCALLERSONLY_METHOD,
            nullptr, (void**)&run_update);
        assert(rc == 0 && run_update != nullptr && "Failure: get_function_pointer()");

        // Function pointer to ScriptHandler.InstantiateScript
        rc = load_assembly_and_get_function_pointer(
            dllPath.c_str(),
            STR("ScriptEngine.ScriptHandler, ScriptHandler"),
            STR("InstantiateScript"),
            UNMANAGEDCALLERSONLY_METHOD,
            nullptr, (void**)&instantiate_script);
        assert(rc == 0 && instantiate_script != nullptr && "Failure: get_function_pointer()");

        // Execution
        string_t scriptDir = root_path + STR("DebugScripts\\");
        HostConfig config{ scriptDir.c_str(), offsetof(Vortex::Physics2D, velocity) };
        initialize_handler(&config, sizeof(config));

        return EXIT_SUCCESS;
    }

    void update_all_scripts_through_bridge()
    {
        run_update();
    }

    void instantiate_script_through_bridge(int* entityID, const string_t& scriptName)
    {
        InstantiateConfig config{ entityID, scriptName.c_str() };
        instantiate_script(&config, sizeof(config));
    }

    void exit_bridge()
    {
        // Any necessary cleanup can be performed here.
        close_fptr(cxt);
    }

    // --------------------------------------------------
    // Component Stuff
    // --------------------------------------------------
    
    // Entity
    
    // GetComponent<T>() Wrappers
    typedef void* (*ComponentGetter)(int* entityID);

	void* GetSpriteRenderer2DWrapper(int* entityID) {
		Vortex::Entity* entity = reinterpret_cast<Vortex::Entity*>(entityID);
		return entity->GetComponent<Vortex::SpriteRenderer2D>();
	}
    
	void* GetPhysics2DWrapper(int* entityID) {
		Vortex::Entity* entity = reinterpret_cast<Vortex::Entity*>(entityID);
		return entity->GetComponent<Vortex::Physics2D>();
	}

    // Registry
    std::map<int, ComponentGetter> getComponentRegistry = {  // Maps component IDs to their corresponding wrappers.
        {1, GetSpriteRenderer2DWrapper},
        {2, GetPhysics2DWrapper}
    };

	// GetComponent<T>() Bridge Function
    extern "C" __declspec(dllexport) inline void* Entity_GetComponentBridge(int* entityID, int componentID) {
		if (getComponentRegistry.find(componentID) != getComponentRegistry.end()) {
			std::cout << "Physics2D found at: " << getComponentRegistry[componentID](entityID) << std::endl;
			std::cout << "Location of Velocity: " << &(reinterpret_cast<Vortex::Physics2D*>(getComponentRegistry[componentID](entityID))->Velocity()) << std::endl;
			return getComponentRegistry[componentID](entityID);
		}
        return nullptr;
    }

	// GetComponents<T>() Wrappers
    typedef std::vector<void*>* (*ComponentsGetter)(int* entityID);
    
    std::vector<void*>* GetAllSpriteRenderer2DWrapper(int* entityID) {
        Vortex::Entity* entity = reinterpret_cast<Vortex::Entity*>(entityID);
        std::vector<Vortex::SpriteRenderer2D*> components = entity->GetComponents<Vortex::SpriteRenderer2D>();
        if (components.size() == 0) {
            return nullptr;
        }
        else {
            std::vector<void*> return_vector;
            return_vector.reserve(components.size());

			std::transform(components.begin(), components.end(), std::back_inserter(return_vector),
				[](Vortex::SpriteRenderer2D* comp) { return static_cast<void*>(comp); });

            return &return_vector;
        }
    }

    std::vector<void*>* GetAllPhysics2DWrapper(int* entityID) {
        Vortex::Entity* entity = reinterpret_cast<Vortex::Entity*>(entityID);
        std::vector<Vortex::Physics2D*> components = entity->GetComponents<Vortex::Physics2D>();

        std::vector<void*> return_vector;
        return_vector.reserve(components.size());

        std::transform(components.begin(), components.end(), std::back_inserter(return_vector),
            [](Vortex::Physics2D* comp) { return static_cast<void*>(comp); });

        return &return_vector;
    }

    std::map<int, ComponentsGetter> getComponentsRegistry = {  // Maps component IDs to their corresponding wrappers.
        {1, GetAllSpriteRenderer2DWrapper},
        {2, GetAllPhysics2DWrapper}
    };

    extern "C" __declspec(dllexport) inline void* Entity_GetComponentsBridge(int* entityID, int componentID, int* length) {
        if (getComponentRegistry.find(componentID) != getComponentRegistry.end()) {

            std::vector<void*>* components = getComponentsRegistry[componentID](entityID);

            *length = components->size();

			return components->data();
        }
        return nullptr;
    }

    // SpriteRenderer2D

    extern "C" __declspec(dllexport) inline void SpriteRenderer2D_LoadSpriteBridge(int* componentPtr, const char* location, bool alpha) {
        if (componentPtr == nullptr) return;

		Vortex::SpriteRenderer2D* component = reinterpret_cast<Vortex::SpriteRenderer2D*>(componentPtr);
		component->LoadSprite(location, alpha);
    }

    // Physics2D

    extern "C" __declspec(dllexport) inline void Physics2D_UpdateBridge(int* componentPtr, float deltaTime) {
        if (componentPtr == nullptr) return;

		Vortex::Physics2D* component = reinterpret_cast<Vortex::Physics2D*>(componentPtr);
		component->Update(deltaTime);
    }
}

// Loading .NET Core and activating it.
namespace {
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