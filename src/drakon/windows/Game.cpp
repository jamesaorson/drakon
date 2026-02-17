#if defined(WIN32) || defined(_WIN64)

#include <drakon/Game.h>

#include <iostream>

#include <Windows.h>

LRESULT CALLBACK WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    auto* game = reinterpret_cast<drakon::Game*>(GetWindowLongPtr(hWnd, GWLP_USERDATA));

    // NOTE: Handle all window messages and enqueue events
    switch (msg) {
    case WM_DESTROY:
        if (game != nullptr) {
            // game->enqueueEvent({ drakon::Event::Type::Quit });
        }
        PostQuitMessage(0);
        return 0;
    case WM_KEYDOWN:
        if (game != nullptr) {
            // game->enqueueEvent({ drakon::Event::Type::KeyDown, static_cast<std::int64_t>(wParam) });
        }
        break;
    case WM_KEYUP:
        if (game != nullptr) {
            // game->enqueueEvent({ drakon::Event::Type::KeyUp, static_cast<std::int64_t>(wParam) });
        }
        break;
    case WM_MOUSEMOVE:
        if (game != nullptr) {
            /*game->enqueueEvent({
                    drakon::Event::Type::MouseMove,
                    static_cast<std::int64_t>(GET_X_LPARAM(lParam)),
                    static_cast<std::int64_t>(GET_Y_LPARAM(lParam))
                    });*/
        }
        break;
    case WM_LBUTTONDOWN:
        if (game != nullptr) {
            // game->enqueueEvent({ drakon::Event::Type::MouseButtonDown, VK_LBUTTON });
        }
        break;
    case WM_LBUTTONUP:
        if (game != nullptr) {
            // game->enqueueEvent({ drakon::Event::Type::MouseButtonUp, VK_LBUTTON });
        }
        break;
    case WM_MOUSEWHEEL:
        if (game != nullptr) {
            // game->enqueueEvent({ drakon::Event::Type::MouseWheel, GET_WHEEL_DELTA_WPARAM(wParam) });
        }
        break;
    }
    return DefWindowProc(hWnd, msg, wParam, lParam);
}

int drakon::Game::makeWindow() {
    // Default main thread to CPU 0
    SetThreadAffinityMask(GetCurrentThread(), 0x1);

    HINSTANCE   hInstance = GetModuleHandle(nullptr);
    const char* className = "GdkWindowClass";

    WNDCLASSEXA wc   = {};
    wc.cbSize        = sizeof(wc);
    wc.lpfnWndProc   = WndProc;
    wc.hInstance     = hInstance;
    wc.lpszClassName = className;
    RegisterClassExA(&wc);

    HWND hwnd = CreateWindowExA(0,
                                className,
                                this->title.c_str(),
                                WS_OVERLAPPEDWINDOW,
                                CW_USEDEFAULT,
                                CW_USEDEFAULT,
                                static_cast<int>(this->windowWidth),
                                static_cast<int>(this->windowHeight),
                                nullptr,
                                nullptr,
                                hInstance,
                                nullptr);
    if (!hwnd) {
        return 1;
    }

    SetWindowLongPtr(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(this));
    ShowWindow(hwnd, SW_SHOW);

    this->windowHandle = hwnd;

    if (!this->renderer.init(this->windowHandle, this->windowWidth, this->windowHeight)) {
        std::cerr << "Failed to initialize renderer." << std::endl;
        return 1;
    }

    return 0;
}

void drakon::Game::cleanup() { this->renderer.cleanup(); }

void drakon::Game::processEvents() {
    MSG msg = {};
    // PeekMessage is non-blocking and drains the queue of events
    while (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE)) {
        if (msg.message == WM_QUIT) {
            this->isRunning = false;
            break;
        }
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
}
#endif
