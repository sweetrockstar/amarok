/****************************************************************************************
 * Copyright (c) 2007 Bart Cerneels <bart.cerneels@kde.org>                             *
 *                                                                                      *
 * This program is free software; you can redistribute it and/or modify it under        *
 * the terms of the GNU General Public License as published by the Free Software        *
 * Foundation; either version 2 of the License, or (at your option) any later           *
 * version.                                                                             *
 *                                                                                      *
 * This program is distributed in the hope that it will be useful, but WITHOUT ANY      *
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A      *
 * PARTICULAR PURPOSE. See the GNU General Public License for more details.             *
 *                                                                                      *
 * You should have received a copy of the GNU General Public License along with         *
 * this program.  If not, see <http://www.gnu.org/licenses/>.                           *
 ****************************************************************************************/

#include "PodcastReader.h"

#include "Debug.h"
#include "statusbar/StatusBar.h"

#include <kio/job.h>
#include <kurl.h>
#include <KDateTime>

#include <QDate>
#include <time.h>

using namespace Meta;

PodcastReader::PodcastReader( PodcastProvider * podcastProvider )
        : QXmlStreamReader()
        , m_feedType( UnknownFeedType )
        , m_podcastProvider( podcastProvider )
        , m_transferJob( 0 )
        , m_current( 0 )
        , m_parsingImage( false )
{}

PodcastReader::~PodcastReader()
{
    DEBUG_BLOCK
}

bool PodcastReader::read ( QIODevice *device )
{
    DEBUG_BLOCK
    setDevice( device );
    return read();
}

bool
PodcastReader::read( const KUrl &url )
{
    DEBUG_BLOCK

    m_url = url;

    m_transferJob = KIO::get( m_url, KIO::Reload, KIO::HideProgressInfo );

    connect( m_transferJob, SIGNAL( data( KIO::Job *, const QByteArray & ) ),
             SLOT( slotAddData( KIO::Job *, const QByteArray & ) ) );

    connect( m_transferJob, SIGNAL(  result( KJob * ) ),
             SLOT( downloadResult( KJob * ) ) );

    connect( m_transferJob, SIGNAL( redirection( KIO::Job *, const KUrl & ) ),
             SLOT( slotRedirection( KIO::Job *, const KUrl & ) ) );
    connect( m_transferJob, SIGNAL( permanentRedirection( KIO::Job *,
             const KUrl &, const KUrl &) ),
             SLOT( slotPermanentRedirection( KIO::Job *, const KUrl &,
             const KUrl &) ) );

    QString description = i18n("Importing podcast channel from %1", url.url());
    if( m_channel )
    {
        description = m_channel->title().isEmpty()
            ? i18n("Updating podcast channel")
            : i18n("Updating \"%1\"", m_channel->title());
    }

    The::statusBar()->newProgressOperation( m_transferJob, description )
        ->setAbortSlot( this, SLOT( slotAbort() ) );

    return true;
}

void
PodcastReader::slotAbort()
{
    DEBUG_BLOCK
}

bool
PodcastReader::update( PodcastChannelPtr channel )
{
    DEBUG_BLOCK
    m_channel = channel;
    m_current = static_cast<PodcastMetaCommon *>(channel.data());

    return read( m_channel->url() );
}

void
PodcastReader::slotAddData( KIO::Job *job, const QByteArray &data )
{
    DEBUG_BLOCK
    Q_UNUSED( job )

    QXmlStreamReader::addData( data );
    //parse some more data
    read();
}

void
PodcastReader::downloadResult( KJob * job )
{
    DEBUG_BLOCK

    KIO::TransferJob *transferJob = dynamic_cast<KIO::TransferJob *>( job );
    if ( transferJob && transferJob->isErrorPage() )
    {
        QString errorMessage =
                i18n( "Importing podcast from %1 failed with error:\n", m_url.url() );
        if( m_channel )
        {
            errorMessage = m_channel->title().isEmpty()
                  ? i18n( "Updating podcast from %1 failed with error:\n", m_url.url() )
                  : i18n( "Updating \"%1\" failed with error:\n", m_channel->title() );
        }
        errorMessage = errorMessage.append( job->errorString() );

        The::statusBar()->longMessage( errorMessage, StatusBar::Sorry );
    }
    else if( job->error() )
    {
        QString errorMessage =
                i18n( "Importing podcast from %1 failed with error:\n", m_url.url() );
        if( m_channel )
        {
            errorMessage = m_channel->title().isEmpty()
                  ? i18n( "Updating podcast from %1 failed with error:\n", m_url.url() )
                  : i18n( "Updating \"%1\" failed with error:\n", m_channel->title() );
        }
        errorMessage = errorMessage.append( job->errorString() );

        The::statusBar()->longMessage( errorMessage, StatusBar::Sorry );
    }
    //parse some more data
    read();
}

