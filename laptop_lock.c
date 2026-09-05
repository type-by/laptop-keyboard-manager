#define _WIN32_WINNT 0x0501
#define _UNICODE
#define UNICODE

#include <windows.h>
#include <tlhelp32.h>
#include <stdbool.h>
#include "interception.h"

#define MUTEX_WORKER L"Global\\LaptopKlavyeWorkerMutex"
#define BTN_LOCK     101
#define BTN_UNLOCK   102

HWND hStatusText;
HWND hBtnLock;
HWND hBtnUnlock;

// Sistemde arka plan filtreleme isleminin calisip calismadigini kontrol eder
bool IsFilterRunning() {
    HANDLE hMutex = OpenMutexW(MUTEX_ALL_ACCESS, FALSE, MUTEX_WORKER);
    if (hMutex != NULL) {
        CloseHandle(hMutex);
        return true;
    }
    return false;
}

// Arka plandaki filtre islemini aninda sonlandirir (Klavyeyi acar)
void StopFilterProcess() {
    PROCESSENTRY32W entry;
    entry.dwSize = sizeof(PROCESSENTRY32W);

    HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (snapshot == INVALID_HANDLE_VALUE) return;

    DWORD currentPid = GetCurrentProcessId();

    if (Process32FirstW(snapshot, &entry)) {
        do {
            if (wcscmp(entry.szExeFile, L"LaptopKlavyeYonetici.exe") == 0 && entry.th32ProcessID != currentPid) {
                HANDLE hProc = OpenProcess(PROCESS_TERMINATE, FALSE, entry.th32ProcessID);
                if (hProc != NULL) {
                    TerminateProcess(hProc, 0);
                    CloseHandle(hProc);
                }
            }
        } while (Process32NextW(snapshot, &entry));
    }
    CloseHandle(snapshot);
}

// Arka planda gorunmez calisacak filtre motoru
void RunBackgroundWorker() {
    // Tek bir worker calismasini garantiye al
    HANDLE hMutex = CreateMutexW(NULL, TRUE, MUTEX_WORKER);
    if (GetLastError() == ERROR_ALREADY_EXISTS) {
        return;
    }

    InterceptionContext context = interception_create_context();
    if (!context) return;

    interception_set_filter(context, interception_is_keyboard, INTERCEPTION_FILTER_KEY_ALL);
    InterceptionDevice device;
    InterceptionStroke stroke;

    while (interception_receive(context, device = interception_wait(context), &stroke, 1) > 0) {
        // Laptop dahili klavyesini (device == 1) yok say / yut
        if (device == 1) {
            continue;
        }
        // Harici klavyeleri aynen ilet
        interception_send(context, device, &stroke, 1);
    }

    interception_destroy_context(context);
    CloseHandle(hMutex);
}

// Arayuz butonlarini ve durumu tazele
void UpdateUI() {
    if (IsFilterRunning()) {
        SetWindowTextW(hStatusText, L"Durum: Dahili Klavye KİLİTLİ (Harici Açık)");
        EnableWindow(hBtnLock, FALSE);
        EnableWindow(hBtnUnlock, TRUE);
    } else {
        SetWindowTextW(hStatusText, L"Durum: Dahili Klavye AÇIK (Normal)");
        EnableWindow(hBtnLock, TRUE);
        EnableWindow(hBtnUnlock, FALSE);
    }
}

// Kilidi baslat: Kendisini arka planda penceresiz kopya olarak calistirir
void StartLock() {
    if (IsFilterRunning()) return;

    wchar_t exePath[MAX_PATH];
    GetModuleFileNameW(NULL, exePath, MAX_PATH);

    wchar_t cmd[MAX_PATH + 30];
    wsprintfW(cmd, L"\"%s\" --worker", exePath);

    STARTUPINFOW si = { sizeof(si) };
    PROCESS_INFORMATION pi;
    si.dwFlags = STARTF_USESHOWWINDOW;
    si.wShowWindow = SW_HIDE; // Tamamen gorunmez

    if (CreateProcessW(NULL, cmd, NULL, NULL, FALSE, CREATE_NO_WINDOW, NULL, NULL, &si, &pi)) {
        CloseHandle(pi.hThread);
        CloseHandle(pi.hProcess);
    }
}

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    switch (uMsg) {
        case WM_CREATE: {
            HFONT hFont = CreateFontW(16, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE, 
                                     DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, 
                                     DEFAULT_QUALITY, DEFAULT_PITCH | FF_SWISS, L"Segoe UI");

            hStatusText = CreateWindowW(L"STATIC", L"Durum taranıyor...", 
                WS_VISIBLE | WS_CHILD | SS_CENTER, 
                20, 20, 340, 25, hwnd, NULL, NULL, NULL);
            SendMessageW(hStatusText, WM_SETFONT, (WPARAM)hFont, TRUE);

            hBtnLock = CreateWindowW(L"BUTTON", L"Klavyeyi Kilitle", 
                WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON, 
                30, 65, 150, 38, hwnd, (HMENU)BTN_LOCK, NULL, NULL);
            SendMessageW(hBtnLock, WM_SETFONT, (WPARAM)hFont, TRUE);

            hBtnUnlock = CreateWindowW(L"BUTTON", L"Klavyeyi Aç", 
                WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON, 
                200, 65, 150, 38, hwnd, (HMENU)BTN_UNLOCK, NULL, NULL);
            SendMessageW(hBtnUnlock, WM_SETFONT, (WPARAM)hFont, TRUE);

            UpdateUI();
            break;
        }
        case WM_COMMAND: {
            if (LOWORD(wParam) == BTN_LOCK) {
                StartLock();
                Sleep(250);
                UpdateUI();
            } else if (LOWORD(wParam) == BTN_UNLOCK) {
                StopFilterProcess();
                Sleep(250);
                UpdateUI();
            }
            break;
        }
        case WM_DESTROY:
            PostQuitMessage(0);
            return 0;
    }
    return DefWindowProcW(hwnd, uMsg, wParam, lParam);
}

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PWSTR pCmdLine, int nCmdShow) {
    // Eger arka plan modu (--worker) ile cagrildiysa hic pencere acma
    if (wcsstr(pCmdLine, L"--worker") != NULL) {
        RunBackgroundWorker();
        return 0;
    }

    // Normal GUI Penceresi
    const wchar_t CLASS_NAME[] = L"LaptopKlavyeKontrolGUI";

    WNDCLASSW wc = {0};
    wc.lpfnWndProc = WindowProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = CLASS_NAME;
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)(COLOR_BTNFACE + 1);

    RegisterClassW(&wc);

    int screenWidth = GetSystemMetrics(SM_CXSCREEN);
    int screenHeight = GetSystemMetrics(SM_CYSCREEN);
    int winWidth = 400;
    int winHeight = 160;

    HWND hwnd = CreateWindowExW(
        0, CLASS_NAME, L"Laptop Klavye Yöneticisi",
        WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX,
        (screenWidth - winWidth) / 2, (screenHeight - winHeight) / 2,
        winWidth, winHeight,
        NULL, NULL, hInstance, NULL
    );

    if (!hwnd) return 0;

    ShowWindow(hwnd, nCmdShow);
    UpdateWindow(hwnd);

    MSG msg;
    while (GetMessageW(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessageW(&msg);
    }

    return 0;
}