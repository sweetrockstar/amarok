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
 *   51 Franklin Steet, Fifth Floor, Boston, MA  02111-1307, USA.          *
 ***************************************************************************/ 


#ifndef AMAROKSCRIPTABLECONTENTITEM_H
#define AMAROKSCRIPTABLECONTENTITEM_H

#include "../servicemodelitembase.h"

#include <QList>


enum {STATIC, DYNAMIC};



class ScriptableServiceContentItem : public ServiceModelItemBase
{
public:
    
    ScriptableServiceContentItem(const QString &name, const QString &url, const QString &infoHtml, ScriptableServiceContentItem * parent); //starting out with the very simple version
    

    //used for creating items that use a script to populate their child items
    ScriptableServiceContentItem(const QString &name, QString const &callbackScript, const QString & callbackArgument, const QString &infoHtml, ScriptableServiceContentItem * parent);

    ~ScriptableServiceContentItem();

    void addChildItem ( ScriptableServiceContentItem * childItem );

    ScriptableServiceContentItem *child(int row);
    int childCount() const;
    int columnCount() const;
    QVariant data(int column) const;
    int row() const;
    QList<ServiceModelItemBase*> getChildItems() const;
    bool hasChildren () const;
    QString getUrl();
    QString getInfoHtml();

    int getType();
    QString getCallbackScript();
    QString getCallbackArgument();
    bool isPopulated();


private:

    /*mutable QList<MagnatuneContentItem*> m_childItems;*/

    QString m_url; 
    QString m_name;
    QString m_infoHtml;

    QString m_callbackScript;
    QString m_callbackArgument;

    int m_type;
    bool m_hasPopulatedChildItems;

};

#endif  
