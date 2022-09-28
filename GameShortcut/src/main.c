#include <stdint.h>
#include <stdio.h>
#include <xtl.h>

#define ERROR_LENGTH 200

// Structs and function prototypes from xboxkrnl.exe
typedef struct _STRING
{
    uint16_t Length;
    uint16_t MaxLength;
    char *Buffer;
} STRING;

void RtlInitAnsiString(STRING *pDestinationString, const char *sourceString);
HRESULT ObCreateSymbolicLink(STRING *pLinkName, STRING *pDevicePath);

// Allow the game to access the entire hard drive.
// The system only allows executables to access the directory they live in and binds it to
// the "game:" drive. Nothing else is accessible unless you create a symbolic link.
static HRESULT MountHdd()
{
    STRING deviceName = { 0 };
    STRING linkName = { 0 };
    const char *destinationDrive = "\\??\\hdd:";
    const char *hddDevicePath = "\\Device\\Harddisk0\\Partition1\\";

    // Initialize the STRING structs
    RtlInitAnsiString(&deviceName, hddDevicePath);
    RtlInitAnsiString(&linkName, destinationDrive);

    // Bind the root of the hard drive to the "hdd:" drive.
    return ObCreateSymbolicLink(&linkName, &deviceName);
}

// Read the path to the executable from the config file and write it to gamePath.
static HRESULT GetGamePath(char *gamePath, size_t maxLength)
{
    HRESULT hr = S_OK;
    size_t i = 0;
    FILE *pConfigFile = NULL;
    size_t gamePathSize = 0;

    // Open the config file in read-only mode
    if (fopen_s(&pConfigFile, "game:\\config\\shortcutInfo.txt", "r") != 0)
        return E_FAIL;

    // Read the second line of the config file into gamePath
    for (i = 0; i < 2; i++)
        if (fgets(gamePath, (int)maxLength, pConfigFile) == NULL)
            return E_FAIL;

    gamePathSize = strnlen_s(gamePath, maxLength);

    // Remove the new line character at the end of the line
    gamePath[gamePathSize - 1] = '\0';

    fclose(pConfigFile);

    return hr;
}

// Display an error dialog on the console.
static HRESULT ShowMessageBoxError(const char *message)
{
    uint32_t result = 0;

    wchar_t buffer[ERROR_LENGTH] = { 0 };
    XOVERLAPPED overlapped = { 0 };
    MESSAGEBOX_RESULT messageBoxResult = { 0 };
    const wchar_t *buttons[] = { L"OK", L"Cancel" };

    // Convert message, which is a narrow string, to a wide string
    mbstowcs_s(NULL, buffer, ERROR_LENGTH, message, _TRUNCATE);

    result = XShowMessageBoxUI(
        0,
        L"Error",
        buffer,
        ARRAYSIZE(buttons),
        buttons,
        0,
        XMB_ERRORICON,
        &messageBoxResult,
        &overlapped
    );

    if (result != ERROR_IO_PENDING)
        return E_FAIL;

    // Wait until the user closes the message box
    while (!XHasOverlappedIoCompleted(&overlapped))
        Sleep(100);

    // Get how the user closed the message box (by clicking "OK", "Cancel" or the Xbox button on the controller)
    result = XGetOverlappedResult(&overlapped, NULL, TRUE);

    if (result == ERROR_ACCESS_DENIED)
        return E_FAIL;

    return S_OK;
}

int main()
{
    HRESULT hr = S_OK;
    char gamePath[MAX_PATH] = { 0 };

    // Mount the entire hard drive to be able to access any path in it
    hr = MountHdd();
    if (FAILED(hr))
    {
        ShowMessageBoxError("Could not mount HDD1.");
        return EXIT_FAILURE;
    }

    // Read the path to the executable
    hr = GetGamePath(gamePath, MAX_PATH);
    if (FAILED(hr))
    {
        ShowMessageBoxError("The game information file (config\\shortcutInfo.txt) could not be loaded or has a wrong format.");
        return EXIT_FAILURE;
    }

    // Launch the executable
    XLaunchNewImage(gamePath, 0);
}
