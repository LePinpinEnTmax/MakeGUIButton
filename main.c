#include <windows.h>
#include <stdio.h>

HWND hButton;
HWND hConsole;

char folder[] = "C:\\Users\\qthel\\Desktop\\3dsmc"; // edit it to put your own path (dir that contain the Makefile file) :)

void AppendText(const char* text)
{
    int len = GetWindowTextLength(hConsole);
    SendMessage(hConsole, EM_SETSEL, len, len);
    SendMessage(hConsole, EM_REPLACESEL, 0, (LPARAM)text);
}

void runMake()
{
    SECURITY_ATTRIBUTES sa;
    sa.nLength = sizeof(SECURITY_ATTRIBUTES);
    sa.lpSecurityDescriptor = NULL;
    sa.bInheritHandle = TRUE;

    HANDLE hRead, hWrite;
    CreatePipe(&hRead, &hWrite, &sa, 0);
    SetHandleInformation(hRead, HANDLE_FLAG_INHERIT, 0);

    STARTUPINFO si;
    PROCESS_INFORMATION pi;

    ZeroMemory(&si, sizeof(si));
    si.cb = sizeof(si);
    si.dwFlags = STARTF_USESTDHANDLES;
    si.hStdOutput = hWrite;
    si.hStdError = hWrite;

    ZeroMemory(&pi, sizeof(pi));

    char command[1024];
    snprintf(command, sizeof(command),
        "cmd /c cd /d %s && make", folder);

    CreateProcess(
        NULL,
        command,
        NULL,
        NULL,
        TRUE,
        CREATE_NO_WINDOW,
        NULL,
        NULL,
        &si,
        &pi
    );

    CloseHandle(hWrite);

    char buffer[256];
    DWORD bytesRead;

    while (ReadFile(hRead, buffer, sizeof(buffer) - 1, &bytesRead, NULL) && bytesRead > 0)
    {
        buffer[bytesRead] = 0;
        AppendText(buffer);
    }

    CloseHandle(hRead);
    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    switch (msg)
    {
    case WM_CREATE:
    {
        hConsole = CreateWindowEx(
            WS_EX_CLIENTEDGE,
            "EDIT",
            "",
            WS_CHILD | WS_VISIBLE |
            ES_MULTILINE | ES_AUTOVSCROLL |
            WS_VSCROLL | ES_READONLY,
            0, 0, 220, 160,
            hwnd,
            NULL,
            NULL,
            NULL
        );

        hButton = CreateWindow(
            "BUTTON",
            "Make",
            WS_CHILD | WS_VISIBLE,
            0, 160, 220, 40,
            hwnd,
            (HMENU)1,
            NULL,
            NULL
        );

        HFONT font = CreateFont(
            16, 0, 0, 0,
            FW_BOLD,
            FALSE, FALSE, FALSE,
            DEFAULT_CHARSET,
            OUT_DEFAULT_PRECIS,
            CLIP_DEFAULT_PRECIS,
            DEFAULT_QUALITY,
            DEFAULT_PITCH | FF_SWISS,
            "Consolas"
        );

        SendMessage(hConsole, WM_SETFONT, (WPARAM)font, TRUE);
        SendMessage(hButton, WM_SETFONT, (WPARAM)font, TRUE);

        break;
    }

    case WM_COMMAND:
        if (LOWORD(wParam) == 1)
            runMake();
        break;

    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;
    }

    return DefWindowProc(hwnd, msg, wParam, lParam);
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance,
    LPSTR lpCmdLine, int nShowCmd)
{
    WNDCLASS wc;
    ZeroMemory(&wc, sizeof(wc));

    wc.lpfnWndProc = WndProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = "MakeDock";

    RegisterClass(&wc);

    RECT rc;
    SystemParametersInfo(SPI_GETWORKAREA, 0, &rc, 0);

    int width = 220;
    int height = 200;

    HWND hwnd = CreateWindowEx(
        WS_EX_TOPMOST | WS_EX_TOOLWINDOW,
        "MakeDock",
        "",
        WS_POPUP,
        rc.left ,
        rc.bottom - height + 100,
        width,
        height,
        NULL,
        NULL,
        hInstance,
        NULL
    );

    ShowWindow(hwnd, SW_SHOW);

    SetWindowPos(hwnd, HWND_TOPMOST,
        rc.left,
        rc.bottom - height,
        width,
        height,
        SWP_SHOWWINDOW);

    UpdateWindow(hwnd);

    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0))
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    return 0;
}