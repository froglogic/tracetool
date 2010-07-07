/**********************************************************************
** Copyright (C) 2010 froglogic GmbH.
** All rights reserved.
**
** This file is part of Squish.
**
** Licensees holding a valid Squish License Agreement may use this
** file in accordance with the Squish License Agreement provided with
** the Software.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
** See the LICENSE file in the toplevel directory of this package.
**
** Contact contact@froglogic.com if any conditions of this licensing are
** not clear to you.
**
**********************************************************************/
#include "winstringconv.h"

#include <vector>

#include <windows.h>

using namespace std;

static string ws2mbs( const wstring &s, UINT codepage )
{
    const int size = ::WideCharToMultiByte( codepage, 0, s.c_str(), -1, NULL, 0, 0, NULL );

    vector<char> buf( size );
    ::WideCharToMultiByte( codepage, 0, s.c_str(), -1, &buf[0], size, 0, NULL );

    return string( &buf[0] );
}

static wstring mbs2ws( const string &s, UINT codepage )
{
    const int size = ::MultiByteToWideChar( codepage, 0, s.c_str(), -1, 0, 0 );

    vector<wchar_t> buf( size );
    ::MultiByteToWideChar( codepage, 0, s.c_str(), -1, &buf[0], size );

    return wstring( &buf[0] );
}

namespace Squish
{

string utf16ToANSI( const wstring &s )
{
    return ws2mbs( s, CP_ACP );
}

string utf16ToUTF8( const wstring &s )
{
    return ws2mbs( s, CP_UTF8 );
}

wstring ansiToUTF16( const string &s )
{
    return mbs2ws( s, CP_ACP );
}

wstring utf8ToUTF16( const string &s )
{
    return mbs2ws( s, CP_UTF8 );
}

}

