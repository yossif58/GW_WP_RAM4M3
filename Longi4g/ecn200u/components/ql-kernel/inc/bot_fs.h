/***********************************************************************************************************************
 * Copyright (C) 2021-2022 Alibaba Group Holding Limited
 * description: head file of 
 * author:      
 * date:        2021-08-20
***********************************************************************************************************************/
#ifndef __BOT_FS_H__
#define __BOT_FS_H__

#if defined(__cplusplus)
extern "C" {
#endif
/***********************************************************************************************************************
 * Including File
***********************************************************************************************************************/
#include "bot_typedef.h"
#include "bot_platform.h"
#include "bot_platform_user_config.h"

/***********************************************************************************************************************
 * Enumeration Definition
***********************************************************************************************************************/
/***********************************************************************************************************************
 * Type & Structure Definition
***********************************************************************************************************************/
// the file handler in specified FS
#if BOT_FS_STREAM_ENABLED
typedef int    bot_fd_t;
typedef FILE * bot_file_t; // support fs stream operation, like linux, windows
#define BOT_FILE_INIT_VALUE  NULL
#else 
typedef int bot_fd_t;
typedef int bot_fs_flag_t;
typedef bot_fd_t bot_file_t; // support I/O operation
#define BOT_FILE_INIT_VALUE  -1
#endif

#if 0
typedef struct dirent   bot_dirent_t;
typedef struct stat     bot_stat_t;  
#endif         


/***********************************************************************************************************************
 * Fuction Declaration
***********************************************************************************************************************/
/**
 * @brief bot_fs_init() initializes vfs system.
 *
 * @param[in] void
 *
 * @return  0: On success.
 *          negative error code: On error, the code indicating the 
 *          cause of the error.
 */
int bot_fs_init(void);

/***************************************** API about directory operation **********************************************/
/**
 * @brief bot_exitdir() check if dir exists or not.
 *
 * @param[in] path  The name of directory to be queried.
 *
 * @return  0: On success.
 *          negative error code: On error, the code indicating the cause
 *          of the error.
 */
int bot_exitdir(const char *path);

/**
 * @brief bot_mkdir() attempts to create a directory named @path
 *
 * @param[in] path  The name of directory to be created.
 *
 * @return  0: On success.
 *          negative error code: On error, the code indicating the cause
 *          of the error.
 */
int bot_mkdir(const char *path);
#if 0
/**
 * @brief bot_rmdir() deletes a directory, which must be emtpy.
 *
 * @param[in] path  The directory to be deleted.
 *
 * @return  0: On success.
 *          negative error code: On error, the code indicating the cause
 *          of the error.
 */
int bot_rmdir(const char *path);

/**
 * @brief bot_opendir() opens a directory stream corresponding to the
 *        directory @path, and returns a pointer to the directory stream.
 *        The stream is positioned at the first entry in the directory.
 *
 * @param[in] path  the path of the directory to open.
 *
 * @return  On success, return a point of directory stream.
 *          On error, NULL is returned.
 */
bot_dir_t *bot_opendir(const char *path);

/**
 * @brief bot_closedir() closes the directory stream associated with
 *        @dir. A successful call to bot_closedir() also closes the
 *        underlying file descriptor associated with @dir. The directory
 *        stream descriptor @dir is not available after this call.
 *
 * @param[in] dir  The directory stream descriptor to be closed.
 *
 * @return  0: On success.
 *          negative error code: On error, the code indicating the cause
 *          of the error.
 */
int bot_closedir(bot_dir_t *dir);

/**
 * @brief bot_readdir() returns a pointer to an @bot_dirent_t representing
 *        the next directory entry in the directory stream pointed to by
 *        @dir. It returns Null on reaching the end of the directory stream
 *        or if an error occurred.
 *
 * @param[in] dir  The directory stream descriptor to read.
 *
 * @return  On success, bot_readdir() returns a pointer to an @bot_dirent_t
 *          structure. If the end of the directory stream is reached, NULL is
 *          returned.
 *          On error, NULL is returned.
 */
bot_dirent_t *bot_readdir(bot_dir_t *dir);

/**
 * @brief bot_rewinddir() resets the position of the directory stream @dir
 *        to the beginning of the directory.
 *
 * @param[in] dir  The directory stream descriptor pointer.
 *
 * @return  none.
 */
void bot_rewinddir(bot_dir_t *dir);

/**
 * @brief bot_telldir() returns the current location associated with the
 *        directory stream @dir.
 *
 * @param[in] dir  The directory stream descriptor pointer.
 *
 * @return  On success, returns the current location in the
 *          directory stream.
 *          negative error code: On error, the code indicating the cause
 *          of the error.
 */
long bot_telldir(bot_dir_t *dir);

/**
 * @brief bot_seekdir() sets the location in the directory stram from
 *        which the next bot_readdir() call will start. The @offset argument
 *        should be a value returnned by a previous call to bot_telldir().
 *
 * @param[in] dir       The directory stream descriptor pointer.
 * @param[in] offset    The location in the directory stream from which the next
 *                 bot_readdir() call will start.
 *
 * @return  none.
 */
void bot_seekdir(bot_dir_t *dir, long offset);

/**
 * @brief bot_chdir() sets the @path directory as the current directory.
 *
 * @param[in] path  the path of the directory.
 *
 * @return  0: On success.
 *          negative error code: On error, the code indicating the cause
 *          of the error.
 */
int bot_chdir(const char *path);
#endif
/******************************************** API about file operation ************************************************/
/* bot_fopen() 支持的 mode */
/*
mode有以下几种方式：
打开方式	说明
r	以只读方式打开文件，该文件必须存在。
r+	以读/写方式打开文件，该文件必须存在。
rb+	以读/写方式打开一个二进制文件，只允许读/写数据。
rt+	以读/写方式打开一个文本文件，允许读和写。
w	打开只写文件，若文件存在则长度清为0，即该文件内容消失，若不存在则创建该文件。
w+	打开可读/写文件，若文件存在则文件长度清为零，即该文件内容会消失。若文件不存在则建立该文件。
a	以附加的方式打开只写文件。若文件不存在，则会建立该文件，如果文件存在，写入的数据会被加到文件尾，即文件原先的内容会被保留（EOF符保留)。
a+	以附加方式打开可读/写的文件。若文件不存在，则会建立该文件，如果文件存在，则写入的数据会被加到文件尾后，即文件原先的内容会被保留（原来的EOF符 不保留)。
wb	以只写方式打开或新建一个二进制文件，只允许写数据。
wb+	以读/写方式打开或建立一个二进制文件，允许读和写。
wt+	以读/写方式打开或建立一个文本文件，允许读写。
at+	以读/写方式打开一个文本文件，允许读或在文本末追加数据。
ab+	以读/写方式打开一个二进制文件，允许读或在文件末追加数据。
*/

/**
 * @brief bot_fopen() opens the file or device by its @path.
 *
 * @param[in] path   the path of the file or device to open.
 * @param[in] mode   the mode of open operation.
 *
 * @return  On success, return new file pointer.
 *          On error, NULL or negative error code is returned to indicate the cause
 *          of the error.
 */
bot_file_t bot_fopen(const char *path, const char *mode);

/**
 * @brief bot_fclose() closes the file or device associated with file
 *        pointer @fp.
 *
 * @param[in] fp  the file handler of the file or device.
 *
 * @return  On success, return 0.
 *          On error, negative error code is returned to indicate the cause
 *          of the error.
 */
int bot_fclose(bot_file_t fp);

/**
 * @brief bot_fread() attempts to read up to (@size * @count) bytes from file
 *        pointer @fp into the buffer starting at @buff.
 *
 * @param[in]  fp      the file handler of the file or device.
 * @param[out] buff    the buffer to read bytes into.
 * @param[in]  count   the number of bytes to read.
 * @param[in]  size    the size of bytes.
 *
 * @return  On success, the number of bytes is returned (0 indicates end
 *          of file) and the file position is advanced by this number.
 *          On error, negative error code is returned to indicate the cause
 *          of the error.
 */
int bot_fread(void *buff, unsigned int size, unsigned int count, bot_file_t fp);

/**
 * @brief bot_fwrite() writes up to @nbytes bytes from the buffer starting
 *        at @buff to the file referred to by the file handler @fp.
 *
 * @param[in] fp      the file handler of the file or device.
 * @param[in] buff    the buffer to write bytes from.
 * @param[in] count   the number of bytes to write.
 * @param[in] size    the size of bytes.
 *
 * @return  On success, the number of bytes written is returned, and the file
 *          position is advanced by this number..
 *          On error, negative error code is returned to indicate the cause
 *          of the error.
 */
int bot_fwrite(const void *buff, unsigned int size, unsigned int count, bot_file_t fp);

/**
 * @brief bot_fseek() repositions the file offset of the open file
 *        description associated with the file handler @fp to the
 *        argument @offset according to the directive @whence as follows:
 *
 *        SEEK_SET: The file offset is set to @offset bytes.
 *        SEEK_CUR: The file offset is set to its current location
 *                  plus @offset bytes.
 *        SEEK_END: The file offset is set to the size of the file
 *                  plus @offset bytes.
 *
 * @param[in] fp      the file handler of the file.
 * @param[in] offset  The offset relative to @whence directive.
 * @param[in] whence  The start position where to seek.
 *
 * @return  On success, return 0.
 *          On error, neagtive error code is returned to indicate the cause
 *          of the error.
 */
int bot_fseek(bot_file_t fp, int offset, int whence);

/**
 * @brief bot_fflush() causes the pending modifications of the specified file to
 *        be written to the underlying filesystems.
 *
 * @param[in] fp  the file handler of the file.
 *
 * @return  On success return 0.
 *          On error, negative error code is returned to indicate the cause
 *          of the error.
 */
int bot_fflush(bot_file_t fp);
#if 0
/**
 * @brief bot_ftell get the current location of file. 
 *
 * @param[in] fp  the file handler of the file.
 *
 * @return  On success return 0.
 *          negative error code: On error, the code indicating the cause
 *          of the error.
 */
long bot_ftell(bot_file_t fp);

/**
 * @brief bot_stat() return information about a file pointed to by @path
 *        in the buffer pointed to by @st_buff.
 *
 * @param[in]  path     The path of the file to be quried.
 * @param[out] st_buff   The buffer to receive information.
 *
 * @return  On success, return 0.
 *          On error, negative error code is returned to indicate the cause
 *          of the error.
 */
int bot_stat(const char *path, bot_stat_t *st_buff);

/**
 * @brief bot_statfs() return information about the file system to by @path
 *        in the buffer pointed to by @sf_buff.
 *
 * @param[in]  path     The path of the file to be quried.
 * @param[out] sf_buff  The buffer to receive information.
 *
 * @return    0: On success.
 *          < 0: On error, negative error code is returned to indicate the cause
 *               of the error.
 */
int bot_statfs(const char *path, bot_statfs_t *sf_buff);


/**
 * @brief bot_remove() deletes a file from the filesystem.
 *
 * @param[in] path  The path of the file to be deleted.
 *
 * @return  0: On success.
 *          negative error code: On error, the code indicating the cause
 *          of the error.
 */
int bot_remove(const char *path); 

/**
 * @brief bot_rename() renames a file, moving it between directories
 *        if required.
 *
 * @param[in] oldpath  The old path of the file to rename.
 * @param[in] newpath  The new path to rename the file to.
 *
 * @return  0: On success.
 *          negative error code: On error, the code indicating the cause
 *          of the error.
 */
int bot_rename(const char *oldpath, const char *newpath);
#endif

/**
 * @brief bot_unlink() deletes a name @path from the filesystem.
 *
 * @param[in]  path       The path of the file to be quried.
 *
 * @return  On success, return 0.
 *          On error, negative error code is returned to indicate the cause
 *          of the error.
 */
int bot_unlink(const char *path);

/**
 * @brief bot_file_is_valid() check if file indicates by @fp is valid.
 *
 * @param[in]  fp      the file handler.
 *
 * @return  true:  On success, the file @fp is valid.
 *          false: On error, the file @fp is invalid.
 */
bool bot_file_is_valid(bot_file_t fp);

/**
 * @brief print error number.
 *
 * @param[in]  void
 *
 * @return    void.
 */
void bot_fs_print_errno(void);


#if defined(__cplusplus)
}
#endif
#endif /* __BOT_FS_H__ */
