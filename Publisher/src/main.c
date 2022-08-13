#include "Utils.h"

int main()
{
    HRESULT hr = S_OK;
    errno_t err = 0;

    char *szXDKPath = NULL;
    size_t nXDKPathSize = 0;
    char szGameName[50] = { 0 };

    err = _dupenv_s(&szXDKPath, &nXDKPathSize, "xedk");
    if (szXDKPath == NULL || err != 0)
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
