/*******************************************************************************
* This file is part of PlexyDesk.
*  Maintained by : Siraj Razick <siraj@plexydesk.com>
*  Authored By  :
*
*  PlexyDesk is free software: you can redistribute it and/or modify
*  it under the terms of the GNU Lesser General Public License as published by
*  the Free Software Foundation, either version 3 of the License, or
*  (at your option) any later version.
*
*  PlexyDesk is distributed in the hope that it will be useful,
*  but WITHOUT ANY WARRANTY; without even the implied warranty of
*  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
*  GNU Lesser General Public License for more details.
*
*  You should have received a copy of the GNU General Public License
*  along with PlexyDesk. If not, see <http://www.gnu.org/licenses/lgpl.html>
*******************************************************************************/
#ifndef GOOGLEPLACESENGINE_DATA_H
#define GOOGLEPLACESENGINE_DATA_H

#include <QtCore>

#include <abstractplugininterface.h>
#include <ck_data_source.h>
#include <QtNetwork>

class GooglePlacesEngineData : public PlexyDesk::DataSource {
  Q_OBJECT

public:
  GooglePlacesEngineData(QObject *object = 0);
  virtual ~GooglePlacesEngineData();
  void init();
  QVariantMap readAll();

  void timerEvent(QTimerEvent *event);

public
Q_SLOTS:
  void setArguments(QVariant sourceUpdated);

private:
  class PrivateGooglePlacesEngine;
  PrivateGooglePlacesEngine *const d;
};

#endif
