/****************************************************************************************
 * Copyright (c) 2007 Nikolaj Hald Nielsen <nhn@kde.org>                                *
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

#include "MagnatuneInfoParser.h"

#include "core/support/Components.h"
#include "core/logger/Logger.h"
#include "MagnatuneConfig.h"

#include <KLocalizedString>


using namespace Meta;

void MagnatuneInfoParser::getInfo(const ArtistPtr &artist)
{

    showLoading( i18n( "Loading artist info..." ) );

    MagnatuneArtist * magnatuneArtist = dynamic_cast<MagnatuneArtist *>( artist.data() );
    if ( !magnatuneArtist ) return;

    // first get the entire artist html page
   /* QString tempFile;
    QString orgHtml;*/

    m_infoDownloadJob = KIO::storedGet( magnatuneArtist->magnatuneUrl(), KIO::Reload, KIO::HideProgressInfo );
    Amarok::Logger::newProgressOperation( m_infoDownloadJob, i18n( "Fetching %1 Artist Info", magnatuneArtist->prettyName() ) );
    connect( m_infoDownloadJob, &KJob::result, this, &MagnatuneInfoParser::artistInfoDownloadComplete );

}


void MagnatuneInfoParser::getInfo(const AlbumPtr &album)
{

    showLoading( i18n( "Loading album info..." ) );
    
    MagnatuneAlbum * magnatuneAlbum = dynamic_cast<MagnatuneAlbum *>( album.data() );

    const QString artistName = album->albumArtist()->name();

    QString infoHtml = "<HTML><HEAD><META HTTP-EQUIV=\"Content-Type\" "
                       "CONTENT=\"text/html; charset=utf-8\"></HEAD><BODY>";

    infoHtml += generateHomeLink();
    infoHtml += "<div align=\"center\"><strong>";
    infoHtml += artistName;
    infoHtml += "</strong><br><em>";
    infoHtml += magnatuneAlbum->name();
    infoHtml += "</em><br><br>";
    infoHtml += "<img src=\"" + magnatuneAlbum->coverUrl() +
                "\" align=\"middle\" border=\"1\">";

    // Disable Genre line in Magnatune applet since, well, it doesn't actually put a genre there...
    // Nikolaj, FYI: either the thumbnails aren't working, or they aren't getting through the proxy here.  That would be odd, however, as the database and
    // all HTML are coming through the proxy
    //infoHtml += "<br><br>" + i18n( "Genre: ");// + magnatuneAlbum->
    infoHtml += "<br>" + i18n( "Release Year: %1", QString::number( magnatuneAlbum->launchYear() ) );

    if ( !magnatuneAlbum->description().isEmpty() ) {

        //debug() << "MagnatuneInfoParser: Writing description: '" << album->getDescription() << "'";
        infoHtml += "<br><br><b>" + i18n( "Description:" ) + "</b><br><p align=\"left\" >" + magnatuneAlbum->description();

    }

    infoHtml += "</p><br><br>" + i18n( "From Magnatune.com" ) + "</div>";
    infoHtml += "</BODY></HTML>";

    Q_EMIT ( info( infoHtml ) );
}

void MagnatuneInfoParser::getInfo(const TrackPtr &track)
{
    Q_UNUSED( track );
    return;
}




void
MagnatuneInfoParser::artistInfoDownloadComplete( KJob *downLoadJob )
{

    if ( downLoadJob->error() != 0 )
    {
        //TODO: error handling here
        return ;
    }
    if ( downLoadJob != m_infoDownloadJob )
        return ; //not the right job, so let's ignore it



    QString infoString = ( (KIO::StoredTransferJob* ) downLoadJob )->data();
    //debug() << "MagnatuneInfoParser: Artist info downloaded: " << infoString;
    infoString = extractArtistInfo( infoString );

    //debug() << "html: " << trimmedInfo;

    Q_EMIT ( info( infoString ) );

}