bool
PodcastReader::read()
{
    DEBUG_BLOCK
    bool result = true;

    while( !atEnd() )
    {
        if( !error() || error() == PrematureEndOfDocumentError )
            readNext();
        else
            break; //error handling after while loop.

        if( m_feedType == UnknownFeedType )
        {
            //Pre Channel
            if( isStartElement() )
            {
                debug() << "Initial StartElement: " << QXmlStreamReader::name().toString();
                debug() << "version: " << attributes().value ( "version" ).toString();
                if( QXmlStreamReader::name() == "rss" && attributes().value ( "version" ) == "2.0" )
                {
                    m_feedType = Rss20FeedType;
                }
                else if( QXmlStreamReader::name() == "html" )
                {
                    m_feedType = ErrorPageType;
                    raiseError( i18n( "An HTML page was received. Expected an RSS 2.0 feed" ) );
                    result = false;
                    break;
                }
                else
                {
                    //TODO: change this string once we support more
                    raiseError( i18n( "%1 is not an RSS version 2.0 feed.", m_url.url() ) );
                    result = false;
                    break;
                }
            }
            else if( tokenType() != QXmlStreamReader::StartDocument )
            {
                debug() << "some weird thing happend at line: "
                        << QXmlStreamReader::lineNumber();
                debug() << "\terror: " << QXmlStreamReader::name().toString();
                debug() << "\ttoken type: " << tokenString();
            }
        }
        else
        {
            if( isStartElement() )
            {
                if( !m_current )
                {
                    if( QXmlStreamReader::name() == "channel" )
                    {
                        debug() << "new channel";
                        m_channel = new Meta::PodcastChannel();
                        m_channel->setUrl( m_url );
                        m_channel->setSubscribeDate( QDate::currentDate() );
                        /* add this new channel to the provider, we get a pointer to a
                        * PodcastChannelPtr of the correct type which we will use from now on.
                        */
                        m_channel = m_podcastProvider->addChannel( m_channel );

                        m_current = static_cast<Meta::PodcastMetaCommon *>( m_channel.data() );
                    }
                    else
                    {
                        raiseError( i18n( "Feed %1 didn't start with a valid channel tag.", m_url.url() ) );
                        result = false;
                        break;
                    }
                }
                else if( QXmlStreamReader::name() == "item" )
                {
                    debug() << "new episode";
                    m_current = new Meta::PodcastEpisode( m_channel );
                }
                else if( QXmlStreamReader::name() == "enclosure" )
                {
                    m_urlString = QXmlStreamReader::attributes().value( QString(), QString("url") ).toString();
                }
                else if( QXmlStreamReader::name() == "image" )
                {
                    m_parsingImage = true;
                }

                m_currentTag = QXmlStreamReader::name().toString();
            }
            else if( isEndElement() )
            {
                if (QXmlStreamReader::name() == "item")
                {
                    commitEpisode();
                }
                else if( QXmlStreamReader::name() == "channel" )
                {
                    commitChannel();
                    emit finished( this );
                    break;
                }
                else if( QXmlStreamReader::name() == "title")
                {
                    if( !m_parsingImage )
                    {
                        // Remove redundant whitespace from the title.
                        m_current->setTitle( m_titleString.simplified() );
                    }
                    m_titleString.clear();
                }
                else if( QXmlStreamReader::name() == "description" )
                {
                    m_current->setDescription( m_descriptionString );
                    m_descriptionString.clear();
                }
                else if( QXmlStreamReader::name() == "guid" )
                {
                    PodcastEpisode * episode = dynamic_cast<PodcastEpisode *>(m_current);
                    if( episode )
                        episode->setGuid( m_guidString );
                    m_guidString.clear();
                }
                else if( QXmlStreamReader::name() == "enclosure" )
                {
                    PodcastEpisode * episode = dynamic_cast<PodcastEpisode *>(m_current);
                    if( episode )
                        episode->setUidUrl( KUrl( m_urlString ) );
                    m_urlString.clear();
                }
                else if( QXmlStreamReader::name() == "link" )
                {
                    if( !m_parsingImage )
                        m_channel->setWebLink( KUrl( m_linkString ) );
                    //TODO save image data
                    m_linkString.clear();
                }
                else if( QXmlStreamReader::name() == "pubDate")
                {
                    PodcastEpisode * episode = dynamic_cast<PodcastEpisode *>(m_current);
                    if( episode )
                        episode->setPubDate( parsePubDate( m_pubDateString ) );
                    m_pubDateString.clear();
                }
                else if( QXmlStreamReader::name() == "image" )
                {
                    m_parsingImage = false;
                }
                else if( QXmlStreamReader::name() == "url" && m_parsingImage )
                {
                    if( m_channel )
                        m_channel->setImageUrl( KUrl( m_urlString ) );
                    m_urlString.clear();
                }
            }
            else if( isCharacters() && !isWhitespace() )
            {
                if( m_currentTag == "title" )
                    m_titleString += text().toString();
                else if( m_currentTag == "link" )
                    m_linkString += text().toString();
                else if( m_currentTag == "description" )
                    m_descriptionString += text().toString();
                else if( m_currentTag == "pubDate" )
                    m_pubDateString += text().toString();
                else if( m_currentTag == "guid" )
                    m_guidString += text().toString();
                else if( m_currentTag == "url" )
                    m_urlString += text().toString();
            }
        }
    }

    if( error() )
    {
        if( error() == QXmlStreamReader::PrematureEndOfDocumentError)
        {
            debug() << "waiting for data at line " << lineNumber();
        }
        else
        {
            debug() << "XML ERROR at line: " << lineNumber()
                    << " column: " << columnNumber();
            debug() << "\t" << errorString();
            debug() << "\ttoken name = " << QXmlStreamReader::name().toString();
            debug() << "\ttokenType = " << tokenString();
            /*FIXME: KJob->kill() with EmitResult doesn't seem to work. If result is not
              emitted the progress won't disappear from the statusbar.
              can be hacked around by directly emitting result and percent from
              PodcastReader.
            */
            m_transferJob->kill( /*KJob::EmitResult*/ );
            m_transferJob = 0;
            emit finished( this );
        }
    }
    return result;
}

