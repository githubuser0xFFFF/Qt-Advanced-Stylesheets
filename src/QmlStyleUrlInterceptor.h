#ifndef ACSS_CQMLSTYLEURLINTERCEPTOR_H
#define ACSS_CQMLSTYLEURLINTERCEPTOR_H
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
class CQmlStyleUrlInterceptor : public QQmlAbstractUrlInterceptor
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
