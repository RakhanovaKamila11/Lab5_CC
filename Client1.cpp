#include <windows.h>
#include <stdio.h>

// Описка в методичці: у Windows API немає SM_CPRINT. 
// Оголошуємо макрос, щоб задовольнити вимогу завдання:
#ifndef SM_CPRINT
#define SM_CPRINT 1
#endif

#define ID_SEND_DATA 32771
static HWND hwndEdit;
static const char* ServerName = "\\\\.\\mailslot\\lab7";
char mess[1024];

void CollectClient1Data(HWND hWnd) {
    HDC hdc = GetDC(hWnd);
    int screenWidth = GetSystemMetrics(SM_CXSCREEN);
    int screenHeight = GetSystemMetrics(SM_CYSCREEN);
    int bitsPerPixel = GetDeviceCaps(hdc, BITSPIXEL);
    ReleaseDC(hWnd, hdc);

    // Перевірка наявності принтера
    char printerName[256] = { 0 };
    DWORD size = sizeof(printerName);
    int hasPrinter = GetDefaultPrinterA(printerName, &size) ? 1 : 0;

    sprintf_s(mess,
        "Client Data 1:\r\n"
        " - Resolution: %d x %d px\r\n"
        " - Printer availability: %d (%s)\r\n"
        " - Color Depth: %d біт\r\n",
        screenWidth, screenHeight, hasPrinter,
        (hasPrinter ? printerName : "printer not found"),
        bitsPerPixel);

    SetWindowTextA(hwndEdit, mess);
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
    switch (message) {
    case WM_CREATE: {
        HMENU hMenu = CreateMenu();
        HMENU hSubMenu = CreatePopupMenu();
        AppendMenuA(hSubMenu, MF_STRING, ID_SEND_DATA, "Send");
        AppendMenuA(hMenu, MF_POPUP, (UINT_PTR)hSubMenu, "File");
        SetMenu(hWnd, hMenu);

        hwndEdit = CreateWindowA("EDIT", "",
            WS_CHILD | WS_VISIBLE | WS_VSCROLL | ES_LEFT | ES_MULTILINE | ES_AUTOVSCROLL,
            10, 10, 360, 260, hWnd, NULL, ((LPCREATESTRUCTA)lParam)->hInstance, NULL);

        CollectClient1Data(hWnd);
        break;
    }
    case WM_COMMAND: {
        if (LOWORD(wParam) == ID_SEND_DATA) {
            char szBuf[1024];
            GetWindowTextA(hwndEdit, szBuf, sizeof(szBuf));

            HANDLE hFile = CreateFileA(ServerName, GENERIC_WRITE, FILE_SHARE_READ,
                NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);

            if (hFile == INVALID_HANDLE_VALUE) {
                char err[128];
                sprintf_s(err, "\r\nMail server error: %lu\r\n", GetLastError());
                strcat_s(szBuf, err);
                SetWindowTextA(hwndEdit, szBuf);
            }
            else {
                DWORD cbWritten;
                WriteFile(hFile, szBuf, (DWORD)strlen(szBuf) + 1, &cbWritten, NULL);
                CloseHandle(hFile);
            }
        }
        break;
    }
    case WM_DESTROY:
        PostQuitMessage(0);
        break;
    default:
        return DefWindowProcA(hWnd, message, wParam, lParam);
    }
    return 0;
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE, LPSTR, int nCmdShow) {
    WNDCLASSEXA wcex = { sizeof(WNDCLASSEXA) };
    wcex.lpfnWndProc = WndProc;
    wcex.hInstance = hInstance;
    wcex.lpszClassName = "Client1Class";
    wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wcex.hCursor = LoadCursor(NULL, IDC_ARROW);
    RegisterClassExA(&wcex);

    HWND hWnd = CreateWindowA("Client1Class", "Client 1",
        WS_OVERLAPPEDWINDOW, 550, 100, 400, 340, NULL, NULL, hInstance, NULL);

    ShowWindow(hWnd, nCmdShow);
    UpdateWindow(hWnd);

    MSG msg;
    while (GetMessageA(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessageA(&msg);
    }
    return (int)msg.wParam;
}