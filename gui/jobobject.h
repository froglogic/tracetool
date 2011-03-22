/**********************************************************************
** Copyright (C) 2011 froglogic GmbH.
** All rights reserved.
**********************************************************************/
#ifndef JOBOBJECT_H
#define JOBOBJECT_H

#include <windows.h>

class JobObject
{
public:
    JobObject();
    ~JobObject();

    void assignProcess( HANDLE process );

private:
    JobObject( const JobObject &rhs ); // disabled
    void operator=( const JobObject &rhs ); // disabled

    HANDLE m_job;
};

#endif // !defined(JOBOBJECT_H)

