/* Copy code and data from ROM to RAM */

#include <types/basic.h>

void just_copy(char *from, char *to, size_t size)
{
	size_t i;
	for (i = 0; i < size; i++)
		to[i] = from[i];
}

void copy_to_RAM()
{
	extern char kern_code_ROM, KERN_CODE_RAM, kern_code_size;
	extern char kern_data_ROM, KERN_DATA_RAM, kern_data_size;
	extern char prog_code_ROM, PROG_CODE_RAM, prog_code_size;
	extern char prog_data_ROM, PROG_DATA_RAM, prog_data_size;

	just_copy(&kern_code_ROM, &KERN_CODE_RAM, (size_t) &kern_code_size);
	just_copy(&kern_data_ROM, &KERN_DATA_RAM, (size_t) &kern_data_size);
	just_copy(&prog_code_ROM, &PROG_CODE_RAM, (size_t) &prog_code_size);
	just_copy(&prog_data_ROM, &PROG_DATA_RAM, (size_t) &prog_data_size);
}
