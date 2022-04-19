#ifndef ACSS_CQMLSTYLEURLINTERCEPTOR_H
#define ACSS_CQMLSTYLEURLINTERCEPTOR_H
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
/// \file   QmlStyleUrlInterceptor.h
/// \author Florian Meinicke (florian.meinicke@t-online.de)
/// \date   06.01.2022
/// \brief  Declaration of the CQmlStyleUrlInterceptor class.
//============================================================================


//============================================================================
//                                  INCLUDES
//============================================================================
#include <QQmlAbstractUrlInterceptor>

#include "acss_globals.h"

namespace acss
{
class QtAdvancedStylesheet;

/**
 * @brief The CQmlStyleUrlInterceptor class provides a URL interceptor that can be
 * set on a @c QQmlEngine so that the functionality of ACSS is available in QML as
 * well.
 *
 * For example,
 * @code
 * auto AdvancedStylesheet = new QtAdvancedStylesheet;
 * //...
 * QQuickWidget Widget;
 * Widget.engine()->setUrlInterceptor(new CQmlStyleUrlInterceptor(AdvancedStylesheet));
 * @endcode
 * Now you can use the "icon:" prefix in QML wherever you want to set an icon from
 * the current style:
 * @code
 * import QtQuickControls 2.15
 *
 * Item {
 *   ToolButton {
 *     id: button
 *     icon.source: "icon:/myicon.svg"
 *   }
 * }
 * @endcode
 * The @c CQmlStyleUrlInterceptor will intercept all URLs with the "icon:" prefix
 * and turn them into absolute paths (with the help of the @c QtAdvancedStylesheet
 * instance passed in the constructor) that can be understood by QML.
 */
class ACSS_EXPORT CQmlStyleUrlInterceptor : public QQmlAbstractUrlInterceptor
{
public:
    /**
     * @brief Constructor
     *
     * @param StyleManager The Style Manager to use for resolving the URLs
     */
    CQmlStyleUrlInterceptor(QtAdvancedStylesheet* AdvancedStylesheet);

    // implements QQmlAbstractUrlInterceptor ---------------------------------
    QUrl intercept(const QUrl& path, DataType type) override;

private:
    QtAdvancedStylesheet* m_AdvancedStylesheet;
};

}  // namespace acss

#endif  // ACSS_CQMLSTYLEURLINTERCEPTOR_H
