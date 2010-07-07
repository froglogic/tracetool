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
#ifndef WINSTRINGCCONV_H
#define WINSTRINGCCONV_H

#include <string>

namespace Squish
{
    std::string utf16ToANSI( const std::wstring &s );
    std::string utf16ToUTF8( const std::wstring &s );

    std::wstring utf8ToUTF16( const std::string &s );
    std::wstring ansiToUTF16( const std::string &s );
}

#endif // !defined(WINSTRINGCCONV_H)
