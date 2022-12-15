#include "main.h"


CMainThread theApp;

int main(int argc, char *argv[]){
	time_t t = time(NULL);
    srand((uint32_t)t);
	
	mongoc_init ();

	theApp.Start();
    theApp.WaitCompleted();
    printf("\n**** END *****\n");	


	mongoc_cleanup ();

	
    return 1;
}