
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
//#include

enum Mode {PARAM,EXPRESSION};

int getMode(int argc,char** argv){
	int argNr = 1;
	int mode = -1; //-1 = argError
	int i;
	for(i = argNr; i<argc; i++){
		if(strcmp("-m",argv[i])){
			if(++i<argc){
				mode = atoi(argv[i]);
			}
			return mode; //if end of args return arg error

		}
	}
	return mode;
}

int getStartStopParam(int argc,char** argv,int* start,int* end){
	int argNr = 1;
	int statusS = -1;
	int statusE = -1;
	int i ;
	for(i= argNr; i<argc; i++){
		if(strcmp("-s",argv[i])){
			if(++i<argc){
				*start = atoi(argv[i]);
				statusS = 1;
			}
		}else{
			if(strcmp("-e",argv[i])){
				if(++i<argc){
					*end = atoi(argv[i]);
					statusE = 1;
				}
			}
		}
	}
	return statusS | statusE;
}

int getExpression(int argc,char** argv,char* expr){
	int argNr = 1;
	int status = -1;
	int i;
	for(i = argNr; i<argc; i++){
		if(strcmp("-ex",argv[i])){
			if(++i<argc){
				expr= argv[i];
				status = 1;
			}
			return status; //if end of args return arg error

		}
	}
	return status;
}
