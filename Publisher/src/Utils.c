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

#define FILE_CONTENT_MAX_LENGTH 2048
#define ERROR_LENGTH 200

// Each bytes is represented as 2 characters in hex and we need an extra character to null-terminate the string
#define HASH_LENGTH ((sizeof(uint32_t) * 2) + 1)

HRESULT BuildXLASTFile(const char *shortcutName)
{
    HRESULT hr = S_OK;

    char filePath[MAX_PATH] = { 0 };
    wchar_t wideShortcutName[SHORCUT_NAME_LENGTH] = { 0 };

    uint32_t shortcutNameHash = 0;
    wchar_t shortcutNameHashAsWideString[HASH_LENGTH] = { 0 };

    size_t i = 0;

    size_t numberOfWideCharToWrite = 0;
    size_t numberOfWideCharWritten = 0;
    FILE *pFile = NULL;
    wchar_t *fileContent = NULL;
    const wchar_t *fileContentFormat =
        L"<?xml version=\"1.0\" encoding=\"UTF-16\" standalone=\"no\"?>\n"
        L"<XboxLiveSubmissionProject Version=\"2.0.21256.0\">\n"
        L"    <ContentProject clsid=\"{AED6156D-A870-4FF7-924F-F375495A222A}\" Name=\"%s\" TitleID=\"%s\" TitleName=\"%s\" ActivationDate=\"10/26/2021\" PubOfferingID=\"FFFFFFF\" PubBitFlags=\"FFFFFFFF\" HasCost=\"false\" IsMarketplace=\"true\" AllowProfileTransfer=\"true\" AllowDeviceTransfer=\"true\" DashIconPath=\".\\resources\\icon.png\" TitleIconPath=\".\\resources\\icon.png\" ContentType=\"0x00080000\">\n"
        L"        <LanguageSettings clsid=\"{F5BA1EE2-D217-447C-93C7-3C7AA6F25DD5}\">\n"
        L"            <Language clsid=\"{6424EAA3-FA00-4B6C-8CFB-BF063FC18845}\" DashDisplayName=\"%s\" Description=\"%s\"/>\n"
        L"        </LanguageSettings>\n"
        L"        <Contents clsid=\"{0D8B38B9-F5A8-4050-8F8D-81F67E3B2456}\">\n"
        L"            <Folder clsid=\"{0D8B38B9-F5A8-4050-8F8D-81F67E3B2456}\" TargetName=\"config\">\n"
        L"                <File clsid=\"{8A0BC3DD-B402-4080-8E34-C22144FC1ECE}\" SourceName=\".\\shortcutInfo.txt\" TargetName=\"shortcutInfo.txt\"/>\n"
        L"            </Folder>\n"
        L"            <File clsid=\"{8A0BC3DD-B402-4080-8E34-C22144FC1ECE}\" SourceName=\".\\default.xex\" TargetName=\"default.xex\"/>\n"
        L"        </Contents>\n"
        L"        <ContentOffers clsid=\"{146485F0-DCD7-4AB1-97E1-9B0E64150499}\"/>\n"
        L"    </ContentProject>\n"
        L"</XboxLiveSubmissionProject>";

    if (shortcutName == NULL)
    {
        fputs("shortcutName is NULL\n", stderr);
        return E_FAIL;
    }

    // Allocate enough memory on the heap to hold the XLAST file content
    fileContent = (wchar_t *)malloc(FILE_CONTENT_MAX_LENGTH * sizeof(wchar_t));
    if (fileContent == NULL)
    {
        fputs("Allocating memory for XLAST file content failed\n", stderr);
        return E_FAIL;
    }

    // Create a hash from the shortcut name and use it has title ID for the shortcut
    hr = HashData(
        (uint8_t *)shortcutName,
        (uint32_t)strnlen_s(shortcutName, SHORCUT_NAME_LENGTH),
        (uint8_t *)&shortcutNameHash,
        sizeof(uint32_t)
    );

    if (FAILED(hr))
    {
        fputs("Could not hash to shortcut name\n", stderr);
        return E_FAIL;
    }

    // Convert shortcutName, which is a narrow string, to a wide string
    mbstowcs_s(NULL, wideShortcutName, SHORCUT_NAME_LENGTH, shortcutName, _TRUNCATE);

    // Write the string representation of the shortcut name hash in shortcutNameHashAsWideString
    _snwprintf_s(shortcutNameHashAsWideString, HASH_LENGTH, _TRUNCATE, L"%08x", shortcutNameHash);

    // Convert the string representation of the shortcut name hash number to uppercase
    for (i = 0; i < HASH_LENGTH; i++)
        shortcutNameHashAsWideString[i] = towupper(shortcutNameHashAsWideString[i]);

    // Create the actual XLAST file content from the shortcut name and the shortcut name hash and write it to fileContent
    _snwprintf_s(
        fileContent,
        FILE_CONTENT_MAX_LENGTH,
        _TRUNCATE,
        fileContentFormat,
        wideShortcutName,
        shortcutNameHashAsWideString,
        wideShortcutName,
        wideShortcutName,
        wideShortcutName
    );

    // Read the executable directory path and write it to filePath
    hr = GetExecDir(filePath, MAX_PATH);
    if (FAILED(hr))
        return E_FAIL;

    // Append the XLAST file name to the executable directory path to get the absolute path to the XLAST file
    strncat_s(filePath, MAX_PATH, "\\tmp.xlast", _TRUNCATE);

    // Open the XLAST file in write mode and create it if it doesn't exist (which should always be the case)
    fopen_s(&pFile, filePath, "w+, ccs=UTF-16LE");
    if (pFile == NULL)
    {
        fprintf_s(stderr, "Could not open XLAST file at location %s\n", filePath);
        return E_FAIL;
    }

    // Get the amount of characters that are supposed to be written
    numberOfWideCharToWrite = wcsnlen_s(fileContent, FILE_CONTENT_MAX_LENGTH);

    // Write the XLAST file content to the actual file on disk and get the amount of characters written
    numberOfWideCharWritten = fwrite(fileContent, sizeof(wchar_t), numberOfWideCharToWrite, pFile);

    // Make sure all characters from the XLAST file content were written to disk
    if (numberOfWideCharWritten != numberOfWideCharToWrite)
    {
        fprintf_s(stderr, "Could not write XLAST file, was expecting to write %llu characters but wrote %llu\n", numberOfWideCharToWrite, numberOfWideCharWritten);
        fclose(pFile);
        return E_FAIL;
    }

    // Close the XLAST file and free the buffer that was holding its content
    fclose(pFile);
    free(fileContent);

    wprintf_s(L"XLAST file successfully generated (ID: %s)\n", shortcutNameHashAsWideString);

    return S_OK;
}

