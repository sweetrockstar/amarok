/***************************************************************************
 *   Copyright (c) 2007  Nikolaj Hald Nielsen <nhnFreespirit@gmail.com>    *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.          *
 ***************************************************************************/

#include "Mp3tunesService.h"

#include "collection/CollectionManager.h"
#include "Debug.h"
#include "Mp3tunesWorkers.h"
#include "Mp3tunesConfig.h"

#include "StatusBar.h"

#include <KPasswordDialog>
#include <KMessageBox>
#include <threadweaver/ThreadWeaver.h>

#include <QByteArray>
#include <QDomDocument>
#include <QRegExp>

AMAROK_EXPORT_PLUGIN( Mp3tunesServiceFactory )

void Mp3tunesServiceFactory::init()
{
    Mp3tunesConfig config;

    ServiceBase* service = new Mp3tunesService( "MP3tunes.com", config.partnerToken(), config.email(), config.password(),  config.harmonyEnabled(), config.identifier() );
    m_activeServices << service;
    emit newService( service );
}


QString Mp3tunesServiceFactory::name()
{
    return "MP3tunes.com";
}


KPluginInfo Mp3tunesServiceFactory::info()
{
    KPluginInfo pluginInfo(  "amarok_service_mp3tunes.desktop", "services" );
    pluginInfo.setConfig( config() );
    return pluginInfo;
}


KConfigGroup Mp3tunesServiceFactory::config()
{
    return Amarok::config( "Service_Mp3tunes" );
}


bool
Mp3tunesServiceFactory::possiblyContainsTrack(const KUrl & url) const
{
    DEBUG_BLOCK
    QRegExp rx( "http://content.mp3tunes.com/storage/locker(?:get|play)/(.*)\\?(?:sid|partner_token)=.*" ) ;
    int matches = rx.indexIn( url.url() );
    if( matches == -1 ) {
        debug() << "not a track no match";
        return false; // not a mp3tunes url
    }
    QStringList list = rx.capturedTexts();
    QString filekey = list[1]; // Because list[0] is the url itself.
    if ( filekey.isEmpty() ) {
        debug() << "not a track bad url";
        return false;
    }
    debug() << "is a track!";
    return true; // for now: if it's a mp3tunes url.. it's likely the track is in the locker
}


Mp3tunesService::Mp3tunesService(const QString & name, const QString &token, const QString &email, const QString &password, bool harmonyEnabled, const QString &identifier )
 : ServiceBase( name )
 , m_partnerToken( token )
 , m_email( email )
 , m_password( password )
 , m_harmonyEnabled( harmonyEnabled )
 , m_identifier( identifier )
 , m_apiOutputFormat( "xml")
 , m_authenticated( false )
 , m_sessionId ( QString() )
{
    DEBUG_BLOCK
    setShortDescription( i18n( "The MP3tunes Locker: Your Music Everywhere!" ) );
    setIcon( KIcon( "view-services-mp3tunes-amarok" ) );
    debug() << "Making new Locker Object";
    m_locker = new Mp3tunesLocker( "4895500420" );
    debug() << "MP3tunes running automated authenticate.";

    authenticate( email, password );

    if( harmonyEnabled ) {
        debug() << "Making new Daemon";

        char* ident = convertToChar( m_identifier );
        debug () << "Using identifier: " << ident;

        Mp3tunesConfig config;
        if( config.pin() == QString() )
            theDaemon = new Mp3tunesHarmonyDaemon( ident ); //first time harmony login
        else
            theDaemon = new Mp3tunesHarmonyDaemon( ident, //they're not harmony virgins
                                                   convertToChar( config.email() ),
                                                   convertToChar( config.pin() ) );

        Mp3tunesHarmonizer * harmonizer = new Mp3tunesHarmonizer( theDaemon );
        connect( theDaemon, SIGNAL( signalDisconnected() ), this, SLOT( harmonyDisconnected() ) );
        connect( theDaemon, SIGNAL( signalWaitingForEmail() ), this, SLOT( harmonyWaitingForEmail() ) );
        connect( theDaemon, SIGNAL( signalConnected() ), this, SLOT( harmonyConnected() ) );
        connect( theDaemon, SIGNAL( signalError( QString ) ), this, SLOT( harmonyError( QString ) ) );
        connect( theDaemon, SIGNAL( signalDownloadReady( Mp3tunesHarmonyDownload ) ),
                 this, SLOT( harmonyDownloadReady( Mp3tunesHarmonyDownload ) ) );
        connect( theDaemon, SIGNAL( signalDownloadPending( Mp3tunesHarmonyDownload ) ),
                 this, SLOT( harmonyDownloadPending( Mp3tunesHarmonyDownload ) ) );

        debug() << "running harmonizer.";
        ThreadWeaver::Weaver::instance()->enqueue( harmonizer );

        //Close your eyes. Cross your legs. Touch middle fingers to thumbs. Extend your arms.
        //OOOooommmmm
    }
}


Mp3tunesService::~Mp3tunesService()
{
    CollectionManager::instance()->removeUnmanagedCollection( m_collection );
    delete m_locker;
//    delete m_daemon;
    delete m_collection;
}


void Mp3tunesService::polish()
{
    m_bottomPanel->hide();

    if ( !m_authenticated )
        authenticate( m_email, m_password );
}

