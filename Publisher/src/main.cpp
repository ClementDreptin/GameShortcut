#include <Windows.h>
#include <fstream>
#include <vector>
#include <string>

#include "Log.h"


static std::string GetExecDir()
{
    CONST size_t MAX_SIZE = 200;
    CHAR szPath[MAX_SIZE] = { 0 };

    GetModuleFileName(NULL, szPath, MAX_SIZE);

    std::string strExecFilePath(szPath);
    return strExecFilePath.substr(0, strExecFilePath.find_last_of("\\"));
}


static HRESULT GetGameName(std::string& strName)
{
    std::ifstream ConfigFile(GetExecDir() + "\\config\\gameInfo.txt");
    if (!ConfigFile.is_open())
        return E_FAIL;

    std::vector<std::string> lines;
    lines.reserve(2);
    std::string strCurrentLine;

    while (std::getline(ConfigFile, strCurrentLine))
        lines.emplace_back(strCurrentLine);

    if (lines.size() != 2)
        return E_FAIL;

    strName = lines[0];

    return S_OK;
}


int __cdecl main()
{
    PCHAR szXDKPath;
    size_t nXDKPathSize;

    errno_t err = _dupenv_s(&szXDKPath, &nXDKPathSize, "xedk");
    if (!szXDKPath || err)
    {
        ExitFailure("You must have the Xbox 360 Development Kit (XDK) installed on your computer.");
        return EXIT_FAILURE;
    }

    std::string strGameName;
    HRESULT hr = GetGameName(strGameName);
    if (FAILED(hr))
    {
        ExitFailure("The game information file (config\\gameInfo.txt) could not be loaded or has a wrong format.");
        return EXIT_FAILURE;
    }

    ExitSuccess("Everything went right");
}
