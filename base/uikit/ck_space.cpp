﻿#include "ck_space.h"

#include <QDebug>
#include <QDropEvent>
#include <QGraphicsItem>
#include <ck_disk_engine.h>
#include <ck_extension_manager.h>
#include <ck_sync_object.h>
#include <QGraphicsScene>
#include <QWeakPointer>
#include <ck_widget.h>
#include <ck_data_sync.h>

#include "ck_window.h"
#include "ck_workspace.h"

namespace cherry_kit {
#define kMaximumZOrder 10000
#define kMinimumZOrder 100
#define kMediumZOrder 5000

typedef std::function<void(space::ViewportNotificationType,
                           const cherry_kit::ui_task_data_t &, const space *)>
NotifyFunc;
class space::PrivateSpace {
public:
  PrivateSpace() : m_surface(0) {}
  ~PrivateSpace();

  QString session_name_of_space();
  void init_session_registry(space *space);

  QString session_controller_name(const QString &controllerName);

  void controller_action_list(const space *space, desktop_controller_ref ptr);

public:
  int m_id;
  QString m_name;
  QRectF m_geometry;
  workspace *m_workspace;
  QGraphicsScene *m_native_scene;
  QList<cherry_kit::desktop_dialog_ref> m_activity_list;
  std::vector<cherry_kit::window *> m_window_list;
  QMap<QString, desktop_controller_ref> m_current_controller_list;
  QList<desktop_controller_ref> m_controller_list;

  QList<NotifyFunc> m_notify_chain;

