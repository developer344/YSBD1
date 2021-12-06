#ifndef _HP_H_
#define _HP_H_

#include "BF.h"

#define BLOCK_INFO_SIZE 12

typedef struct
{
     int fileDesc;   /*4bytes αναγνωριστικός αριθμός ανοίγματος αρχείου από το επίπεδο block */
     char attrType;  /*1byte ο τύπος του πεδίου που είναι κλειδί για το συγκεκριμένο αρχείο, 'c' ή'i' */
     char *attrName; /*attrLength + 1bytes το όνομα του πεδίου που είναι κλειδί για το συγκεκριμένο αρχείο */
     int attrLength; /*4bytes το μέγεθος του πεδίου που είναι κλειδί για το συγκεκριμένο αρχείο */
     int nextBlock;
} HP_info;

typedef struct
{
     int nextblock;
     int numOfRec;
     int bytesLeft;
} BlockInfo;

int HP_CreateFile(char *fileName, char attrType, char *attrName, int attrLength);

HP_info *HP_OpenFile(char *fileName);

int HP_CloseFile(HP_info *header_info);

int HP_InsertEntry(HP_info header_info, /* επικεφαλίδα του αρχείου*/ Record record /* δομή που προσδιορίζει την εγγραφή */);

int HP_DeleteEntry(HP_info header_info, /* επικεφαλίδα του αρχείου*/ void *value /* τιμή του πεδίου-κλειδιού προς διαγραφή */);

int HP_GetAllEntries(HP_info header_info, /* επικεφαλίδα του αρχείου */ void *value /* τιμή του πεδίου-κλειδιού προς αναζήτηση */);
#endif