void Mp3tunesService::authenticate( const QString & uname, const QString & passwd )
{
    DEBUG_BLOCK
    QString username, password;

    if ( uname.isEmpty() || passwd.isEmpty() ) {
        KPasswordDialog dlg( 0 , KPasswordDialog::ShowUsernameLine );  //FIXME 0x02 = KPasswordDialog::showUsername according to api, but that does not work
        dlg.setPrompt( i18n( "Enter your MP3tunes login and password" ) );
        if( !dlg.exec() )
            return; //the user canceled

        username = dlg.username();
        password = dlg.password();
    }
    else
    {
        username = uname;
        password = passwd;
    }

    Mp3tunesLoginWorker * loginWorker = new Mp3tunesLoginWorker( m_locker, username, password);
    //debug() << "Connecting finishedLogin -> authentication complete.";
    connect( loginWorker, SIGNAL( finishedLogin( QString ) ), this, SLOT( authenticationComplete( QString ) ) );
    //debug() << "Connection complete. Enqueueing..";
    ThreadWeaver::Weaver::instance()->enqueue( loginWorker );
    //debug() << "LoginWorker queue";
    The::statusBar()->shortMessage( i18n( "Authenticating"  ) );
}


void Mp3tunesService::authenticationComplete( const QString & sessionId )
{
    DEBUG_BLOCK
    debug() << "Authentication reply: " << sessionId;
    if ( sessionId.isEmpty() )
    {
        QString error = i18n("Mp3tunes failed to Authenticate.");
        if ( m_locker->errorMessage() != QString() )
        {
            error = m_locker->errorMessage(); // Not sure how to i18n this
        }
        The::statusBar()->longMessage( error );

        m_serviceready = false;
    }
    else
    {
        m_sessionId = sessionId;
        m_authenticated = true;

        m_collection = new Mp3tunesServiceCollection( this, m_sessionId, m_locker );
        CollectionManager::instance()->addUnmanagedCollection( m_collection,
                                    CollectionManager::CollectionDisabled );
        QList<int> levels;
        levels << CategoryId::Artist << CategoryId::Album;
        setModel( new SingleCollectionTreeItemModel( m_collection, levels ) );

        m_serviceready = true;
    }
}

void Mp3tunesService::harmonyDisconnected()
{
    DEBUG_BLOCK
    debug() << "Harmony Disconnected!";
    The::statusBar()->shortMessage( i18n( "MP3Tunes Harmony: Disconnected"  ) );
}

void Mp3tunesService::harmonyWaitingForEmail()
{
    debug() << "Waiting for user to input PIN: " << theDaemon->pin();
    The::statusBar()->shortMessage( i18n( "MP3Tunes Harmony: Waiting for PIN Input"  ) );
    KMessageBox::information( this,
                              "Please go to <a href=\"http://www.mp3tunes.com/pin\">mp3tunes.com/pin</a> and enter the following pin.\n\tPIN: " + theDaemon->pin(),
                              "MP3tunes Harmony",
                              QString(),
                              KMessageBox::AllowLink );
}

void Mp3tunesService::harmonyConnected()
{
    debug() << "Harmony Connected!";
    The::statusBar()->shortMessage( i18n( "MP3Tunes Harmony: Successfully Connected"  ) );

    /* at this point since the user has input the pin, we will save the info
       for later authentication */
    Mp3tunesConfig config;
    config.setHarmonyEmail( theDaemon->email() );
    config.setPin( theDaemon->pin() );
    config.save();

}

void Mp3tunesService::harmonyError( const QString &error )
{
    DEBUG_BLOCK
    debug() << "Harmony Error: " << error;
    The::statusBar()->longMessage( "Mp3tunes Harmony Error\n" + error );
}

void Mp3tunesService::harmonyDownloadReady( Mp3tunesHarmonyDownload download )
{
    DEBUG_BLOCK
    debug() << "Got message about " << download.trackTitle() << " by " << download.artistName() << " on " << download. albumTitle();
    foreach( Collection *coll, CollectionManager::instance()->collections().keys() ) {
        if( coll && coll->isWritable())
        {
            debug() << "got collection" << coll->prettyName();
            if ( coll->prettyName() == "Local Collection") //TODO Allow user to choose which collection to sync down to.
            {
                debug() << "got local collection";
                CollectionLocation *dest = coll->location();
                CollectionLocation *source = m_collection->location();
                if( !m_collection->possiblyContainsTrack( download.url() ) )
                    return; //TODO some sort of error handling
                Meta::TrackPtr track( m_collection->trackForUrl( download.url() ) );
                source->prepareCopy( track, dest );
                break;
            }

        }
    }

}

void Mp3tunesService::harmonyDownloadPending( Mp3tunesHarmonyDownload download )
{
    debug() << "Got message about " << download.trackTitle() << " by " << download.artistName() << " on " << download. albumTitle();
}

char *
Mp3tunesService::convertToChar ( const QString &source ) const
{
    QByteArray b = source.toAscii();
    const char *c_tok = b.constData();
    char * ret = ( char * ) malloc ( strlen ( c_tok ) );
    strcpy ( ret, c_tok );
    return ret;
}

#include "Mp3tunesService.moc"
