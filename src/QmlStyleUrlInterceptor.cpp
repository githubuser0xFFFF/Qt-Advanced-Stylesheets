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
