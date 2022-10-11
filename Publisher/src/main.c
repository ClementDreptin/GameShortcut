#include "Utils.h"

int main()
{
    HRESULT hr = S_OK;
    char shortcutName[SHORCUT_NAME_LENGTH] = { 0 };

    hr = AddXdkBinDirToPath();
    if (FAILED(hr))
    {
        system("pause");
        return EXIT_FAILURE;
    }

    hr = CheckXbdmConnection();
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

    hr = BuildXLastFile(shortcutName);
    if (FAILED(hr))
    {
        system("pause");
        return EXIT_FAILURE;
    }

    hr = ExecBlast();
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
