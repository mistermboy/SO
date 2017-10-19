#include <stdio.h>
#include <string.h>
#include "Messages.h"

#define MSGMAXIMUMLENGTH 132

DEBUG_MESSAGES DebugMessages[NUMBEROFMSGS] = {[0 ... NUMBEROFMSGS-1] {-1,""}};

int Messages_Load_Messages(int position, char * nameFileMessage) {

	char lineRead[MSGMAXIMUMLENGTH];
	FILE *mf;
	char *number, *text;
	int msgNumber;
	int lineNumber=0;;
	int rc;

	mf=fopen(nameFileMessage, "r");
	if (mf==NULL) {
	   printf("Verbose messages unavailable\n");
	   return -1;
	  }
	   
	while (fgets(lineRead, MSGMAXIMUMLENGTH, mf) != NULL) {
		lineNumber++;
		number=strtok(lineRead,",");
 		if ((number!=NULL) && (number[0]!='/') && (number[0]!='\n')) {
  		rc=sscanf(number,"%d",&msgNumber);
    	if (rc==0){
				printf("Illegal Message Number in line %d of file %s\n",lineNumber,nameFileMessage);
				continue;
			}
	
			text=strtok(NULL,"\n");
  		if (text==NULL){
				printf("Illegal Message Format in line %d of file %s\n",lineNumber,nameFileMessage);
				continue;
			}

			strcpy(DebugMessages[position].format,text);
			DebugMessages[position++].number=msgNumber;
		}
	}
  fclose(mf);
  return position;
}

int Messages_Get_Pos(int number) {
	int pos;

	for (pos=0; DebugMessages[pos].number !=-1; pos++)
 		if (DebugMessages[pos].number==number)
 			return pos;

	return -1;
}