HRESULT ExecBLAST(const char *xdkDirPath)
{
    HRESULT hr = S_OK;

    char execDirBuffer[MAX_PATH] = { 0 };
    char BLASTParameters[MAX_PATH] = { 0 };
    char BLASTPath[MAX_PATH] = { 0 };

    SHELLEXECUTEINFO shellExecInfo = { 0 };

    if (xdkDirPath == NULL)
    {
        fputs("xdkDirPath is NULL\n", stderr);
        return E_FAIL;
    }

    // Read the executable directory path and write it to execDirBuffer
    hr = GetExecDir(execDirBuffer, MAX_PATH);
    if (FAILED(hr))
        return E_FAIL;

    // Create the full list of parameters to pass to BLAST
    strncpy_s(BLASTParameters, MAX_PATH, execDirBuffer, _TRUNCATE);
    strncat_s(BLASTParameters, MAX_PATH, "\\tmp.xlast /build /install:Local /nologo", _TRUNCATE);

    // Create the absolute path to the BLAST executable
    strncpy_s(BLASTPath, MAX_PATH, xdkDirPath, _TRUNCATE);
    strncat_s(BLASTPath, MAX_PATH, "\\bin\\win32\\blast.exe", _TRUNCATE);

    // Populate the SHELLEXECUTEINFO struct
    shellExecInfo.cbSize = sizeof(SHELLEXECUTEINFO);
    shellExecInfo.fMask = SEE_MASK_NOCLOSEPROCESS | SEE_MASK_NOASYNC | SEE_MASK_NO_CONSOLE;
    shellExecInfo.hwnd = NULL;
    shellExecInfo.lpVerb = NULL;
    shellExecInfo.lpFile = BLASTPath;
    shellExecInfo.lpParameters = BLASTParameters;
    shellExecInfo.lpDirectory = execDirBuffer;
    shellExecInfo.nShow = SW_SHOW;
    shellExecInfo.hInstApp = NULL;

    // Execute the command
    ShellExecuteEx(&shellExecInfo);

    // Wait for the command's process to complete
    WaitForSingleObject(shellExecInfo.hProcess, INFINITE);
    CloseHandle(shellExecInfo.hProcess);

    // Get the status code returned by the BLAST process and check if it returned an error
    if ((int)shellExecInfo.hInstApp <= 32)
    {
        uint32_t error = GetLastError();
        char errorMsg[ERROR_LENGTH] = { 0 };
        strerror_s(errorMsg, ERROR_LENGTH, error);

        fputs(errorMsg, stderr);

        return E_FAIL;
    }

    return S_OK;
}

