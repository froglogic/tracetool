/**********************************************************************
** Copyright (C) 2010 froglogic GmbH.
** All rights reserved.
**********************************************************************/

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
}
