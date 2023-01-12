#include <stdio.h>
#include <mlx.h>

int main(){
	char *ver = mlx_getver();
	printf("Hello World from MLXLIB v%s!\n", ver);
	return 0;
}