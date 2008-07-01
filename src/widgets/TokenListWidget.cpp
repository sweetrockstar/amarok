/******************************************************************************
 * Copyright (C) 2008 Teo Mrnjavac <teo.mrnjavac@gmail.com>                   *
 *                                                                            *
 * This program is free software; you can redistribute it and/or              *
 * modify it under the terms of the GNU General Public License as             *
 * published by the Free Software Foundation; either version 2 of             *
 * the License, or (at your option) any later version.                        *
 *                                                                            *
 * This program is distributed in the hope that it will be useful,            *
 * but WITHOUT ANY WARRANTY; without even the implied warranty of             *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the              *
 * GNU General Public License for more details.                               *
 *                                                                            *
 * You should have received a copy of the GNU General Public License          *
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.      *
 ******************************************************************************/
#include "TokenListWidget.h"

#include <KApplication>
#include <KDialog>

#include <QMouseEvent>


TokenListWidget::TokenListWidget(QWidget *parent) : KListWidget(parent)
{
    setAcceptDrops(true);
}

void
TokenListWidget::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton)
        startPos = event->pos();            //store the start position
    KListWidget::mousePressEvent(event);    //feed it to parent's event
}

void
TokenListWidget::mouseMoveEvent(QMouseEvent *event)
{
    if (event->buttons() & Qt::LeftButton) {
        int distance = (event->pos() - startPos).manhattanLength();
        if (distance >= KApplication::startDragDistance())
        {
            performDrag();
        }
    }
    KListWidget::mouseMoveEvent(event);
}

void
TokenListWidget::dragEnterEvent(QDragEnterEvent *event)
{
    QWidget *source = qobject_cast<QWidget *>(event->source());
    if (source && source != this) {
        event->setDropAction(Qt::CopyAction);
        event->accept();
    }
}

void
TokenListWidget::dragMoveEvent(QDragMoveEvent *event)        //overrides QListWidget's implementation
{
    QWidget *source = qobject_cast<QWidget *>(event->source());
    if (source && source != this) {
        event->setDropAction(Qt::CopyAction);
        event->accept();
    }
}

void
TokenListWidget::dropEvent(QDropEvent *event)
{
    QWidget *source = qobject_cast<QWidget *>(event->source());
    if (source && source != this) {
        addItem(event->mimeData()->text());     //TODO:    mimeData->setData("application/x-amarok-tag-token", itemData);

        event->setDropAction(Qt::CopyAction);
        event->accept();
    }
}

void
TokenListWidget::performDrag()
{
    QListWidgetItem *item = currentItem();
    if (item) {
        QMimeData *mimeData = new QMimeData;
        mimeData->setText(item->text());

        QDrag *drag = new QDrag(this);
        drag->setMimeData(mimeData);
        //TODO:set a pointer for the drag, like this: drag->setPixmap(QPixmap("foo.png"));
    }
}