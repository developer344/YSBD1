#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include "HP.h"

int HP_CreateFile(char *fileName, char attrType, char *attrName, int attrLength)
{
     BF_Init();
     HP_info *info;
     info = malloc(sizeof(HP_info));

     if (BF_CreateFile(fileName) < 0)
     {
          BF_PrintError("Error creating file");
          return -1;
     }
     if (info->fileDesc = BF_OpenFile(fileName) < 0)
     {
          BF_PrintError("Error opening file");
          return -1;
     }
     if (BF_AllocateBlock(info->fileDesc) < 0)
     {
          BF_PrintError("Error allocating block");
          return -1;
     }
     void *block;
     if (BF_ReadBlock(info->fileDesc, 0, &block) < 0)
     {
          BF_PrintError("Error reading block");
          return -1;
     }
     //Initialisation of HP_info
     info->attrLength = attrLength;
     info->attrName = malloc(sizeof(char) * info->attrLength);
     info->attrType = attrType;
     info->nextBlock = -1;
     strncpy(info->attrName, attrName, attrLength);
     //First block has only HP info
     memcpy(block, info, sizeof(HP_info));
     if (BF_WriteBlock(info->fileDesc, 0))
     {
          BF_PrintError("Error writting block");
          return -1;
     }

     if (BF_CloseFile(info->fileDesc) < 0)
     {
          BF_PrintError("Error closing file");
          return -1;
     }
     return 0;
}

HP_info *HP_OpenFile(char *fileName)
{
     //Opening file and returning header info
     int fDesc;
     if (fDesc = BF_OpenFile(fileName) < 0)
     {
          BF_PrintError("Error opening file");
          return NULL;
     }
     void *block;
     if (BF_ReadBlock(fDesc, 0, &block) < 0)
     {
          BF_PrintError("Error reading block");
          return NULL;
     }
     HP_info *returnValue = malloc(sizeof(HP_info));
     memcpy(returnValue, block, sizeof(HP_info));
     if (BF_WriteBlock(fDesc, 0))
     {
          BF_PrintError("Error writting block");
          return NULL;
     }
     return returnValue;
}

int HP_CloseFile(HP_info *header_info)
{
     //Closing file
     if (BF_CloseFile(header_info->fileDesc) < 0)
     {
          BF_PrintError("Error closing file");
          return -1;
     }
     return 0;
}

