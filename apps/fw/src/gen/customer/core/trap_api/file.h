#ifndef __FILE_H__
#define __FILE_H__
#include <csrtypes.h>
#include <app/file/file_if.h>

/*! file  @brief Access to the read-only file-system */
/*! @sa \#StreamFileSource, \#KalimbaLoad */

#if TRAPSET_FILE

/**
 *  \brief Find the index for a named file or directory, relative to the starting
 *  directory.
 *   
 *   Leading and trailing directory separators in @a name are ignored.
 *   @a start is commonly \#FILE_ROOT.
 *   @note
 *   For file in read-write filesystem start parameter needs to be \#FILE_ROOT
 *   and path should have "/rwfs/" prepended to the file name.
 *  \param start index of directory in which to start the search 
 *  \param name the name (possibly including a path) of the item to find 
 *  \param length the number of characters in @a name
 *  \return The index of the item searched for, or \#FILE_NONE if no such item exists.
 * 
 * \note This trap may NOT be called from a high-priority task handler
 * 
 * \ingroup trapset_file
 */
FILE_INDEX FileFind(FILE_INDEX start, const char * name, uint16 length);

/**
 *  \brief Find the type of a file specified by its index
 *   
 *  \param index the index of the file whose type is required
 *  \return The type of the specified file.
 * 
 * \note This trap may NOT be called from a high-priority task handler
 * 
 * \ingroup trapset_file
 */
FILE_TYPE FileType(FILE_INDEX index);

/**
 *  \brief Find the index of the directory containing this file or directory. 
 *   
 *  \param item The index of the item we know about.
 *  \return The index of the directory containing item, or \#FILE_NONE.
 * 
 * \note This trap may NOT be called from a high-priority task handler
 * 
 * \ingroup trapset_file
 */
FILE_INDEX FileParent(FILE_INDEX item);

/**
 *  \brief Create file in read-write filesystem.
 *   @note  
 *   read-write filesystem does not support directory structure. File path 
 *   should not contain any extra directory apart from "/rwfs/" which  
 *   needs to be prepended to the file name.
 *   
 *  \param name the name (including a path) of the item to create 
 *  \param length the number of characters in @a name
 *  \return The index of the file created, or \#FILE_NONE if could not.
 * 
 * \note This trap may NOT be called from a high-priority task handler
 * 
 * \ingroup trapset_file
 */
FILE_INDEX FileCreate(const char * name, uint16 length);

/**
 *  \brief Delete file in read-write filesystem.
 *   
 *  \param index the file which needs to be deleted.
 *  \return Success or not.
 * 
 * \note This trap may NOT be called from a high-priority task handler
 * 
 * \ingroup trapset_file
 */
bool FileDelete(FILE_INDEX index);

/**
 *  \brief Rename file in read-write filesystem.
 *   @note 
 *   If a file is renamed to itself then it will return FALSE.
 *   @note  
 *   For files in read-write filesystem, path needs to have "/rwfs/" prepended to
 *  the file names.
 *   
 *  \param old_path the name (including a path) of the file to rename 
 *  \param old_path_len the number of characters in old path name
 *  \param new_path the new name (including a path) of the file 
 *  \param new_path_len the number of characters in new path name
 *  \return Success or not.
 * 
 * \note This trap may NOT be called from a high-priority task handler
 * 
 * \ingroup trapset_file
 */
bool FileRename(const char * old_path, uint16 old_path_len, const char * new_path, uint16 new_path_len);

/**
 *  \brief Unmount filesystem
 *   Used to free up memory on the system processor when the current file
 *  operations 
 *   have been completed. After this call the file system will be mounted again 
 *   automatically when any trap referencing it is used (such as FileFind or
 *  FileCreate).
 *   
 *   @note
 *   This trap does not support all filesystems. If the given filesystem is not
 *  supported then 
 *   this trap will return FALSE.
 *   
 *   @note
 *   For read-write filesystem mount path needs to be "/rwfs/".
 *   
 *  \param mount_path the mount path of the filesystem to unmount 
 *  \return TRUE if unmounted successfully, otherwise FALSE
 * 
 * \note This trap may NOT be called from a high-priority task handler
 * 
 * \ingroup trapset_file
 */
