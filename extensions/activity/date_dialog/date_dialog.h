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
#ifndef DATEPICKER_ACTIVITY_H
#define DATEPICKER_ACTIVITY_H

#include <ck_desktop_dialog.h>
#include <ck_window.h>

class date_dialog : public cherry_kit::desktop_dialog {
public:
  date_dialog(QObject *object = 0);
  virtual ~date_dialog();

  void create_window(const QRectF &window_geometry, const QString &window_title,
                     const QPointF &window_pos);
  cherry_kit::window *dialog_window() const;

  void purge();

private:
  void notify_calendar_value();

private:
  class PrivateDatePicker;
  PrivateDatePicker *const priv;
};

#endif
