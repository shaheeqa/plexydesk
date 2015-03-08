#ifndef DIAL_WIDGET_H
#define DIAL_WIDGET_H

#include <plexy.h>
#include <widget.h>
#include <QGraphicsSceneMouseEvent>
#include <plexydesk_ui_exports.h>

namespace UIKit
{
class DECL_UI_KIT_EXPORT DialWidget : public UIKit::Widget
{
  Q_OBJECT
public:
  DialWidget(QGraphicsObject *a_parent_ptr = 0);

  virtual ~DialWidget();

  virtual void setMaxValue(float maxValue);

  virtual float maxValue() const;

  virtual float currentValue() const;

Q_SIGNALS:
  void value(float value);

protected:
  virtual void paint_view(QPainter *painter, const QRectF &rect);
  virtual void dragMoveEvent(QGraphicsSceneDragDropEvent *event);
  virtual void mousePressEvent(QGraphicsSceneMouseEvent *event);
  virtual void mouseReleaseEvent(QGraphicsSceneMouseEvent *event);
  virtual void mouseMoveEvent(QGraphicsSceneMouseEvent *event);

private:
  class PrivateDialWidget;
  PrivateDialWidget *const d;
};
}

#endif // DIAL_WIDGET_H
