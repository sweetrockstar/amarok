#include <iostream>
#include "itunescmtbox.h"
#include "itunesdatabox.h"
#include "mp4isobox.h"
#include "mp4file.h"
#include "tfile.h"
#include "mp4tagsproxy.h"

using namespace TagLib;

class MP4::ITunesCmtBox::ITunesCmtBoxPrivate
{
public:
  ITunesDataBox* dataBox;
};

MP4::ITunesCmtBox::ITunesCmtBox( TagLib::File* file, MP4::Fourcc fourcc, uint size, long offset )
	:Mp4IsoBox(file, fourcc, size, offset)
{
  d = new MP4::ITunesCmtBox::ITunesCmtBoxPrivate();
  d->dataBox = 0;
}

MP4::ITunesCmtBox::~ITunesCmtBox()
{
  if( d->dataBox != 0 )
    delete d->dataBox;
  delete d;
}

//! parse the content of the box
void MP4::ITunesCmtBox::parse()
{
  TagLib::MP4::File* mp4file = static_cast<MP4::File*>( file() );

  // parse data box
  TagLib::uint size;
  MP4::Fourcc  fourcc;

  if(mp4file->readSizeAndType( size, fourcc ) == true)  
  {
    // check for type - must be 'data'
    if( fourcc != MP4::Fourcc("data") )
    {
      std::cerr << "bad atom in itunes tag - skipping it." << std::endl; 
      // jump over data tag
      mp4file->seek( size-8, TagLib::File::Current );
      return;
    }
    d->dataBox = new ITunesDataBox( mp4file, fourcc, size, mp4file->tell() );
    d->dataBox->parsebox();
  }
  else
  {
    // reading unsuccessful - serious error!
    std::cerr << "Error in parsing ITunesCmtBox - serious Error in taglib!" << std::endl;
    return;
  }
  // register data box
  mp4file->proxy()->registerBox( Mp4TagsProxy::comment, d->dataBox );

#if 0
  // get data pointer - just for debuging...
  TagLib::String dataString( d->dataBox->data() );
  std::cout << "Content of title box: " << dataString << std::endl;
#endif
}

