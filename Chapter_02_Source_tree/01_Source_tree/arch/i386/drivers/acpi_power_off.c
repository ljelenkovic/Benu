/*! ACPI power off (just for convenience and automation in testing) */
/* from: http://stackoverflow.com/questions/3145569/how-to-power-down-the-computer-from-a-freestanding-environment */
//
/* here is the slightly complicated ACPI power-off code */
//

#define _ACPI_POWER_OFF_C_
#include "acpi_power_off.h"

#include <lib/string.h>
#include "../io.h"
#include <arch/processor.h>


static dword	*SMI_CMD;
static byte	 ACPI_ENABLE;
static byte	 ACPI_DISABLE;
static dword	*PM1a_CNT;
static dword	*PM1b_CNT;
static word	 SLP_TYPa;
static word	 SLP_TYPb;
static word	 SLP_EN;
static word	 SCI_EN;
static byte	 PM1_CNT_LEN;

// check if the given address has a valid header
static unsigned int *acpiCheckRSDPtr ( unsigned int *ptr )
{
	char *sig = "RSD PTR ";
	struct RSDPtr *rsdp = (struct RSDPtr *) ptr;
	byte *bptr;
	byte check = 0;
	int i;

	if ( memcmp(sig, rsdp, 8) == 0 )
	{
		// check checksum rsdpd
		bptr = (byte *) ptr;
		for ( i = 0; i < sizeof(struct RSDPtr); i++ )
		{
			check += *bptr;
			bptr++;
		}

		// found valid rsdpd
		if ( check == 0 )
		{
			/*
			 if (desc->Revision == 0)
				LOG ( DEBUG, "acpi 1");
			else
				LOG ( DEBUG, "acpi 2");
			*/
			return (unsigned int *) rsdp->RsdtAddress;
		}
	}

	return NULL;
}



// finds the acpi header and returns the address of the rsdt
static unsigned int *acpiGetRSDPtr (void)
{
	unsigned int *addr;
	unsigned int *rsdp;

	// search below the 1mb mark for RSDP signature
	for ( addr = (unsigned int *) 0x000E0000; (int) addr < 0x00100000;
	      addr += 0x10/sizeof(addr))
	{
		rsdp = acpiCheckRSDPtr (addr);
		if ( rsdp != NULL )
			return rsdp;
	}


	// at address 0x40:0x0E is the RM segment of the ebda
	int ebda = *((short *) 0x40E);	// get pointer
	ebda = ebda * 0x10 & 0x000FFFFF;// transform segment into linear address

	// search Extended BIOS Data Area for
	// the Root System Description Pointer signature
	for ( addr = (unsigned int *) ebda; (int) addr < ebda + 1024;
	      addr += 0x10/sizeof(addr) )
	{
		rsdp = acpiCheckRSDPtr (addr);
		if ( rsdp != NULL )
			return rsdp;
	}

	return NULL;
}



// checks for a given header and validates checksum
static int acpiCheckHeader ( unsigned int *ptr, char *sig )
{
	if ( memcmp(ptr, sig, 4) == 0 )
	{
		char *checkPtr = (char *) ptr;
		int len = *(ptr + 1);
		char check = 0;
		while ( 0 < len--)
		{
			check += *checkPtr;
			checkPtr++;
		}
		if ( check == 0 )
			return 0;
	}
	return -1;
}



static int acpiEnable (void)
{
	// check if acpi is enabled
	if ( ( inw ( (unsigned int) PM1a_CNT ) & SCI_EN ) == 0 )
	{
		// check if acpi can be enabled
		if ( SMI_CMD != 0 && ACPI_ENABLE != 0 )
		{
			outb ( (unsigned int) SMI_CMD, ACPI_ENABLE );
			// send acpi enable command
			// give 3 seconds time to enable acpi

			int i;
			for ( i = 0; i < 300; i++ )
			{
				if ((inw((unsigned int) PM1a_CNT) & SCI_EN)==1)
					break;
				//sleep(10);
			}
			if ( PM1b_CNT != 0 )
				for ( ; i < 300; i++ )
				{
				if ((inw((unsigned int) PM1b_CNT) & SCI_EN)==1)
					break;
				//sleep(10);
				}
			if (i<300) {
				//LOG ( DEBUG, "enabled acpi.");
				return 0;
			} else {
				//LOG ( DEBUG, "couldn't enable acpi.");
				return -1;
			}
		} else {
			//LOG ( DEBUG, "no known way to enable acpi.");
			return -1;
		}
	} else {
		//LOG ( DEBUG, "acpi was already enabled.");
		return 0;
	}
}

