#include "Log.h"


int __cdecl main()
{
    PCHAR szXDKPath;
    size_t nXDKPathSize;

    errno_t err = _dupenv_s(&szXDKPath, &nXDKPathSize, "xedk");
    if (!szXDKPath || err)
    {
        ExitFailure("You must have the Xbox 360 Development Kit (XDK) installed on your computer!");
        return EXIT_FAILURE;
    }

    ExitSuccess("Everything went right");
}
