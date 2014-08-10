/**************************************************************
  **					C O N F I D E N T I A L					**
 **					Copyright 2002-2011					**
  **					Red Bend Software					**
 **************************************************************/

/*
Change log:
Date		Owner		Description
9/8/2011	Amir S.		changed MAX_PATH to RB_MAX_PATH and use it
						format if statement for return in RB_DeleteFile
						remove if from RB_ResizeFile and RB_CloseFile
						call RB_SyncFile in RB_CloseFile
						change implementation of RB_FSTrace to run on Android
5/6/2012	Amir S.		changed API function prototype to be of v8.0 i.e. use UTF8 instead of UTF16 in file path
15/7/2012	Amir S.		Fix bug in RB_GetDelta - call fclose for the file descriptor in case of error
20/7/2012	Amir S.		fix strtol base of user and group ID to be 0x10 and not 10. enlarge the attributes buffer to be on the safe side.
30/7/2012	Amir S.		remove the use of #include <sys/statvfs.h>
22/8/2012	Amir S.		Added compare on the number of writen bytes to the number requested
12/9/2012	Amir S.		Fix code for getting partition free size

2013-04-02	notmmao		采用多次读/写的IO方式.
2013-04-07	notmmao		去除__BIONIC_FORTIFY_INLINE宏定义
*/

#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/statfs.h>
#include <sys/types.h>
#include <stdarg.h>

#ifdef __BIONIC_FORTIFY_INLINE
#undef __BIONIC_FORTIFY_INLINE
#endif
#include <fcntl.h>

#include "uafs.h"
#include "./headerfiles/RB_FileSystemUpdate.h"
#include "./headerfiles/RB_vRM_Errors.h"
#include "./headerfiles/RB_vRM_Update.h"

#define FS_U_RAM_SIZE	30*1024*1024
#define FS_ID_MAX_LEN 4
#define GMT_WRITEN_BUFFER_LEN 4096
#ifndef RB_MAX_PATH
#define RB_MAX_PATH 1024
#endif

#define DBG 1
#ifdef DBG
#define printf_dbg printf
#else
#define printf_dbg
#endif

/************************************************************
 *                     common functions
 ************************************************************/
long RB_GetAvailableFreeSpace(
	void *pbUserData,
	const char *partition_name,
	unsigned long* available_flash_size)
{
#if 1
	struct statfs vfs;

	printf_dbg("%s : %s\n", __func__, partition_name);
	if ( statfs(partition_name, &vfs) < 0 )
	{
		printf_dbg("%s : failed to get statvfs\n", __func__);
		*available_flash_size = 0;
		return -1;
	}
	printf_dbg("%s : success %u\n", __func__, vfs.f_bsize);
	printf_dbg("%s : success %llu\n", __func__, vfs.f_bfree);

	*available_flash_size = vfs.f_bsize * vfs.f_bfree;
	printf_dbg("%s : success %lu\n", __func__, *available_flash_size);
#else
	*available_flash_size =  1024 * 1024 * 64;
#endif
	return 0;
}

long RB_GetRBDeltaOffset(
	void *pbUserData,
	unsigned long signed_delta_offset,
	unsigned long* delta_offset)
{
	* delta_offset = signed_delta_offset;
	return 0;
}

long RecursiveFolderCreater(
	const char*	folderpath,
	const		mode_t mode)
{
	int ret = 0;
	char temppath[RB_MAX_PATH] =  {'\0'};
	int pathOffset = strlen(folderpath);// For counting back until the '/' delimiter

	printf_dbg(" %s path: %s\n", __func__, folderpath);
	
	if(pathOffset == 0)
		return -1;//if from some reason we got to the end return error!!!.
		
	while(folderpath[pathOffset] != '/')// get to the next '/' place
		pathOffset--;
			
	strncpy(temppath, folderpath, pathOffset);// copy one depth below till and without the char '/'
	printf_dbg(" temppath: %s\n", temppath);
	
	ret = mkdir(temppath, mode);
	printf_dbg(" mkdir result: %d errno: %d\n", ret, errno);
	if (ret == 0 || ((ret == -1) && (errno == EEXIST)))
	{
		return 0;//meaning the depth creation is success.
	}
	else if((ret == -1) && (errno == ENOENT))
	{
		ret = RecursiveFolderCreater(temppath, mode);
		if (ret == 0)
		{
			ret = mkdir(temppath, mode);
		}
		return ret;
	}
	else
	{
		return -1;
	}
}

/*!
 ************************************************************
 *                     RB_CopyFile
 ************************************************************
 *
 * @brief
 *	This function copies a file to another file with a different name or path.
 *
 *
 *	@param pbUserData	Any user data structure, that may be useful for the user
 *					implementation If not needed set to NULL.
 *					The calling function supply you with the user data,
 *					previously supplied in the RB_FileSystemUpdate.
 *
 *	@param strFromPath	The path where the file exist.
 *
 *	@param strToPath	New destination of the file.
 *
 *	@return			One of the return codes as defined in RB_vRM_Errors.h
 *
 ************************************************************
 */

