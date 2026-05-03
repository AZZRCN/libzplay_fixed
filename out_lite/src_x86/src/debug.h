#include <assert.h>

#if _DEBUG
	#define ASSERT_W(f)	assert(f)
	#define PRINT_INT(a) printf("%i\n", a)
	#define PRINT_UINT(a) printf("%u\n", a)
	#define PRINT_FLOAT(a) printf("%f\n", a)
	#define PRINT_STRING(a) printf("%s\n", a)
#else
	#define ASSERT_W(f) ((void)0)
	#define PRINT_INT(a) ((void)0)
	#define PRINT_UINT(a) ((void)0)
	#define PRINT_FLOAT(a) ((void)0)
	#define PRINT_STRING(a) ((void)0)
#endif