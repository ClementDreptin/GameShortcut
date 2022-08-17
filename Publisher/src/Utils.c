#include <stdio.h>
#include <string.h>
#include <Windows.h>
#include <Shlwapi.h>

// XBDM uses bit field types other than int which triggers a warning at warning level 4
// so we just disable it for XBDM
#pragma warning(push)
#pragma warning(disable : 4214)
#include <xbdm.h>
#pragma warning(pop)

#include "Utils.h"

HRESULT BuildXLASTFile(const char *szShortcutName)
{
    HRESULT hr = S_OK;

    char szFilePath[MAX_PATH] = { 0 };
    wchar_t wszShortcutName[SHORCUT_NAME_LENGTH] = { 0 };

    uint32_t nShortcutNameHash = 0;
    wchar_t wszShortcutNameHash[9] = { 0 };

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

    if (szShortcutName == NULL)
    {
        fputs("szShortcutName is NULL", stderr);
        return E_FAIL;
    }

    // Allocate enough memory on the heap to hold the config file content
    wszFileContent = (wchar_t *)malloc(2048 * sizeof(wchar_t));
    if (wszFileContent == NULL)
    {
        fputs("Allocating memory for XLAST file content failed", stderr);
        return E_FAIL;
    }

    // Create a hash from the shortcut name and use it has title ID for the shortcut
    hr = HashData((uint8_t *)szShortcutName, (uint32_t)strnlen_s(szShortcutName, SHORCUT_NAME_LENGTH), (uint8_t *)&nShortcutNameHash, 4);
    if (FAILED(hr))
    {
        fputs("Could not hash to shortcut name", stderr);
        return E_FAIL;
    }

    // Convert szShortcutName, which is a narrow string, to a wide string
    mbstowcs_s(NULL, wszShortcutName, SHORCUT_NAME_LENGTH, szShortcutName, _TRUNCATE);

    // Write the string representation of the random number in wszRandomNumber
    _snwprintf_s(wszShortcutNameHash, 9, 9, L"%08x", nShortcutNameHash);

    // Convert the string representation of the random number to uppercase
    for (i = 0; i < 9; i++)
        wszShortcutNameHash[i] = towupper(wszShortcutNameHash[i]);

    // Create the actual config file content from the shortcut name and the random number and write it to wszFileContent
    _snwprintf_s(wszFileContent, 2048, 2048, wszFileContentFormat, wszShortcutName, wszShortcutNameHash, wszShortcutName, wszShortcutName, wszShortcutName);

    // Read the executable directory path and write it to szFilePath
    hr = GetExecDir(szFilePath, MAX_PATH);
    if (FAILED(hr))
        return E_FAIL;

    // Append the config file name to the executable directory path to get the absolute path to the config file
    strncat_s(szFilePath, MAX_PATH, "\\tmp.xlast", _TRUNCATE);

    // Open the config file in write mode and create it if it doesn't exist (which should always be the case)
    fopen_s(&pFile, szFilePath, "w+, ccs=UTF-16LE");
    if (pFile == NULL)
    {
        fprintf_s(stderr, "Could not open XLAST file at location %s\n", szFilePath);
        return E_FAIL;
    }

    // Write the config file content to the actual file on disk and get the amount of characters written
    nWritten = fwrite(wszFileContent, sizeof(wchar_t), nWCharCount, pFile);

    // Get the amount of characters that are supposed to be written
    nWCharCount = wcsnlen_s(wszFileContent, 2048);

    // Make sure all characters from the config file content were written to disk
    if (nWritten != nWCharCount)
    {
        fprintf_s(stderr, "Could not write XLAST file, was expecting to write %llu characters but wrote %llu\n", nWCharCount, nWritten);
        fclose(pFile);
        return E_FAIL;
    }

    // Close the config file and free the buffer that was holding its content
    fclose(pFile);
    free(wszFileContent);

    wprintf_s(L"XLAST file successfully generated (ID: %s)\n", wszShortcutNameHash);

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

    // Read the executable directory path and write it to szExecDirBuffer
    hr = GetExecDir(szExecDirBuffer, MAX_PATH);
    if (FAILED(hr))
        return E_FAIL;

    // Create the full list of parameters to pass to BLAST
    strncpy_s(szBLASTParameters, MAX_PATH, szExecDirBuffer, _TRUNCATE);
    strncat_s(szBLASTParameters, MAX_PATH, "\\tmp.xlast /build /install:Local /nologo", _TRUNCATE);

    // Create the absolute path to the BLAST executable
    strncpy_s(szBLASTPath, MAX_PATH, szXDKPath, _TRUNCATE);
    strncat_s(szBLASTPath, MAX_PATH, "\\bin\\win32\\blast.exe", _TRUNCATE);

    // Populate the SHELLEXECUTEINFO struct
    ShExecInfo.cbSize = sizeof(SHELLEXECUTEINFO);
    ShExecInfo.fMask = SEE_MASK_NOCLOSEPROCESS | SEE_MASK_NOASYNC | SEE_MASK_NO_CONSOLE;
    ShExecInfo.hwnd = NULL;
    ShExecInfo.lpVerb = NULL;
    ShExecInfo.lpFile = szBLASTPath;
    ShExecInfo.lpParameters = szBLASTParameters;
    ShExecInfo.lpDirectory = szExecDirBuffer;
    ShExecInfo.nShow = SW_SHOW;
    ShExecInfo.hInstApp = NULL;

    // Execute the command
    ShellExecuteEx(&ShExecInfo);

    // Wait for the command's process to complete
    WaitForSingleObject(ShExecInfo.hProcess, INFINITE);
    CloseHandle(ShExecInfo.hProcess);

    // Get the status code returned by the BLAST process and check if it returned an error
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

HRESULT GetExecDir(char *szExecDir, size_t nMaxLength)
{
    char szPath[MAX_PATH] = { 0 };
    char *szLastBackslash = NULL;
    size_t nExecDirPathLength = 0;

    if (szExecDir == NULL)
    {
        fputs("szExecDir is NULL", stderr);
        return E_FAIL;
    }

    // Get the absolute path to the Publisher executable
    GetModuleFileName(NULL, szPath, MAX_PATH);

    // Get a pointer to the last backslash in the path
    szLastBackslash = strrchr(szPath, '\\');

    // It should not happen but just in case no blackslashes were found, return
    if (szLastBackslash == NULL)
        return E_FAIL;

    // Copy szPath into szExecDir but only up until the last backslash
    nExecDirPathLength = strnlen_s(szPath, MAX_PATH) - strnlen_s(szLastBackslash, MAX_PATH);
    strncpy_s(szExecDir, nMaxLength, szPath, nExecDirPathLength);

    return S_OK;
}

HRESULT GetShortcutName(char *szShortcutName, uint32_t nMaxLength)
{
    HRESULT hr = S_OK;
    FILE *pConfigFile = NULL;
    char szConfigFilePath[MAX_PATH] = { 0 };
    size_t nShortcutNameSize = 0;

    if (szShortcutName != NULL)
    {
        fputs("szShortcutName is NULL", stderr);
        return E_FAIL;
    }

    hr = GetExecDir(szConfigFilePath, MAX_PATH);
    if (FAILED(hr))
    {
        fputs("Failed to read execution directory", stderr);
        return hr;
    }

    // Append the path to the config file to the executable directory to get the
    // absolute path to the config file
    strncat_s(szConfigFilePath, MAX_PATH, "\\config\\gameInfo.txt", _TRUNCATE);

    // Open the config file in read-only mode
    if (fopen_s(&pConfigFile, szConfigFilePath, "r") != 0)
    {
        fprintf_s(stderr, "Failed to open config file at location %s\n", szConfigFilePath);
        return E_FAIL;
    }

    // Read the first line of the config file, which contains the shortcut name, into szShortcutName
    if (fgets(szShortcutName, (int)nMaxLength, pConfigFile) == NULL)
    {
        fprintf_s(stderr, "Failed to read from config file at location %s\n", szConfigFilePath);
        fclose(pConfigFile);
        return E_FAIL;
    }

    nShortcutNameSize = strnlen_s(szShortcutName, nMaxLength);

    // Remove the new line character at the end of the line
    szShortcutName[nShortcutNameSize - 1] = '\0';

    fclose(pConfigFile);

    return hr;
}

HRESULT CheckXBDMConnection(void)
{
    HRESULT hr = S_OK;
    DWORD dwXboxNameSize = MAX_PATH;
    char szXboxName[MAX_PATH];

    // Get the name of the default Xbox 360 set up in Neighborhood
    hr = DmGetNameOfXbox(szXboxName, &dwXboxNameSize, TRUE);
    if (FAILED(hr))
        fputs("Could not connect to console", stderr);

    return hr;
}

// Delete a directory and all of its files/subdirectories.
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

    // Create the pattern to start searching the files
    strncpy_s(szPattern, MAX_PATH, szDirPath, _TRUNCATE);
    strncat_s(szPattern, MAX_PATH, "\\*.*", _TRUNCATE);

    // Find the first file corresponding to the pattern
    hFile = FindFirstFile(szPattern, &FileInfo);
    if (hFile == INVALID_HANDLE_VALUE)
    {
        fprintf_s(stderr, "Could not find file at location %s\n", szDirPath);
        return E_FAIL;
    }

    // Loop as long as there are files in the directory
    while (FindNextFile(hFile, &FileInfo))
    {
        char szFilePath[MAX_PATH] = { 0 };

        // Don't try to get "." (current directory) or ".." (parent directory)
        if (!strcmp(FileInfo.cFileName, ".") || !strcmp(FileInfo.cFileName, ".."))
            continue;

        // Create the full path to the current file to delete
        strncpy_s(szFilePath, MAX_PATH, szDirPath, _TRUNCATE);
        strncat_s(szFilePath, MAX_PATH, "\\", _TRUNCATE);
        strncat_s(szFilePath, MAX_PATH, FileInfo.cFileName, _TRUNCATE);

        // If the current file is a directory, start the recursion
        if (FileInfo.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
        {
            hr = DeleteDirectory(szFilePath);
            if (FAILED(hr))
                return E_FAIL;
        }
        else
        {
            // Delete the file
            bResult = DeleteFile(szFilePath);
            if (!bResult)
            {
                fprintf_s(stderr, "Could not delete %s\n", szFilePath);
                return E_FAIL;
            }
        }
    }

    FindClose(hFile);

    // Check for errors
    dwError = GetLastError();
    if (dwError != ERROR_NO_MORE_FILES)
        return E_FAIL;

    // Remove the directory once it's empty
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

    // Read the executable directory path and write it to szPathToOnlineDir
    hr = GetExecDir(szPathToOnlineDir, MAX_PATH);
    if (FAILED(hr))
        return;

    // Copy the executable directory path (which lives in szPathToOnlineDir) into szPathToXLASTFile
    strncpy_s(szPathToXLASTFile, MAX_PATH, szPathToOnlineDir, _TRUNCATE);

    // Finish the paths to the XLAST file and Online directory
    strncat_s(szPathToXLASTFile, MAX_PATH, "\\tmp.xlast", _TRUNCATE);
    strncat_s(szPathToOnlineDir, MAX_PATH, "\\Online", _TRUNCATE);

    // Delete the Online directory and the XLAST config file
    DeleteDirectory(szPathToOnlineDir);
    DeleteFile(szPathToXLASTFile);
}