QString
MagnatuneInfoParser::extractArtistInfo( const QString &artistPage )
{
    QString trimmedHtml;


    int sectionStart = artistPage.indexOf( "<!-- ARTISTBODY -->" );
    int sectionEnd = artistPage.indexOf( "<!-- /ARTISTBODY -->", sectionStart );

    trimmedHtml = artistPage.mid( sectionStart, sectionEnd - sectionStart );

    int buyStartIndex = trimmedHtml.indexOf( "<!-- PURCHASE -->" );
    int buyEndIndex;

    //we are going to integrate the buying of music (I hope) so remove these links

    while ( buyStartIndex != -1 )
    {
        buyEndIndex = trimmedHtml.indexOf( "<!-- /PURCHASE -->", buyStartIndex ) + 18;
        trimmedHtml.remove( buyStartIndex, buyEndIndex - buyStartIndex );
        buyStartIndex = trimmedHtml.indexOf( "<!-- PURCHASE -->", buyStartIndex );
    }


    QString infoHtml = "<HTML><HEAD><META HTTP-EQUIV=\"Content-Type\" "
                       "CONTENT=\"text/html; charset=iso-8859-1\"></HEAD><BODY>";
    infoHtml += generateHomeLink();
    infoHtml += trimmedHtml;
    infoHtml += "</BODY></HTML>";


    return infoHtml;
}

void MagnatuneInfoParser::getFrontPage()
{

    if( !m_cachedFrontpage.isEmpty() )
    {
        Q_EMIT ( info( m_cachedFrontpage ) );
        return;
    }

    showLoading( i18n( "Loading Magnatune.com frontpage..." ) );
    
    m_pageDownloadJob = KIO::storedGet( QUrl("http://magnatune.com/amarok_frontpage.html"), KIO::Reload, KIO::HideProgressInfo );
    Amarok::Logger::newProgressOperation( m_pageDownloadJob, i18n( "Fetching Magnatune.com front page" ) );
    connect( m_pageDownloadJob, &KJob::result, this, &MagnatuneInfoParser::frontpageDownloadComplete );
}

void MagnatuneInfoParser::getFavoritesPage()
{
    MagnatuneConfig config;

    if ( !config.isMember() )
        return;

    showLoading( i18n( "Loading your Magnatune.com favorites page..." ) );

    QString type;
    if( config.membershipType() == MagnatuneConfig::STREAM )
        type = "stream";
    else
         type = "download";

    QString user = config.username();
    QString password = config.password();

    QUrl url = QUrl::fromUserInput( "http://" + user + ":" + password + "@" + type.toLower() + ".magnatune.com/member/amarok_favorites.php" );

    m_pageDownloadJob = KIO::storedGet( url, KIO::Reload, KIO::HideProgressInfo );
    Amarok::Logger::newProgressOperation( m_pageDownloadJob, i18n( "Loading your Magnatune.com favorites page..." ) );
    connect( m_pageDownloadJob, &KJob::result, this, &MagnatuneInfoParser::userPageDownloadComplete );
}

void MagnatuneInfoParser::getRecommendationsPage()
{
    MagnatuneConfig config;

    if ( !config.isMember() )
        return;

    showLoading( i18n( "Loading your personal Magnatune.com recommendations page..." ) );

    QString type;
    if( config.membershipType() == MagnatuneConfig::STREAM )
        type = "stream";
    else
         type = "download";

    QString user = config.username();
    QString password = config.password();

    QUrl url = QUrl::fromUserInput( "http://" + user + ":" + password + "@" + type.toLower() + ".magnatune.com/member/amarok_recommendations.php" );

    m_pageDownloadJob = KIO::storedGet( url, KIO::Reload, KIO::HideProgressInfo );
    Amarok::Logger::newProgressOperation( m_pageDownloadJob, i18n( "Loading your personal Magnatune.com recommendations page..." ) );
    connect( m_pageDownloadJob, &KJob::result, this, &MagnatuneInfoParser::userPageDownloadComplete );
}