static int initAcpi (void)
{
	unsigned int *ptr = acpiGetRSDPtr ();

	// check if address is correct  ( if acpi is available on this pc )
	if ( ptr != NULL && acpiCheckHeader ( ptr, "RSDT" ) == 0 )
	{
		//the RSDT contains an unknown number of pointers to acpi tables
		int entrys = *(ptr + 1);
		entrys = (entrys - 36) /4;
		ptr += 36/4;	// skip header information

		while (0 < entrys--)
		{
//<<<<<<

// check if the desired table is reached
if ( acpiCheckHeader ( (unsigned int *) *ptr, "FACP") == 0 )
{
	entrys = -2;
	struct FACP *facp = (struct FACP *) *ptr;
	if ( acpiCheckHeader ( (unsigned int *) facp->DSDT, "DSDT" ) == 0 )
	{
		// search the \_S5 package in the DSDT
		char *S5Addr = (char *) facp->DSDT +36; // skip header
		int dsdtLength = *(facp->DSDT+1) -36;

		while ( 0 < dsdtLength-- )
		{
			if ( memcmp ( S5Addr, "_S5_", 4 ) == 0 )
				break;
			S5Addr++;
		}
		// check if \_S5 was found
		if ( dsdtLength > 0 )
		{
			// check for valid AML structure
			if (	( *(S5Addr-1) == 0x08 ||
				( *(S5Addr-2) == 0x08 && *(S5Addr-1) == '\\') )
				&& *(S5Addr+4) == 0x12 )
			{
				// calculate PkgLength size
				S5Addr += 5;
				S5Addr += ((*S5Addr &0xC0)>>6) +2;

				if ( *S5Addr == 0x0A )
					S5Addr++;	// skip byteprefix
				SLP_TYPa = *(S5Addr) << 10;
				S5Addr++;

				if ( *S5Addr == 0x0A )
					S5Addr++;	// skip byteprefix
				SLP_TYPb = *(S5Addr) << 10;

				SMI_CMD = facp->SMI_CMD;

				ACPI_ENABLE = facp->ACPI_ENABLE;
				ACPI_DISABLE = facp->ACPI_DISABLE;

				PM1a_CNT = facp->PM1a_CNT_BLK;
				PM1b_CNT = facp->PM1b_CNT_BLK;

				PM1_CNT_LEN = facp->PM1_CNT_LEN;

				SLP_EN = 1 << 13;
				SCI_EN = 1;

				return 0;
			} else {
				//LOG ( DEBUG, "\\_S5 parse error.");
			}
		} else {
			//LOG ( DEBUG, "\\_S5 not present.");
		}
	} else {
		//LOG ( DEBUG, "DSDT invalid.");
	}
}
//<<<<<<

		ptr++;
		}
		//LOG ( DEBUG, "no valid FACP present.");
	} else {
		//LOG ( DEBUG, "no acpi.");
	}

	return -1;
}



void acpiPowerOff (void)
{
	initAcpi ();

	// SCI_EN is set to 1 if acpi shutdown is possible
	if ( SCI_EN == 0 )
		return;

	acpiEnable ();

	// send the shutdown command
	outw ( (unsigned int) PM1a_CNT, SLP_TYPa | SLP_EN );
	if ( PM1b_CNT != 0 )
		outw ( (unsigned int) PM1b_CNT, SLP_TYPb | SLP_EN );

	/* delay a little for poweroff to kick in :) */

	#define _SOME_ 100000000
	int i;
	for ( i = 0; i < _SOME_; i++ )
		arch_memory_barrier();

	//LOG ( DEBUG, "acpi poweroff failed.");
}
