#include "Utils.h"

int main()
{
    HRESULT hr = S_OK;
    errno_t err = 0;

    char *xdkDirPath = NULL;
    size_t xdkDirPathSize = 0;
    char shortcutName[SHORCUT_NAME_LENGTH] = { 0 };

    err = _dupenv_s(&xdkDirPath, &xdkDirPathSize, "xedk");
    if (xdkDirPath == NULL || err != 0)
    {
        system("pause");
        return EXIT_FAILURE;
    }

    hr = AddXdkBinDirToPath();
    if (FAILED(hr))
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

    hr = GetShortcutName(shortcutName, SHORCUT_NAME_LENGTH);
    if (FAILED(hr))
    {
        system("pause");
        return EXIT_FAILURE;
    }

    hr = BuildXLASTFile(shortcutName);
    if (FAILED(hr))
    {
        system("pause");
        return EXIT_FAILURE;
    }

    hr = ExecBLAST(xdkDirPath);
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
