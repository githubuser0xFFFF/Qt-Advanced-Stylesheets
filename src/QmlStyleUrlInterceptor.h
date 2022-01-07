//============================================================================
/// \file   QmlStyleUrlInterceptor.h
/// \author Florian Meinicke (florian.meinicke@t-online.de)
/// \date   06.01.2022
/// \brief  Declaration of the CQmlStyleUrlInterceptor class.
//============================================================================
#ifndef ACSS_CQMLSTYLEURLINTERCEPTOR_H
#define ACSS_CQMLSTYLEURLINTERCEPTOR_H

//============================================================================
//                                  INCLUDES
//============================================================================
#include <QQmlAbstractUrlInterceptor>

namespace acss
{
class CStyleManager;

/**
 * @brief The CQmlStyleUrlInterceptor class provides a URL interceptor that can be
 * set on a @c QQmlEngine so that the functionality of ACSS is available in QML as
 * well.
 *
 * For example,
 * @code
 * auto StyleManager = new CStyleManager;
 * //...
 * QQuickWidget Widget;
 * Widget.engine()->setUrlInterceptor(new CQmlStyleUrlInterceptor(StyleManager));
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
 * and turn them into absolute paths (with the help of the @c CStyleManager
 * instance passed in the constructor) that can be understood by QML.
 */
class CQmlStyleUrlInterceptor : public QQmlAbstractUrlInterceptor
{
public:
    /**
     * @brief Constructor
     *
     * @param StyleManager The Style Manager to use for resolving the URLs
     */
    CQmlStyleUrlInterceptor(CStyleManager* StyleManager);

    // implements QQmlAbstractUrlInterceptor ---------------------------------
    QUrl intercept(const QUrl& path, DataType type) override;

private:
    CStyleManager* m_StyleManager;
};

}  // namespace acss

#endif  // ACSS_CQMLSTYLEURLINTERCEPTOR_H
