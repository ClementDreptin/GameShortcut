#pragma once

#include <string>

#include <Windows.h>

std::string GetExecDir()
{
    char szPath[MAX_PATH] = { 0 };

    GetModuleFileName(NULL, szPath, MAX_PATH);

    std::string strExecFilePath(szPath);

    return strExecFilePath.substr(0, strExecFilePath.find_last_of("\\"));
}

static HRESULT DeleteDirectory(const std::string &strDirPath)
{
    BOOL bResult;
    std::string strPattern = strDirPath + "\\*.*";
    WIN32_FIND_DATA FileInfo;

    HANDLE hFile = FindFirstFile(strPattern.c_str(), &FileInfo);
    if (hFile == INVALID_HANDLE_VALUE)
        return E_FAIL;

    while (FindNextFile(hFile, &FileInfo))
    {
        if (FileInfo.cFileName[0] == '.')
            continue;

        std::string strFilePath = strDirPath + "\\" + FileInfo.cFileName;

        if (FileInfo.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
        {
            HRESULT hr = DeleteDirectory(strFilePath);
            if (FAILED(hr))
                return E_FAIL;
        }
        else
        {
            bResult = SetFileAttributes(strFilePath.c_str(), FILE_ATTRIBUTE_NORMAL);
            if (!bResult)
                return E_FAIL;

            bResult = DeleteFile(strFilePath.c_str());
            if (!bResult)
                return E_FAIL;
        }
    }

    FindClose(hFile);

    DWORD dwError = GetLastError();
    if (dwError != ERROR_NO_MORE_FILES)
        return E_FAIL;

    bResult = SetFileAttributes(strDirPath.c_str(), FILE_ATTRIBUTE_NORMAL);
    if (!bResult)
        return E_FAIL;

    bResult = RemoveDirectory(strDirPath.c_str());
    if (!bResult)
        return E_FAIL;

    return S_OK;
}

void Cleanup()
{
    DeleteDirectory(GetExecDir() + "\\Online");

    std::string strXLASTFilePath = GetExecDir() + "\\tmp.xlast";
    DeleteFile(strXLASTFilePath.c_str());
}
