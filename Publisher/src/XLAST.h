#pragma once

#include <stdio.h>

#include <Windows.h>

#include "IO.h"
#include "Log.h"

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
    wchar_t *wszFileContent = (wchar_t *)malloc(2048 * sizeof(wchar_t));
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

    mbstowcs_s(NULL, wszGameName, 50, szGameName, _TRUNCATE);
    _snwprintf_s(wszRandomNumber, 9, 9, L"%08x", nRandomNumber);

    for (i = 0; i < 9; i++)
        wszRandomNumber[i] = towlower(wszRandomNumber[i]);

    if (wszFileContent == NULL)
        return E_FAIL;

    _snwprintf_s(wszFileContent, 2048, 2048, wszFileContentFormat, wszGameName, wszRandomNumber, wszGameName, wszGameName, wszGameName);

    hr = GetExecDir(szFilePath, MAX_PATH);
    if (FAILED(hr))
        return E_FAIL;

    strncat_s(szFilePath, MAX_PATH, "\\tmp.xlast", _TRUNCATE);

    fopen_s(&pFile, szFilePath, "w+, ccs=UTF-16LE");
    if (pFile == NULL)
        return E_FAIL;

    nWCharCount = wcsnlen_s(wszFileContent, 2048);
    nWritten = fwrite(wszFileContent, sizeof(wchar_t), nWCharCount, pFile);

    if (nWritten != nWCharCount)
    {
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

        LogError(szErrorMsg);

        return E_FAIL;
    }

    return S_OK;
}
