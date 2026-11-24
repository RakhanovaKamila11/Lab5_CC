#include <windows.h>
#include <stdio.h>

#define IDT_TIMER 1001
#define ID_MAILSLOT_CREATE 32771
#define ID_MAILSLOT_CLOSE  32772

static HWND hwndEdit;
static HANDLE hMailslot = INVALID_HANDLE_VALUE;
static const char* MailslotName = "\\\\.\\mailslot\\lab7";

char mess[4096] = "Mailbox\r\n";
char szBuf[1024];

void AppendTextToEdit(HWND hEdit, const char* newText) {
    strcat_s(mess, newText);
    SetWindowTextA(hEdit, mess);
    SendMessageA(hEdit, EM_SETSEL, (WPARAM)strlen(mess), (LPARAM)strlen(mess));
    SendMessageA(hEdit, EM_SCROLLCARET, 0, 0);
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
    switch (message) {
    case WM_CREATE: {
        HMENU hMenu = CreateMenu();
        HMENU hSubMenu = CreatePopupMenu();
        AppendMenuA(hSubMenu, MF_STRING, ID_MAILSLOT_CREATE, "Create");
        AppendMenuA(hSubMenu, MF_STRING, ID_MAILSLOT_CLOSE, "Close");
        AppendMenuA(hMenu, MF_POPUP, (UINT_PTR)hSubMenu, "MailSlot");
        SetMenu(hWnd, hMenu);

        hwndEdit = CreateWindowA("EDIT", mess,
            WS_CHILD | WS_VISIBLE | WS_VSCROLL | ES_LEFT | ES_MULTILINE | ES_AUTOVSCROLL | ES_READONLY,
            10, 10, 480, 420, hWnd, NULL, ((LPCREATESTRUCTA)lParam)->hInstance, NULL);
        break;
    }

    case WM_COMMAND: {
        int wmId = LOWORD(wParam);
        switch (wmId) {
        case ID_MAILSLOT_CREATE:
            if (hMailslot != INVALID_HANDLE_VALUE) {
                AppendTextToEdit(hwndEdit, "The server is already running!\r\n");
                break;
            }
            hMailslot = CreateMailslotA(MailslotName, 0, MAILSLOT_WAIT_FOREVER, NULL);
            if (hMailslot == INVALID_HANDLE_VALUE) {
                char errBuf[128];
                sprintf_s(errBuf, "Mail server startup error: %lu\r\n", GetLastError());
                AppendTextToEdit(hwndEdit, errBuf);
            }
            else {
                AppendTextToEdit(hwndEdit, "Mail server started\r\nWaiting for messages...\r\n");
                SetTimer(hWnd, IDT_TIMER, 1000, NULL);
            }
            break;

        case ID_MAILSLOT_CLOSE:
            if (hMailslot != INVALID_HANDLE_VALUE) {
                KillTimer(hWnd, IDT_TIMER);
                CloseHandle(hMailslot);
                hMailslot = INVALID_HANDLE_VALUE;
                AppendTextToEdit(hwndEdit, "Mail server closed\r\n");
            }
            break;
        }
        break;
    }

    case WM_TIMER: {
        if (wParam == IDT_TIMER && hMailslot != INVALID_HANDLE_VALUE) {
            DWORD cbMessage = 0, cMessage = 0, cbRead = 0;
            BOOL fResult = GetMailslotInfo(hMailslot, NULL, &cbMessage, &cMessage, NULL);

            if (fResult && cMessage > 0) {
                while (cMessage > 0) {
                    memset(szBuf, 0, sizeof(szBuf));
                    if (ReadFile(hMailslot, szBuf, sizeof(szBuf) - 1, &cbRead, NULL)) {
                        AppendTextToEdit(hwndEdit, "\r\n Message received \r\n");
                        AppendTextToEdit(hwndEdit, szBuf);
                        AppendTextToEdit(hwndEdit, "\r\n----------------------\r\n");
                    }
                    GetMailslotInfo(hMailslot, NULL, &cbMessage, &cMessage, NULL);
                }
            }
        }
        break;
    }

    case WM_DESTROY:
        if (hMailslot != INVALID_HANDLE_VALUE) {
            KillTimer(hWnd, IDT_TIMER);
            CloseHandle(hMailslot);
        }
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
    wcex.lpszClassName = "MailslotServerClass";
    wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wcex.hCursor = LoadCursor(NULL, IDC_ARROW);
    RegisterClassExA(&wcex);

    HWND hWnd = CreateWindowA("MailslotServerClass", "Server (Mailslot)",
        WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, 520, 500, NULL, NULL, hInstance, NULL);

    ShowWindow(hWnd, nCmdShow);
    UpdateWindow(hWnd);

    MSG msg;
    while (GetMessageA(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessageA(&msg);
    }
    return (int)msg.wParam;
}