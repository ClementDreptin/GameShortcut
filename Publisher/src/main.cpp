#include <Windows.h>
#include <xbdm.h>

#include "IO.h"
#include "Utils.h"
#include "XLAST.h"
#include "Log.h"

static HRESULT CheckXBDMConnection()
{
    DWORD dwXboxNameSize = MAX_PATH;
    char szXboxName[MAX_PATH];

    return DmGetNameOfXbox(szXboxName, &dwXboxNameSize, TRUE);
}

int __cdecl main()
{
    HRESULT hr;
    char *szXDKPath;
    size_t nXDKPathSize;
    char szGameName[50] = { 0 };

    errno_t err = _dupenv_s(&szXDKPath, &nXDKPathSize, "xedk");
    if (!szXDKPath || err)
    {
        ExitFailure("You must have the Xbox 360 Development Kit (XDK) installed on your computer.");
        return EXIT_FAILURE;
    }

    hr = CheckXBDMConnection();
    if (FAILED(hr))
    {
        ExitFailure("Could not connect to console.");
        return EXIT_FAILURE;
    }

    hr = GetGameName(szGameName, sizeof(szGameName));
    if (FAILED(hr))
    {
        ExitFailure("The game information file (config\\gameInfo.txt) could not be loaded or has a wrong format.");
        return EXIT_FAILURE;
    }

    hr = BuildXLASTFile(szGameName);
    if (FAILED(hr))
    {
        ExitFailure("Could not generate the XLAST file.");
        return EXIT_FAILURE;
    }

    hr = ExecBLAST(szXDKPath);
    if (FAILED(hr))
    {
        Cleanup();
        ExitFailure("Could not execute BLAST.");
        return EXIT_FAILURE;
    }

    Cleanup();

    ExitSuccess(NULL);

    return EXIT_SUCCESS;
}
