/*
 *******************************************************************************
 * vRapid Mobile(R) Notice
 *  
 * Copyright(C) 1999-2012, Red Bend Software. All Rights Reserved.
 *
 * Patented: www.redbend.com/red-bend-patents.pdf
 * 
 * This Software is the property of Red Bend Ltd. and contains trade
 * secrets, know-how, confidential information and other intellectual
 * property of Red Bend Ltd.
 *
 * vRapid Mobile(R), Red Bend(R), and other Red Bend names, as well as the
 * Red Bend Logo are trademarks or registered trademarks of Red Bend
 * Ltd.
 *
 * All other names and trademarks are the property of their respective
 * owners.
 *
 * The Product contains components owned by third parties. Copyright
 * notices and terms under which such components are licensed can be
 * found at the following URL, and are hereby incorporated by
 * reference: www.redbend.com/red-bend-legal-notices.pdf
 *******************************************************************************
 */
/*!
 *******************************************************************************
 * \file	RB_ImageUpdate.h
 *
 * \brief	UPI Image Update API
 *******************************************************************************
 */
#ifndef	__RB_IMAGEUPDATE_H__
#define	__RB_IMAGEUPDATE_H__

#include "RB_vRM_Errors.h"

#ifdef __cplusplus
extern "C" {
#endif  /* __cplusplus */

/**
 *******************************************************************************
 * Get data from temporary storage.
 *
 * \param	pbUserData		Optional opaque data-structure to pass to IPL
 *							functions
 * \param	pbBuffer		(out) Buffer to store the data
 * \param	dwBlockAddress	Location of data
 * \param	dwSize			Size of data
 *
 * \return	S_RB_SUCCESS on success or an \ref RB_vRM_Errors.h error code
 *******************************************************************************
 */
long RB_ReadBackupBlock(
	void *pbUserData,
	unsigned char *pbBuffer,
	unsigned long dwBlockAddress,
	unsigned long dwSize);

/**
 *******************************************************************************
 * Erase sector from temporary storage.<p>
 *
 * A default implementation is included. You must use the default implementation
 * if both of the following are true:
 * \li	<b>InstantFailSafe</b> is set to false
 * \li	Only Image updates are used OR the backup is stored in flash and not on
 *		the file system
 * <p>The address must be aligned to the sector size.
 *
 * \param	pbUserData		Optional opaque data-structure to pass to IPL
 *							functions
 * \param	dwStartAddress	Sector address to erase
 *
 * \return	S_RB_SUCCESS on success or an \ref RB_vRM_Errors.h error code
 *******************************************************************************
 */
long RB_EraseBackupBlock(
	void *pbUserData,
	unsigned long dwStartAddress);

/**
 *******************************************************************************
 * Write complete sector to temporary storage.<p>
 *
 * Must overwrite entire sector. To write part of a sector, see
 * \ref RB_WriteBackupPartOfBlock.
 *
 * \param	pbUserData			Optional opaque data-structure to pass to IPL
 *								functions
 * \param	dwBlockStartAddress	Address to which to write
 * \param	pbBuffer			Data to write
 *
 * \return	S_RB_SUCCESS on success or an \ref RB_vRM_Errors.h error code
 *******************************************************************************
 */
long RB_WriteBackupBlock(
	void *pbUserData,
	unsigned long dwBlockStartAddress,
	unsigned char *pbBuffer);

/**
 *******************************************************************************
 * Write data to temporary storage.<p>
 *
 * Used to write part of a sector that has already been written to by
 * \ref RB_WriteBackupBlock. Must NOT overwrite entire sector.<p>
 *
 * A default implementation is included. You must use the default implementation
 * if both of the following are true:
 * \li	<b>InstantFailSafe</b> is set to false
 * \li	Only Image updates are used OR the backup is stored in flash and not on
 *		the file system
 *
 * \param	pbUserData		Optional opaque data-structure to pass to IPL
 *							functions
 * \param	dwStartAddress	Address to which to write
 * \param	dwSize			Size of \a pbBuffer
 * \param	pbBuffer		Data to write
 *
 * \return	S_RB_SUCCESS on success or an \ref RB_vRM_Errors.h error code
 *******************************************************************************
 */
long RB_WriteBackupPartOfBlock(
	void *pbUserData,
	unsigned long dwStartAddress,
	unsigned long dwSize,
	unsigned char* pbBuffer);

/**
 *******************************************************************************
 * Write sector to flash.<p>
 *
 * Must overwrite a complete sector.
 *
 * \param	pbUserData		Optional opaque data-structure to pass to IPL
 *							functions
 * \param	dwBlockAddress	Address to which to write
 * \param	pbBuffer		Data to write
 *
 * \return	S_RB_SUCCESS on success, E_RB_WRITE_ERROR if there is a write error,
 *			or an \ref RB_vRM_Errors.h error code
 *******************************************************************************
 */
long RB_WriteBlock(
	void *pbUserData,
	unsigned long dwBlockAddress,
	unsigned char *pbBuffer);
							 
/**
 *******************************************************************************
 * Read data from flash.<p>
 *
 * See \ref RB_ReadImageNewKey.
 *
 * \param	pbUserData		Optional opaque data-structure to pass to IPL
 *							functions
 * \param	pbBuffer		(out) Buffer to store data
 * \param	dwStartAddress	Address from which to read
 * \param	dwSize			Size of data to read (must be less than a sector); 0
 *							must immediately return success
 *
 * \return	S_RB_SUCCESS on success or an \ref RB_vRM_Errors.h error code
 *******************************************************************************
 */
long RB_ReadImage(
	void *pbUserData,
	unsigned char *pbBuffer,			/* pointer to user buffer */
    unsigned long dwStartAddress,		/* memory address to read from */
    unsigned long dwSize);				/* number of bytes to copy */

/**
 *******************************************************************************
 * Read data from flash.<p>
 *
 * As opposed to \ref RB_ReadImage, this function is intended to read already
 * updated data and is used for encrypted images. If no part of the image is
 * encrypted, simply call \ref RB_ReadImage.
 *
 * Used by the UPI when resuming an interrupted update.
 *
 * \param	pbUserData		Optional opaque data-structure to pass to IPL
 *							functions
 * \param	pbBuffer		(out) Buffer to store data
 * \param	dwStartAddress	Address from which to read
 * \param	dwSize			Size of data to read (must be less than a sector); 0
 *							must immediately return success
 *
 * \return	S_RB_SUCCESS on success or an \ref RB_vRM_Errors.h error code
 *******************************************************************************
 */
long RB_ReadImageNewKey(
	void *pbUserData,
	unsigned char *pbBuffer,
	unsigned long dwStartAddress,
	unsigned long dwSize);

/**
 *******************************************************************************
 * Get sector size.<p>
 *
 * \return	Sector size, in bytes
 *******************************************************************************
 */
long RB_GetBlockSize(void);


#ifdef __cplusplus
	}
#endif  /* __cplusplus */

#endif	/*	__RB_UPDATE_IMAGE_H__	*/
