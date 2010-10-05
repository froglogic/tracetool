/**********************************************************************
** Copyright (C) 2010 froglogic GmbH.
** All rights reserved.
**********************************************************************/

#ifndef FIXEDHEADERVIEW_H
#define FIXEDHEADERVIEW_H

#include <QHeaderView>

class FixedHeaderView : public QHeaderView
{
    Q_OBJECT
public:
    FixedHeaderView(int digits,
                    Qt::Orientation orientation,
                    QWidget * parent = 0);

protected:
    QSize sectionSizeFromContents(int logicalIndex) const;
    void changeEvent(QEvent *event);

private:
    void calcSize();

    int m_digits;
    QSize m_sectionSize;
};

#endif

