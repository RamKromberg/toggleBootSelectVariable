/*
toggleBootSelectVariable
ramkromberg@mail.com

Dual Boot\Architecture UEFI boot menu toggle for the Meegopad T01 under Windows.
Reversed from WinToAnd.exe.
Tested on the BayTrail Z3735f Meegopad T01 & Windows 10 Home.
Compiled with Visual Studio 2015.
Released to the Public Domain without any warranties or guarantees.

Under Linux, the following should work instead:
	sudo su
	mount -t efivarfs none /sys/firmware/efi/efivars
	printf "\x07\x00\x00\x00\x80\x00\x00\x00" > /sys/firmware/efi/efivars/BootSelectVariable-944fb13b-773f-4cbb-9c6f-326cebde4c23
	#the linux file has a 4byte attributes bitmask as an header. Here, the values are EFI_VARIABLE_NON_VOLATILE, EFI_VARIABLE_BOOTSERVICE_ACCESS and EFI_VARIABLE_RUNTIME_ACCESS.
	umount /sys/firmware/efi/efivars
	exit

Similarly, in the efi shell:
	fs0:
	mkdir tmp
	cd tmp
	dmpstore BootSelectVariable -s bak.dmp
	cp bak.dmp fixed.dmp
	hexedit fixed.dmp
		Change that last 4byte little endian integer to 0x80. So, say that last sequence is:
			A8 00 00 00
		Change it to:
			80 00 00 00
		When you've changed it, F2 to save and F3 to get back to the shell.
	dmpstore -l fixed.dmp
	reset
*/

#include <windows.h>
#include <tchar.h>
#include <stdio.h>

/* Boilerplate asking privileges to write to the nvram's efi store. */
void
prettyPlease(){
	HANDLE hToken;
	TOKEN_PRIVILEGES tkp;

	OpenProcessToken(GetCurrentProcess(), TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY, &hToken);
	LookupPrivilegeValue(NULL, SE_SYSTEM_ENVIRONMENT_NAME, &tkp.Privileges[0].Luid);
	tkp.PrivilegeCount = 1;
	tkp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;
	AdjustTokenPrivileges(hToken, FALSE, &tkp, 0, (PTOKEN_PRIVILEGES)NULL, 0);
}

int
_tmain(){	
	LPCTSTR lpName = _T("BootSelectVariable");
	LPCTSTR lpGuid = _T("{944FB13B-773F-4CBB-9C6F-326CEBDE4C23}");
	unsigned _int32 enabledMenu = 0x80;
	unsigned _int32 disabledMenu = 0x00;
	unsigned _int32 bootSelectVariable = 0;
	unsigned int errCode = 0;
	DWORD bootSelectVariableReturn = 0;
	BOOL retVal = 1;

	prettyPlease();

	printf("toggleBootSelectVariable.\n");

	bootSelectVariableReturn = GetFirmwareEnvironmentVariable(lpName, lpGuid, &bootSelectVariable, sizeof(DWORD));

	if (bootSelectVariableReturn){
		if (bootSelectVariable == 0x80){
			bootSelectVariableReturn = SetFirmwareEnvironmentVariable(lpName, lpGuid, &disabledMenu, sizeof(unsigned _int32));
			if (bootSelectVariableReturn){
				printf("Boot selection menu is now disabled.\nBootSelectVariable = 0x00.\n");
				retVal = 0;
			}
			else {
				printf("ErrCode = %lu.\n", GetLastError());
			}
		}
		else {
			if (bootSelectVariable != 0x00){
				printf("Unexpected value in BootSelectVariable = 0x%x.\n", (bootSelectVariable));
			}

			bootSelectVariableReturn = SetFirmwareEnvironmentVariable(lpName, lpGuid, &enabledMenu, sizeof(unsigned _int32));
			if (bootSelectVariableReturn){
				printf("Boot selection menu is now enabled.\nBootSelectVariable = 0x80.\n");
				retVal = 0;
			}
			else {
				printf("ErrCode = %lu.\n", GetLastError());
			}
		}
	}
	else {
		errCode = GetLastError();
		if (errCode == 1314){
			printf("It's necessary to run toggleBootSelectVariable with Administrator privileges.\n");
		}
		else {
			printf("ErrCode = %lu.\n", errCode);
		}
	}

	printf("Press enter to continue...\n");
	(void)getchar();
	return retVal;
}
