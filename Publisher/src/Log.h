#pragma once

#include <iostream>
#include <string>
#include <cstdint>

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

void LogInfo(const std::string &strMessage)
{
    SetConsoleColor(STD_OUTPUT_HANDLE, 7);
    std::cout << strMessage << '\n';
    ResetConsoleColor(STD_OUTPUT_HANDLE);
}

void LogError(const std::string &strMessage)
{
    SetConsoleColor(STD_ERROR_HANDLE, FOREGROUND_RED);
    std::cerr << strMessage << '\n';
    ResetConsoleColor(STD_ERROR_HANDLE);
}

void ExitFailure(const std::string &strMessage)
{
    LogError(strMessage);
    system("pause");
}

void ExitSuccess(const std::string &strMessage = "")
{
    if (strMessage != "")
        LogInfo(strMessage);

    system("pause");
}
