/***************************************************************************
 *   Copyright (C) 2009 Sven Krohlas <sven@getamarok.com>                  *
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
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.         *
 ***************************************************************************/

/* Test script to test our test framework itself */

/* Initialisation stuff that should not be taken intoa ccount for performance    */
/* measuring.                                                                    */

// none


/* Measure passed time between each call to AmarokTest.testResult() from now on. */
AmarokTest.startTimer();


/* The tests */
AmarokTest.testResult( "AmarokTest: Successful test", "OK", "OK" );
AmarokTest.testResult( "AmarokTest: Failed test",     "OK", "not OK" );
