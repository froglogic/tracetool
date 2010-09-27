/**********************************************************************
** Copyright (C) 2010 froglogic GmbH.
** All rights reserved.
**********************************************************************/

#ifndef APPLICATIONTABLE_H
#define APPLICATIONTABLE_H

#include "../server/server.h"

#include <QDateTime>
#include <QList>
#include <QMap>
#include <QString>
#include <QTableWidget>

class ApplicationTable : public QTableWidget
{
    Q_OBJECT
public:
    ApplicationTable();

    void setApplications( const QList<TracedApplicationInfo> &apps );

public slots:
    void handleNewTraceEntry( const TraceEntry &entry );
    void handleProcessShutdown( const ProcessShutdownEvent &ev );

private:
    struct TracedApplicationId {
        QString name;
        QDateTime startTime;
        bool operator<( const TracedApplicationId &other ) const {
            if ( name < other.name )
                return true;
            return name == other.name
                   && startTime < other.startTime;
        }
    };

    QTableWidgetItem *insertEntry(int row,
            unsigned int pid,
            const QString &name,
            const QDateTime &startTime,
            const QDateTime &endTime );
    void setTimesForApplication(QTableWidgetItem *nameItem,
                                const QDateTime &startTime,
                                const QDateTime &endTime);

    QMap<TracedApplicationId, QTableWidgetItem *> m_items;
};

#endif // !defined(APPLICATIONTABLE_H)
