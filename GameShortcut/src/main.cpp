#include <xtl.h>
#include <fstream>
#include <string>
#include <vector>
#include <cstdint>


struct STRING
{
    uint16_t wLength;
    uint16_t wMaxLength;
    const char *szBuffer;
};

#define MakeString(s) { static_cast<uint16_t>(strlen(s)), static_cast<uint16_t>(strlen(s)) + 1, s }

extern "C" HRESULT __stdcall ObCreateSymbolicLink(STRING *, STRING *);

static HRESULT MountHdd()
{
    const char *szDestDrive = "\\??\\hdd:";
    const char *szHddDeviceName = "\\Device\\Harddisk0\\Partition1\\";

    STRING DeviceName = MakeString(szHddDeviceName);
    STRING LinkName = MakeString(szDestDrive);

    return ObCreateSymbolicLink(&LinkName, &DeviceName);
}

static HRESULT GetGamePath(std::string &strPath)
{
    std::ifstream ConfigFile("game:\\config\\gameInfo.txt");
    if (!ConfigFile.is_open())
        return E_FAIL;

    std::vector<std::string> Lines;
    Lines.reserve(2);
    std::string strCurrentLine;

    while (std::getline(ConfigFile, strCurrentLine))
        Lines.emplace_back(strCurrentLine);

    ConfigFile.close();

    if (Lines.size() != 2)
        return E_FAIL;

    strPath = Lines[1];

    return S_OK;
}

static HRESULT ShowMessageBoxError(const std::string& strMessage)
{
    XOVERLAPPED Overlapped = { 0 };
    MESSAGEBOX_RESULT Result = { 0 };

    std::wstring wstrMessage;
    wstrMessage.assign(strMessage.begin(), strMessage.end());

    const wchar_t *pwstrButtons[] = { L"OK", L"Cancel" };

    DWORD dwResult = XShowMessageBoxUI(
        0,
        L"Error",
        wstrMessage.c_str(),
        ARRAYSIZE(pwstrButtons),
        pwstrButtons,
        0,
        XMB_ERRORICON,
        &Result,
        &Overlapped
    );

    if (dwResult != ERROR_IO_PENDING)
        return E_FAIL;

    while (!XHasOverlappedIoCompleted(&Overlapped))
        Sleep(100);

    dwResult = XGetOverlappedResult(&Overlapped, nullptr, true);

    if (dwResult == ERROR_ACCESS_DENIED)
        return E_FAIL;

    return S_OK;
}

int __cdecl main()
{
    HRESULT hr = MountHdd();
    if (FAILED(hr))
    {
        ShowMessageBoxError("Could not mount HDD1.");
        return EXIT_FAILURE;
    }

    std::string strGamePath;
    hr = GetGamePath(strGamePath);
    if (FAILED(hr))
    {
        ShowMessageBoxError("The game information file (config\\gameInfo.txt) could not be loaded or has a wrong format.");
        return EXIT_FAILURE;
    }

    XLaunchNewImage(strGamePath.c_str(), 0);
}
