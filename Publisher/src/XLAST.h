#pragma once

#include <Windows.h>

#include <iostream>
#include <random>
#include <algorithm>
#include <fstream>
#include <codecvt>

#include "IO.h"


HRESULT BuildXLASTFile(CONST std::string& strGameName)
{
    std::wstring wstrGameName;
    wstrGameName.assign(strGameName.begin(), strGameName.end());

    std::random_device RandomDevice;
    std::mt19937 rng(RandomDevice());
    std::uniform_int_distribution<std::mt19937::result_type> RandomDistance(0, MAXDWORD);
    UINT uiRandomNumber = RandomDistance(rng);

    WCHAR wszBuffer[9] = { 0 };
    _snwprintf_s(wszBuffer, 9, 9, L"%08x", uiRandomNumber);

    std::wstring wstrRandomNumberAsHex(wszBuffer);
    std::transform(wstrRandomNumberAsHex.begin(), wstrRandomNumberAsHex.end(), wstrRandomNumberAsHex.begin(), towupper);

    std::wstring wstrFileContent =
        L"<?xml version=\"1.0\" encoding=\"UTF-16\" standalone=\"no\"?>\n"
        L"<XboxLiveSubmissionProject Version=\"2.0.21256.0\">\n"
        L"	<ContentProject clsid=\"{AED6156D-A870-4FF7-924F-F375495A222A}\" Name=\"" + wstrGameName + L"\" TitleID=\"" + wstrRandomNumberAsHex + L"\" TitleName=\"" + wstrGameName + L"\" ActivationDate=\"10/26/2021\" PubOfferingID=\"FFFFFFF\" PubBitFlags=\"FFFFFFFF\" HasCost=\"false\" IsMarketplace=\"true\" AllowProfileTransfer=\"true\" AllowDeviceTransfer=\"true\" DashIconPath=\".\\resources\\icon.png\" TitleIconPath=\".\\resources\\icon.png\" ContentType=\"0x00080000\">\n"
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


HRESULT ExecBLAST(CONST std::string& strXDKPath)
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

        std::cerr << szErrorMsg << std::endl;

        return E_FAIL;
    }

    return S_OK;
}
