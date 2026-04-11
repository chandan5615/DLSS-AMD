#include "tray_app.h"
#include "../common/logger.h"
#include <windows.h>
#include <iostream>

using namespace afglobal;

int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
    (void)hPrevInstance;
    (void)lpCmdLine;
    (void)nCmdShow;

    // Create and initialize tray app
    TrayApp app;
    
    if (!app.Initialize(hInstance)) {
        MessageBoxA(nullptr, "Failed to initialize AMD Global Frame Generator", "Error", MB_OK | MB_ICONERROR);
        return 1;
    }

    // Run the application message loop
    app.Run();

    return 0;
}