long RB_CopyFile(
	void*					pbUserData,
	const char*	strFromPath,
	const char*	strToPath)
{
	FILE* fp1 = NULL;
	FILE* fp2 = NULL;
	const int MAX_BUFF_SIZE = 2*1024; //2KB
	unsigned int readCount = 0, writeCount = 0;
	char buf[MAX_BUFF_SIZE];
	int ret = 0;
	
	printf_dbg ("%s: %s -> %s ", __func__, strFromPath, strToPath);
	if (!strFromPath || !strToPath)
	{
		printf_dbg("NULL file name find. Abort.\n");
		return -1;			//should never happen
	}
	
	fp1 = fopen(strFromPath, "r");
	if (!fp1)
	{
		printf_dbg(" Open %s ENOENT %d\n", strFromPath, errno);
		printf_dbg("Open %s failed. Abort.\n", strFromPath);
		return E_RB_OPENFILE_ONLYR;
	}

	fp2 = fopen(strToPath, "w");
	if (!fp2)
	{
		char* folder = strrchr(strToPath,'/'); 
		char* folderPath = (char *) malloc(folder - strToPath + 1); 
		
		if (folderPath == NULL)
		{
			printf_dbg("malloc failure (folderPath).\n");
			return -4;
		}
		memset(folderPath,'\0',folder - strToPath + 1); 
		strncpy(folderPath,strToPath,folder - strToPath);
		if ( RB_CreateFolder(pbUserData, folderPath) != S_RB_SUCCESS )
		{
			fclose(fp1);
			printf_dbg("Open %s failed. Abort.\n", strToPath);
			free(folderPath);
			return E_RB_OPENFILE_WRITE;
		}
		else
		{
			free(folderPath);
			fp2 = fopen(strToPath, "w");
			if(!fp2)
			{
				fclose(fp1);
				printf_dbg("Open %s failed. Abort.\n", strToPath);
				return E_RB_OPENFILE_WRITE;
			}
		}
	}

	while( (readCount = fread(buf, 1, MAX_BUFF_SIZE, fp1))> 0)
	{
		writeCount = fwrite(buf, 1, readCount, fp2);
		if (writeCount != readCount)
		{
			printf_dbg(" read %d, but write %d, abort.\n", readCount, writeCount);
			ret = E_RB_WRITE_ERROR;
 			break;
		}
	}

	fclose(fp1);
	fflush(fp2);
	fsync(fileno(fp2));
	fclose(fp2);

	return ret;
}

/*!
 ************************************************************
 *                     RB_DeleteFile
 ************************************************************
 *
 * @brief
 *	This function deletes a specified file and removes it from the File System.
 *
 *
 *	@param pbUserData	Any user data structure, that may be useful for the user
 *						implementation If not needed set to NULL.
 *						The calling function supply you with the user data,
 *						previously supplied in the RB_FileSystemUpdate.
 *
 *	@param strPath		The path of the file.
 *
 *	@return				One of the return codes as defined in RB_vRM_Errors.h
 *
 ************************************************************
 */

long RB_DeleteFile(
	void*		pbUserData,
	const char*	strPath)
{
	int ret = 0;
 
	printf_dbg ("%s: %s\n", __func__, strPath);
	ret = unlink(strPath);
	printf_dbg (" unlink value: %d, errno: %d\n", ret, ret ? errno : 0);
	
	if (ret < 0 && errno != ENOENT)	//if file does not exist then we can say that we deleted it successfully
	{
		printf("delete failed with return value: %d errno %d\n",ret, errno);
		return E_RB_DELETEFILE;
	}
  	return S_RB_SUCCESS;
}

/*!
 ************************************************************
 *                     RB_DeleteFolder
 ************************************************************
 *
 * @brief
 *	This function deletes a specified folder and removes it from the File System.
 *
 *
 *	@param pbUserData	Any user data structure, that may be useful for the user
 *						implementation If not needed set to NULL.
 *						The calling function supply you with the user data,
 *						previously supplied in the RB_FileSystemUpdate.
 *
 *	@param strPath		The path of the folder.
 *
 *	@return				One of the return codes as defined in RB_vRM_Errors.h
 *
 ************************************************************
 */

long RB_DeleteFolder(
	void*		pbUserData,
	const char*	strPath)
{
	int ret = 0;
	
	printf_dbg ("%s: %s\n", __func__, strPath);

	ret = rmdir(strPath);
 	printf_dbg (" rmdir value: %d, errno: %d\n", ret, errno);
	
	if ((ret == 0) || ((ret < 0) && ((errno == ENOENT) || (errno == ENOTEMPTY ))))
		return S_RB_SUCCESS;
	printf("Delete folder problem value: %d, errno: %d\n", ret, errno);
 	return E_RB_FAILURE;
}

/*!
 ************************************************************
 *                     RB_CreateFolder
 ************************************************************
 *
 * @brief
 *	This function deletes a specified folder and removes it from the File System.
 *
 *
 *	@param pbUserData	Any user data structure, that may be useful for the user
 *						implementation If not needed set to NULL.
 *						The calling function supply you with the user data,
 *						previously supplied in the RB_FileSystemUpdate.
 *
 *	@param strPath		The path of the folder.
 *
 *	@return				One of the return codes as defined in RB_vRM_Errors.h
 *
 ************************************************************
 */

