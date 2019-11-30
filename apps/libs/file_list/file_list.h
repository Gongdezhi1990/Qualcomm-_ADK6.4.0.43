/****************************************************************************
Copyright (c) 2017 Qualcomm Technologies International, Ltd.

FILE NAME
    file_list.h
DESCRIPTION
    Library to store file specific information and primarily its file system index.
    The file entries are organised in groups based on a role.
*/

#ifndef _FILE_LIST_H_
#define _FILE_LIST_H_

#include <file.h>

/*!
 * @brief Definition of the different file "roles" used in the file_list library so that they do not overlap.
 */
typedef enum
{
    downloadable_capabilities_file_role
} file_list_roles_t;

/*!
 * @brief Used as a handle to the file entry we want to access from the library.
 */
typedef const struct file_list_entry_t * file_list_handle;

/*!
 * @brief Any additional file related data whose structure only the caller knows about
 *        are passed to the library by using this structure.
 *
 * @param size_of_data The size of the data structure passed in bytes.
 * @param data A pointer to the data structure passed, the data should be continuous in
 *        memory.
 */
typedef struct
{
    size_t size_of_data;
    const void *data;
} file_related_data_t;

/*!
 * @brief Adds a file specific information entry to the library.
 *
 * @param role The file information can be accessed based on a role, which is simply an unsigned integer.
 * @param index The file's index used by the file system.
 * @param related_data Should contain the address to a user defined structure to be passed in the library,
 *        can be NULL if not used.
 *
 * @return TRUE if the function was successful and FALSE if there were any errors.
 *         The function will not cause a panic.
 */
bool FileListAddFile(unsigned role, FILE_INDEX index, const file_related_data_t *related_data);

/*!
 * @brief Get a handle to the file entry with a specific role and number.
 *
 * @param role The role that was used when adding the file entry to the library.
 * @param number Since multiple file entries can have the same role, we need an additional identifier.
 *        It is zero based and follows the order in which file entries with the same role were added to the library.
 *
 * @return A handle to access the information stored for that entry. If an error occurred the handle will
 *         be a NULL pointer. The function will not cause a panic.
 */
file_list_handle FileListGetFileHandle(unsigned role, unsigned number);

/*!
 * @brief Get the number of file entries with a specific role.
 *
 * @param role The role that was used when adding the file entry to the library.
 *
 * @return The number of file entries that are associated with that role.
 */
unsigned FileListGetNumberOfFiles(unsigned role);

/*!
 * @brief Get the file system index of the file entry associated with that handle.
 *        We assume that the handle passed is valid (returned by FileListGetFileHandle).
 *
 * @param handle Returned by FileListGetFileHandle and used to access information about a specific file entry.
 *
 * @return The file system index of the file entry associated with that handle.
 *         If the handle passed is NULL the function will return FILE_NONE.
 *         The function will not cause a panic.
 */
FILE_INDEX FileListGetFileIndex(file_list_handle file_handle);

/*!
 * @brief Get the user defined data related to the file entry associated with that handle.
 *        We assume that the handle passed is valid (returned by FileListGetFileHandle).
 *
 * @param handle Returned by FileListGetFileHandle and used to access information about a specific file entry.
 *
 * @return A pointer to the user defined structure associated with that file entry.
 *         Since the structure is user defined we assume that the caller knows the size and structure of this data.
 *         If the handle passed is NULL or there is no user defined data associated with that entry the function will return NULL.
 *         The function will not cause a panic.
 */
const void * FileListGetFileRelatedData(file_list_handle file_handle);

/*!
 * @brief Remove all file entries associated with that role in the library.
 *
 * @param role The role that was used when adding those file entries to the library.
 */
void FileListRemoveFiles(unsigned role);

#ifdef HOSTED_TEST_ENVIRONMENT
/* Reset the internal state of the file list library's structure. Only intended for unit tests */
void FileListLibraryTestReset(void);
#endif

#endif /* _FILE_LIST_H_ */
