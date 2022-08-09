#pragma once

#include <stdint.h>

#include <Windows.h>

static void SetConsoleColor(DWORD dwStdHandle, uint16_t wColor)
{
    HANDLE hConsole = GetStdHandle(dwStdHandle);
    SetConsoleTextAttribute(hConsole, FOREGROUND_INTENSITY | wColor);
}

static void ResetConsoleColor(DWORD dwStdHandle)
{
    HANDLE hConsole = GetStdHandle(dwStdHandle);
    SetConsoleTextAttribute(hConsole, FOREGROUND_INTENSITY | 7);
}

void LogInfo(const char *szMessage)
{
    SetConsoleColor(STD_OUTPUT_HANDLE, 7);
    fprintf_s(stdout, "%s\n", szMessage);
    ResetConsoleColor(STD_OUTPUT_HANDLE);
}

void LogInfoW(const wchar_t *wszMessage)
{
    SetConsoleColor(STD_OUTPUT_HANDLE, 7);
    fwprintf_s(stdout, L"%s\n", wszMessage);
    ResetConsoleColor(STD_OUTPUT_HANDLE);
}

void LogError(const char *szMessage)
{
    SetConsoleColor(STD_ERROR_HANDLE, FOREGROUND_RED);
    fprintf_s(stderr, "%s\n", szMessage);
    ResetConsoleColor(STD_ERROR_HANDLE);
}

void LogErrorW(const wchar_t *wszMessage)
{
    SetConsoleColor(STD_ERROR_HANDLE, FOREGROUND_RED);
    fwprintf_s(stderr, L"%s\n", wszMessage);
    ResetConsoleColor(STD_ERROR_HANDLE);
}

void ExitFailure(const char *szMessage)
{
    LogError(szMessage);
    system("pause");
}

void ExitSuccess(const char *szMessage)
{
    if (szMessage != NULL)
        LogInfo(szMessage);

    system("pause");
}