long RB_CreateFolder(
	void*		pbUserData,
	const char*	strPath)
{
	mode_t mode = 0;
	int ret = 0;
	char path[RB_MAX_PATH] = {'\0'};

	mode = 
		S_IRUSR /*Read by owner*/ | 
		S_IWUSR /*Write by owner*/ | 
		S_IXUSR /*Execute by owner*/ | 
		S_IRGRP /*Read by group*/ | 
		S_IWGRP /*Write by group*/ | 
		S_IXGRP /*Execute by group*/ | 
		S_IROTH /*Read by others*/ | 
		S_IWOTH /*Write by others*/ | 
		S_IXOTH /*Execute by others*/;
	
	printf_dbg ("%s: %s, mode:0x%x\n", __func__, strPath, mode);
	
	ret = mkdir(strPath, mode);
	
	if (ret == 0 || ((ret == -1) && (errno == EEXIST)))
	{
		return S_RB_SUCCESS;
	}
	else if((ret == -1) && (errno == ENOENT))//maybe multi directory problem
	{
		ret = RecursiveFolderCreater(strPath, mode);
		if(ret == 0) 
		{
			ret = mkdir(strPath, mode);//After creating all the depth Directories we try to create the Original one again.
			if(ret == 0)
				return S_RB_SUCCESS;
			else
			{
				printf("Create folder problem value: %d, errno: %d\n", ret, errno);
				return E_RB_FAILURE;
			}
		}
		else
		{
			printf("Create folder problem value: %d, errno: %d\n", ret, errno);
			return E_RB_FAILURE;
		}
	}
	else
	{
		printf("Create folder problem value: %d, errno: %d\n", ret, errno);
		return E_RB_FAILURE;
	}
}

/*!
 ************************************************************
 *                     RB_OpenFile
 ************************************************************
 *
 * @brief
 *	Opens a file in the file system.
 *
 *	A glue function that needs to be implemented by the customer.
 *
 *	It should follow the following restrictions:
 *
 * 1. Returning the proper error level \see RB_vRM_Errors.h
 *
 *
 *	@param pbUserData	Any user data structure, that may be useful for the user
 *						implementation If not needed set to NULL.
 *						The calling function supply you with the user data,
 *						previously supplied in the RB_FileSystemUpdate.
 *
 *	@param strPath		An absolute path to the file location in the FS.
 *
 *	@param wFlag		Controls the access mode read, write or both.
 *						opens a file to write deletes the file content.
 *
 *	@param pwHandle		A handle to the file.
 *
 *	@return				One of the return codes as defined in RB_vRM_Errors.h
 *
 ************************************************************
 */
mode_t get_mode(E_RW_TYPE wFlag)
{
	switch (wFlag)
	{
	case ONLY_R:
		printf_dbg(" RDONLY \n");
		return O_RDONLY;
	case ONLY_W:
		printf_dbg(" WRONLY \n");
		return O_WRONLY | O_CREAT;
	case BOTH_RW:
		printf_dbg(" RDWR \n");
		return O_RDWR | O_CREAT;
	default:
		printf_dbg(" Unknown \n");
		return 0;
	}
}

long RB_OpenFile(
	void*		pbUserData,
	const char*	strPath,
	E_RW_TYPE	wFlag,
	long*		pwHandle)
{
	mode_t mode;
	char path[RB_MAX_PATH] = {'\0'};

	mode = get_mode(wFlag);
	printf_dbg("%s: Path:%s | Mode: %d\n", __func__, strPath, mode);
	
	*pwHandle = open(strPath, mode);
	if (*pwHandle == -1)
	{
		*pwHandle = 0;
		printf_dbg(" First open() with error %d\n", errno);
		if (wFlag == ONLY_R)
			return E_RB_OPENFILE_ONLYR;

		//if  we need to open the file for write or read/write then we need to create the folder (in case it does not exist)
		if ((wFlag != ONLY_R) && (errno == ENOENT))
		{
			char dir[RB_MAX_PATH] = {'\0'};
			int i = 0;
			//copy the full file path to directory path variable
			while (strPath[i] != '\0')
			{
				dir[i] = strPath[i];
				i++;
			}
			printf_dbg(" copy dir[]=%s\n", dir);
			//search for the last '/' char
			while (dir[i--] != '/')
				;
			dir[i+1] = '\0';
			printf_dbg(" remove dir[]=%s\n", dir);
			
			if (RB_CreateFolder(pbUserData, dir))
			{
				printf_dbg(" Fail create folder, Leave RB_OpenFile\n");
				return E_RB_OPENFILE_WRITE;
			}
	
			*pwHandle = open(strPath, mode);
			if (*pwHandle == -1 || *pwHandle == 0)
			{
				*pwHandle = 0;
				printf(" After successful creating folder, fail open() with error %d\n", errno);
				return E_RB_OPENFILE_WRITE;
			}
		}
		else
			return E_RB_OPENFILE_WRITE;
 	}
	printf_dbg(" Successful open() *pwHandle:%ld\n", *pwHandle);

	return S_RB_SUCCESS;
}

/*!
 ************************************************************
 *                     RB_ResizeFile
 ************************************************************
 *
 * @brief
 *	set the size of a file in the file system.
 *
 *	@param pbUserData	Any user data structure, that may be useful for the user
 *						implementation If not needed set to NULL.
 *						The calling function supply you with the user data,
 *						previously supplied in the RB_FileSystemUpdate.
 *
 *	@param wHandle		A handle to the file.
 *
 *	@param dwSize		The new size of the file.
 *
 *	@return				One of the return codes as defined in RB_vRM_Errors.h
 *
 ************************************************************
 */

