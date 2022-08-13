#include <stdio.h>
#include <string.h>

#include <Windows.h>

#pragma warning(push)
#pragma warning(disable : 4214)
#include <xbdm.h>
#pragma warning(pop)

#include "Utils.h"

HRESULT GetGameName(char *szGameName, uint32_t nMaxLength)
{
    HRESULT hr = S_OK;
    FILE *pConfigFile = NULL;
    char szConfigFilePath[MAX_PATH] = { 0 };
    size_t nGameNameSize = 0;

    if (szGameName != NULL)
    {
        fputs("szGameName is NULL", stderr);
        return E_FAIL;
    }

    hr = GetExecDir(szConfigFilePath, MAX_PATH);
    if (FAILED(hr))
    {
        fputs("Failed to read execution diriectory", stderr);
        return hr;
    }

    strncat_s(szConfigFilePath, MAX_PATH, "\\config\\gameInfo.txt", _TRUNCATE);

    if (fopen_s(&pConfigFile, szConfigFilePath, "r") != 0)
    {
        fprintf_s(stderr, "Failed to open config file at location %s\n", szConfigFilePath);
        return E_FAIL;
    }

    if (fgets(szGameName, (int)nMaxLength, pConfigFile) == NULL)
    {
        fprintf_s(stderr, "Failed to read from config file at location %s\n", szConfigFilePath);
        fclose(pConfigFile);
        return E_FAIL;
    }

    nGameNameSize = strnlen_s(szGameName, nMaxLength);
    szGameName[nGameNameSize - 1] = '\0';

    fclose(pConfigFile);

    return hr;
}

HRESULT GetExecDir(char *szExecDir, size_t nMaxLength)
{
    char szPath[MAX_PATH] = { 0 };
    char *szLastBackslash = NULL;

    if (szExecDir == NULL)
    {
        fputs("szExecDir is NULL", stderr);
        return E_FAIL;
    }

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

    if (szDirPath == NULL)
    {
        fputs("szDirPath is NULL", stderr);
        return E_FAIL;
    }

    strncpy_s(szPattern, MAX_PATH, szDirPath, _TRUNCATE);
    strncat_s(szPattern, MAX_PATH, "\\*.*", _TRUNCATE);

    hFile = FindFirstFile(szPattern, &FileInfo);
    if (hFile == INVALID_HANDLE_VALUE)
    {
        fprintf_s(stderr, "Could not find file at location %s\n", szDirPath);
        return E_FAIL;
    }

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
            {
                fprintf_s(stderr, "Could not set file attributes of %s\n", szFilePath);
                return E_FAIL;
            }

            bResult = DeleteFile(szFilePath);
            if (!bResult)
            {
                fprintf_s(stderr, "Could not delete %s\n", szFilePath);
                return E_FAIL;
            }
        }
    }

    FindClose(hFile);

    dwError = GetLastError();
    if (dwError != ERROR_NO_MORE_FILES)
        return E_FAIL;

    bResult = SetFileAttributes(szDirPath, FILE_ATTRIBUTE_NORMAL);
    if (!bResult)
    {
        fprintf_s(stderr, "Could not set file attributes of %s\n", szDirPath);
        return E_FAIL;
    }

    bResult = RemoveDirectory(szDirPath);
    if (!bResult)
    {
        fprintf_s(stderr, "Could not delete %s\n", szDirPath);
        return E_FAIL;
    }

    return S_OK;
}

void Cleanup(void)
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

