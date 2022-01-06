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

#include "StyleManager.h"

namespace acss
{
//=============================================================================
CQmlStyleUrlInterceptor::CQmlStyleUrlInterceptor(CStyleManager* StyleManager)
    : m_StyleManager{StyleManager}
{}

//=============================================================================
QUrl CQmlStyleUrlInterceptor::intercept(const QUrl& path, DataType type)
{
    if (type == UrlString && path.scheme() == "icon")
    {
        if (m_StyleManager)
        {
            return QUrl::fromLocalFile(m_StyleManager->currentStyleOutputPath()
                                       + '/' + path.path());
        }
        qWarning() << "AdvancedStylesheet Error: CQmlStyleUrlInterceptor has no "
                      "valid CStyleManager!";
    }
    return path;
}
}  // namespace acss