long RB_ResizeFile(
	void*			pbUserData,
	long			wHandle,
	unsigned long	dwSize)
{
	int ret = -1;

	printf_dbg("%s: handle %ld, dwSize %lu\n", __func__, wHandle, dwSize);

	ret = ftruncate(wHandle, dwSize);

	if (ret)
		ret = E_RB_RESIZEFILE;
	
	printf_dbg("%s: ret %d handle %ld %d\n", __func__, ret, wHandle, errno);
	
	return ret;
}

/*!
 ************************************************************
 *                     RB_CloseFile
 ************************************************************
 *
 * @brief
 *	Close a file in the file system.
 *
 *	A glue function that needs to be implemented by the customer.
 *
 *	It should follow the following restrictions:
 *
 * 1. Returning the proper error level \see RB_vRM_Errors.h
 *
 *
 *	@param pbUserData	Any user data structure, that may be useful for the user
 *						implementation, if not needed set to NULL.
 *						The calling function supply you with the user data,
 *						previously supplied in the RB_FileSystemUpdate.
 *
 *	@param wHandle		A handle to the file.
 *
 *	@return				One of the return codes as defined in RB_vRM_Errors.h
 *
 ************************************************************
 */

long RB_CloseFile(
	void*	pbUserData,
	long 	wHandle)
{
	int ret = E_RB_CLOSEFILE_ERROR;
	printf_dbg("%s: wHandle = %ld\n", __func__, wHandle);
	
	if (RB_SyncFile(pbUserData, wHandle) != S_RB_SUCCESS)
	{
		//try to close in any case
		close(wHandle);
		return E_RB_CLOSEFILE_ERROR;
	}
	
	ret = close(wHandle);
	
	if (ret == 0)
		return S_RB_SUCCESS;
	
	return E_RB_CLOSEFILE_ERROR;
}

/*!
 ************************************************************
 *                     RB_WriteFile
 ************************************************************
 *
 * @brief
 *	Writes block of data to an open file in a reliable manner.
 *
 *	A glue function that needs to be implemented by the customer.
 *
 *	It should follow the following restrictions:
 *
 *	1. Returning the proper error level \see RB_vRM_Errors.h
 *	2. The writing procedure should be a transaction.
 *		In case of returning successfully after writing a block means that
 *		the block has been written to its target location, or at least resides
 *		in a NV memory, and an automatic procedure will restore it to its target
 *		location. e.g. a power fail right after returning from the function invocation.
 *
 *	@param pbUserData		Any user data structure, that may be useful for the user
 *							implementation, if not needed set to NULL.
 *							The calling function supply you with the user data,
 *							previously supplied in the RB_FileSystemUpdate.
 *
 *	@param wHandle			Handle to the file.
 *
 *	@param dwPosition		Position were to write
 *
 *	@param pbBuffer			The block of data that should be written.
 *
 *	@param dwSize			The size in bytes of the block to be written.
 *
 *	@return					One of the return codes as defined in RB_vRM_Errors.h
 *
 ************************************************************
 */

long RB_WriteFile(
	void*			pbUserData,
	long			wHandle,
	unsigned long	dwPosition,
	unsigned char*	pbBuffer,
	unsigned long	dwSize)
{
	int ret = 0;
	unsigned long size = 0;
	unsigned long to_writen = 0;

	printf_dbg("%s: Handle:%ld , Pos:%ld , Size: %ld", __func__, wHandle,dwPosition,dwSize);
	size = lseek(wHandle, 0, SEEK_END);
	/* from the guide: if dwPosition is beyond size of file the gap between end-of-file and the position should be filled with 0xff */
	if (size < dwPosition)
	{
		unsigned long heap_size = dwPosition - size;
		unsigned char* p_heap = (unsigned char*)malloc(GMT_WRITEN_BUFFER_LEN);
		if (p_heap == NULL)
		{
			printf_dbg("malloc failure (p_heap).\n");
			return E_RB_WRITE_ERROR;
		}
		memset(p_heap, 0xFF, GMT_WRITEN_BUFFER_LEN);
		to_writen = heap_size;	
		while(to_writen > 0) {
			int min = to_writen < GMT_WRITEN_BUFFER_LEN ? to_writen : GMT_WRITEN_BUFFER_LEN;
			ret = write(wHandle, p_heap, min);
			if (ret < 0) {
				printf_dbg("write failed with return value: %d(%s)\n", ret, strerror(errno));
				return E_RB_WRITE_ERROR;
			}
			to_writen -= ret;
		}
		free(p_heap);
	}
	ret = lseek(wHandle, dwPosition, SEEK_SET);
	if (ret < 0)
	{
		printf_dbg("lseek failed with return value: %d\n",ret);
		return E_RB_WRITE_ERROR;
	}

	to_writen = dwSize;
	while(to_writen > 0) {
		
		ret = write(wHandle, pbBuffer + (dwSize - to_writen), to_writen);
		if (ret < 0) {
			printf_dbg("pbBuffer write failed with return value: %d(%s)\n", ret, strerror(errno));
			return E_RB_WRITE_ERROR;
		}
		to_writen -= ret;
	}

	printf_dbg("Bytes Write: %d\n",ret);

	return S_RB_SUCCESS;
}

