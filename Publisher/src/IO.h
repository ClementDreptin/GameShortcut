#pragma once

#include <string.h>

#include <Windows.h>

HRESULT GetExecDir(char *szExecDir, size_t nMaxLength)
{
    char szPath[MAX_PATH] = { 0 };

    GetModuleFileName(NULL, szPath, MAX_PATH);

    const char *szLastBackslash = strrchr(szPath, '\\');
    if (szLastBackslash == NULL)
        return E_FAIL;

    memcpy_s(szExecDir, nMaxLength, szPath, strnlen_s(szPath, MAX_PATH) - strnlen_s(szLastBackslash, MAX_PATH));

    return S_OK;
}

static HRESULT DeleteDirectory(const char *szDirPath)
{
    BOOL bResult = FALSE;
    char szPattern[MAX_PATH] = { 0 };
    size_t nDirPathLength = strnlen_s(szDirPath, MAX_PATH);

    strncpy_s(szPattern, szDirPath, nDirPathLength);
    strncat_s(szPattern, "\\*.*", 4);
    WIN32_FIND_DATA FileInfo;

    HANDLE hFile = FindFirstFile(szPattern, &FileInfo);
    if (hFile == INVALID_HANDLE_VALUE)
        return E_FAIL;

    while (FindNextFile(hFile, &FileInfo))
    {
        if (FileInfo.cFileName[0] == '.')
            continue;

        char szFilePath[MAX_PATH] = { 0 };
        strncpy_s(szFilePath, szDirPath, nDirPathLength);
        strncat_s(szFilePath, "\\", 1);
        strncpy_s(szFilePath, FileInfo.cFileName, strnlen_s(FileInfo.cFileName, MAX_PATH));

        if (FileInfo.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
        {
            HRESULT hr = DeleteDirectory(szFilePath);
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

    DWORD dwError = GetLastError();
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
    char szPathToOnlineDir[MAX_PATH] = { 0 };
    char szPathToXLASTFile[MAX_PATH] = { 0 };

    HRESULT hr = GetExecDir(szPathToOnlineDir, MAX_PATH);
    if (FAILED(hr))
        return;

    strncpy_s(szPathToXLASTFile, szPathToOnlineDir, strnlen_s(szPathToOnlineDir, MAX_PATH));
    strncat_s(szPathToXLASTFile, "\\tmp.xlast", 10);
    strncat_s(szPathToOnlineDir, "\\Online", 7);

    DeleteDirectory(szPathToOnlineDir);
    DeleteFile(szPathToXLASTFile);
}
