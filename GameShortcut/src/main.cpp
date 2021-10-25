#include <xtl.h>


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


int __cdecl main()
{
    HRESULT hr = MountHdd();
    if (FAILED(hr))
        return EXIT_FAILURE;

    XLaunchNewImage("hdd:\\Games\\Call of Duty 4\\default_mp.xex", NULL);

    return EXIT_SUCCESS;
}
