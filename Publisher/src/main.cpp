#include <Windows.h>
#include <xbdm.h>
#include <fstream>
#include <vector>
#include <string>
#include <codecvt>

#include "Log.h"


static HRESULT CheckXBDMConnection()
{
    DWORD dwXboxNameSize = MAX_PATH;
    CHAR szXboxName[MAX_PATH];

    return DmGetNameOfXbox(szXboxName, &dwXboxNameSize, TRUE);
}


static std::string GetExecDir()
{
    CHAR szPath[MAX_PATH] = { 0 };

    GetModuleFileName(NULL, szPath, MAX_PATH);

    std::string strExecFilePath(szPath);
    return strExecFilePath.substr(0, strExecFilePath.find_last_of("\\"));
}


static HRESULT GetGameName(std::string& strName)
{
    std::ifstream ConfigFile(GetExecDir() + "\\config\\gameInfo.txt");
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

    strName = lines[0];

    return S_OK;
}


static HRESULT BuildXLASTFile(CONST std::string& strGameName)
{
    std::wstring wstrGameName;
    wstrGameName.assign(strGameName.begin(), strGameName.end());

    std::wstring wstrFileContent =
L"<?xml version=\"1.0\" encoding=\"UTF-16\" standalone=\"no\"?>\n"
L"<XboxLiveSubmissionProject Version=\"2.0.21256.0\">\n"
L"	<ContentProject clsid=\"{AED6156D-A870-4FF7-924F-F375495A222A}\" Name=\"" + wstrGameName + L"\" TitleID=\"12345678\" TitleName=\"" + wstrGameName + L"\" ActivationDate=\"10/26/2021\" PubOfferingID=\"FFFFFFF\" PubBitFlags=\"FFFFFFFF\" HasCost=\"false\" IsMarketplace=\"true\" AllowProfileTransfer=\"true\" AllowDeviceTransfer=\"true\" DashIconPath=\".\\resources\\icon.png\" TitleIconPath=\".\\resources\\icon.png\" ContentType=\"0x00080000\">\n"
L"		<LanguageSettings clsid=\"{F5BA1EE2-D217-447C-93C7-3C7AA6F25DD5}\">\n"
L"			<Language clsid=\"{6424EAA3-FA00-4B6C-8CFB-BF063FC18845}\" DashDisplayName=\"" + wstrGameName + L"\" Description=\"" + wstrGameName + L"\"/>\n"
L"		</LanguageSettings>\n"
L"		<Contents clsid=\"{0D8B38B9-F5A8-4050-8F8D-81F67E3B2456}\">\n"
L"			<Folder clsid=\"{0D8B38B9-F5A8-4050-8F8D-81F67E3B2456}\" TargetName=\"config\">\n"
L"				<File clsid=\"{8A0BC3DD-B402-4080-8E34-C22144FC1ECE}\" SourceName=\".\\gameInfo.txt\" TargetName=\"gameInfo.txt\"/>\n"
L"			</Folder>\n"
L"			<File clsid=\"{8A0BC3DD-B402-4080-8E34-C22144FC1ECE}\" SourceName=\".\\default.xex\" TargetName=\"default.xex\"/>\n"
L"		</Contents>\n"
L"		<ContentOffers clsid=\"{146485F0-DCD7-4AB1-97E1-9B0E64150499}\"/>\n"
L"	</ContentProject>\n"
L"</XboxLiveSubmissionProject>";

    std::wofstream XLASTFile(GetExecDir() + "\\tmp.xlast", std::ios::binary);
    CONST std::codecvt_mode mode = (std::codecvt_mode)(std::generate_header | std::little_endian);
    std::locale UTF16Locale(XLASTFile.getloc(), new std::codecvt_utf16<WCHAR, 0x10ffff, mode>);

    if (!XLASTFile.is_open())
        return E_FAIL;

    XLASTFile.imbue(UTF16Locale);

    XLASTFile << wstrFileContent;

    XLASTFile.close();

    return S_OK;
}