HRESULT GetExecDir(char *execDir, size_t maxLength)
{
    char path[MAX_PATH] = { 0 };
    char *lastBackslash = NULL;
    size_t execDirPathLength = 0;

    if (execDir == NULL)
    {
        fputs("execDir is NULL\n", stderr);
        return E_FAIL;
    }

    // Get the absolute path to the Publisher executable
    GetModuleFileName(NULL, path, MAX_PATH);

    // Get a pointer to the last backslash in the path
    lastBackslash = strrchr(path, '\\');

    // It should not happen but just in case no blackslashes were found, return
    if (lastBackslash == NULL)
        return E_FAIL;

    // Copy path into execDir but only up until the last backslash
    execDirPathLength = strnlen_s(path, MAX_PATH) - strnlen_s(lastBackslash, MAX_PATH);
    strncpy_s(execDir, maxLength, path, execDirPathLength);

    return S_OK;
}

HRESULT GetShortcutName(char *shortcutName, size_t maxLength)
{
    HRESULT hr = S_OK;
    FILE *pConfigFile = NULL;
    char configFilePath[MAX_PATH] = { 0 };
    size_t shortcutNameSize = 0;

    if (shortcutName == NULL)
    {
        fputs("shortcutName is NULL\n", stderr);
        return E_FAIL;
    }

    hr = GetExecDir(configFilePath, MAX_PATH);
    if (FAILED(hr))
    {
        fputs("Failed to read execution directory\n", stderr);
        return hr;
    }

    // Append the path to the config file to the executable directory to get the
    // absolute path to the config file
    strncat_s(configFilePath, MAX_PATH, "\\config\\shortcutInfo.txt", _TRUNCATE);

    // Open the config file in read-only mode
    if (fopen_s(&pConfigFile, configFilePath, "r") != 0)
    {
        fprintf_s(stderr, "Failed to open config file at location %s\n", configFilePath);
        return E_FAIL;
    }

    // Read the first line of the config file, which contains the shortcut name, into shortcutName
    if (fgets(shortcutName, (int)maxLength, pConfigFile) == NULL)
    {
        fprintf_s(stderr, "Failed to read from config file at location %s\n", configFilePath);
        fclose(pConfigFile);
        return E_FAIL;
    }

    shortcutNameSize = strnlen_s(shortcutName, maxLength);

    // Remove the new line character at the end of the line
    shortcutName[shortcutNameSize - 1] = '\0';

    fclose(pConfigFile);

    return hr;
}

