#define UNICODE
#define _UNICODE
#include <windows.h>
#include <stdbool.h>
#include "interception.h"

#define BTN_LOCK 101
#define BTN_UNLOCK 102
#define STATUS_LABEL 103

#define MUTEX_NAME L"Global\\LaptopKeyboardDaemonMutex"
#define EVENT_STOP L"Global\\LaptopKeyboardDaemonStopEvent"

static HWND hStatus;

// Arka planda gorunmez calisan filtre servisi
void RunBackgroundDaemon() {
    HANDLE hMutex = CreateMutexW(NULL, TRUE, MUTEX_NAME);
    if (GetLastError() == ERROR_ALREADY_EXISTS) {
        return;
    }

    HANDLE hStop = CreateEventW(NULL, TRUE, FALSE, EVENT_STOP);

    InterceptionContext context = interception_create_context();
    if (!context) {
        CloseHandle(hStop);
        CloseHandle(hMutex);
        return;
    }

    interception_set_filter(context, interception_is_keyboard, INTERCEPTION_FILTER_KEY_ALL);

    InterceptionStroke stroke;
    InterceptionDevice device;

    while (WaitForSingleObject(hStop, 0) != WAIT_OBJECT_0) {
        device = interception_wait_with_timeout(context, 100);
        if (device > 0 && interception_receive(context, device, &stroke, 1) > 0) {
            if (device == 1) {
                continue; // Dahili klavye tusunu engelle
            }
            interception_send(context, device, &stroke, 1);
        }
    }

    interception_destroy_context(context);
    CloseHandle(hStop);
    CloseHandle(hMutex);
}

bool IsDaemonRunning() {
    HANDLE hMutex = OpenMutexW(SYNCHRONIZE, FALSE, MUTEX_NAME);
    if (hMutex != NULL) {
        CloseHandle(hMutex);
        return true;
    }
    return false;
}

void StopDaemon() {
    HANDLE hStop = OpenEventW(EVENT_MODIFY_STATE, FALSE, EVENT_STOP);
    if (hStop) {
        SetEvent(hStop);
        CloseHandle(hStop);
    }
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
    case WM_CREATE: {
        HFONT hFont = CreateFontW(16, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE, DEFAULT_CHARSET,
            OUT_OUTLINE_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY, VARIABLE_PITCH, L"Segoe UI");

        HWND hTitle = CreateWindowW(L"STATIC", L"Internal Keyboard Filter",
            WS_VISIBLE | WS_CHILD | SS_CENTER, 20, 15, 280, 20, hwnd, NULL, NULL, NULL);
        SendMessageW(hTitle, WM_SETFONT, (WPARAM)hFont, TRUE);

        HWND btnLock = CreateWindowW(L"BUTTON", L"Lock Keyboard",
            WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON, 20, 50, 130, 36, hwnd, (HMENU)BTN_LOCK, NULL, NULL);
        SendMessageW(btnLock, WM_SETFONT, (WPARAM)hFont, TRUE);

        HWND btnUnlock = CreateWindowW(L"BUTTON", L"Unlock Keyboard",
            WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON, 170, 50, 130, 36, hwnd, (HMENU)BTN_UNLOCK, NULL, NULL);
        SendMessageW(btnUnlock, WM_SETFONT, (WPARAM)hFont, TRUE);

        bool locked = IsDaemonRunning();
        hStatus = CreateWindowW(L"STATIC", locked ? L"Status: LOCKED" : L"Status: Ready (Unlocked)",
            WS_VISIBLE | WS_CHILD | SS_CENTER, 20, 98, 280, 20, hwnd, (HMENU)STATUS_LABEL, NULL, NULL);
        SendMessageW(hStatus, WM_SETFONT, (WPARAM)hFont, TRUE);
        break;
    }

    case WM_COMMAND: {
        switch (LOWORD(wParam)) {
        case BTN_LOCK:
            if (!IsDaemonRunning()) {
                // Onceki stop event kalintisi varsa temizle
                HANDLE hStop = OpenEventW(EVENT_MODIFY_STATE, FALSE, EVENT_STOP);
                if (hStop) {
                    ResetEvent(hStop);
                    CloseHandle(hStop);
                }

                wchar_t szPath[MAX_PATH];
                GetModuleFileNameW(NULL, szPath, MAX_PATH);

                STARTUPINFOW si = { sizeof(si) };
                PROCESS_INFORMATION pi;
                wchar_t cmd[MAX_PATH + 32];
                wsprintfW(cmd, L"\"%s\" --daemon", szPath);

                if (CreateProcessW(NULL, cmd, NULL, NULL, FALSE, CREATE_NO_WINDOW, NULL, NULL, &si, &pi)) {
                    CloseHandle(pi.hProcess);
                    CloseHandle(pi.hThread);
                }
            }
            DestroyWindow(hwnd);
            break;

        case BTN_UNLOCK:
            StopDaemon();
            SetWindowTextW(hStatus, L"Status: UNLOCKED");
            // Donma yapmadan hemen pencereyi kapat
            DestroyWindow(hwnd);
            break;
        }
        break;
    }

    case WM_DESTROY:
        PostQuitMessage(0);
        break;

    default:
        return DefWindowProcW(hwnd, msg, wParam, lParam);
    }
    return 0;
}

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PWSTR lpCmdLine, int nCmdShow) {
    if (lpCmdLine && wcsstr(lpCmdLine, L"--daemon")) {
        RunBackgroundDaemon();
        return 0;
    }

    WNDCLASSEXW wc = { 0 };
    wc.cbSize = sizeof(WNDCLASSEXW);
    wc.lpfnWndProc = WndProc;
    wc.hInstance = hInstance;
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wc.lpszClassName = L"LaptopKeyboardManagerClass";

    RegisterClassExW(&wc);

    HWND hwnd = CreateWindowExW(
        WS_EX_APPWINDOW,
        wc.lpszClassName,
        L"Laptop Keyboard Manager",
        WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX,
        CW_USEDEFAULT, CW_USEDEFAULT, 335, 170,
        NULL, NULL, hInstance, NULL
    );

    ShowWindow(hwnd, nCmdShow);
    UpdateWindow(hwnd);

    MSG msg;
    while (GetMessageW(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessageW(&msg);
    }
    return (int)msg.wParam;
}