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
#include "label.h"

#include <QStyleOptionGraphicsItem>
#include <QPainter>
#include <QFontMetrics>
#include <QFont>
#include <QDebug>
#include <QTextOption>
#include <resource_manager.h>

namespace cherry_kit {

class Label::PrivateLabel {
public:
  PrivateLabel() {}
  ~PrivateLabel() {}

  QSizeF pixelSizeOfText(const QString &txt);

  QSizeF m_label_size;
  QString m_label_string;
  uint m_font_size;
  Qt::Alignment m_alignment;
  bool m_mode;
};

Label::Label(widget *parent) : widget(parent), o_label(new PrivateLabel) {
  o_label->m_font_size = 14;
  o_label->m_alignment = Qt::AlignCenter;
  o_label->m_mode = false;

  setFlag(QGraphicsItem::ItemIsMovable, false);
}

Label::~Label() { delete o_label; }

void Label::set_label(const QString &a_txt) {
  o_label->m_label_string = a_txt;
  update();
}

QString Label::label() const { return o_label->m_label_string; }

QRectF Label::boundingRect() const {
  return QRectF(0.0, 0.0, o_label->m_label_size.width(),
                o_label->m_label_size.height());
}

void Label::set_size(const QSizeF &_asize) { o_label->m_label_size = _asize; }

void Label::set_font_size(uint pixelSize) { o_label->m_font_size = pixelSize; }

QSizeF Label::sizeHint(Qt::SizeHint which, const QSizeF &a_constraint) const {
  return boundingRect().size();
}

void Label::setGeometry(const QRectF &a_rect) {
  prepareGeometryChange();
  setPos(a_rect.topLeft());
  o_label->m_label_size = a_rect.size();

  update();
  widget::setGeometry(a_rect);
}

QRectF Label::contents_bounding_rect() const {
  QFont font;
  font.setFamily(font.defaultFamily());
  font.setPixelSize(o_label->m_font_size);
  QFontMetrics metic(font);
  QRectF rect = metic.boundingRect(o_label->m_label_string);
  rect.setX(0.0);
  rect.setY(0.0);

  return rect;
}

void Label::set_label_style(const QColor &a_backgroundColor,
                            const QColor &a_textColor) {
  update();
}

void Label::set_highlight(bool a_mode) {
  o_label->m_mode = a_mode;
  update();
}

int Label::alignment() { return o_label->m_alignment; }

void Label::set_alignment(int a_alignment) {
  o_label->m_alignment = (Qt::Alignment)a_alignment;
}

void Label::paint_view(QPainter *a_painter_ptr, const QRectF &a_rect) {
  style_data feature;

  feature.text_data = o_label->m_label_string;
  feature.geometry = a_rect;
  if (!o_label->m_mode)
    feature.render_state = style_data::kRenderElement;
  else
    feature.render_state = style_data::kRenderRaised;

  QTextOption text_option;
  text_option.setAlignment(o_label->m_alignment);
  feature.text_options = text_option;
  feature.attributes["font_size"] = o_label->m_font_size;

  if (cherry_kit::resource_manager::style()) {
    cherry_kit::resource_manager::style()->draw("label", feature,
                                                a_painter_ptr);
  }
}

QSizeF Label::PrivateLabel::pixelSizeOfText(const QString &txt) {
  QFont font("Times", 20, QFont::Bold);
  QFontMetrics metic(font);
  QRectF rect = metic.boundingRect(txt);

  return rect.size();
}
}