/*!
 ************************************************************
 *                     RB_ReadFile
 ************************************************************
 *
 * @brief
 *	Reads data from an open file.
 *
 *	A glue function that needs to be implemented by the customer.
 *
 *	It should follow the following restrictions:
 *
 *	1. Returning the proper error level \see RB_vRM_Errors.h
 *
 *	@param pbUserData	Any user data structure, that may be useful for the user
 *						implementation, if not needed set to NULL.
 *						The calling function supply you with the user data,
 *						previously supplied in the RB_FileSystemUpdate.
 *
 *	@param wHandle		Handle to the file.
 *
 *	@param dwPosition	The offset in the read file that should be
 *						the starting point for the copy.
 *
 *	@param pbBuffer		The gives buffer that the data from the open file should be.
 *						copy into.
 *
 *	@param dwSize		The size in bytes that should be copied from the open file,
 *						starting from the given position offset.
 *
 *	@return				One of the return codes as defined in RB_vRM_Errors.h
 *
 ************************************************************
 */

long RB_ReadFile(
	void*			pbUserData,
	long			wHandle,
	unsigned long	dwPosition,
	unsigned char*	pbBuffer,
	unsigned long	dwSize)
{
	int ret = 0;
	unsigned long to_readn = 0;

	printf_dbg(" %s: Handle:%ld , Pos:%ld , Size: %ld", __func__, wHandle,dwPosition,dwSize);
	
	ret = lseek (wHandle, dwPosition, SEEK_SET);
	if (ret < 0)
	{
		printf_dbg(" lseek failed with return value: %d\n",ret);
		return E_RB_READ_ERROR;
	}
	to_readn = 0;
	while(to_readn < dwSize) {
		ret = read(wHandle, pbBuffer + to_readn, dwSize - to_readn);
		if (ret < 0) {
			printf_dbg(" read failed with return value: %d\n",ret);
			return E_RB_READ_ERROR;
		}
		to_readn += ret;
	}
	
	printf_dbg(" Bytes Read: %d\n",ret);
	if (ret != dwSize && ((ret + dwPosition) != RB_GetFileSize(pbUserData, wHandle)))
		return E_RB_READ_ERROR;
	
	return S_RB_SUCCESS;
}

long RB_GetFileSize(
	void*	pbUserData,
	long	wHandle)
{

/*
	int ret = 0;

	printf_dbg("%s: %ld ", __func__, wHandle);

	ret = lseek(wHandle, 0, SEEK_END);

	if (ret == -1)
	{
		printf_dbg(" lseek errno: %d\n", errno);
		return E_RB_READFILE_SIZE;
	}
	printf_dbg("Returning Size = 0x%lx\n",ret);
	
	return ret;
*/
	struct stat statbuf;
	
	fstat(wHandle, &statbuf);
	printf_dbg("RB_GetFileSize handle=%ld size=%lld\n",wHandle,statbuf.st_size);
	return statbuf.st_size;
}

long RB_Unlink(
	void*	pbUserData,
	char*	pLinkName)
{
	int ret = 0;
	
	printf_dbg("%s \n", __func__);

	ret = unlink(pLinkName);
	if(ret < 0 && errno != ENOENT)
	{
		printf("unlink failed with return value: %d errno %d\n",ret, errno);
		return E_RB_FAILURE;
	}
	printf_dbg("unlink with return value: %d\n",ret);

	return S_RB_SUCCESS;
}

long RB_VerifyLinkReference(
	void*		pbUserData,
	const char*	pLinkName,
	const char*	pReferenceFileName)
{
	int ret = 0;
	char linkedpath[RB_MAX_PATH]={'\0'};
	
	printf_dbg("%s \n", __func__);

	ret = readlink(pLinkName, linkedpath, RB_MAX_PATH);
	if (ret < 0)
	{
		printf("readlink failed with return value: %d\n",ret);
		return E_RB_FAILURE;
	}
	
	if ((memcmp(&linkedpath, &pReferenceFileName, ret))!=0)
	{
		printf("not same linked path - linkedpath[%s] pReferenceFileName[%s]\n", linkedpath, pReferenceFileName);
		return E_RB_FAILURE;
	}
	printf_dbg(" same linked path\n");

	return S_RB_SUCCESS;
}

long RB_Link(
	void*		pbUserData,
	char*		pLinkName,
	char*		pReferenceFileName)
{
	int ret = 0;
	
	printf_dbg("%s %s %s\n", __func__, pLinkName, pReferenceFileName);
	
	if(!RB_VerifyLinkReference(pbUserData, pLinkName, pReferenceFileName))
		return S_RB_SUCCESS;

	ret = symlink(pReferenceFileName, pLinkName);
	if (ret != 0)
	{
		printf_dbg(" symlink failed with return value: %d, errno: %d\n", ret, errno);
		if (errno == EEXIST && RB_VerifyLinkReference(pbUserData, pLinkName, pReferenceFileName))
		{
			return S_RB_SUCCESS;
		}
		return E_RB_FAILED_CREATING_SYMBOLIC_LINK;
	}
	printf_dbg(" symlink with return value: %d\n",ret);

	return S_RB_SUCCESS;
}