void MagnatuneInfoParser::frontpageDownloadComplete( KJob * downLoadJob )
{
    if ( downLoadJob->error() != 0 )
    {
        //TODO: error handling here
        return ;
    }
    if ( downLoadJob != m_pageDownloadJob )
        return ; //not the right job, so let's ignore it

    QString infoString = ((KIO::StoredTransferJob* )downLoadJob)->data();

    //insert menu
    MagnatuneConfig config;
    if( config.isMember() )
        infoString.replace( "<!--MENU_TOKEN-->", generateMemberMenu() );

    //insert fancy amarok url links to the artists
    infoString = createArtistLinks( infoString );

    if( m_cachedFrontpage.isEmpty() )
        m_cachedFrontpage = infoString;

    Q_EMIT ( info( infoString ) );
}

void MagnatuneInfoParser::userPageDownloadComplete( KJob * downLoadJob )
{
    if ( downLoadJob->error() )
    {
        //TODO: error handling here
        return ;
    }
    if ( downLoadJob != m_pageDownloadJob )
        return ; //not the right job, so let's ignore it



    QString infoString = ((KIO::StoredTransferJob* )downLoadJob)->data();

    //insert menu
    MagnatuneConfig config;
    if( config.isMember() )
        infoString.replace( "<!--MENU_TOKEN-->", generateMemberMenu() );

    //make sure that any pages that use the old command name "service_magnatune" replaces it with "service-magnatune"
    infoString.replace( "service_magnatune", "service-magnatune" );

    Q_EMIT ( info( infoString ) );
}


QString MagnatuneInfoParser::generateMemberMenu()
{
    QString homeUrl = "amarok://service-magnatune?command=show_home";
    QString favoritesUrl = "amarok://service-magnatune?command=show_favorites";
    QString recommendationsUrl = "amarok://service-magnatune?command=show_recommendations";

    QString menu = "<div align='right'>"
                       "[<a href='" + homeUrl + "' >Home</a>]&nbsp;"
                       "[<a href='" + favoritesUrl + "' >Favorites</a>]&nbsp;"
                       "[<a href='" + recommendationsUrl + "' >Recommendations</a>]&nbsp;"
                    "</div>";

    return menu;
}

QString
MagnatuneInfoParser::generateHomeLink()
{
    QString homeUrl = "amarok://service-magnatune?command=show_home";
    QString link = "<div align='right'>"
                    "[<a href='" + homeUrl + "' >Home</a>]&nbsp;"
                   "</div>";

    return link;
}

QString
MagnatuneInfoParser::createArtistLinks( const QString &page )
{
    //the artist name is wrapped in <!--ARTIST_TOKEN-->artist<!--/ARTIST_TOKEN-->

    QString returnPage = page;

    int startTokenLength = QStringLiteral( "<!--ARTIST_TOKEN-->" ).length();

    int offset = 0;
    int startTokenIndex = page.indexOf( "<!--ARTIST_TOKEN-->", offset );
    int endTokenIndex = 0;

    while( startTokenIndex != -1 )
    {
        endTokenIndex = page.indexOf( "<!--/ARTIST_TOKEN-->", startTokenIndex );
        if( endTokenIndex == -1 )
            break; //bail out

        offset = endTokenIndex;

        //get the artist namespace

        int artistLength = endTokenIndex - ( startTokenIndex + startTokenLength );
        QString artist = page.mid( startTokenIndex + startTokenLength, artistLength );

        //replace in the artist amarok url

        QString replaceString = "<!--ARTIST_TOKEN-->" + artist + "<!--/ARTIST_TOKEN-->";
        QString artistLink = "<a href='amarok://navigate/internet/Magnatune.com?filter=artist:%22" + AmarokUrl::escape( artist ) + "%22&levels=artist-album'>" + artist + "</a>";

        returnPage = returnPage.replace( replaceString, artistLink );

        startTokenIndex = page.indexOf( "<!--ARTIST_TOKEN-->", offset );
    }

    return returnPage;
}



