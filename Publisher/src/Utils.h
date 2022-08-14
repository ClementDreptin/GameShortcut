#pragma once

#include <stdint.h>

#include <Windows.h>

// Read the name of the shortcut from the config file and write it to szGameName.
HRESULT GetGameName(char *szGameName, uint32_t nMaxLength);

// Get the path of the directory the Publisher executable lives in and write it to szGameName.
HRESULT GetExecDir(char *szExecDir, size_t nMaxLength);

// Delete the XLAST files generated by Publisher.
void Cleanup(void);

// Create the XML config file XLAST uses to build the shortcut.
HRESULT BuildXLASTFile(const char *szGameName);

// Run BLAST in a separate process.
HRESULT ExecBLAST(const char *szXDKPath);

// Make sure an XBDM connection is properly set up.
HRESULT CheckXBDMConnection(void);