// TODO ??? - ui16pFilePath should be (unsigned short *) or (char*) ???

#define ATT 14
#ifdef ATT //SeetFileAttributes from 6.2 till 7.0
long RB_SetFileAttributes(
	void*				pbUserData,
	const char*			ui16pFilePath,
	const unsigned long ui32AttribSize,
	const unsigned char *ui8pAttribs)
{
	const int ATTRSIZE = 30;
	char tmpAttribs[ATTRSIZE];
	char *tp;
	char *endstr;
	char * rb_sig;
	
	uid_t setUserID		= 0;
	gid_t setGroupID	= 0;
	mode_t setFileMode	= 0;
	struct stat sbuf;
	int ret = 0;
	// debug start
	int count = 0;
	// debug end
	
	printf_dbg("%s \n", __func__);
	
	if(NULL == ui16pFilePath)
	{
		printf_dbg("ui16pFilePath NULL [error]\n");
		return E_RB_BAD_PARAMS;
	}
	else if(NULL == ui8pAttribs)
	{
		printf_dbg("ui8pAttribs NULL [error]\n");
		return E_RB_BAD_PARAMS;
	}

	ret = lstat(ui16pFilePath, &sbuf);
	if(ret < 0)
	{	
			printf("stat failed with return value: %d\n",ret);
			return E_RB_FAILURE;
	}
	else
	{
		if(S_ISLNK(sbuf.st_mode))
		{
			printf_dbg(" stat->st_mode = symbolic link file\n");
			return S_RB_SUCCESS;
		}
		if(S_ISREG(sbuf.st_mode))
		{
			printf_dbg(" stat->st_mode = regular file\n");
		}
		if (S_ISDIR(sbuf.st_mode))
		{
			printf_dbg(" stat->st_mode = directory\n");
		}
	}
	
	if(0 == ui32AttribSize)
	{
		printf_dbg("ui32AttribSize 0\n");
		return S_RB_SUCCESS;
	}
	
	printf_dbg("ui16pFilePath = %s\n", ui16pFilePath);
	printf_dbg("ui32AttribSize = %lu\n", ui32AttribSize);
	printf_dbg("ui8pAttribs = %s\n", ui8pAttribs);
	// debug start
	for(count=0;count<ATTRSIZE;count++)
	{
		printf_dbg("ui8pAttribs[%d] = %c\n", count, ui8pAttribs[count]);
	}
	// debug end

	memset((void*)tmpAttribs, 0x0, ATTRSIZE);
	memcpy(tmpAttribs, ui8pAttribs, (size_t)ui32AttribSize);
	
	//Check that the structure is Valid
	if(NULL == strstr(tmpAttribs,"_redbend_"))
	{
		printf("Attributes structure is not valid\n");
		return E_RB_FAILURE;
	}


	tp = strtok((char *)tmpAttribs, ":");

	//Remove the _redbend_ SAFIX
	rb_sig = strrchr(tp,'_');
	rb_sig++;

	
	// Get FileMode
	setFileMode = strtol(rb_sig, &endstr, 8);
	tp = strtok(NULL, ":");

	// Get UserID
	if (tp != NULL)
	{
		setUserID = (uid_t)strtol(tp, &endstr, 0x10);
		tp = strtok(NULL, ":");
	}

	// Get GroupID
	if (tp != NULL)
	{
		setGroupID = (gid_t)strtol(tp, &endstr, 0x10);
	}

	// Set FileMode
	printf_dbg("ui16pFilePath = %s\n",ui16pFilePath);
	printf_dbg("setFileMode = %d\n", setFileMode);
	printf_dbg("setFileMode = %o\n", setFileMode);
	printf_dbg("setUserID = %d\n", setUserID);
	printf_dbg("setGroupID = %d\n", setGroupID);
	
	// Set UserID,GroupID
	if( chown(ui16pFilePath, setUserID, setGroupID) )
	{
		printf("%s chown error\n", __func__);
		// debug start
		printf("%s setUserID = %d\n", __func__, setUserID);
		printf("%s setGroupID = %d\n", __func__, setGroupID);
		printf("%s chown errno = %d\n", __func__, errno);
		// debug end
		return E_RB_FAILURE;
	}

	if( chmod(ui16pFilePath, setFileMode) )
	{
		printf("%s chmod error\n", __func__);
		return E_RB_FAILURE;
	}

	printf_dbg("%s SUCCESS\n", __func__);
	return S_RB_SUCCESS;
}
#else
long RB_SetFileAttributes(
void*					pbUserData,
const char*				ui16pFilePath,
const unsigned long		ui32AttribSize,
const unsigned char*	ui8pAttribs)
{
	char tmpAttribs[15];
	char *tp;
	char *endstr;
	uid_t setUserID		= 0;
	gid_t setGroupID	= 0;
	mode_t setFileMode	= 0;
	struct stat sbuf;
	int ret = 0;
	// debug start
	int count = 0;
	// debug end
	
	printf("%s \n", __func__);
	
	if(NULL == ui16pFilePath)
	{
		printf("ui16pFilePath NULL [error]\n");
		return E_RB_BAD_PARAMS;
	}
	else if(NULL == ui8pAttribs)
	{
		printf("ui8pAttribs NULL [error]\n");
		return E_RB_BAD_PARAMS;
	}
	else if(0 == ui32AttribSize)
	{
		printf("ui32AttribSize 0\n");
		return S_RB_SUCCESS;
	}
		
	ret = lstat(ui16pFilePath, &sbuf);
	if(ret < 0)
	{	
			printf(" stat failed with return value: %d\n",ret);
			return E_RB_FAILURE;
	}
	else
	{
		if(S_ISLNK(sbuf.st_mode))
		{
			printf(" stat->st_mode = symbolic link file\n");
			return S_RB_SUCCESS;
		}
		if(S_ISREG(sbuf.st_mode))
		{
			printf(" stat->st_mode = regular file\n");
		}
		if (S_ISDIR(sbuf.st_mode))
		{
			printf(" stat->st_mode = directory\n");
		}
	}

	printf("ui16pFilePath = %s\n", ui16pFilePath);
	printf("ui32AttribSize = %u\n", ui32AttribSize);
	printf("ui8pAttribs = %s\n", ui8pAttribs);
	// debug start
	for(count=0;count<14;count++)
	{
		printf("ui8pAttribs[%d] = %c\n", count, ui8pAttribs[count]);
	}
	// debug end
	memset(tmpAttribs, 0x0, 15);
	memcpy(tmpAttribs, ui8pAttribs, (size_t)ui32AttribSize);
	tp = strtok(tmpAttribs, ":");
	// Get FileMode
	setFileMode = strtol(tp, &endstr, 8);
	tp = strtok(NULL, ":");
	// Get UserID
	if (tp != NULL)
	{
		setUserID = (uid_t)strtol(tp, &endstr, 0x10);
		tp = strtok(NULL, ":");
	}
	// Get GroupID
	if (tp != NULL)
	{
		setGroupID = (gid_t)strtol(tp, &endstr, 0x10);
	}
	// Set FileMode
	printf("ui16pFilePath = %s\n",ui16pFilePath);
	printf("setFileMode = %d\n", setFileMode);
	printf("setFileMode = %o\n", setFileMode);
	printf("setUserID = %d\n", setUserID);
	printf("setGroupID = %d\n", setGroupID);
	// Set UserID,GroupID
	if( chown(ui16pFilePath, setUserID, setGroupID) )
	{
		printf("%s chown error\n", __func__);
		// debug start
		printf("%s setUserID = %d\n", __func__, setUserID);
		printf("%s setGroupID = %d\n", __func__, setGroupID);
		printf("%s chown errno = %d\n", __func__, errno);
		// debug end
		return E_RB_FAILURE;
	}
	
	if( chmod(ui16pFilePath, setFileMode) )
	{
		printf("%s chmod error\n", __func__);
		return E_RB_FAILURE;
	}
	
	printf("%s SUCCESS\n", __func__);
	return S_RB_SUCCESS;
}