static HRESULT ExecBLAST(CONST std::string& strXDKPath)
{
    std::string strBLASTPath = strXDKPath + "\\bin\\win32\\blast.exe";
    std::string strBLASTParameters = GetExecDir() + "\\tmp.xlast /build /install:Local /nologo";

    SHELLEXECUTEINFO ShExecInfo = { 0 };
    ShExecInfo.cbSize = sizeof(SHELLEXECUTEINFO);
    ShExecInfo.fMask = SEE_MASK_NOCLOSEPROCESS | SEE_MASK_NOASYNC | SEE_MASK_NO_CONSOLE;
    ShExecInfo.hwnd = NULL;
    ShExecInfo.lpVerb = NULL;
    ShExecInfo.lpFile = strBLASTPath.c_str();        
    ShExecInfo.lpParameters = strBLASTParameters.c_str();   
    ShExecInfo.lpDirectory = GetExecDir().c_str();
    ShExecInfo.nShow = SW_SHOW;
    ShExecInfo.hInstApp = NULL; 
    ShellExecuteEx(&ShExecInfo);
    WaitForSingleObject(ShExecInfo.hProcess, INFINITE);
    CloseHandle(ShExecInfo.hProcess);

    if ((INT_PTR)ShExecInfo.hInstApp <= 32)
    {
        DWORD dwError = GetLastError();
        CHAR szErrorMsg[200] = { 0 };
        strerror_s(szErrorMsg, 200, dwError);

        LogError(szErrorMsg);

        return E_FAIL;
    }

    return S_OK;
}

static HRESULT DeleteDirectory(CONST std::string& strDirPath)
{
    BOOL bResult;
    std::string strPattern = strDirPath + "\\*.*";
    WIN32_FIND_DATA FileInfo;

    HANDLE hFile = FindFirstFile(strPattern.c_str(), &FileInfo);
    if (hFile == INVALID_HANDLE_VALUE)
        return E_FAIL;

    while (FindNextFile(hFile, &FileInfo) == TRUE)
    {
        if (FileInfo.cFileName[0] == '.')
            continue;

        std::string strFilePath = strDirPath + "\\" + FileInfo.cFileName;

        if (FileInfo.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
        {
            HRESULT hr = DeleteDirectory(strFilePath);
            if (FAILED(hr))
                return E_FAIL;
        }
        else
        {
            bResult = SetFileAttributes(strFilePath.c_str(), FILE_ATTRIBUTE_NORMAL);
            if (!bResult)
                return E_FAIL;

            bResult = DeleteFile(strFilePath.c_str());
            if (!bResult)
                return E_FAIL;
        }
    }

    FindClose(hFile);

    DWORD dwError = GetLastError();
    if (dwError != ERROR_NO_MORE_FILES)
        return E_FAIL;

    bResult = SetFileAttributes(strDirPath.c_str(), FILE_ATTRIBUTE_NORMAL);
    if (!bResult)
        return E_FAIL;

    bResult = RemoveDirectory(strDirPath.c_str());
    if (!bResult)
        return E_FAIL;

    return S_OK;
}


static VOID Cleanup()
{
    DeleteDirectory(GetExecDir() + "\\Online");

    std::string strXLASTFilePath = GetExecDir() + "\\tmp.xlast";
    DeleteFile(strXLASTFilePath.c_str());
}


int __cdecl main()
{
    HRESULT hr;
    PCHAR szXDKPath;
    size_t nXDKPathSize;

    errno_t err = _dupenv_s(&szXDKPath, &nXDKPathSize, "xedk");
    if (!szXDKPath || err)
    {
        ExitFailure("You must have the Xbox 360 Development Kit (XDK) installed on your computer.");
        return EXIT_FAILURE;
    }

    hr = CheckXBDMConnection();
    if (FAILED(hr))
    {
        ExitFailure("Could not connect to console");
        return EXIT_FAILURE;
    }

    std::string strGameName;
    hr = GetGameName(strGameName);
    if (FAILED(hr))
    {
        ExitFailure("The game information file (config\\gameInfo.txt) could not be loaded or has a wrong format.");
        return EXIT_FAILURE;
    }

    hr = BuildXLASTFile(strGameName);
    if (FAILED(hr))
    {
        ExitFailure("Could not generate the XLAST file.");
        return EXIT_FAILURE;
    }

    LogInfo("XLAST file successfully generated.");

    hr = ExecBLAST(szXDKPath);
    if (FAILED(hr))
    {
        Cleanup();
        ExitFailure("Could not execute BLAST.");
        return EXIT_FAILURE;
    }

    Cleanup();

    ExitSuccess("Game built and deployed to console successfully.");

    return EXIT_SUCCESS;
}
