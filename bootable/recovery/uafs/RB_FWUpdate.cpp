/************************************************
** Empty implementation
*************************************************/

#include "./headerfiles/RB_ImageUpdate.h"


long RB_ReadImageNewKey(void *pbUserData, unsigned char *pbBuffer, unsigned long dwStartAddress, unsigned long dwSize)
{
	return RB_ReadImage(pbUserData, pbBuffer, dwStartAddress, dwSize);
}

long RB_ReadImage(
void *pbUserData,
unsigned char *pbBuffer,			/* pointer to user buffer */
unsigned long dwStartAddress,		/* memory address to read from */
unsigned long dwSize)
{
	return 0;
}

long RB_WriteBlock(
void *pbUserData,
unsigned long dwBlockAddress,
unsigned char *pbBuffer)
{
	return 0;
}

long RB_ReadBackupBlock(
void *pbUserData,
unsigned char *pbBuffer,
unsigned long dwBlockAddress,
unsigned long dwSize)
{
	return 0;
}
long RB_WriteBackupBlock(
void *pbUserData,
unsigned long dwBlockStartAddress,
unsigned char *pbBuffer)
{
	return 0;
}

long RB_GetBlockSize(void) 
{
	return 1024*1024*64;
}