#endif //ATT

/*!
 ************************************************************
 *                     RB_FSGetDelta
 ************************************************************
 *
 * @brief
 *	Reads data from the delta.
 *
 *	A glue function that needs to be implemented by the customer.
 *
 *	It should follow the following restrictions:
 *
 *	1. Returning the proper error level \see RB_vRM_Errors.h
 *
 *	@param pbBuffer					The gives buffer that the data from the open file should be.
 *									copy into.
 *
 *	@param dwStartAddressOffset		The offset in the read file that should be
 *									the starting point for the copy.
 *
 *	@param dwSize					The size in bytes that should be copied from the open file,
 *									starting from the given position offset.
 *
 *	@return							One of the return codes as defined in RB_vRM_Errors.h
 *
 ************************************************************
 */


long RB_GetDelta(
	void*			pbUserData,				    /* User data passed to all porting routines */
	unsigned char*	pbBuffer,			/* pointer to user buffer */
    unsigned long	dwStartAddressOffset, /* offset from start of delta file */
    unsigned long	dwSize)
{
	int readCount = 0, ret = 0;
	FILE* fp;

	printf_dbg("%s: offset 0x%lx(%ld), size 0x%lx(%ld)\n", __func__,
		dwStartAddressOffset, dwStartAddressOffset, dwSize, dwSize);

	fp = fopen(DELTA_FILE, "r");
	if (!fp)
	{
		printf_dbg("%s: open delta file %s failed.\n", __func__, DELTA_FILE);
		return E_RB_OPENFILE_ONLYR;
	}
	ret = fseek(fp, dwStartAddressOffset, 0);
	if (ret == -1)
	{
		fclose(fp);
		return E_RB_READ_ERROR;
	}
	readCount = fread(pbBuffer, 1, dwSize, fp);
	if (readCount != dwSize)
	{
		printf_dbg("%s: error in read size\n", __func__);
		fclose(fp);
		return E_RB_READ_ERROR;
	}
	fclose(fp);
	
	return S_RB_SUCCESS;
}

void RB_Progress(
		void*			pbUserData,
		unsigned long	uPercent)
	
{
	printf_dbg("%s: update %ld percent\n", __func__, uPercent);
	//To do: display the progress bar
	float p = uPercent / 100.0f;
	ui_set_progress(p);
}

