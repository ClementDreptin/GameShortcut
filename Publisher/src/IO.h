#pragma once

#include <string.h>

#include <Windows.h>

HRESULT GetExecDir(char *szExecDir, size_t nMaxLength)
{
    char szPath[MAX_PATH] = { 0 };
    char *szLastBackslash = NULL;

    GetModuleFileName(NULL, szPath, MAX_PATH);

    szLastBackslash = strrchr(szPath, '\\');
    if (szLastBackslash == NULL)
        return E_FAIL;

    strncpy_s(szExecDir, nMaxLength, szPath, strnlen_s(szPath, MAX_PATH) - strnlen_s(szLastBackslash, MAX_PATH));

    return S_OK;
}

static HRESULT DeleteDirectory(const char *szDirPath)
{
    HRESULT hr = S_OK;
    BOOL bResult = FALSE;
    DWORD dwError = 0;

    char szPattern[MAX_PATH] = { 0 };

    WIN32_FIND_DATA FileInfo = { 0 };
    HANDLE hFile = INVALID_HANDLE_VALUE;

    strncpy_s(szPattern, MAX_PATH, szDirPath, _TRUNCATE);
    strncat_s(szPattern, MAX_PATH, "\\*.*", _TRUNCATE);

    hFile = FindFirstFile(szPattern, &FileInfo);
    if (hFile == INVALID_HANDLE_VALUE)
        return E_FAIL;

    while (FindNextFile(hFile, &FileInfo))
    {
        char szFilePath[MAX_PATH] = { 0 };

        if (FileInfo.cFileName[0] == '.')
            continue;

        strncpy_s(szFilePath, MAX_PATH, szDirPath, _TRUNCATE);
        strncat_s(szFilePath, MAX_PATH, "\\", _TRUNCATE);
        strncpy_s(szFilePath, MAX_PATH, FileInfo.cFileName, _TRUNCATE);

        if (FileInfo.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
        {
            hr = DeleteDirectory(szFilePath);
            if (FAILED(hr))
                return E_FAIL;
        }
        else
        {
            bResult = SetFileAttributes(szFilePath, FILE_ATTRIBUTE_NORMAL);
            if (!bResult)
                return E_FAIL;

            bResult = DeleteFile(szFilePath);
            if (!bResult)
                return E_FAIL;
        }
    }

    FindClose(hFile);

    dwError = GetLastError();
    if (dwError != ERROR_NO_MORE_FILES)
        return E_FAIL;

    bResult = SetFileAttributes(szDirPath, FILE_ATTRIBUTE_NORMAL);
    if (!bResult)
        return E_FAIL;

    bResult = RemoveDirectory(szDirPath);
    if (!bResult)
        return E_FAIL;

    return S_OK;
}

void Cleanup()
{
    HRESULT hr = S_OK;
    char szPathToOnlineDir[MAX_PATH] = { 0 };
    char szPathToXLASTFile[MAX_PATH] = { 0 };

    hr = GetExecDir(szPathToOnlineDir, MAX_PATH);
    if (FAILED(hr))
        return;

    strncpy_s(szPathToXLASTFile, MAX_PATH, szPathToOnlineDir, _TRUNCATE);
    strncat_s(szPathToXLASTFile, MAX_PATH, "\\tmp.xlast", _TRUNCATE);
    strncat_s(szPathToOnlineDir, MAX_PATH, "\\Online", _TRUNCATE);

    DeleteDirectory(szPathToOnlineDir);
    DeleteFile(szPathToXLASTFile);
}
