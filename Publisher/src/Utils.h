#pragma once

#include <Windows.h>

#include <string>
#include <fstream>
#include <vector>

#include "IO.h"


HRESULT GetGameName(std::string& strName)
{
    std::ifstream ConfigFile(GetExecDir() + "\\config\\gameInfo.txt");
    if (!ConfigFile.is_open())
        return E_FAIL;

    std::vector<std::string> lines;
    lines.reserve(2);
    std::string strCurrentLine;

    while (std::getline(ConfigFile, strCurrentLine))
        lines.emplace_back(strCurrentLine);

    ConfigFile.close();

    if (lines.size() != 2)
        return E_FAIL;

    strName = lines[0];

    return S_OK;
}
