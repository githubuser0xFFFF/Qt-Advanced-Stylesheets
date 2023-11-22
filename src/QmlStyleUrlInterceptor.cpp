/*******************************************************************************
** Qt Advanced Stylesheets
** Copyright (C) 2022 Uwe Kindler
**
** This library is free software; you can redistribute it and/or
** modify it under the terms of the GNU Lesser General Public
** License as published by the Free Software Foundation; either
** version 2.1 of the License, or (at your option) any later version.
**
** This library is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
** Lesser General Public License for more details.
**
** You should have received a copy of the GNU Lesser General Public
** License along with this library; If not, see <http://www.gnu.org/licenses/>.
******************************************************************************/


//============================================================================
/// \file   QmlStyleUrlInterceptor.cpp
/// \author Florian Meinicke (florian.meinicke@t-online.de)
/// \date   06.01.2022
/// \brief  Implementation of the CQmlStyleUrlInterceptor class.
//============================================================================

//============================================================================
//                                  INCLUDES
//============================================================================
#include "QmlStyleUrlInterceptor.h"

#include <QDebug>
#include <QUrl>

#include "QtAdvancedStylesheet.h"

namespace acss
{
//=============================================================================
CQmlStyleUrlInterceptor::CQmlStyleUrlInterceptor(QtAdvancedStylesheet* AdvancedStylesheet)
    : m_AdvancedStylesheet{AdvancedStylesheet}
{}

//=============================================================================
QUrl CQmlStyleUrlInterceptor::intercept(const QUrl& path, DataType type)
{
    if (type == UrlString && path.scheme() == "icon")
    {
        if (m_AdvancedStylesheet)
        {
            return QUrl::fromLocalFile(m_AdvancedStylesheet->currentStyleOutputPath()
                                       + '/' + path.path());
        }
        qWarning() << "AdvancedStylesheet Error: CQmlStyleUrlInterceptor has no "
                      "valid CStyleManager!";
    }
    return path;
}
}  // namespace acss
