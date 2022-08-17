#include "Utils.h"

int main()
{
    HRESULT hr = S_OK;
    errno_t err = 0;

    char *szXDKDirPath = NULL;
    size_t nXDKDirPathSize = 0;
    char szGameName[50] = { 0 };

    err = _dupenv_s(&szXDKDirPath, &nXDKDirPathSize, "xedk");
    if (szXDKDirPath == NULL || err != 0)
    {
        system("pause");
        return EXIT_FAILURE;
    }

    hr = CheckXBDMConnection();
    if (FAILED(hr))
    {
        system("pause");
        return EXIT_FAILURE;
    }

    hr = GetGameName(szGameName, sizeof(szGameName));
    if (FAILED(hr))
    {
        system("pause");
        return EXIT_FAILURE;
    }

    hr = BuildXLASTFile(szGameName);
    if (FAILED(hr))
    {
        system("pause");
        return EXIT_FAILURE;
    }

    hr = ExecBLAST(szXDKDirPath);
    if (FAILED(hr))
    {
        Cleanup();
        system("pause");
        return EXIT_FAILURE;
    }

    Cleanup();

    system("pause");

    return EXIT_SUCCESS;
}
