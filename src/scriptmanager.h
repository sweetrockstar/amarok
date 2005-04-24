/***************************************************************************
 *   Copyright (C) 2004-2005 by Mark Kretschmann <markey@web.de>           *
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
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/

#ifndef AMAROK_SCRIPTMANAGER_H
#define AMAROK_SCRIPTMANAGER_H

#include "engineobserver.h"   //baseclass
#include "playlistwindow.h"

#include <qmap.h>

#include <kdialogbase.h>      //baseclass
#include <kurl.h>

class MetaBundle;
class ScriptManagerBase;
class QListViewItem;
class KArchiveDirectory;
class KProcess;
class KProcIO;


/**
 * @class ScriptManager
 * @short Script management widget and backend
 * @author Mark Kretschmann <markey@web.de>
 *
 * Script notifications, sent to stdin:
 *   configure
 *   engineStateChange: {empty|idle|paused|playing}
 *   trackChange
 *
 * @see http://amarok.kde.org/wiki/index.php/Script-Writing_HowTo
 */

class ScriptManager : public KDialogBase, public EngineObserver
{
        Q_OBJECT

    friend class AmarokScriptNewStuff;

    public:
        ScriptManager( QWidget *parent = 0, const char *name = 0 );
        virtual ~ScriptManager();

        static ScriptManager* instance() { return s_instance ? s_instance : new ScriptManager( PlaylistWindow::self() ); }

        /**
         * Runs the script with the given name. Used by the DCOP handler.
         * @param name The name of the script.
         * @return True if successful.
         */
        bool runScript( const QString& name );

        /**
         * Stops the script with the given name. Used by the DCOP handler.
         * @param name The name of the script.
         * @return True if successful.
         */
        bool stopScript( const QString& name );

        /** Returns a list of all currently running scripts. Used by the DCOP handler. */
        QStringList listRunningScripts();

    private slots:
        /** Finds all installed scripts and adds them to the listview */
        void findScripts();

        /** Enables/disables the buttons */
        void slotCurrentChanged( QListViewItem* );

        bool slotInstallScript( const QString& path = QString::null );
        void slotRetrieveScript();
        void slotUninstallScript();
        void slotEditScript();
        bool slotRunScript();
        void slotStopScript();
        void slotConfigureScript();
        void slotAboutScript();

        void scriptFinished( KProcess* process );

    private:
        /** Sends a string message to all running scripts */
        void notifyScripts( const QString& message );

        /** Adds a script to the listview */
        void loadScript( const QString& path );

        /** Copies the file permissions from the tarball and loads the script */
        void recurseInstall( const KArchiveDirectory* archiveDir, const QString& destination );

        /** Observer reimplementations **/
        void engineStateChanged( Engine::State state );
        void engineNewMetaData( const MetaBundle& /*bundle*/, bool /*trackChanged*/ );

        static ScriptManager* s_instance;
        ScriptManagerBase*    m_base;
        bool                  m_installSuccess;

        struct ScriptItem {
            KURL           url;
            KProcIO*       process;
            QListViewItem* li;
        };

        typedef QMap<QString, ScriptItem> ScriptMap;

        ScriptMap m_scripts;
};


#endif /* AMAROK_SCRIPTMANAGER_H */


