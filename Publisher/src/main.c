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

    hr = ExecBLAST(szXDKPath);
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