/* Prints a string like the C printf() function */
unsigned long RB_Trace(
	void*			pUser,
	const char*		aFormat,
	...)
{
	char buf[1024];
	va_list list;

	va_start(list, aFormat);
	vsnprintf(buf, sizeof(buf), aFormat, list);
	va_end(list);
	// #! enable UI Print
	//ui_print("%s: %s", __func__, buf);
	printf("%s: %s", __func__, buf);
	return S_RB_SUCCESS;
}

long RB_MoveFile(
	void*		pbUserData,
	const char* strFromPath,
	const char* strToPath)
{
	long ret = 0;
	long pwHandle = 0;
	
	printf_dbg ("%s: %s -> %s ", __func__, strFromPath, strToPath);

	if (!strFromPath || !strToPath)
	{
		printf_dbg("NULL file name find. Abort.\n");
		return -1;			//should never happen
	}

	if (RB_OpenFile(pbUserData, strToPath, ONLY_R, &pwHandle) == S_RB_SUCCESS)
	{
		RB_CloseFile(pbUserData, pwHandle);
		return S_RB_SUCCESS;
	}
	
	ret = rename (strFromPath,strToPath);	
	if (ret < 0)
	{
			printf ("failed to rename file %s: %s -> %s ", __func__, strFromPath, strToPath);
			return -2;
	}
	return S_RB_SUCCESS;
}

long RB_SyncFile(
	void*	pbUserData,
	long	wHandle)
{
	long ret = -1;
	ret = fsync(wHandle);
	if (ret < 0)
	{
		printf_dbg("fsync Failed with return value: %ld\n",ret);
		return E_RB_WRITE_ERROR;
	}
	printf_dbg("fsync after write: %ld\n",ret);

	return S_RB_SUCCESS;
}

long RB_ResetTimerA(void)
{
        printf("%s \n", __func__);
        return S_RB_SUCCESS;
}


int FSMain(void)
{
	unsigned char del[RB_MAX_PATH]={0};
	int i;
	unsigned char* pRamMem =(unsigned char*)calloc(FS_U_RAM_SIZE,1);
	long ret = 0;
	//vRM_DeviceData pDeviceDatum;
	//CustomerPartitionData pCustomerPartData;
	vRM_DeviceData pDeviceDatum ;
	memset(&pDeviceDatum, 0, sizeof(pDeviceDatum));
	CustomerPartitionData pCustomerPartData;
	memset(&pCustomerPartData, 0, sizeof(pCustomerPartData));
	
	unsigned long ComponentInstallerType[1]={0};
	char PartitionName[RB_MAX_PATH] = {"system"};
	char MountPoint[RB_MAX_PATH] = {"/"};
	
	/** Android 更新重要参数 **/
	pCustomerPartData.partition_name			= PartitionName;
	pCustomerPartData.rom_start_address			= 0;
	pCustomerPartData.mount_point				= MountPoint;
	pCustomerPartData.partition_type			= PT_FS;
	


	pDeviceDatum.pFirstPartitionData			= &pCustomerPartData;
	/* ui32NumberOfPartitions - Number of partitions listed in pFirstPartitionData. */
	pDeviceDatum.ui32NumberOfPartitions 		= 1;
	
	// ui32Operation - UPI Mode 
	// 0 scout and update 		-- 检查升级文件+更新+更新后验证
	// 1 Scout only				-- 只检查升级文件
	// 2 Dry-run				-- 检查升级文件+更新
	// 3 Update only			-- 只更新
	// 6 Verify post-install	-- 只做更新后验证
	pDeviceDatum.ui32Operation 					= 0;
	
	// enmUpdateType - 更新参数, 很重要的参数,UT_NO_SELF_UPDATE为不更新自己
	pDeviceDatum.enmUpdateType					= UT_NO_SELF_UPDATE;
	
	// pbUserData 自定义上下文指针 
	pDeviceDatum.pbUserData						= NULL;
	
	// pRam  很重要的参数,预先分配的内存,越大越好. Pre-allocated RAM space.
	// ui32RamSize 指示pRam大小
	pDeviceDatum.pRam 							= pRamMem;
	pDeviceDatum.ui32RamSize					= FS_U_RAM_SIZE;
	
	// pTempPath 临时文件读写目录 
	pDeviceDatum.pTempPath						= "/cache";
	
	/** Android FileSystem 更新不用关心的参数 **/
	/* pDeltaPath 已经弃用的参数,不用管 Deprecated */
	pDeviceDatum.pDeltaPath						= (char*)del;
	
	/* pComponentInstallerTypes 数组类型,仅用作 Image 更新,暂时不用关心*/
	pDeviceDatum.pComponentInstallerTypes		= ComponentInstallerType;
	/* ui32ComponentInstallerTypesNum 标识pComponentInstallerTypes的数量 */
	pDeviceDatum.ui32ComponentInstallerTypesNum	= 1;
	
	/* pBufferBlocks 不清楚具体作用,只知道更新Android FileSystem时,出入参数0即可 */
	pDeviceDatum.pBufferBlocks					= 0;
	/* ui32NumberOfBuffers - Number of backup sectors listed in pBufferBlocks. */
	pDeviceDatum.ui32NumberOfBuffers			= 0;
	
	
	// RB_vRM_Update 
	//ui_show_progress(0.75, 60);
	ret = RB_vRM_Update(&pDeviceDatum);
	
	free(pRamMem);
	sync();	
	printf("return value from RB_vRM_Update %ld\n", ret);


	return ret;
}




