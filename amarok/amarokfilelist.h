/***************************************************************************
                        amarokfilelist.cpp  -  description
                           -------------------
  begin                : Nov 5 2003
  copyright            : (C) 2003 by Mark Kretschmann
  email                :
***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef AMAROKFILELIST_H
#define AMAROKFILELIST_H

#include <kfileitem.h>


// CLASS AmarokFileList =================================================================

class AmarokFileList : public KFileItemList
{
    public:
        AmarokFileList( KFileItemList list, int sortSpec );
        ~AmarokFileList();

    private:
        int compareItems( Item item1, Item item2 );

        // ATTRIBUTES ------
        int m_sortSpec;
};

#endif
