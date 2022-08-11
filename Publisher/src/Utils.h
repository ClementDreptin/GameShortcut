#pragma once

#include <stdio.h>
#include <stdint.h>

#include <Windows.h>

#include "IO.h"

HRESULT GetGameName(char *szGameName, uint32_t nMaxLength)
{
    HRESULT hr = S_OK;
    FILE *ConfigFile = NULL;
    char szConfigFilePath[MAX_PATH] = { 0 };
    size_t nGameNameSize = strnlen_s(szGameName, nMaxLength);

    hr = GetExecDir(szConfigFilePath, MAX_PATH);
    if (FAILED(hr))
    {
        fputs("Failed to read exec dir", stderr);
        return hr;
    }

    strncat_s(szConfigFilePath, MAX_PATH, "\\config\\gameInfo.txt", _TRUNCATE);

    if (fopen_s(&ConfigFile, szConfigFilePath, "r") != 0)
    {
        fprintf_s(stderr, "Failed to open config file at location %s\n", szConfigFilePath);
        return E_FAIL;
    }

    if (fgets(szGameName, (int)nMaxLength, ConfigFile) == NULL)
    {
        fclose(ConfigFile);
        return E_FAIL;
    }

    szGameName[nGameNameSize - 1] = '\0';

    fclose(ConfigFile);

    return hr;
}