QDateTime
PodcastReader::parsePubDate( const QString &dateString )
{
    DEBUG_BLOCK
    QDateTime pubDate;

    debug() << "Parsing pubdate: " << dateString;
    if( dateString.contains( ',' ) )
        pubDate = KDateTime::fromString( dateString, KDateTime::RFCDateDay ).dateTime();
    else
        pubDate = KDateTime::fromString( dateString, KDateTime::RFCDate ).dateTime();

    debug() << "result: " << pubDate.toString();
    return pubDate;
}

void
PodcastReader::readUnknownElement()
{
    DEBUG_BLOCK
    Q_ASSERT ( isStartElement() );

    debug() << "unknown element: " << QXmlStreamReader::name().toString();

    while ( !atEnd() )
    {
        readNext();

        if ( isEndElement() )
            break;

        if ( isStartElement() )
            readUnknownElement();
    }
}

void
PodcastReader::slotRedirection( KIO::Job * job, const KUrl & url )
{
    DEBUG_BLOCK
    Q_UNUSED( job );
    debug() << "redirected to: " << url.url();

}

void
PodcastReader::slotPermanentRedirection( KIO::Job * job, const KUrl & fromUrl,
        const KUrl & toUrl )
{
    DEBUG_BLOCK
    Q_UNUSED( job ); Q_UNUSED( fromUrl );
    debug() << "permanently redirected to: " << toUrl.url();
    m_url = toUrl;
    /* change the url for existing feeds as well. Permanent redirection means the old one
    might dissapear soon. */
    if( m_channel )
        m_channel->setUrl( m_url );
}

void
PodcastReader::commitChannel()
{
    Q_ASSERT( m_channel );
    //TODO: we probably need to notify the provider here to we are done updating the channel
//     emit finished( this );
}

void
PodcastReader::commitEpisode()
{
    DEBUG_BLOCK
    Q_ASSERT( m_current );
    PodcastEpisodePtr item = PodcastEpisodePtr( static_cast<PodcastEpisode *>(m_current) );
//
//     PodcastEpisodePtr episodeMatch = podcastEpisodeCheck( item );
//     if( episodeMatch == item )
//     {
//         debug() << "commit episode " << item->title();
//
//         Q_ASSERT( m_channel );
//         //make a copy of the pointer and add that to the channel
//         m_channel->addEpisode( PodcastEpisodePtr( item ) );
//     }

    if( !m_podcastProvider->possiblyContainsTrack( item->uidUrl() ) )
    {
        Meta::PodcastEpisodePtr episode = PodcastEpisodePtr( item );
        episode = m_channel->addEpisode( episode );
        //also let the provider know an episode has been added
        //TODO: change into a signal
        m_podcastProvider->addEpisode( episode );
    }

    m_current = static_cast<PodcastMetaCommon *>( m_channel.data() );
}

Meta::PodcastEpisodePtr
PodcastReader::podcastEpisodeCheck( Meta::PodcastEpisodePtr episode )
{
//     DEBUG_BLOCK
    Meta::PodcastEpisodePtr episodeMatch = episode;
    Meta::PodcastEpisodeList episodes = m_channel->episodes();

//     debug() << "episode title: " << episode->title();
//     debug() << "episode url: " << episode->prettyUrl();
//     debug() << "episode guid: " << episode->guid();

    foreach( PodcastEpisodePtr match, episodes )
    {
//         debug() << "match title: " << match->title();
//         debug() << "match url: " << match->prettyUrl();
//         debug() << "match guid: " << match->guid();

        int score = 0;
        if( !episode->title().isEmpty() && episode->title() == match->title() )
            score += 1;
        if( !episode->prettyUrl().isEmpty() && episode->prettyUrl() == match->prettyUrl() )
            score += 3;
        if( !episode->guid().isEmpty() && episode->guid() == match->guid() )
            score += 3;

//         debug() << "score: " << score;
        if( score >= 3 )
        {
            episodeMatch = match;
            break;
        }
    }

    return episodeMatch;
}

#include "PodcastReader.moc"
