#include <stdio.h>

#define PRINT_DEFINE "void print_define(char *name, char *text){printf(\"#define\" ); printf(name); printf(text);}"
#define PRINT_F "void print_f(char *text){printf(text);}"
#define MAIN "int main(){print_define(\"PRINT_DEFINE\", PRINT_DEFINE); printf_define(\"PRINT_F\", PRINT_F); print_define(\"MAIN\", MAIN); print_f(PRINT_DEFINE); print_f(PRINT_F); print_f(MAIN);}"

// Outside

void print_define(char *name, char *text){printf("#define" ); printf(name); printf(text);}

void print_f(char *text){printf(text);}

int main(){print_define("PRINT_DEFINE", PRINT_DEFINE); printf_define("PRINT_F", PRINT_F); print_define("MAIN", MAIN); print_f(PRINT_DEFINE); print_f(PRINT_F); print_f(MAIN);}