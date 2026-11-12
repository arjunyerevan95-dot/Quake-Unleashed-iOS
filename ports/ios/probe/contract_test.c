#include "probe.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>
static void*missing(const char*n){(void)n;return 0;}
static void noop(void){}
static void old_version(unsigned n,int*out){*out=n==0x821B?3:1;}
static void*old(const char*n){if(!strcmp(n,"glGetIntegerv"))return(void*)old_version;return(void*)noop;}
int main(void){
 qvru_probe_result a=qvru_probe(0);assert(!a.passed&&strstr(a.detail,"resolver"));
 qvru_probe_result b=qvru_probe(missing);assert(!b.passed&&strstr(b.detail,"Missing glGetString"));
 qvru_probe_result c=qvru_probe(old);assert(!c.passed&&strstr(c.detail,"got 3.1"));
 puts("PASS null resolver / missing entry point / insufficient GL version rejected");
}
