#include <Windows.h>
#include <iostream>
#include <string>


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


VOID LogInfo(CONST std::string& strMessage)
{
    SetConsoleColor(STD_OUTPUT_HANDLE, 7);
    std::cout << strMessage << std::endl;
    ResetConsoleColor(STD_OUTPUT_HANDLE);
}


VOID LogError(CONST std::string& strMessage)
{
    SetConsoleColor(STD_ERROR_HANDLE, FOREGROUND_RED);
    std::cerr << strMessage << std::endl;
    ResetConsoleColor(STD_ERROR_HANDLE);
}


VOID LogSuccess(CONST std::string& strMessage)
{
    SetConsoleColor(STD_OUTPUT_HANDLE, FOREGROUND_GREEN);
    std::cout << strMessage << std::endl;
    ResetConsoleColor(STD_OUTPUT_HANDLE);
}


VOID ExitFailure(CONST std::string& strMessage)
{
    LogError(strMessage);
    system("pause");
}


VOID ExitSuccess(CONST std::string& strMessage)
{
    LogSuccess(strMessage);
    system("pause");
}
