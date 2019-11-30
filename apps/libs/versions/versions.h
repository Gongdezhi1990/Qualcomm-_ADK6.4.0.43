/*******************************************************************************
Copyright (c) 2018 Qualcomm Technologies International, Ltd.
*/

#ifndef VERSIONS_H
#define VERSIONS_H

/** Default value used as a placeholder for development builds
 * When devkit packages are crated the version for each library is defined
 * in the command line at build time.
 * I.e. by adding:
 *      DEFS=LIB_VERSION_MY_LIB_NAME=123456
 * to the make command
 */
#define LIB_VERSIONS_DEV_VERSION 0xCAFECAFE

typedef struct {
    uint32 version;
    char *name;
} lib_ver_t;

#define __LIB_VERSION_STR(a) #a

#define __LIB_VERSION_JOIN(a, b, c) a##_##b##_##c

#define __LIB_VERSION_UNSIGNED(a) a##u

#define SET_LIB_VERSION(lib_name, version_number)                               \
    _Pragma("datasection LIB_VERSIONS")                                         \
    const lib_ver_t __LIB_VERSION_JOIN(lib_version, lib_name, version_number) = \
    { .version = __LIB_VERSION_UNSIGNED(version_number), .name = __LIB_VERSION_STR(lib_name) }

lib_ver_t *get_lib_versions(void);

#endif /* VERSIONS_H */