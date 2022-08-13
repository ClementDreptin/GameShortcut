#pragma once

#include <stdint.h>

#include <Windows.h>

HRESULT GetGameName(char *szGameName, uint32_t nMaxLength);

HRESULT GetExecDir(char *szExecDir, size_t nMaxLength);

void Cleanup(void);

HRESULT BuildXLASTFile(const char *szGameName);

HRESULT ExecBLAST(const char *szXDKPath);

HRESULT CheckXBDMConnection(void);

void ExitSuccess(const char *szMessage);

void ExitFailure(const char *szMessage);
