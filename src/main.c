#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "BF.h"
#include "HP.h"
#include "HT.h"

#define FILENAMEHP "build/heapFile"
#define FILENAMEHT "build/hashFile"
#define MAX_FILES 100
#define MAX_BLOCKS 500

int main(int argc, char **argv)
{
    //Getting information about the method, the input file and the buckets(only HT)
    //I'm going to use from the command line
    if (argc < 3 || argc > 4)
    {
        printf("Wrong amount of arguments: \nThe correct format is:\n./test (integer with value 1 or 5 or 10 or 15) (method to be used HT or HP) (if method is HT here goes an integer the amount of buckets to be used)\n");
        return -1;
    }
    char numOfRecs[10];
    char method[10];
    char numOfBuckets[10];
    strcpy(numOfRecs, argv[1]);
    strcpy(method, argv[2]);

    if (!strcmp(method, "HT") && (atoi(numOfRecs) == 1 || atoi(numOfRecs) == 5 || atoi(numOfRecs) == 10 || atoi(numOfRecs) == 15))
    {
        printf("HT here!!\n");
        if (argc != 4)
        {
            printf("Wrong amount of arguments: \nThe correct format is:\n./test (integer with value 1 or 5 or 10 or 15) (method to be used HT or HP) (if method is HT here goes an integer the amount of buckets to be used)\n");
            return -1;
        }
        strcpy(numOfBuckets, argv[3]);
        for (int i = 0, n = strlen(numOfBuckets); i < n; i++)
        {
            if (numOfBuckets[i] < 48 || numOfBuckets[i] > 57)
            {
                printf("Error! The number of buckets can only be an integer!\n");
                printf("The correct format is:\n./test (integer with value 1 or 5 or 10 or 15) (method to be used HT or HP) (if method is HT here goes an integer the amount of buckets to be used)\n");
                return -1;
            }
        }
    }
    else if (!strcmp(method, "HP") && (atoi(numOfRecs) == 1 || atoi(numOfRecs) == 5 || atoi(numOfRecs) == 10 || atoi(numOfRecs) == 15))
    {
        printf("HP here!!\n");
        if (argc != 3)
        {
            printf("Wrong amount of arguments: \nThe correct format is:\n./test (integer with value 1 or 5 or 10 or 15) (method to be used HT or HP) (if method is HT here goes an integer the amount of buckets to be used)\n");
            return -1;
        }
    }
    else
    {
        printf("Wrong format of arguments: \nThe correct format is:\n./test (integer with value 1 or 5 or 10 or 15) (method to be used HT or HP) (if method is HT here goes an integer the amount of buckets to be used)\n");
        return -1;
    }
    //Extraacting records from the file according to the argv input
    char file[25];
    strcpy(file, "examples/records");
    strcat(file, numOfRecs);
    strcat(file, "K.txt");
    FILE *fileInput = fopen(file, "r");
    int entryCount = atoi(numOfRecs) * 1000;
    Record recs[entryCount];
    int i, j;
    char *koma = ",";
    char *key;
    char input[110];
    char strings[4][50];
    int id;

    for (i = 0; i < entryCount; i++)
    {
        fscanf(fileInput, "%s", input);

        key = strtok(input, koma);
        j = 0;
        for (j = 0; key != NULL; j++)
        {
            strcpy(strings[j], key);
            key = strtok(NULL, koma);
        }

        sscanf(strings[0], "{%d", &id);

        int k;
        for (j = 1; j < 4; j++)
        {
            for (k = 0; k < strlen(strings[j]) - 1; k++)
            {
                strings[j][k] = strings[j][k + 1];
            }
            strings[j][k - 1] = '\0';
        }
        strings[3][k - 2] = '\0';
        //Copying  the records
        recs[i].id = id;
        strcpy(recs[i].name, strings[1]);
        strcpy(recs[i].surname, strings[2]);
        strcpy(recs[i].address, strings[3]);
    }
    //Initializing viriables
    BF_Init();
    int insertedSucc = 0;
    int insertedUnsucc = 0;
    int foundBeforeDelete = 0;
    int notFoundBeforeDeleted = 0;
    int foundAfterDelete = 0;
    int notFoundAfterDelete = 0;
    int deletedSucc = 0;
    int deletedUnsucc = 0;
    //If the method given is HP
    if (!strcmp(method, "HP"))
    {
        //Creating file
        if (HP_CreateFile(FILENAMEHP, 'c', "keyhp", 5) < 0)
        {
            perror("Error creating file\n");
            exit(1);
        }
        //Opening file
        HP_info *header = HP_OpenFile(FILENAMEHP);
        //Inserting all records that we extracted from the input file
        //to the file
        for (i = 0; i < entryCount; i++)
        {
            if (HP_InsertEntry(*header, recs[i]) > 0)
            {
                printf("Entry inserted correctly\n");
                insertedSucc++;
            }
            else
            {
                printf("couldn't insert entry\n");
                insertedUnsucc++;
            }
        }
        //Checking to see if all records that we inserted are in the file
        for (i = 0; i < entryCount; i++)
        {
            if (HP_GetAllEntries(*header, &recs[i].id) > 0)
            {
                printf("Entry found\n");
                foundBeforeDelete++;
            }
            else
            {
                printf("Entry not found\n");
                notFoundBeforeDeleted++;
            }
        }
        //Deleting the first half
        for (i = 0; i < entryCount / 2; i++)
        {
            if (HP_DeleteEntry(*header, &(recs[i].id)) == 0)
            {
                printf("Entry deleted correctly\n");
                deletedSucc++;
            }
            else
            {
                printf("Couldn't delete entry\n");
                deletedUnsucc++;
            }
        }
        //Checking that indeed half of them got deleted
        for (i = 0; i < entryCount; i++)
        {
            if (HP_GetAllEntries(*header, &(recs[i].id)) > 0)
            {
                printf("Entry found\n");
                foundAfterDelete++;
            }
            else
            {
                printf("Entry not found\n");
                notFoundAfterDelete++;
            }
        }
        //Closing file
        HP_CloseFile(header);
    }
    //If the method that is given is HT
    else if (!strcmp(method, "HT"))
    {
        //Creating file
        if (HT_CreateIndex(FILENAMEHT, 'c', "keyht", 5, atoi(numOfBuckets)) < 0)
        {
            perror("Error creating file\n");
            exit(1);
        }
        //Opening file
        HT_info *header = HT_OpenIndex(FILENAMEHT);
        //Inserting all records from input file to the file I created
        for (i = 0; i < entryCount; i++)
        {
            if (HT_InsertEntry(*header, recs[i]) > 0)
            {
                printf("Entry inserted correctly\n");
                insertedSucc++;
            }
            else
            {
                printf("couldn't insert entry\n");
                insertedUnsucc++;
            }
        }
        //Checking if they are there
        for (i = 0; i < entryCount; i++)
        {
            if (HT_GetAllEntries(*header, &recs[i].id) > 0)
            {
                printf("Entry found\n");
                foundBeforeDelete++;
            }
            else
            {
                printf("Entry not found\n");
                notFoundBeforeDeleted++;
            }
        }
        //Deleting half of them
        for (i = 0; i < entryCount / 2; i++)
        {
            if (HT_DeleteEntry(*header, &(recs[i].id)) == 0)
            {
                printf("Entry deleted correctly\n");
                deletedSucc++;
            }
            else
            {
                printf("Couldn't delete entry\n");
                deletedUnsucc++;
            }
        }
        //Checking again to see that half of them indeed got deleted
        for (i = 0; i < entryCount; i++)
        {
            if (HT_GetAllEntries(*header, &(recs[i].id)) > 0)
            {
                printf("Entry found\n");
                foundAfterDelete++;
            }
            else
            {
                printf("Entry not found\n");
                notFoundAfterDelete++;
            }
        }
        //Closing file
        HT_CloseIndex(header);
        //Printing Hash Statistics
        if (HashStatistics(FILENAMEHT) < 0)
        {
            printf("Error getting statistics\n");
        }
    }
    //Else Error
    else
    {
        perror("Error");
        return -1;
    }
    //Printing usefull info
    printf("\nMethod used: %s\n", method);
    if (!strcmp(method, "HT"))
    {
        printf("Number of buckets used: %s\n", numOfBuckets);
    }
    printf("Input file used: %s\n\n", file);
    printf("Inserted %d entries successfully\n", insertedSucc);
    printf("Inserted %d entries unsuccessfully\n", insertedUnsucc);
    printf("Found %d entries before deleting\n", foundBeforeDelete);
    printf("Did not find %d entries before deleting\n", notFoundBeforeDeleted);
    printf("Deleted %d entries successfully\n", deletedSucc);
    printf("Deleted %d entries unsuccessfully\n", deletedUnsucc);
    printf("Found %d entries after deleting\n", foundAfterDelete);
    printf("Did not find %d entries after deleting\n", notFoundAfterDelete);

    return 0;
}
