/* tracetool - a framework for tracing the execution of C++ programs
 * Copyright 2011-2016 froglogic GmbH
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

