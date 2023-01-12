#include <asd.h>

#define STR_IMPL(x) #x
#define STR(x) STR_IMPL(x)

#ifdef MLX_LIBVER
#pragma message "overriden"
char *libver = STR(MLX_LIBVER);
#else
#pragma message "not overriden"
char *libver = "1.0";
#endif

char *mlx_getver(){
	asd();
	return libver;
}