//
// Created by Gleb Krasilich on 03.10.2019.
//

#include <hostfxr.h>
#include <unistd.h>
#include <cstdio>
#include <dlfcn.h>

int main()
{
    hostfxr_handle host_fxr_hadle = nullptr;
    char_t exec_path[300];
    readlink("/proc/self/exe", exec_path, 299);
    hostfxr_initialize_parameters runtime_params = {sizeof(hostfxr_initialize_parameters), exec_path, "dotnet"};

    fprintf(stdout, exec_path);
    fprintf(stdout, "\n");

    void * hostfxr_library = dlopen("dotnet/host/fxr/3.0.0/libhostfxr.so", RTLD_NOW);
    if(hostfxr_library == nullptr)
    {
        fprintf(stderr, "Unable to load hostfxr! \n");
        return 1;
    }

    hostfxr_initialize_for_runtime_config_fn  hostfxr_initialize_for_runtime_config = (hostfxr_initialize_for_runtime_config_fn)dlsym(hostfxr_library,
            "hostfxr_initialize_for_runtime_config");

    if(hostfxr_initialize_for_runtime_config == nullptr)
    {
        fprintf(stderr, "Unable to locate hostfxr_initialize_for_runtime_config! \n");
        return 1;
    }

    hostfxr_initialize_for_runtime_config("GmodNET.runtimeconfig.json", &runtime_params, &host_fxr_hadle);
    if(host_fxr_hadle == nullptr)
    {
        fprintf(stderr, "Unable to init runtime! \n");
        return 1;
    }

    hostfxr_get_runtime_delegate_fn hostfxr_get_runtime_delegate = (hostfxr_get_runtime_delegate_fn)dlsym(hostfxr_library, "hostfxr_get_runtime_delegate");

    typedef int (*load_assembly_and_get_function_pointer_fn)(
            const char_t *assembly_path,
            const char_t *type_name,
            const char_t *method_name,
            const char_t *delegate_type_name,
            void         *reserved,
            /*out*/ void **delegate);

    load_assembly_and_get_function_pointer_fn load_assembly_and_get_function_pointer = nullptr;

    hostfxr_get_runtime_delegate(host_fxr_hadle, hdt_load_assembly_and_get_function_pointer, (void**)&load_assembly_and_get_function_pointer);
    if(load_assembly_and_get_function_pointer == nullptr)
    {
        fprintf(stderr, "Unable to get Runtime delegate! \n");
        return 1;
    }

    typedef void (*managed_function_fn)();
    managed_function_fn managed_function = nullptr;
    load_assembly_and_get_function_pointer("GmodNET.dll", "GmodNET.Startup, GmodNET", "Main", "GmodNET.MainDelegate, GmodNET",
                                           nullptr, (void**)&managed_function);
    if(managed_function == nullptr)
    {
        fprintf(stderr, "Unable to get managed function pointer! \n");
        return 1;
    }

    managed_function();

    hostfxr_close_fn hostfxr_close = (hostfxr_close_fn)dlsym(hostfxr_library, "hostfxr_close");
    hostfxr_close(host_fxr_hadle);

    dlclose(hostfxr_library);

    fprintf(stdout, "Execution was successful! \n");

    return 0;
}