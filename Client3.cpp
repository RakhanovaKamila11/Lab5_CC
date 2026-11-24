#include <windows.h>
#include <stdio.h>

#define ID_SEND_DATA 32771
static HWND hwndEdit;
static const char* ServerName = "\\\\.\\mailslot\\lab7";
char mess[1024];

void CalcMathData() {
    double a = 15.5;
    double b = 7.2;
    double area = a * b;
    double perimeter = 2 * (a + b);

    sprintf_s(mess,
        "Client Data 3:\r\n"
        " - Shape: Rectangle\r\n"
        " - Side a = %.2f, Side b = %.2f\r\n"
        " - Calculated S = a * b = %.2f\r\n"
        " - Calculated perimeter P = 2*(a+b) = %.2f\r\n",
        a, b, area, perimeter);

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

        CalcMathData();
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
    wcex.lpszClassName = "Client3Class";
    wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wcex.hCursor = LoadCursor(NULL, IDC_ARROW);
    RegisterClassExA(&wcex);

    HWND hWnd = CreateWindowA("Client3Class", "Client 3",
        WS_OVERLAPPEDWINDOW, 970, 250, 400, 340, NULL, NULL, hInstance, NULL);

    ShowWindow(hWnd, nCmdShow);
    UpdateWindow(hWnd);

    MSG msg;
    while (GetMessageA(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessageA(&msg);
    }
    return (int)msg.wParam;
}