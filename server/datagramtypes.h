/**********************************************************************
** Copyright (C) 2010 froglogic GmbH.
** All rights reserved.
**********************************************************************/

#ifndef TRACE_DATAGRAMTYPES_H
#define TRACE_DATAGRAMTYPES_H

#define MagicServerProtocolCookie (quint32)0x22021990

enum ServerDatagramType {
    TraceFileNameDatagram,
    TraceEntryDatagram,
    ProcessShutdownEventDatagram,
    DatabaseNukeDatagram,
    DatabaseNukeFinishedDatagram
};

#endif // !defined(TRACE_DATAGRAMTYPES_H)

