/***************************************************************************
                          gloscope.h  -  description
                             -------------------
    begin                : Jan 17 2004
    copyright            : (C) 2004 by Adam Pigg
    email                : adam@piggz.co.uk
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef GLOSCOPE_H
#define GLOSCOPE_H

#include <config.h>

#ifdef HAVE_QGLWIDGET

#include "analyzerbase3d.h"
#include <qgl.h>
#include <vector>

class QMouseEvent;
class QWidget;

/**
 *@author piggz
 */

class GLAnalyzer : public AnalyzerBase3d
{
  Q_OBJECT

private:
  std::vector<float> m_bands;
  std::vector<float> m_oldy;
  void interpolate(std::vector<float> *);
  void drawScope();
  
  GLfloat x, y;
public:
  GLAnalyzer(QWidget *parent=0, const char *name=0);
  virtual ~GLAnalyzer();
  virtual void drawAnalyzer( std::vector<float> * );

protected:
  virtual void init();
  void initializeGL();
  void resizeGL( int w, int h );
  void paintGL();
};
#endif
#endif
