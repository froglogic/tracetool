/* tracetool - a framework for tracing the execution of C++ programs
 * Copyright 2010-2016 froglogic GmbH
 *
 * This file is part of tracetool.
 *
 * tracetool is free software: you can redistribute it and/or modify it under
 * the terms of the GNU Lesser General Public License as published by the Free
 * Software Foundation, either version 3 of the License, or (at your option)
 * any later version.
 *
 * tracetool is distributed in the hope that it will be useful, but WITHOUT ANY
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License for
 * more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with tracetool.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "fixedheaderview.h"

#include <QEvent>

FixedHeaderView::FixedHeaderView(int digits,
                                 Qt::Orientation orientation,
                                 QWidget *parent)
    : QHeaderView(orientation, parent),
      m_digits(digits)

{
    calcSize();
}

// constant time compared to the base class implementation
QSize
FixedHeaderView::sectionSizeFromContents(int /*logicalIndex*/) const
{
    return m_sectionSize;
}

void FixedHeaderView::changeEvent(QEvent *event)
{
    QHeaderView::changeEvent(event);
    if (event->type() == QEvent::FontChange) {
        calcSize();
    }
}

void FixedHeaderView::calcSize()
{
    // ignore actual content, icons and sort indicators
    QString label;
    label.fill('8', m_digits);
    QStyleOptionHeader opt;
    initStyleOption(&opt);
    opt.section = 0;
    QFont fnt = font();
    fnt.setBold(true);
    opt.fontMetrics = QFontMetrics(fnt);
    opt.text = label;
    m_sectionSize =
        style()->sizeFromContents(QStyle::CT_HeaderSection,
                                  &opt, QSize(), this);

    setDefaultSectionSize( this->orientation() == Qt::Horizontal ? m_sectionSize.width()
                                                                 : m_sectionSize.height() );
}
