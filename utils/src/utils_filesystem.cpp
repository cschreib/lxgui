#include "lxgui/utils_filesystem.hpp"
#include "lxgui/utils_string.hpp"
#include <fstream>

#include <sys/types.h>
#include <sys/stat.h>

#ifdef LINUX
    // Emulate directory and file listing windows functions
    // Note : Taken directly from Ogre3D
    // http://www.ogre3d.org/

    #include <dirent.h>
    #include <unistd.h>
    #include <fnmatch.h>
    #include <cstring>
    #include <cstdio>

    struct _finddata_t
    {
        char *name;
        int attrib;
        unsigned long size;
    };

    struct _find_search_t
    {
        char *pattern;
        char *curfn;
        char *directory;
        int dirlen;
        DIR *dirfd;
    };

    #define _A_NORMAL 0x00  /* Normalfile-Noread/writerestrictions */
    #define _A_RDONLY 0x01  /* Read only file */
    #define _A_HIDDEN 0x02  /* Hidden file */
    #define _A_SYSTEM 0x04  /* System file */
    #define _A_SUBDIR 0x10  /* Subdirectory */
    #define _A_ARCH   0x20  /* Archive file */

    int _findclose(long id);
    int _findnext(long id, struct _finddata_t *data);

    long _findfirst(const char *pattern, struct _finddata_t *data)
    {
        _find_search_t *fs = new _find_search_t;
        fs->curfn = nullptr;
        fs->pattern = nullptr;

        const char *mask = strrchr(pattern, '/');
        if (mask)
        {
            fs->dirlen = mask - pattern;
            mask++;
            fs->directory = (char *)malloc(fs->dirlen + 1);
            memcpy(fs->directory, pattern, fs->dirlen);
            fs->directory[fs->dirlen] = 0;
        }
        else
        {
            mask = pattern;
            fs->directory = strdup(".");
            fs->dirlen = 1;
        }

        fs->dirfd = opendir(fs->directory);
        if (!fs->dirfd)
        {
            _findclose((long)fs);
            return -1;
        }

        if (strcmp(mask, "*.*") == 0)
            mask += 2;

        fs->pattern = strdup(mask);

        if (_findnext((long)fs, data) < 0)
        {
            _findclose((long)fs);
            return -1;
        }

        return (long)fs;
    }

    int _findnext(long id, struct _finddata_t *data)
    {
        _find_search_t *fs = (_find_search_t*)id;

        dirent *entry;
        for (;;)
        {
            if (!(entry = readdir(fs->dirfd)))
                return -1;

            if (fnmatch(fs->pattern, entry->d_name, 0) == 0)
                break;
        }

        if (fs->curfn)
            free(fs->curfn);

        data->name = fs->curfn = strdup(entry->d_name);

        size_t namelen = strlen(entry->d_name);
        char *xfn = new char[fs->dirlen + 1 + namelen + 1];
        sprintf(xfn, "%s/%s", fs->directory, entry->d_name);

        struct stat stat_buf;
        if (stat(xfn, &stat_buf))
        {
            data->attrib = _A_NORMAL;
            data->size = 0;
        }
        else
        {
            if (S_ISDIR(stat_buf.st_mode))
                data->attrib = _A_SUBDIR;
            else
                data->attrib = _A_NORMAL;

            data->size = stat_buf.st_size;
        }

        delete[] xfn;

        if (data->name [0] == '.')
            data->attrib |= _A_HIDDEN;

        return 0;
    }

    int _findclose(long id)
    {
        int ret;
        _find_search_t *fs = (_find_search_t *)id;

        ret = fs->dirfd ? closedir(fs->dirfd) : 0;
        free(fs->pattern);
        free(fs->directory);
        if (fs->curfn)
            free(fs->curfn);
        delete fs;

        return ret;
    }
#else
    #include <windows.h>
    #include <direct.h>
    #include <io.h>
#endif

