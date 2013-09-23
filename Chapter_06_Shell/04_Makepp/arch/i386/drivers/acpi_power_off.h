/*! ACPI power off (just for convenience and automation in testing)
 * From:
 * http://stackoverflow.com/questions/3145569/how-to-power-down-the-computer-from-a-freestanding-environment
 */

#pragma once

void acpiPowerOff(void);

#ifdef _ACPI_POWER_OFF_C_ //rest only for acpi_power_off.h

#include <types/basic.h>

typedef uint8 byte;
typedef uint16 word;
typedef uint32 dword;

struct RSDPtr
{
	byte	 Signature[8];
	byte	 CheckSum;
	byte	 OemID[6];
	byte	 Revision;
	dword	*RsdtAddress;
};

struct FACP
{
	byte	 Signature[4];
	dword	 Length;
	byte	 unneded1[40 - 8];
	dword	*DSDT;
	byte	 unneded2[48 - 44];
	dword	*SMI_CMD;
	byte	 ACPI_ENABLE;
	byte	 ACPI_DISABLE;
	byte	 unneded3[64 - 54];
	dword	*PM1a_CNT_BLK;
	dword	*PM1b_CNT_BLK;
	byte	 unneded4[89 - 72];
	byte	 PM1_CNT_LEN;
};

#endif /* _ACPI_POWER_OFF_C_ */