HRESULT BuildXLASTFile(const char *szGameName)
{
    HRESULT hr = S_OK;

    char szFilePath[MAX_PATH] = { 0 };
    wchar_t wszGameName[50] = { 0 };

    uint32_t nRandomNumber = 0x12345678;
    wchar_t wszRandomNumber[9] = { 0 };

    size_t i = 0;

    size_t nWCharCount = 0;
    size_t nWritten = 0;
    FILE *pFile = NULL;
    wchar_t *wszFileContent = NULL;
    const wchar_t *wszFileContentFormat =
        L"<?xml version=\"1.0\" encoding=\"UTF-16\" standalone=\"no\"?>\n"
        L"<XboxLiveSubmissionProject Version=\"2.0.21256.0\">\n"
        L"    <ContentProject clsid=\"{AED6156D-A870-4FF7-924F-F375495A222A}\" Name=\"%s\" TitleID=\"%s\" TitleName=\"%s\" ActivationDate=\"10/26/2021\" PubOfferingID=\"FFFFFFF\" PubBitFlags=\"FFFFFFFF\" HasCost=\"false\" IsMarketplace=\"true\" AllowProfileTransfer=\"true\" AllowDeviceTransfer=\"true\" DashIconPath=\".\\resources\\icon.png\" TitleIconPath=\".\\resources\\icon.png\" ContentType=\"0x00080000\">\n"
        L"        <LanguageSettings clsid=\"{F5BA1EE2-D217-447C-93C7-3C7AA6F25DD5}\">\n"
        L"            <Language clsid=\"{6424EAA3-FA00-4B6C-8CFB-BF063FC18845}\" DashDisplayName=\"%s\" Description=\"%s\"/>\n"
        L"        </LanguageSettings>\n"
        L"        <Contents clsid=\"{0D8B38B9-F5A8-4050-8F8D-81F67E3B2456}\">\n"
        L"            <Folder clsid=\"{0D8B38B9-F5A8-4050-8F8D-81F67E3B2456}\" TargetName=\"config\">\n"
        L"                <File clsid=\"{8A0BC3DD-B402-4080-8E34-C22144FC1ECE}\" SourceName=\".\\gameInfo.txt\" TargetName=\"gameInfo.txt\"/>\n"
        L"            </Folder>\n"
        L"            <File clsid=\"{8A0BC3DD-B402-4080-8E34-C22144FC1ECE}\" SourceName=\".\\default.xex\" TargetName=\"default.xex\"/>\n"
        L"        </Contents>\n"
        L"        <ContentOffers clsid=\"{146485F0-DCD7-4AB1-97E1-9B0E64150499}\"/>\n"
        L"    </ContentProject>\n"
        L"</XboxLiveSubmissionProject>";

    if (szGameName == NULL)
    {
        fputs("szGameName is NULL", stderr);
        return E_FAIL;
    }

    wszFileContent = (wchar_t *)malloc(2048 * sizeof(wchar_t));
    if (wszFileContent == NULL)
    {
        fputs("Allocating memory for XLAST file content failed", stderr);
        return E_FAIL;
    }

    mbstowcs_s(NULL, wszGameName, 50, szGameName, _TRUNCATE);
    _snwprintf_s(wszRandomNumber, 9, 9, L"%08x", nRandomNumber);

    for (i = 0; i < 9; i++)
        wszRandomNumber[i] = towlower(wszRandomNumber[i]);

    _snwprintf_s(wszFileContent, 2048, 2048, wszFileContentFormat, wszGameName, wszRandomNumber, wszGameName, wszGameName, wszGameName);

    hr = GetExecDir(szFilePath, MAX_PATH);
    if (FAILED(hr))
        return E_FAIL;

    strncat_s(szFilePath, MAX_PATH, "\\tmp.xlast", _TRUNCATE);

    fopen_s(&pFile, szFilePath, "w+, ccs=UTF-16LE");
    if (pFile == NULL)
    {
        fprintf_s(stderr, "Could not open XLAST file at location %s\n", szFilePath);
        return E_FAIL;
    }

    nWCharCount = wcsnlen_s(wszFileContent, 2048);
    nWritten = fwrite(wszFileContent, sizeof(wchar_t), nWCharCount, pFile);

    if (nWritten != nWCharCount)
    {
        fprintf_s(stderr, "Could not write XLAST file, was expecting to write %llu characters but wrote %llu\n", nWCharCount, nWritten);
        fclose(pFile);
        return E_FAIL;
    }

    fclose(pFile);
    free(wszFileContent);

    wprintf_s(L"XLAST file successfully generated (ID: %s)\n", wszRandomNumber);

    return S_OK;
}

HRESULT ExecBLAST(const char *szXDKPath)
{
    HRESULT hr = S_OK;

    char szExecDirBuffer[MAX_PATH] = { 0 };
    char szBLASTParameters[MAX_PATH] = { 0 };
    char szBLASTPath[MAX_PATH] = { 0 };

    SHELLEXECUTEINFO ShExecInfo = { 0 };

    if (szXDKPath == NULL)
    {
        fputs("szXDKPath is NULL", stderr);
        return E_FAIL;
    }

    hr = GetExecDir(szExecDirBuffer, MAX_PATH);
    if (FAILED(hr))
        return E_FAIL;

    strncpy_s(szBLASTParameters, MAX_PATH, szExecDirBuffer, _TRUNCATE);
    strncat_s(szBLASTParameters, MAX_PATH, "\\tmp.xlast /build /install:Local /nologo", _TRUNCATE);

    strncpy_s(szBLASTPath, MAX_PATH, szXDKPath, _TRUNCATE);
    strncat_s(szBLASTPath, MAX_PATH, "\\bin\\win32\\blast.exe", _TRUNCATE);

    ShExecInfo.cbSize = sizeof(SHELLEXECUTEINFO);
    ShExecInfo.fMask = SEE_MASK_NOCLOSEPROCESS | SEE_MASK_NOASYNC | SEE_MASK_NO_CONSOLE;
    ShExecInfo.hwnd = NULL;
    ShExecInfo.lpVerb = NULL;
    ShExecInfo.lpFile = szBLASTPath;
    ShExecInfo.lpParameters = szBLASTParameters;
    ShExecInfo.lpDirectory = szExecDirBuffer;
    ShExecInfo.nShow = SW_SHOW;
    ShExecInfo.hInstApp = NULL;
    ShellExecuteEx(&ShExecInfo);
    WaitForSingleObject(ShExecInfo.hProcess, INFINITE);
    CloseHandle(ShExecInfo.hProcess);

    if ((int)ShExecInfo.hInstApp <= 32)
    {
        DWORD dwError = GetLastError();
        char szErrorMsg[200] = { 0 };
        strerror_s(szErrorMsg, 200, dwError);

        fputs(szErrorMsg, stderr);

        return E_FAIL;
    }

    return S_OK;
}

HRESULT CheckXBDMConnection(void)
{
    HRESULT hr = S_OK;
    DWORD dwXboxNameSize = MAX_PATH;
    char szXboxName[MAX_PATH];

    hr = DmGetNameOfXbox(szXboxName, &dwXboxNameSize, TRUE);
    if (FAILED(hr))
        fputs("Could not connect to console", stderr);

    return hr;
}