namespace utils
{
bool file_exists(const std::string& file)
{
    if (file.empty())
        return false;

    std::ifstream mFile(file.c_str());
    return mFile.is_open();
}

string_vector get_directory_list(const std::string& sRelPath)
{
    string_vector lDirList;

    long lHandle, res;
    struct _finddata_t tagData;

    std::string sPattern;
    if (sRelPath.empty())
        sPattern = "*";
    else
        sPattern = sRelPath + "/*";

    lHandle = _findfirst(sPattern.c_str(), &tagData);
    res = 0;
    while (lHandle != -1 && res != -1)
    {
        if ((tagData.attrib & _A_HIDDEN) != 0)
        {
            res = _findnext(lHandle, &tagData);
            continue;
        }

        if ((tagData.attrib & _A_SUBDIR) != 0)
        {
            std::string s = tagData.name;
            if (s != "." && s != "..")
                lDirList.push_back(s);
        }

        res = _findnext(lHandle, &tagData);
    }

    if (lHandle != -1)
        _findclose(lHandle);

    return lDirList;
}

string_vector get_file_list(const std::string& sRelPath, bool bWithPath)
{
    string_vector lFileList;

    long lHandle, res;
    struct _finddata_t tagData;

    std::string sPattern;
    if (sRelPath.empty())
        sPattern = "*";
    else
        sPattern = sRelPath + "/*";

    lHandle = _findfirst(sPattern.c_str(), &tagData);
    res = 0;
    while (lHandle != -1 && res != -1)
    {
        if ((tagData.attrib & _A_HIDDEN) != 0)
        {
            res = _findnext(lHandle, &tagData);
            continue;
        }

        if ((tagData.attrib & _A_SUBDIR) == 0)
        {
            if (bWithPath) lFileList.push_back(sRelPath + "/" + tagData.name);
            else           lFileList.push_back(tagData.name);
        }

        res = _findnext(lHandle, &tagData);
    }

    if (lHandle != -1)
        _findclose(lHandle);

    return lFileList;
}

string_vector get_file_list(const std::string& sRelPath, bool bWithPath, const std::string& sExtensions)
{
    string_vector lExtensions = utils::cut(sExtensions, ",");
    for (auto& sExtension : lExtensions)
        utils::trim(sExtension, ' ');

    string_vector lFileList;

    long lHandle, res;
    struct _finddata_t tagData;

    std::string sPattern;
    if (sRelPath.empty())
        sPattern = "*";
    else
        sPattern = sRelPath + "/*";

    lHandle = _findfirst(sPattern.c_str(), &tagData);
    res = 0;
    while (lHandle != -1 && res != -1)
    {
        if ((tagData.attrib & _A_HIDDEN) != 0)
        {
            res = _findnext(lHandle, &tagData);
            continue;
        }

        if ((tagData.attrib & _A_SUBDIR) == 0)
        {
            std::string sFile = tagData.name;
            std::string sExtension = utils::cut(sFile, ".").back();
            if (utils::find(lExtensions, sExtension) != lExtensions.end())
            {
                if (bWithPath) lFileList.push_back(sRelPath + "/" + tagData.name);
                else           lFileList.push_back(tagData.name);
            }
        }

        res = _findnext(lHandle, &tagData);
    }

    if (lHandle != -1)
        _findclose(lHandle);

    return lFileList;
}

bool make_directory_(const std::string& sPath)
{
#ifdef LINUX
    return (mkdir(sPath.c_str(), 0775) == 0) || (errno == EEXIST);
#else
    return (CreateDirectory(sPath.c_str(), 0) != 0) || (GetLastError() == ERROR_ALREADY_EXISTS);
#endif
}

bool make_directory(const std::string& sPath)
{
    std::string sTemp = sPath;
    utils::trim(sTemp, " \t");
    utils::replace(sTemp, "\\", "/");
    std::vector<std::string> lDirs = utils::cut(sTemp, "/");
    sTemp = "";

    // Add back the root directory or drive
#ifdef LINUX
    if (!sTemp.empty() && sTemp[0] == '/')
    {
        sTemp = "/";
    }
#else
    // Warning: untested code
    if (!lDirs.empty() && !lDirs.front().empty() && lDirs.front().back() == ':')
    {
        sTemp = lDirs.front()+'/';
        lDirs.erase(lDirs.begin());
    }
#endif

    for (const auto& sDir : lDirs)
    {
        if (sDir.empty())
            continue;

        if (sTemp.empty())
            sTemp = sDir;
        else
            sTemp += "/" + sDir;

        if (!make_directory_(sTemp))
            return false;
    }

    return true;
}
}
