#include <stdio.h>
#include <signal.h>

void handler(){
  printf("Catched exception!\n");
}

int main(){
  int a=5,b=1;
  signal(SIGFPE, handler);
  printf("%d", (a+b)/(a-5));
  printf("\nPost error!\n");
  return 0;
}
