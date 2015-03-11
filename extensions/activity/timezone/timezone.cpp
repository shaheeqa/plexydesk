﻿/*******************************************************************************
* This file is part of PlexyDesk.
*  Maintained by : Siraj Razick <siraj@plexydesk.org>
*  Authored By  : *
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
#include "timezone.h"
#include <widget.h>
#include <plexyconfig.h>
#include <QTimer>
#include <view_controller.h>
#include <modelview.h>
#include <label.h>

class TimeZoneActivity::PrivateTimeZone
{
public:
  PrivateTimeZone() {}
  ~PrivateTimeZone()
  {
    if (mWindowPtr) {
      delete mWindowPtr;
    }
    qDebug() << Q_FUNC_INFO << "Delete TimeZone Activity";
  }

  void loadTimeZones();

  UIKit::Window *mWindowPtr;
  UIKit::ModelView *mTimeZoneBrowserPtr;
};

TimeZoneActivity::TimeZoneActivity(QGraphicsObject *aParent)
  : UIKit::DesktopActivity(aParent), mPrivConstPtr(new PrivateTimeZone) {}

TimeZoneActivity::~TimeZoneActivity() { delete mPrivConstPtr; }

void TimeZoneActivity::create_window(const QRectF &aWindowGeometry,
                                    const QString &aWindowTitle,
                                    const QPointF &aWindowPos)
{
  mPrivConstPtr->mWindowPtr = new UIKit::Window();

  mPrivConstPtr->mWindowPtr->set_window_title(aWindowTitle);
  mPrivConstPtr->mTimeZoneBrowserPtr =
    new UIKit::ModelView(mPrivConstPtr->mWindowPtr);
  mPrivConstPtr->mTimeZoneBrowserPtr->setGeometry(aWindowGeometry);
  mPrivConstPtr->mTimeZoneBrowserPtr->set_view_geometry(aWindowGeometry);

  mPrivConstPtr->mWindowPtr->set_window_content(
    mPrivConstPtr->mTimeZoneBrowserPtr);

  set_geometry(aWindowGeometry);

  exec(aWindowPos);

  mPrivConstPtr->mWindowPtr->on_window_discarded([this](UIKit::Window * aWindow) {
    discard_activity();
  });

  mPrivConstPtr->loadTimeZones();
}

QVariantMap TimeZoneActivity::result() const { return QVariantMap(); }

void TimeZoneActivity::update_attribute(const QString &aName,
                                       const QVariant &aVariantData) {}

UIKit::Window *TimeZoneActivity::window() const { return mPrivConstPtr->mWindowPtr; }

void TimeZoneActivity::cleanup()
{
  if (mPrivConstPtr->mWindowPtr) {
    delete mPrivConstPtr->mWindowPtr;
  }
  mPrivConstPtr->mWindowPtr = 0;
}

void TimeZoneActivity::PrivateTimeZone::loadTimeZones()
{
  foreach(const QByteArray id,  QTimeZone::availableTimeZoneIds()) {
    UIKit::Label *lTimeZoneLabelPtr =
      new UIKit::Label(mTimeZoneBrowserPtr);
    lTimeZoneLabelPtr->setMinimumSize(
      mTimeZoneBrowserPtr->geometry().width(),
      32);
    lTimeZoneLabelPtr->set_size(QSizeF(mTimeZoneBrowserPtr->boundingRect().width(),
                                      32));
    lTimeZoneLabelPtr->show();

    qDebug() << Q_FUNC_INFO << mTimeZoneBrowserPtr->boundingRect();

    lTimeZoneLabelPtr->set_label(QString(id));
    mTimeZoneBrowserPtr->insert(lTimeZoneLabelPtr);
  }
}
