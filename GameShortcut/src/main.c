#include <stdint.h>
#include <stdio.h>
#include <xtl.h>

#define ERROR_LENGTH 200

// Structs and function prototypes from xboxkrnl.exe
typedef struct _STRING
{
    uint16_t wLength;
    uint16_t wMaxLength;
    char *szBuffer;
} STRING;

void RtlInitAnsiString(STRING *pDestinationString, const char *szSourceString);
HRESULT ObCreateSymbolicLink(STRING *pLinkName, STRING *pDevicePath);

// Allow the game to access the entire hard drive.
// The system only allows executables to access the directory they live in and binds it to
// the "game:" drive. Nothing else is accessible unless you create a symbolic link.
static HRESULT MountHdd()
{
    STRING DeviceName = { 0 };
    STRING LinkName = { 0 };
    const char *szDestinationDrive = "\\??\\hdd:";
    const char *szHddDevicePath = "\\Device\\Harddisk0\\Partition1\\";

    // Initialize the STRING structs
    RtlInitAnsiString(&DeviceName, szHddDevicePath);
    RtlInitAnsiString(&LinkName, szDestinationDrive);

    // Bind the root of the hard drive to the "hdd:" drive.
    return ObCreateSymbolicLink(&LinkName, &DeviceName);
}

// Read the path to the executable from the config file and write it to szGamePath.
static HRESULT GetGamePath(char *szGamePath, size_t nMaxLength)
{
    HRESULT hr = S_OK;
    size_t i = 0;
    FILE *pConfigFile = NULL;
    size_t nGamePathSize = 0;

    // Open the config file in read-only mode
    if (fopen_s(&pConfigFile, "game:\\config\\shortcutInfo.txt", "r") != 0)
        return E_FAIL;

    // Read the second line of the config file into szGamePath
    for (i = 0; i < 2; i++)
        if (fgets(szGamePath, (int)nMaxLength, pConfigFile) == NULL)
            return E_FAIL;

    nGamePathSize = strnlen_s(szGamePath, nMaxLength);

    // Remove the new line character at the end of the line
    szGamePath[nGamePathSize - 1] = '\0';

    fclose(pConfigFile);

    return hr;
}

// Display an error dialog on the console.
static HRESULT ShowMessageBoxError(const char *szMessage)
{
    DWORD dwResult = 0;

    wchar_t wszMessage[ERROR_LENGTH] = { 0 };
    XOVERLAPPED Overlapped = { 0 };
    MESSAGEBOX_RESULT Result = { 0 };
    const wchar_t *pwszButtons[] = { L"OK", L"Cancel" };

    // Convert szMessage, which is a narrow string, to a wide string
    mbstowcs_s(NULL, wszMessage, ERROR_LENGTH, szMessage, _TRUNCATE);

    dwResult = XShowMessageBoxUI(
        0,
        L"Error",
        wszMessage,
        ARRAYSIZE(pwszButtons),
        pwszButtons,
        0,
        XMB_ERRORICON,
        &Result,
        &Overlapped
    );

    if (dwResult != ERROR_IO_PENDING)
        return E_FAIL;

    // Wait until the user closes the message box
    while (!XHasOverlappedIoCompleted(&Overlapped))
        Sleep(100);

    // Get how the user closed the message box (by clicking "OK", "Cancel" or the Xbox button on the controller)
    dwResult = XGetOverlappedResult(&Overlapped, NULL, TRUE);

    if (dwResult == ERROR_ACCESS_DENIED)
        return E_FAIL;

    return S_OK;
}

int main()
{
    HRESULT hr = S_OK;
    char szGamePath[MAX_PATH] = { 0 };

    // Mount the entire hard drive to be able to access any path in it
    hr = MountHdd();
    if (FAILED(hr))
    {
        ShowMessageBoxError("Could not mount HDD1.");
        return EXIT_FAILURE;
    }

    // Read the path to the executable
    hr = GetGamePath(szGamePath, MAX_PATH);
    if (FAILED(hr))
    {
        ShowMessageBoxError("The game information file (config\\shortcutInfo.txt) could not be loaded or has a wrong format.");
        return EXIT_FAILURE;
    }

    // Launch the executable
    XLaunchNewImage(szGamePath, 0);
}
