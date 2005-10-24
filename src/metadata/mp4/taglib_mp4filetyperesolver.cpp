// (c) 2005 Martin Aumueller <aumuell@reserv.at>
// See COPYING file for licensing information

#include "taglib_mp4filetyperesolver.h"
#include "taglib_mp4file.h"

#ifdef HAVE_MP4V2
#include <mp4.h>
#endif

TagLib::File *MP4FileTypeResolver::createFile(const char *fileName,
        bool readProperties,
        TagLib::AudioProperties::ReadStyle propertiesStyle) const
{
#ifdef HAVE_MP4V2
    const char *ext = strrchr(fileName, '.');
    if(ext && (!strcasecmp(ext, ".m4a")
                || !strcasecmp(ext, ".m4b") || !strcasecmp(ext, ".m4p")
                || !strcasecmp(ext, ".aac") || !strcasecmp(ext, ".mp4")))
    {
        MP4FileHandle h = MP4Read(fileName, 0);
        if(MP4_INVALID_FILE_HANDLE == h)
        {
            return 0;
        }

        return new TagLib::MP4::File(fileName, readProperties, propertiesStyle, h);
    }
#else
    (void)fileName;
    (void)readProperties;
    (void)propertiesStyle;
#endif
    return 0;
}
