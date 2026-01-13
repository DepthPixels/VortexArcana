#include "Core/Engine.h"

int main(int argc, char* argv[]) {
    // Create the engine instance
    Engine vortexArcana;

    // Initialize Video, Audio, Renderer, and Window.
    if (vortexArcana.Initialize()) {

        // Start the Main Loop.
        vortexArcana.Run();
    }

    // Cleanup before exiting if init fails.
    vortexArcana.Shutdown();

    return 0;
}