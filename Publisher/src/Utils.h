#pragma once

#include <string>
#include <fstream>
#include <vector>

#include <Windows.h>

#include "IO.h"

HRESULT GetGameName(std::string &strName)
{
    char szExecDirBuffer[MAX_PATH] = { 0 };
    HRESULT hr = GetExecDir(szExecDirBuffer, MAX_PATH);
    if (FAILED(hr))
        return E_FAIL;

    std::ifstream ConfigFile(std::string(szExecDirBuffer) + "\\config\\gameInfo.txt");
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