HRESULT CheckXBDMConnection(void)
{
    HRESULT hr = S_OK;
    size_t xboxNameSize = MAX_PATH;
    char xboxName[MAX_PATH];

    // Get the name of the default Xbox 360 set up in Neighborhood
    hr = DmGetNameOfXbox(xboxName, (DWORD *)&xboxNameSize, TRUE);
    if (FAILED(hr))
        fputs("Could not connect to console\n", stderr);

    return hr;
}

// Delete a directory and all of its files/subdirectories.
static HRESULT DeleteDirectory(const char *dirPath)
{
    HRESULT hr = S_OK;
    BOOL result = FALSE;
    uint32_t error = 0;

    char pattern[MAX_PATH] = { 0 };

    WIN32_FIND_DATA fileInfo = { 0 };
    HANDLE fileHandle = INVALID_HANDLE_VALUE;

    if (dirPath == NULL)
    {
        fputs("dirPath is NULL\n", stderr);
        return E_FAIL;
    }

    // Create the pattern to start searching the files
    strncpy_s(pattern, MAX_PATH, dirPath, _TRUNCATE);
    strncat_s(pattern, MAX_PATH, "\\*.*", _TRUNCATE);

    // Find the first file corresponding to the pattern
    fileHandle = FindFirstFile(pattern, &fileInfo);
    if (fileHandle == INVALID_HANDLE_VALUE)
    {
        fprintf_s(stderr, "Could not find file at location %s\n", dirPath);
        return E_FAIL;
    }

    // Loop as long as there are files in the directory
    while (FindNextFile(fileHandle, &fileInfo))
    {
        char filePath[MAX_PATH] = { 0 };

        // Don't try to get "." (current directory) or ".." (parent directory)
        if (!strcmp(fileInfo.cFileName, ".") || !strcmp(fileInfo.cFileName, ".."))
            continue;

        // Create the full path to the current file to delete
        strncpy_s(filePath, MAX_PATH, dirPath, _TRUNCATE);
        strncat_s(filePath, MAX_PATH, "\\", _TRUNCATE);
        strncat_s(filePath, MAX_PATH, fileInfo.cFileName, _TRUNCATE);

        // If the current file is a directory, start the recursion
        if (fileInfo.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
        {
            hr = DeleteDirectory(filePath);
            if (FAILED(hr))
                return E_FAIL;
        }
        else
        {
            // Delete the file
            result = DeleteFile(filePath);
            if (!result)
            {
                fprintf_s(stderr, "Could not delete %s\n", filePath);
                return E_FAIL;
            }
        }
    }

    FindClose(fileHandle);

    // Check for errors
    error = GetLastError();
    if (error != ERROR_NO_MORE_FILES)
        return E_FAIL;

    // Remove the directory once it's empty
    result = RemoveDirectory(dirPath);
    if (!result)
    {
        fprintf_s(stderr, "Could not delete %s\n", dirPath);
        return E_FAIL;
    }

    return S_OK;
}

void Cleanup(void)
{
    HRESULT hr = S_OK;
    char pathToOnlineDir[MAX_PATH] = { 0 };
    char pathToXLASTFile[MAX_PATH] = { 0 };

    // Read the executable directory path and write it to pathToOnlineDir
    hr = GetExecDir(pathToOnlineDir, MAX_PATH);
    if (FAILED(hr))
        return;

    // Copy the executable directory path (which lives in pathToOnlineDir) into pathToXLASTFile
    strncpy_s(pathToXLASTFile, MAX_PATH, pathToOnlineDir, _TRUNCATE);

    // Finish the paths to the XLAST file and Online directory
    strncat_s(pathToXLASTFile, MAX_PATH, "\\tmp.xlast", _TRUNCATE);
    strncat_s(pathToOnlineDir, MAX_PATH, "\\Online", _TRUNCATE);

    // Delete the Online directory and the XLAST file
    DeleteDirectory(pathToOnlineDir);
    DeleteFile(pathToXLASTFile);
}