bool FileSystemUnmount(const char * mount_path);

/**
 *  \brief Find the address of a file specified by its index
 * Returns the address of a read only mapping of the file specified starting at
 *  the given byte offset.
 * The returned address has the same alignment guarantees as the underlying
 *  filesystem.
 * As the memory map is different between the customer and firmware processors,
 *  this address can only be passed into firmware calls that support it. This is
 *  currently restricted to BitSerialTransfer() and BitSerialWrite() as the source
 *  data address, and CryptoAes128Ctr()/CryptoAes12Cbc() as the source data.
 * \p index must be a valid file index in a read only filesystem. Mapping files in
 *  a writable filesystem is unsupported.
 * \p offset must be less than the number of bytes in the file, if it's not the
 *  function will return NULL. Attempting to map a file that is 0 bytes long will
 *  return NULL.
 * If \p offset + \p size extends beyond the length of the file the mapping is
 *  truncated to the file boundary. The value \#FILE_MAP_SIZE_ALL can be passed to
 *  the \p size argument to indicate that the entire file should be mapped.
 * In ADK6 the \p size parameter is ignored, the entire file is always made
 *  readable, this is an implementation detail of this release and must not be
 *  relied on.
 * In addition to invalid arguments, this function will return NULL if the flash
 *  layout is such that Apps P1 is unable to access the file via the memory map,
 *  or the file is unable to be mapped at this particular time.
 * For a given implementation of this API, it may not be possible for the
 *  implementation to prevent file mapping pointers from working after the file
 *  has been unmapped. Hence, code that incorrectly uses a pointer to a mapped
 *  file after unmapping the file may appear to operate successfully. This
 *  behaviour must not be relied on as it may vary from file to file and it is
 *  subject to change without notice.
 * If the same file is mapped in multiple times at the same offset then the
 *  implementation of this API may return different pointers for each map or it
 *  may return the same pointer. In either event, the implementation guarantees
 *  that the following code will work:
 * \code
 *     char x;
 *     const char *ptr1, *ptr2;
 *     FILE_INDEX ind = (some code to retrieve an index);
 *     ptr1 = FileMap(ind, 0, 50);
 *     ptr2 = FileMap(ind, 0, 100);
 *     FileUnmap(ptr1);
 *     x = ptr2[99];
 * \endcode
 * However, if the FileUnmap() call were to pass ptr2 then that code would not be
 *  guaranteed to work. It may work coincidentally in a given implementation but
 *  this behaviour must not be relied on as it may vary from file to file and is
 *  subject to change without notice.
 *         
 *  \param index The index of the file whose address is required
 *  \param offset The offset within the file in bytes
 *  \param size The size of the file to be addressable in bytes
 *  \return The address of the file within the application subsystem or NULL on failure
 * 
 * \note This trap may NOT be called from a high-priority task handler
 * 
 * \ingroup trapset_file
 */
const void * FileMap(FILE_INDEX index, uint32 offset, uint32 size);

/**
 *  \brief Release access to the address of a file
 * \p addr should be a currently mapped address previously returned by FileMap().
 *  If \p addr is set to NULL, FileUnmap() will return TRUE without performing an
 *  unmap operation.
 * The implementation may validate that unmap operations are only performed on
 *  currently mapped pointers and that they are performed the correct number of
 *  times when the same file section has been repeatedly mapped. If invalid usage
 *  is detected this function will return FALSE. However, for efficiency, the
 *  implementation may decide not do detect some invalid uses.
 *         
 *  \param addr The mapped address of the file to unmap.
 *  \return TRUE if the given address was seccessfully unmapped, FALSE on failure.
 * 
 * \note This trap may NOT be called from a high-priority task handler
 * 
 * \ingroup trapset_file
 */
bool FileUnmap(const void * addr);
#endif /* TRAPSET_FILE */
#endif
