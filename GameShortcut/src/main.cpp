#include <xtl.h>
#include <fstream>
#include <string>
#include <vector>


struct STRING
{
    WORD wLength;
    WORD wMaxLength;
    PCHAR szBuffer;
};

#define MakeString(s) { (WORD)strlen(s), (WORD)strlen(s) + 1, s }


extern "C" HRESULT __stdcall ObCreateSymbolicLink(STRING*, STRING*);


static HRESULT MountHdd()
{
    PCHAR szDestDrive = "\\??\\hdd:";
    PCHAR szHddDeviceName = "\\Device\\Harddisk0\\Partition1\\";

    STRING DeviceName = MakeString(szHddDeviceName);
    STRING LinkName = MakeString(szDestDrive);

    return ObCreateSymbolicLink(&LinkName, &DeviceName);
}


static HRESULT GetGamePath(std::string& strPath)
{
    std::ifstream ConfigFile("game:\\config\\gameInfo.txt");
    if (!ConfigFile.is_open())
        return E_FAIL;

    std::vector<std::string> lines;
    lines.reserve(2);
    std::string strCurrentLine;

    while (std::getline(ConfigFile, strCurrentLine))
        lines.emplace_back(strCurrentLine);

    ConfigFile.close();

    if (lines.size() != 2)
        return E_FAIL;

    strPath = lines[1];

    return S_OK;
}


static HRESULT ShowMessageBoxError(CONST std::string& strMessage)
{
    XOVERLAPPED overlapped;
    ZeroMemory(&overlapped, sizeof(XOVERLAPPED));

    MESSAGEBOX_RESULT result;
    ZeroMemory(&result, sizeof(MESSAGEBOX_RESULT));

    std::wstring wstrMessage;
    wstrMessage.assign(strMessage.begin(), strMessage.end());

    LPCWSTR pwstrButtons[] = { L"OK", L"Cancel" };

    DWORD dwResult = XShowMessageBoxUI(
        0,
        L"Error",
        wstrMessage.c_str(),
        ARRAYSIZE(pwstrButtons),
        pwstrButtons,
        0,
        XMB_ERRORICON,
        &result,
        &overlapped
    );

    if (dwResult != ERROR_IO_PENDING)
        return E_FAIL;

    while (!XHasOverlappedIoCompleted(&overlapped))
        Sleep(100);

    dwResult = XGetOverlappedResult(&overlapped, NULL, TRUE);

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

    XLaunchNewImage(strGamePath.c_str(), NULL);
}
