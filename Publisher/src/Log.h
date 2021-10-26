#include <Windows.h>
#include <iostream>


static VOID SetConsoleColor(DWORD dwStdHandle, WORD wColor)
{
    HANDLE hConsole = GetStdHandle(dwStdHandle);
    SetConsoleTextAttribute(hConsole, FOREGROUND_INTENSITY | wColor);
}


static VOID ResetConsoleColor(DWORD dwStdHandle)
{
    HANDLE hConsole = GetStdHandle(dwStdHandle);
    SetConsoleTextAttribute(hConsole, FOREGROUND_INTENSITY | 7);
}


VOID LogInfo(LPCSTR szMessage)
{
    SetConsoleColor(STD_OUTPUT_HANDLE, 7);
    std::cout << szMessage << std::endl;
    ResetConsoleColor(STD_OUTPUT_HANDLE);
}


VOID LogError(LPCSTR szMessage)
{
    SetConsoleColor(STD_ERROR_HANDLE, FOREGROUND_RED);
    std::cerr << szMessage << std::endl;
    ResetConsoleColor(STD_ERROR_HANDLE);
}


VOID LogSuccess(LPCSTR szMessage)
{
    SetConsoleColor(STD_OUTPUT_HANDLE, FOREGROUND_GREEN);
    std::cout << szMessage << std::endl;
    ResetConsoleColor(STD_OUTPUT_HANDLE);
}


VOID ExitFailure(LPCSTR szMessage)
{
    LogError(szMessage);
    system("pause");
}


VOID ExitSuccess(LPCSTR szMessage)
{
    LogSuccess(szMessage);
    system("pause");
}