int HP_InsertEntry(HP_info header_info, Record record)
{
     //I check to see if Record with the same id already exists
     if (HP_GetAllEntries(header_info, &record.id) > 0)
     {
          printf("Error: record with the same id is already in the file\n");
          return -1;
     }
     int headerindex = 0;
     int blockindex = -1;
     int next;
     void *block;
     //Opening header block to see the position of the first block
     if (BF_ReadBlock(header_info.fileDesc, 0, &block) < 0)
     {

          BF_PrintError("Error reading block");
          return -1;
     }
     //Storing the position of the first block with records
     HP_info *hpinfo = block;
     blockindex = hpinfo->nextBlock;
     if (BF_WriteBlock(header_info.fileDesc, 0) < 0)
     {
          BF_PrintError("Error writting block");
          return -1;
     }

     //If the position of the first block is -1 I make the first block and I put the record there
     if (blockindex != -1)
     {
          next = blockindex;
          //If the first block exists I check every block to see where I can put the record
          do
          {
               blockindex = next;

               if (BF_ReadBlock(header_info.fileDesc, blockindex, &block) < 0)
               {

                    BF_PrintError("Error reading block");
                    return -1;
               }
               BlockInfo *b_info = block;
               //If there is space in the block so that at least 20% of the block is empty
               //then I put the record in the block
               if (b_info->bytesLeft > sizeof(Record) + 0.2 * BLOCK_SIZE)
               {
                    //I copy the record to the correct place
                    memcpy(block + sizeof(BlockInfo) + (b_info->numOfRec * sizeof(Record)), &record, sizeof(Record));
                    //I raise the number of records of the block
                    b_info->numOfRec += 1;
                    //I decrease the number of bytes left in the block by the size of the record
                    b_info->bytesLeft -= (int)sizeof(Record);
                    memcpy(block, b_info, sizeof(BlockInfo));
                    if (BF_WriteBlock(header_info.fileDesc, blockindex) < 0)
                    {
                         BF_PrintError("Error writting block");
                         return -1;
                    }
                    printf("\nRecord with id: %d, name: %s, surname: %s, address: %s\n", record.id, record.name, record.surname, record.address);
                    printf("Added to Block: %d\n\n", blockindex);
                    //returning the position of the block that it was stored in
                    return blockindex;
               }
               next = b_info->nextblock;
               memcpy(block, b_info, sizeof(BlockInfo));
               if (BF_WriteBlock(header_info.fileDesc, blockindex) < 0)
               {
                    BF_PrintError("Error writting block");
                    return -1;
               }

          } while (next != -1);
          if (BF_AllocateBlock(header_info.fileDesc) < 0)
          {
               BF_PrintError("Error allocating block");
               return -1;
          }
          //If the program exits the loop that means that I need to create a new block
          //and the last block takes position of the new block and puts it to nextblock variable
          if (BF_ReadBlock(header_info.fileDesc, blockindex, &block) < 0)
          {
               BF_PrintError("Error reading block");
               return -1;
          }

          BlockInfo *b_info = block;
          b_info->nextblock = BF_GetBlockCounter(header_info.fileDesc) - 1;
          memcpy(block, b_info, sizeof(BlockInfo));
          if (BF_WriteBlock(header_info.fileDesc, blockindex))
          {
               BF_PrintError("Error writting block");
               return -1;
          }
     }
     else
     {
          //Allocating the new block
          if (BF_AllocateBlock(header_info.fileDesc) < 0)
          {
               BF_PrintError("Error allocating block");
               return -1;
          }
          //If I create the first block after the header block then I put the position of the new block
          //to the next position variable of the header block.
          if (BF_ReadBlock(header_info.fileDesc, headerindex, &block) < 0)
          {
               BF_PrintError("Error reading block");
               return -1;
          }
          HP_info *hp_i = block;
          hp_i->nextBlock = BF_GetBlockCounter(header_info.fileDesc) - 1;
          memcpy(block, hp_i, sizeof(HP_info));
          if (BF_WriteBlock(header_info.fileDesc, headerindex))
          {
               BF_PrintError("Error writting block");
               return -1;
          }
     }

     if (BF_ReadBlock(header_info.fileDesc, (BF_GetBlockCounter(header_info.fileDesc) - 1), &block) < 0)
     {
          BF_PrintError("Error reading block");
          return -1;
     }
     //Reading the new block and initializing its variables

     BlockInfo *binfo = malloc(sizeof(BlockInfo));
     binfo->numOfRec = 1;
     binfo->nextblock = -1;
     binfo->bytesLeft = BLOCK_SIZE - sizeof(BlockInfo) - sizeof(Record);
     memcpy(block, binfo, sizeof(BlockInfo));
     //Storing the record to the new block
     memcpy(block + sizeof(BlockInfo), &record, sizeof(Record));
     if (BF_WriteBlock(header_info.fileDesc, (BF_GetBlockCounter(header_info.fileDesc) - 1)) < 0)
     {
          BF_PrintError("Error writting block");
          return -1;
     }
     free(binfo);
     printf("\nRecord with id: %d, name: %s, surname: %s, address: %s\n", record.id, record.name, record.surname, record.address);
     printf("Added to Block: %d\n\n", BF_GetBlockCounter(header_info.fileDesc) - 1);
     //returning the position of the block that it was stored in
     return BF_GetBlockCounter(header_info.fileDesc) - 1;
}