  // experiment
  unsigned char *m_surface;
};

space::space() : o_space(new PrivateSpace) {
  o_space->m_workspace = 0;
  o_space->m_native_scene = 0;
}

space::~space() {
  clear();
  delete o_space;
}

void space::add_controller(const QString &a_name) {
  if (o_space->m_current_controller_list.keys().contains(a_name)) {
    return;
  }

  QSharedPointer<desktop_controller_interface> controllerPtr =
      (cherry_kit::extension_manager::instance()->controller(a_name));

  if (!controllerPtr) {
    qWarning() << Q_FUNC_INFO << "Error loading extension" << a_name;
    return;
  }

  o_space->m_current_controller_list[a_name] = controllerPtr;

  controllerPtr->set_viewport(this);
  controllerPtr->set_controller_name(a_name);

  controllerPtr->init();
  controllerPtr->set_view_rect(geometry());

  o_space->controller_action_list(this, controllerPtr);

  register_controller(a_name);

  ui_task_data_t argument;
  argument["controller"] = a_name.toStdString();

  foreach(NotifyFunc l_notify_handler, o_space->m_notify_chain) {
    if (l_notify_handler)
      l_notify_handler(space::kControllerAddedNotification, argument, this);
  }
}

cherry_kit::desktop_dialog_ref
space::open_desktop_dialog(const QString &a_activity, const QString &a_title,
                           const QPointF &a_pos, const QRectF &a_rect,
                           const QVariantMap &a_data_map) {
  cherry_kit::desktop_dialog_ref intent =
      cherry_kit::extension_manager::instance()->activity(a_activity);

  if (!intent) {
    qWarning() << Q_FUNC_INFO << "No such Activity: " << a_activity;
    return cherry_kit::desktop_dialog_ref();
  }

  intent->set_activity_attribute("data", QVariant(a_data_map));

  intent->create_window(a_rect, a_title, QPointF(a_pos.x(), a_pos.y()));

  if (intent->dialog_window()) {
    intent->dialog_window()->set_window_title(a_title);
    intent->dialog_window()->set_window_viewport(this);
    intent->dialog_window()->setPos(a_pos);
  }

  add_activity(intent);

  return intent;
}

void space::update_session_value(const QString &a_controller_name,
                                 const QString &a_key, const QString &a_value) {
  cherry_kit::data_sync *sync = new cherry_kit::data_sync(
      o_space->session_controller_name(a_controller_name).toStdString());
  cherry_kit::disk_engine *engine = new cherry_kit::disk_engine();

  sync->set_sync_engine(engine);

  sync->on_object_found([&](cherry_kit::sync_object &a_object,
                            const std::string &a_app_name, bool a_found) {
    if (!a_found) {
      cherry_kit::sync_object obj;
      obj.set_name("AppSession");
      obj.set_property("name", o_space->session_controller_name(
                                            a_controller_name).toStdString());

      sync->add_object(obj);
    }

    controller(a_controller_name)->submit_session_data(&a_object);
    a_object.sync();
  });

  sync->find("AppSession", "", "");

  delete sync;
}

void space::add_activity(cherry_kit::desktop_dialog_ref a_activity_ptr) {
  if (!a_activity_ptr || !a_activity_ptr->dialog_window()) {
    return;
  }

  a_activity_ptr->on_discarded([&](const desktop_dialog *a_activity) {
    on_activity_finished(a_activity);
  });

  a_activity_ptr->dialog_window()->on_window_discarded([=](window *a_window) {
    a_activity_ptr->discard_activity();
  });

  if (a_activity_ptr->dialog_window() && o_space->m_native_scene) {
    if (o_space->m_activity_list.contains(a_activity_ptr)) {
      qWarning() << Q_FUNC_INFO << "Space already contains the activity";
      return;
    }

    o_space->m_activity_list << a_activity_ptr;

    a_activity_ptr->set_viewport(this);

    insert_window_to_view(a_activity_ptr->dialog_window());
  }
}

void space::insert_window_to_view(window *a_window) {
  if (!a_window) {
    return;
  }

  if (!o_space->m_native_scene) {
    qWarning() << Q_FUNC_INFO << "Scene not Set";
    return;
  }

  QPointF _center_of_space_location = this->geometry().center();
  QPoint _widget_location;

  _widget_location.setX(_center_of_space_location.x() -
                        a_window->boundingRect().width() / 2);
  _widget_location.setY(_center_of_space_location.y() -
                        a_window->boundingRect().height() / 2);

  o_space->m_native_scene->addItem(a_window);
  if (a_window->window_type() == window::kApplicationWindow)
    a_window->setZValue(kMaximumZOrder);

  if (a_window->controller()) {
    a_window->controller()->set_view_rect(o_space->m_geometry);
  }

  o_space->m_window_list.push_back(a_window);

  a_window->on_update([this](const widget *window) {
    // qDebug() << Q_FUNC_INFO << "Got Update Request";
  });

  a_window->on_window_closed([this](window *window) {
    qDebug() << Q_FUNC_INFO << "Request Window Removal from Space";
    remove_window_from_view(window);
  });

  a_window->on_window_focused([this](window *a_window) {
    if (!a_window) {
      return;
    }

    std::for_each(std::begin(o_space->m_window_list),
                  std::end(o_space->m_window_list), [&](window *a_win) {
      if (a_win->window_type() == window::kFramelessWindow) {
        a_win->setZValue(kMinimumZOrder);
      }

      if (a_win->window_type() == window::kPanelWindow) {
        a_win->setZValue(kMaximumZOrder + 2);
      }

      if (a_win->window_type() == window::kPopupWindow) {
        a_win->setZValue(kMaximumZOrder + 1);
        if (a_win != a_window) {
          a_win->hide();
          a_win->removeFocus();
          // if window gets deleted we shoudln't proceed further.
          return;
        }
      }

      if (a_win->window_type() == window::kApplicationWindow) {
        if (a_win->zValue() == kMaximumZOrder && a_win != a_window)
          a_win->setZValue((kMaximumZOrder - 1));
      }
    });

    if (a_window->window_type() == window::kApplicationWindow)
      a_window->setZValue(kMaximumZOrder);
  });

  a_window->raise();
  a_window->show();
}

void space::remove_window_from_view(window *a_window) {
  if (!o_space->m_native_scene) {
    return;
  }

  if (!a_window) {
    return;
  }

  if (a_window->controller()) {
    if (a_window->controller()->remove_widget(a_window)) {
      return;
    }
  }

  o_space->m_native_scene->removeItem(a_window);

  qDebug() << Q_FUNC_INFO << "Before :" << o_space->m_window_list.size();
  o_space->m_window_list.erase(std::remove(o_space->m_window_list.begin(),
                                           o_space->m_window_list.end(),
                                           a_window),
                               o_space->m_window_list.end());
  qDebug() << Q_FUNC_INFO << "After:" << o_space->m_window_list.size();

  a_window->discard();
}

void space::on_viewport_event_notify(
    std::function<void(ViewportNotificationType, const ui_task_data_t &a_data,
                       const space *)> a_notify_handler) {
  o_space->m_notify_chain.append(a_notify_handler);
}

void space::on_activity_finished(const desktop_dialog *a_activity) {
  if (a_activity) {
    int i = 0;
    foreach(desktop_dialog_ref _activity, o_space->m_activity_list) {
      // todo : enable runtime identification of activities.
      if (_activity.data() == a_activity) {
        _activity.clear();
        qDebug() << Q_FUNC_INFO
                 << "Before :" << o_space->m_activity_list.count();
        o_space->m_activity_list.removeAt(i);
        o_space->m_activity_list.removeAll(_activity);
        qDebug() << Q_FUNC_INFO
                 << "After :" << o_space->m_activity_list.count();
      }
      i++;
    }
  }
}

int qtz_read_color_value_at_pos(GraphicsSurface *surface, int width, int x,
                                int y, int comp) {

  if (!(*surface))
    return -1;

  return (*surface)[((y * width + x) * 4) + comp];
}

void qtz_set_color_value_at_pos(GraphicsSurface *surface, int width, int x,
                                int y, int red, int green, int blue,
                                int alpha) {

  if (!(*surface))
    return;

  (*surface)[((y * width + x) * 4)] = red;
  (*surface)[((y * width + x) * 4) + 1] = green;
  (*surface)[((y * width + x) * 4) + 2] = blue;
  (*surface)[((y * width + x) * 4) + 3] = alpha;
}

void space::draw() {
  // todo: Move this expensive loop out of here.
  std::sort(
      std::begin(o_space->m_window_list), std::end(o_space->m_window_list),
      [&](window *a_a, window *a_b) { return a_a->zValue() < a_b->zValue(); });

  std::for_each(std::begin(o_space->m_window_list),
                std::end(o_space->m_window_list), [&](window *a_win) {

    if (!a_win)
      return;

    if (!a_win->isVisible())
      return;

    a_win->draw();

    qDebug() << Q_FUNC_INFO << " Z Index" << a_win->zValue()
             << " TITLE : " << a_win->window_title()
             << " Geometry: " << a_win->geometry()
             << " Bounding Rect : " << a_win->geometry()
             << " (x : " << a_win->x() << " (y :)  " << a_win->y();

    GraphicsSurface *window_surface = a_win->surface();

    if (!(*window_surface))
      return;

    for (int w = 0; w < geometry().width(); w++) {
      for (int h = 0; h < geometry().height(); h++) {
        // qDebug() << Q_FUNC_INFO << "(" << w << "," << h << ")";
        if (w >= a_win->geometry().width())
          continue;
        if (h >= a_win->geometry().height())
          continue;

        int red = qtz_read_color_value_at_pos(
            window_surface, a_win->geometry().width(), w, h, 0);
        int green = qtz_read_color_value_at_pos(
            window_surface, a_win->geometry().width(), w, h, 1);
        int blue = qtz_read_color_value_at_pos(
            window_surface, a_win->geometry().width(), w, h, 2);
        int alpha = qtz_read_color_value_at_pos(
            window_surface, a_win->geometry().width(), w, h, 3);

        if (alpha < 255)
          continue;

        qtz_set_color_value_at_pos(&o_space->m_surface, geometry().width(),
                                   (a_win->x() + w), (a_win->y() + h), red,
                                   green, blue, alpha);
      }
    }
  });
}

GraphicsSurface *space::surface() { return &o_space->m_surface; }

void space::save_controller_to_session(const QString &a_controller_name) {
  cherry_kit::data_sync *sync =
      new cherry_kit::data_sync(o_space->session_name_of_space().toStdString());
  cherry_kit::disk_engine *engine = new cherry_kit::disk_engine();

  sync->set_sync_engine(engine);

  sync->on_object_found([&](cherry_kit::sync_object &a_object,
                            const std::string &a_app_name, bool a_found) {
    if (!a_found) {
      cherry_kit::sync_object obj;
      obj.set_name("Controller");
      obj.set_property("name", a_controller_name.toStdString());

      sync->add_object(obj);
    }
  });

  sync->find("Controller", "name", a_controller_name.toStdString());

  delete sync;
}

void
space::revoke_controller_session_attributes(const QString &a_controller_name) {
  cherry_kit::data_sync *sync = new cherry_kit::data_sync(
      o_space->session_controller_name(a_controller_name).toStdString());
  cherry_kit::disk_engine *engine = new cherry_kit::disk_engine();

  sync->set_sync_engine(engine);

  sync->on_object_found([&](cherry_kit::sync_object &a_object,
                            const std::string &a_app_name, bool a_found) {
    qDebug() << Q_FUNC_INFO << "Restore Session For Controllers"
             << a_controller_name;

    a_object.set_property(
        "name",
        o_space->session_controller_name(a_controller_name).toStdString());
    if (!a_found) {
      a_object.set_name("AppSession");
      sync->add_object(a_object);
    }

    if (controller(a_controller_name)) {
      controller(a_controller_name)->session_data_ready(a_object);
    }
  });

  sync->find("AppSession", "", "");

  delete sync;
}

void space::register_controller(const QString &a_controller_name) {
  save_controller_to_session(a_controller_name);
  revoke_controller_session_attributes(a_controller_name);
}

void space::PrivateSpace::controller_action_list(const space *space,
                                                 desktop_controller_ref ptr) {}

void space::PrivateSpace::init_session_registry(space *space) {
  cherry_kit::data_sync *sync =
      new cherry_kit::data_sync(session_name_of_space().toStdString());
  cherry_kit::disk_engine *engine = new cherry_kit::disk_engine();
  sync->set_sync_engine(engine);

  sync->on_object_found([&](cherry_kit::sync_object &a_object,
                            const std::string &a_app_name, bool a_found) {
    if (a_found) {
      QString _current_controller_name = a_object.property("name").c_str();

      desktop_controller_ref _controller_ptr =
          space->controller(_current_controller_name);

      if (!_controller_ptr) {
        space->add_controller(_current_controller_name);
      }
    }
  });

  sync->find("Controller", "", "");

  delete sync;
}

space::PrivateSpace::~PrivateSpace() {
  if (m_surface)
    free(m_surface);

  m_current_controller_list.clear();
}

QString space::PrivateSpace::session_name_of_space() {
  return QString("%1_%2_Space_%3")
      .arg(QString::fromStdString(m_workspace->workspace_instance_name()))
      .arg(m_name)
      .arg(m_id);
}

QString
space::PrivateSpace::session_controller_name(const QString &controllerName) {
  return QString("%1_Controller_%2").arg(session_name_of_space()).arg(
      controllerName);
}

QString space::session_name() const { return o_space->session_name_of_space(); }

QString space::session_name_for_controller(const QString &a_controller_name) {
  return o_space->session_controller_name(a_controller_name);
}

void space::clear() {
  int i = 0;
  foreach(desktop_dialog_ref _activity, o_space->m_activity_list) {
    qDebug() << Q_FUNC_INFO << "Remove Activity: ";
    if (_activity) {
      o_space->m_activity_list.removeAt(i);
    }
    i++;
  }

  // delete owner widgets
  for (window *_widget : o_space->m_window_list) {
    if (_widget) {
      if (o_space->m_native_scene->items().contains(_widget)) {
        o_space->m_native_scene->removeItem(_widget);
        _widget->discard();
      }
      qDebug() << Q_FUNC_INFO << "Widget Deleted: OK";
    }
  }

  o_space->m_window_list.clear();
  // remove spaces which belongs to the space.
  foreach(const QString & _key, o_space->m_current_controller_list.keys()) {
    qDebug() << Q_FUNC_INFO << _key;
    desktop_controller_ref _controller_ptr =
        o_space->m_current_controller_list[_key];
    _controller_ptr->prepare_removal();
    qDebug() << Q_FUNC_INFO
             << "Before Removal:" << o_space->m_current_controller_list.count();
    o_space->m_current_controller_list.remove(_key);
    qDebug() << Q_FUNC_INFO
             << "After Removal:" << o_space->m_current_controller_list.count();
  }

  o_space->m_current_controller_list.clear();
}

QPointF space::cursor_pos() const {
  if (!o_space->m_workspace) {
    return QCursor::pos();
  }

  QGraphicsView *_view_parent =
      qobject_cast<QGraphicsView *>(o_space->m_workspace);

  if (!_view_parent) {
    return QCursor::pos();
  }

  return _view_parent->mapToScene(QCursor::pos());
}

QPointF space::center(const QRectF &a_view_geometry,
                      const QRectF a_window_geometry,
                      const ViewportLocation &a_location) const {
  QPointF _rv;
  float _x_location;
  float _y_location;

  switch (a_location) {
  case kCenterOnViewport:
    _y_location = (geometry().height() / 2) - (a_view_geometry.height() / 2);
    _x_location = (geometry().width() / 2) - (a_view_geometry.width() / 2);
    break;
  case kCenterOnViewportLeft:
    _y_location = (geometry().height() / 2) - (a_view_geometry.height() / 2);
    _x_location = (geometry().topLeft().x());
    break;
  case kCenterOnViewportRight:
    _y_location = (geometry().height() / 2) - (a_view_geometry.height() / 2);
    _x_location = (geometry().width() - (a_view_geometry.width()));
    break;
  case kCenterOnViewportTop:
    _y_location = (0.0);
    _x_location = ((geometry().width() / 2) - (a_view_geometry.width() / 2));
    break;
  case kCenterOnViewportBottom:
    _y_location = (geometry().height() - (a_view_geometry.height()));
    _x_location = ((geometry().width() / 2) - (a_view_geometry.width() / 2));
    break;
  case kCenterOnWindow:
    _y_location = (a_window_geometry.height() / 2) -
                  (a_view_geometry.height() / 2) + a_window_geometry.y();
    _x_location = (a_window_geometry.width() / 2) -
                  (a_view_geometry.width() / 2) + a_window_geometry.x();
    _rv.setX(_x_location);
    _rv.setY(_y_location);
    break;
  default:
    qWarning() << Q_FUNC_INFO << "Error : Unknown Viewprot Location Type:";
  }

  if (a_location != kCenterOnWindow) {
    _rv.setY(geometry().y() + _y_location);
    _rv.setX(_x_location);
  }

  return _rv;
}

float space::scaled_width(float a_value) {
  if (!owner_workspace())
    return a_value;

  float scale_factor = owner_workspace()->desktop_horizontal_scale_factor();

  if (scale_factor < 1.0f) {
    a_value = a_value / scale_factor;
  } else {
    a_value = scale_factor * a_value;
  }

  return a_value;
}

float space::scaled_height(float a_value) {
  if (!owner_workspace())
    return a_value;

  float scale_factor = owner_workspace()->desktop_verticle_scale_factor();

  if (scale_factor < 1.0f) {
    a_value = a_value / scale_factor;
  } else {
    a_value = scale_factor * a_value;
  }

  return a_value;
}

desktop_controller_ref space::controller(const QString &a_name) {
  if (!o_space->m_current_controller_list.keys().contains(a_name)) {
    return desktop_controller_ref();
  }

  return o_space->m_current_controller_list[a_name];
}

QStringList space::current_controller_list() const {
  return o_space->m_current_controller_list.keys();
}

void space::set_name(const QString &a_name) { o_space->m_name = a_name; }

QString space::name() const { return o_space->m_name; }

void space::set_id(int a_id) { o_space->m_id = a_id; }

int space::id() const { return o_space->m_id; }

void space::reset_focus() {
  std::for_each(std::begin(o_space->m_window_list),
                std::end(o_space->m_window_list), [&](window *a_win) {
    if (!a_win)
      return;

    if (a_win->window_type() == window::kPopupWindow) {
      a_win->hide();
      a_win->removeFocus();
    }
  });
}

void space::setGeometry(const QRectF &a_geometry) {
  o_space->m_geometry = a_geometry;

  if (!o_space->m_surface) {
    o_space->m_surface =
        (unsigned char *)malloc(4 * a_geometry.width() * a_geometry.height());
    memset(o_space->m_surface, 0, 4 * a_geometry.width() * a_geometry.height());
  } else {
    o_space->m_surface = (unsigned char *)realloc(
        o_space->m_surface, (4 * a_geometry.width() * a_geometry.height()));
  }

  foreach(desktop_dialog_ref _activity, o_space->m_activity_list) {
    if (_activity && _activity->dialog_window()) {
      QPointF _activity_pos = _activity->dialog_window()->pos();

      if (_activity_pos.y() > a_geometry.height()) {
        _activity_pos.setY(_activity_pos.y() - a_geometry.height());
        _activity->dialog_window()->setPos(_activity_pos);
      }
    }
  }

  foreach(const QString & _key, o_space->m_current_controller_list.keys()) {
    desktop_controller_ref _controller_ptr =
        o_space->m_current_controller_list[_key];
    _controller_ptr->set_view_rect(o_space->m_geometry);
  }

  ui_task_data_t argument;

  foreach(NotifyFunc l_notify_handler, o_space->m_notify_chain) {
    if (l_notify_handler)
      l_notify_handler(space::kGeometryChangedNotification, argument, this);
  }
}

workspace *space::owner_workspace() { return o_space->m_workspace; }

void space::set_workspace(workspace *a_workspace_ptr) {
  o_space->m_workspace = a_workspace_ptr;
}

void space::restore_session() { o_space->init_session_registry(this); }

void space::set_qt_graphics_scene(QGraphicsScene *a_qt_graphics_scene_ptr) {
  o_space->m_native_scene = a_qt_graphics_scene_ptr;
}

QRectF space::geometry() const { return o_space->m_geometry; }

void space::drop_event_handler(QDropEvent *event,
                               const QPointF &local_event_location) {
  if (o_space->m_native_scene) {
    QList<QGraphicsItem *> items =
        o_space->m_native_scene->items(local_event_location);

    Q_FOREACH(QGraphicsItem * item, items) {
      QGraphicsObject *itemObject = item->toGraphicsObject();

      if (!itemObject) {
        continue;
      }

      widget *ck_widget = qobject_cast<widget *>(itemObject);

      if (!ck_widget || !ck_widget->controller()) {
        qDebug() << Q_FUNC_INFO << "Not a Valid Item";
        continue;
      }

      ck_widget->controller()->handle_drop_event(ck_widget, event);
      return;
    }
  }
}
}
