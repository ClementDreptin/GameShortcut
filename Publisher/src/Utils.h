#pragma once

#include <Windows.h>

#define SHORCUT_NAME_LENGTH 50

// Create the XML config file XLAST uses to build the shortcut.
HRESULT BuildXLastFile(const char *shortcutName);

// Run BLAST in a separate process.
HRESULT ExecBlast(void);

// Get the path of the directory the Publisher executable lives in and write it to execDir.
HRESULT GetExecDir(char *execDir, size_t maxLength);

// Read the name of the shortcut from the config file and write it to shortcutName.
HRESULT GetShortcutName(char *shortcutName, size_t maxLength);

// Make sure an XBDM connection is properly set up.
HRESULT CheckXbdmConnection(void);

// Append %XEDK%/bin/win32 to %PATH% so that xbdm.dll can be found and delay loaded.
HRESULT AddXdkBinDirToPath(void);

// Delete the XLAST files generated by Publisher.
void Cleanup(void);
