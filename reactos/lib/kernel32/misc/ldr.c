/* $Id: ldr.c,v 1.8 2000/08/27 22:37:45 ekohl Exp $
 *
 * COPYRIGHT: See COPYING in the top level directory
 * PROJECT  : ReactOS user mode libraries
 * MODULE   : kernel32.dll
 * FILE     : reactos/lib/kernel32/misc/ldr.c
 * AUTHOR   : Boudewijn Dekker
 */

#include <ddk/ntddk.h>
#include <ntdll/ldr.h>
#include <windows.h>

#define NDEBUG
#include <kernel32/kernel32.h>
#include <kernel32/error.h>


/* FUNCTIONS ****************************************************************/

HINSTANCE
STDCALL
LoadLibraryA (
	LPCSTR	lpLibFileName
	)
{
	return LoadLibraryExA (lpLibFileName, 0, 0);
}


HINSTANCE
STDCALL
LoadLibraryExA (
	LPCSTR	lpLibFileName,
	HANDLE	hFile,
	DWORD	dwFlags
	)
{
	UNICODE_STRING LibFileNameU;
	ANSI_STRING LibFileName;
	HINSTANCE hInstance;

	RtlInitAnsiString (&LibFileName,
	                   (LPSTR)lpLibFileName);

	/* convert ansi (or oem) string to unicode */
	if (bIsFileApiAnsi)
		RtlAnsiStringToUnicodeString (&LibFileNameU,
		                              &LibFileName,
		                              TRUE);
	else
		RtlOemStringToUnicodeString (&LibFileNameU,
		                             &LibFileName,
		                             TRUE);

	hInstance = LoadLibraryExW (LibFileNameU.Buffer,
	                            hFile,
	                            dwFlags);

	RtlFreeUnicodeString (&LibFileNameU);

	return hInstance;
}


HINSTANCE
STDCALL
LoadLibraryW (
	LPCWSTR	lpLibFileName
	)
{
	return LoadLibraryExW (lpLibFileName, 0, 0);
}


HINSTANCE
STDCALL
LoadLibraryExW (
	LPCWSTR	lpLibFileName,
	HANDLE	hFile,
	DWORD	dwFlags
	)
{
	HINSTANCE hInst;
	int i;
	LPWSTR lpDllName;
	NTSTATUS Status;
	UNICODE_STRING DllName;

	if ( lpLibFileName == NULL )
		return NULL;

	i = wcslen (lpLibFileName);
// full path specified
	if ( lpLibFileName[2] == L':' ) {
		lpDllName = HeapAlloc(GetProcessHeap(),0,(i+3)*sizeof(WCHAR));
		wcscpy (lpDllName,L"\\??\\");
		wcscat (lpDllName,lpLibFileName);
	}
// point at the end means no extension 
	else if ( lpLibFileName[i-1] == L'.' ) {
		lpDllName = HeapAlloc(GetProcessHeap(),0,(i+1)*sizeof(WCHAR));
		wcscpy (lpDllName,lpLibFileName);
		lpDllName[i-1] = 0;
	}
// no extension
	else if (i > 3 && lpLibFileName[i-3] != L'.' ) {
		lpDllName = HeapAlloc(GetProcessHeap(),0,(i+4)*sizeof(WCHAR));
		wcscpy (lpDllName,lpLibFileName);
		wcscat (lpDllName,L".dll");
	}
	else {
		lpDllName = HeapAlloc(GetProcessHeap(),0,(i+1)*sizeof(WCHAR));
		wcscpy (lpDllName,lpLibFileName);
	}

	RtlInitUnicodeString (&DllName, lpDllName);
	Status = LdrLoadDll(NULL, dwFlags, &DllName, (PVOID*)&hInst);
	HeapFree(GetProcessHeap(), 0, lpDllName);
	if ( !NT_SUCCESS(Status))
	{
		SetLastErrorByStatus (Status);
		return NULL;
	}
	
	return hInst;
}


FARPROC
STDCALL
GetProcAddress( HMODULE hModule, LPCSTR lpProcName )
{
	ANSI_STRING ProcedureName;
	FARPROC fnExp = NULL;

	if (HIWORD(lpProcName) != 0)
	{
		RtlInitAnsiString (&ProcedureName,
		                   (LPSTR)lpProcName);
		LdrGetProcedureAddress ((PVOID)hModule,
		                        &ProcedureName,
		                        0,
		                        (PVOID*)&fnExp);
	}
	else
	{
		LdrGetProcedureAddress ((PVOID)hModule,
		                        NULL,
		                        (ULONG)lpProcName,
		                        (PVOID*)&fnExp);
	}

	return fnExp;
}


WINBOOL
STDCALL
FreeLibrary( HMODULE hLibModule )
{
	LdrUnloadDll(hLibModule);
	return TRUE;
}


VOID
STDCALL
FreeLibraryAndExitThread (
	HMODULE	hLibModule,
	DWORD	dwExitCode
	)
{

	if ( FreeLibrary(hLibModule) )
		ExitThread(dwExitCode);
	return;
}


HMODULE
STDCALL
GetModuleHandleA ( LPCSTR lpModuleName )
{
	UNICODE_STRING ModuleName;
	PVOID BaseAddress;
	NTSTATUS Status;

	RtlCreateUnicodeStringFromAsciiz (&ModuleName,
					  (LPSTR)lpModuleName);

	Status = LdrGetDllHandle (0,
				  0,
				  &ModuleName,
				  &BaseAddress);

	RtlFreeUnicodeString (&ModuleName);

	if (!NT_SUCCESS(Status))
	{
		SetLastErrorByStatus (Status);
		return NULL;
	}

	return ((HMODULE)BaseAddress);
}


HMODULE
STDCALL
GetModuleHandleW (LPCWSTR lpModuleName)
{
	UNICODE_STRING ModuleName;
	PVOID BaseAddress;
	NTSTATUS Status;

	if (lpModuleName == NULL)
		return ((HMODULE)NtCurrentPeb()->ImageBaseAddress);

	RtlInitUnicodeString (&ModuleName,
			      (LPWSTR)lpModuleName);

	Status = LdrGetDllHandle (0,
				  0,
				  &ModuleName,
				  &BaseAddress);
	if (!NT_SUCCESS(Status))
	{
		SetLastErrorByStatus (Status);
		return NULL;
	}

	return ((HMODULE)BaseAddress);
}

/* EOF */
