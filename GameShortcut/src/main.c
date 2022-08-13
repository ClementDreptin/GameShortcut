#include <stdint.h>
#include <stdio.h>

#include <xtl.h>

typedef struct
{
    uint16_t wLength;
    uint16_t wMaxLength;
    char *szBuffer;
} STRING;

void RtlInitAnsiString(STRING *pDestinationString, const char *szSourceString);
HRESULT ObCreateSymbolicLink(STRING *pLinkName, STRING *pDevicePath);

static HRESULT MountHdd()
{
    STRING DeviceName = { 0 };
    STRING LinkName = { 0 };
    const char *szDestDrive = "\\??\\hdd:";
    const char *szHddDevicePath = "\\Device\\Harddisk0\\Partition1\\";

    RtlInitAnsiString(&DeviceName, szHddDevicePath);
    RtlInitAnsiString(&LinkName, szDestDrive);

    return ObCreateSymbolicLink(&LinkName, &DeviceName);
}

static HRESULT GetGamePath(char *szGamePath, uint32_t nMaxLength)
{
    HRESULT hr = S_OK;
    size_t i = 0;
    FILE *pConfigFile = NULL;
    size_t nGamePathSize = 0;

    if (fopen_s(&pConfigFile, "game:\\config\\gameInfo.txt", "r") != 0)
        return E_FAIL;

    for (i = 0; i < 2; i++)
        if (fgets(szGamePath, (int)nMaxLength, pConfigFile) == NULL)
            return E_FAIL;

    nGamePathSize = strnlen_s(szGamePath, nMaxLength);
    szGamePath[nGamePathSize - 1] = '\0';

    fclose(pConfigFile);

    return hr;
}

static HRESULT ShowMessageBoxError(const char *szMessage)
{
    DWORD dwResult = 0;

    wchar_t wszMessage[200] = { 0 };
    XOVERLAPPED Overlapped = { 0 };
    MESSAGEBOX_RESULT Result = { 0 };
    const wchar_t *pwszButtons[] = { L"OK", L"Cancel" };

    mbstowcs_s(NULL, wszMessage, 200, szMessage, _TRUNCATE);

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

    while (!XHasOverlappedIoCompleted(&Overlapped))
        Sleep(100);

    dwResult = XGetOverlappedResult(&Overlapped, NULL, TRUE);

    if (dwResult == ERROR_ACCESS_DENIED)
        return E_FAIL;

    return S_OK;
}

int main()
{
    HRESULT hr = S_OK;
    char szGamePath[MAX_PATH] = { 0 };

    hr = MountHdd();
    if (FAILED(hr))
    {
        ShowMessageBoxError("Could not mount HDD1.");
        return EXIT_FAILURE;
    }

    hr = GetGamePath(szGamePath, MAX_PATH);
    if (FAILED(hr))
    {
        ShowMessageBoxError("The game information file (config\\gameInfo.txt) could not be loaded or has a wrong format.");
        return EXIT_FAILURE;
    }

    XLaunchNewImage(szGamePath, 0);
}