int HP_DeleteEntry(HP_info header_info, void *value)
{
     int blockindex;
     //Checking if there is a record in the file with such value so that it can be deleted
     if ((blockindex = HP_GetAllEntries(header_info, value)) < 0)
     {
          printf("There is no record with value: %d\n", *((int *)value));
          return -1;
     }
     //If the record with the value given is in one of the blocks then HT_GetAllEntries  returns the
     //position of the block that it is in so it can be deleted
     void *block;
     //Reading the block given by HT_GetAllEntries
     if (BF_ReadBlock(header_info.fileDesc, blockindex, &block) < 0)
     {
          BF_PrintError("Error reading block");
          return -1;
     }
     BlockInfo *b_info = block;
     int i;
     //Searching the record in the block
     for (i = 0; i < b_info->numOfRec; i++)
     {
          Record *record = block + sizeof(BlockInfo) + (i * (int)sizeof(Record));
          if (record->id == *((int *)value))
          {
               //When I find it I check if it is located at the last position of the block
               if (i == (b_info->numOfRec - 1))
               {
                    //If it is I just decrease the amount of blocks in the block and I increase the amount of
                    //bytes left by the size of the record
                    b_info->numOfRec -= 1;
                    b_info->bytesLeft += (int)sizeof(Record);
                    //Copy the info back to the block
                    memcpy(block, b_info, sizeof(BlockInfo));
                    if (BF_WriteBlock(header_info.fileDesc, blockindex) < 0)
                    {
                         BF_PrintError("Error writting block");
                         return -1;
                    }
                    //Return 0 for success
                    return 0;
               }
               else
               {
                    //If its not the last record in the block I take the last record and put it in its place
                    Record *lastBlockRecord = block + sizeof(BlockInfo) + ((b_info->numOfRec - 1) * (int)sizeof(Record));
                    memcpy(block + sizeof(BlockInfo) + (i * (int)sizeof(Record)), lastBlockRecord, sizeof(Record));
                    //Updating inforation
                    b_info->numOfRec -= 1;
                    b_info->bytesLeft += sizeof(Record);
                    //Storing back inforamtion
                    memcpy(block, b_info, sizeof(BlockInfo));
                    if (BF_WriteBlock(header_info.fileDesc, blockindex) < 0)
                    {
                         BF_PrintError("Error writting block");
                         return -1;
                    }
                    //Return 0 for success

                    return 0;
               }
          }
     }
     if (BF_WriteBlock(header_info.fileDesc, blockindex) < 0)
     {
          BF_PrintError("Error writting block");
          return -1;
     }
     //Return -1 for failure
     return -1;
}

int HP_GetAllEntries(HP_info header_info, void *value)
{
     void *block;
     int headerindex = 0;
     int blockindex = -1;
     int next = 1;
     //Opening header block to see the position of the first block

     if (BF_ReadBlock(header_info.fileDesc, headerindex, &block) < 0)
     {

          BF_PrintError("Error reading block");
          return -1;
     }
     //Storing the position of the first block with records
     HP_info *hpinfo = block;
     blockindex = hpinfo->nextBlock;
     if (BF_WriteBlock(header_info.fileDesc, headerindex) < 0)
     {
          BF_PrintError("Error writting block");
          return -1;
     }
     //If the position of the first block is -1 then the file has no records so I return -1
     if (blockindex != -1)
     {
          next = blockindex;
          //I search every block for the record with the given value
          do
          {
               blockindex = next;
               if (BF_ReadBlock(header_info.fileDesc, blockindex, &block) < 0)
               {
                    BF_PrintError("Error reading block");
                    return -1;
               }
               BlockInfo *b_info = block;
               //Searching all the position of each block
               for (int i = 0; i < b_info->numOfRec; i++)
               {
                    Record *record = block + sizeof(BlockInfo) + (i * (int)sizeof(Record));
                    if (record->id == *((int *)value))
                    {
                         //If I find th record I return the position of the block that it was stored in
                         if (BF_WriteBlock(header_info.fileDesc, blockindex) < 0)
                         {
                              BF_PrintError("Error writting block");
                              return -1;
                         }

                         return blockindex;
                    }
               }
               //If not I go to the next block
               next = b_info->nextblock;
               if (BF_WriteBlock(header_info.fileDesc, blockindex) < 0)
               {
                    BF_PrintError("Error writting block");
                    return -1;
               }
               //If I traverse through all the blocks and I don't find it I return -1
          } while (next != -1);
     }
     return -1;
}