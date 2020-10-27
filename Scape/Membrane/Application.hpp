#pragma once

#if GARLIC_PLATFORM_WINDOWS
    #ifdef EXPORT
        #define SCAPE_DLL __declspec(dllexport)
    #else
        #define SCAPE_DLL __declspec(dllimport)
    #endif
#else
    #define SCAPE_DLL
#endif

namespace stm {
    class Application;
}

extern "C" {
SCAPE_DLL stm::Application* application_create();
SCAPE_DLL void application_destroy(stm::Application* app);

SCAPE_DLL void application_start(stm::Application* app);

SCAPE_DLL void application_run(stm::Application* app);
}