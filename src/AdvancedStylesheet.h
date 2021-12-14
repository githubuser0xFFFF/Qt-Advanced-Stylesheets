#ifndef AdvancedStylesheetH
#define AdvancedStylesheetH
//============================================================================
/// \file   AdvancedStylesheet.h
/// \author Uwe Kindler
/// \date   13.12.2021
/// \brief  Declaration of CAdvancedStylesheet class
//============================================================================

//============================================================================
//                                   INCLUDES
//============================================================================
#include <QString>
#include <QVector>
#include <QPair>

namespace acss
{
struct AdvancedStylesheetPrivate;
using QStringPair = QPair<QString, QString>;

/**
 * Encapsulates all information about a single stylesheet
 */
class CAdvancedStylesheet
{
private:
	AdvancedStylesheetPrivate* d; ///< private data (pimpl)
	friend struct AdvancedStylesheetPrivate;

public:
	enum eResourceState
	{
		ResourceDisabled = 0,
		ResourceNormal = 1
	};

	/**
	 * Default Constructor
	 */
	CAdvancedStylesheet(const QString& StylesheetFolder);

	/**
	 * Virtual Destructor
	 */
	virtual ~CAdvancedStylesheet();

	/**
	 * Returns the folder with the resource template
	 */
	QString resourcesTemplatesFolder() const;

	/**
	 * Returns the folders that contains all themes
	 */
	QString themesFolder() const;

	/**
	 * Returns the fonts folder for the theme fonts
	 */
	QString fontsFolder() const;

	/**
	 * Returns a vector with a list of colors that need to get replaced in
	 * the resource template files
	 */
	QVector<QStringPair> resourceColorReplaceList(eResourceState State) const;

	/**
	 * The output dir path where the generated files will get stored
	 */
	QString outputDirPath() const;

	/**
	 * Sets the output directory path where the generated theme will be stored
	 */
	void setOutputDirPath(const QString& Path);

	/**
	 * Returns the theme color for the given ColorId.
	 * For example themeColor("primaryColor") may return "#ac2300".
	 * Returns an empty string, if the given ColorId does not exist
	 */
	QString themeVariable(const QString& VariableId) const;

	/**
	 * You can use this function to set or overwrite a theme variable
	 */
	void setThemeVariabe(const QString& VariableId, const QString& Value);

	/**
	 * Sets the theme to use.
	 * Returns true on success and false on error
	 */
	bool setTheme(const QString& Theme);

	/**
	 * Returns the processed theme stylesheet
	 */
	QString styleSheet() const;
}; // class AdvancedStylesheet
}
 // namespace namespace_name
//-----------------------------------------------------------------------------
#endif // AdvancedStylesheetH

