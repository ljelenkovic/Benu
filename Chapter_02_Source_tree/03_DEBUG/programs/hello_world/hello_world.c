/*! Hello world program */

#include <stdio.h>
#include <api/prog_info.h>
#include <api/errno.h>
#include <kernel/errno.h>

int hello_world()
{
	printf("Example program: [%s:%s]\n%s\n\n", __FILE__, __FUNCTION__,
		 hello_world_PROG_HELP);

	printf("Hello World!\n");

#if 0	/* test escape sequence */
	printf("\x1b[20;40H" "Hello World at 40, 20!\n");
#endif


#if 1	/* compile with 'debug=yes' and without */
	LOG(WARN, "This is log entry with WARN relevance");
	LOG(INFO, "Address of 'hello_world' is %x", hello_world);

	ASSERT_ERRNO_AND_RETURN(TRUE, EINVAL);

	ASSERT(TRUE);
	//ASSERT(FALSE);
#endif

	return 0;
}
