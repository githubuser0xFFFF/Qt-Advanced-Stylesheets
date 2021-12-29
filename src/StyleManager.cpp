//============================================================================
/// \file   AdvancedStylesheet.cpp
/// \author Uwe Kindler
/// \date   13.12.2021
/// \brief  Implementation of CAdvancedStylesheet class
//============================================================================

//============================================================================
//                                   INCLUDES
//============================================================================
#include <StyleManager.h>

#include <iostream>

#include <QMap>
#include <QXmlStreamReader>
#include <QFile>
#include <QDebug>
#include <QDir>
#include <QRegularExpression>
#include <QFontDatabase>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QIcon>
#include <QApplication>
#include <QPalette>
#include <QStyle>

namespace acss
{
struct PaletteColorEntry
{
	QPalette::ColorGroup Group;
	QPalette::ColorRole Role;
	QString ColorVariable;

	PaletteColorEntry(QPalette::ColorGroup group = QPalette::Active,
		QPalette::ColorRole role = QPalette::NoRole,
		const QString& variable = QString())
		: Group(group), Role(role), ColorVariable(variable) {}

	bool isValid() const
	{
		return !ColorVariable.isEmpty() && Role != QPalette::NoRole;
	}
};

/**
 * Converts a color role string into a color role enum
 */
static QPalette::ColorRole colorRoleFromString(const QString& Text)
{
	static QMap<QString, QPalette::ColorRole> ColorRoleMap =
		{{"WindowText", QPalette::WindowText},
		 {"Button", QPalette::Button},
		 {"Light", QPalette::Light},
		 {"Midlight", QPalette::Midlight},
		 {"Dark", QPalette::Dark},
		 {"Mid", QPalette::Mid},
		 {"Text", QPalette::Text},
		 {"BrightText", QPalette::BrightText},
		 {"ButtonTextd", QPalette::ButtonText},
		 {"Base", QPalette::Base},
		 {"Window", QPalette::Window},
		 {"Shadow", QPalette::Shadow},
		 {"Highlight", QPalette::Highlight},
		 {"HighlightedText", QPalette::HighlightedText},
		 {"Link", QPalette::Link},
		 {"LinkVisited", QPalette::LinkVisited},
		 {"AlternateBase", QPalette::AlternateBase},
         {"NoRole", QPalette::NoRole},
         {"ToolTipBase", QPalette::ToolTipBase},
         {"ToolTipText", QPalette::ToolTipText},
#if QT_VERSION >= 0x050C00
         {"PlaceholderText", QPalette::PlaceholderText}
#endif
	};

	return ColorRoleMap.value(Text, QPalette::NoRole);
}

/**
 * Returns the color group string for a given QPalette::ColorGroup
 */
static QString colorGroupString(QPalette::ColorGroup ColorGroup)
{
	switch (ColorGroup)
	{
	case QPalette::Active: return "active";
	case QPalette::Disabled: return "disabled";
	case QPalette::Inactive: return "inactive";
	default:
		return QString();
	}

	return QString();
}


/**
 * Private data class of CAdvancedStylesheet class (pimpl)
 */
struct StyleManagerPrivate
{
	CStyleManager *_this;
	QString StylesDir;
	QString OutputDir;
	QMap<QString, QString> StyleVariables;
	QMap<QString, QString> ThemeColors;
	QMap<QString, QString> ThemeVariables;// theme variables contains StyleVariables and ThemeColors
	QString Stylesheet;
	QString CurrentStyle;
	QString CurrentTheme;
	QString StyleName;
	QString IconFile;
	QVector<QStringPair> ResourceReplaceList;
	QVector<PaletteColorEntry> PaletteColors;
	QString PaletteBaseColor;
	QJsonObject JsonStyleParam;
	QString ErrorString;
	CStyleManager::eError Error;
	mutable QIcon Icon;
	QStringList Styles;
	QStringList Themes;

	/**
	 * Private data constructor
	 */
	StyleManagerPrivate(CStyleManager *_public);

	/**
	 * Generate the final stylesheet from the stylesheet template file
	 */
	bool generateStylesheet();

	/**
	 * Export the internal generated stylesheet
	 */
	bool exportInternalStylesheet(const QString& Filename);

	/**
	 * Store the given stylesheet
	 */
	bool storeStylesheet(const QString& Stylesheet, const QString& Filename);

	/**
	 * Parse a list of theme variables
	 */
	bool parseVariablesFromXml(QXmlStreamReader& s, const QString& VariableTagName,
		QMap<QString, QString>& Variable);

	/**
	 * Parse the theme file for
	 */
	bool parseThemeFile(const QString& ThemeFilename);

	/**
	 * Parse the style JSON file
	 */
	bool parseStyleJsonFile();

	/**
	 * Creates an Rgba color from a given color and an opacity value in the
	 * range from 0 (transparent) to 1 (opaque)
	 */
	QString rgbaColor(const QString& RgbColor, float Opacity);

	/**
	 *	Replace the stylesheet variables in the given template
	 */
	void replaceStylesheetVariables(QString& Template);

	/**
	 * Register the style fonts to the font database
	 */
	void addFonts(QDir* Dir = nullptr);

	/**
	 * Generate the resources for the variuous states
	 */
	bool generateResourcesFor(const QString& SubDir,
		const QJsonObject& JsonObject, const QFileInfoList& Entries);

	/**
	 * Replace the in the given content the template color string with the
	 * theme color string
	 */
	void replaceColor(QByteArray& Content, const QString& TemplateColor,
		const QString& ThemeColor) const;

	/**
	 * Set error code and error string
	 */
	void setError(CStyleManager::eError Error, const QString& ErrorString);

	/**
	 * Convenience function to ease clearing the error
	 */
	void clearError()
	{
		setError(CStyleManager::NoError, QString());
	}

	/**
	 * Parse palette from JSON file
	 */
	void parsePaletteFromJson();

	/**
	 * Parse palette color group from the given palette json parameters
	 */
	void parsePaletteColorGroup(QJsonObject& jPalette, QPalette::ColorGroup ColorGroup);
};// struct AdvancedStylesheetPrivate


//============================================================================
StyleManagerPrivate::StyleManagerPrivate(
    CStyleManager *_public) :
	_this(_public)
{

}


//============================================================================
void StyleManagerPrivate::setError(CStyleManager::eError Error,
	const QString& ErrorString)
{
	this->Error = Error;
	this->ErrorString = ErrorString;
	if (Error != CStyleManager::NoError)
	{
		qDebug() << "CAdvancedStylesheet Error: " << Error << " " << ErrorString;
	}
}


//============================================================================
QString StyleManagerPrivate::rgbaColor(const QString& RgbColor, float Opacity)
{
	int Alpha = 255 * Opacity;
	auto RgbaColor = RgbColor;
	RgbaColor.insert(1, QString::number(Alpha, 16));
	return RgbaColor;
}


//============================================================================
void StyleManagerPrivate::replaceStylesheetVariables(QString& Content)
{
	static const int OpacityStrSize = QString("opacity(").size();

	QRegularExpression re("\\{\\{.*\\}\\}");
	QRegularExpressionMatch match;
	int index = 0;
	while ((index = Content.indexOf(re, index, &match)) != -1)
	{
		QString ValueString;
		QString MatchString = match.captured();
		// Use only the value inside of the brackets {{ }} without the brackets
		auto TemplateVariable = MatchString.midRef(2, MatchString.size() - 4);
		bool HasOpacity = TemplateVariable.endsWith(')');

		if (HasOpacity)
		{
			auto Values = TemplateVariable.split("|");
			ValueString = _this->themeVariableValue(Values[0].toString());
			auto OpacityStr = Values[1].mid(OpacityStrSize, Values[1].size() - OpacityStrSize - 1);
			bool Ok;
			auto Opacity = OpacityStr.toFloat(&Ok);
			ValueString = rgbaColor(ValueString, Opacity);
		}
		else
		{
			ValueString = _this->themeVariableValue(TemplateVariable.toString());
		}

		Content.replace(index, MatchString.size(), ValueString);
		index += ValueString.size();
	}
}


//============================================================================
bool StyleManagerPrivate::generateStylesheet()
{
	auto CssTemplateFileName = JsonStyleParam.value("css_template").toString();
	if (CssTemplateFileName.isEmpty())
	{
		return false;
	}

	QString TemplateFilePath = _this->currentStylePath() + "/" + CssTemplateFileName;
	if (!QFile::exists(TemplateFilePath))
	{
		setError(CStyleManager::CssTemplateError, "Stylesheet folder "
			"does not contain the CSS template file " + CssTemplateFileName);
		return false;
	}

	QFile TemplateFile(_this->currentStylePath() + "/" + CssTemplateFileName);
	TemplateFile.open(QIODevice::ReadOnly);
	QString Content(TemplateFile.readAll());
	replaceStylesheetVariables(Content);
	Stylesheet = Content;
	exportInternalStylesheet(QFileInfo(TemplateFilePath).baseName() + ".css");
	return true;
}


//============================================================================
bool StyleManagerPrivate::exportInternalStylesheet(const QString& Filename)
{
	return storeStylesheet(this->Stylesheet, Filename);
}


//============================================================================
bool StyleManagerPrivate::storeStylesheet(const QString& Stylesheet, const QString& Filename)
{
	auto OutputPath = _this->currentStyleOutputPath();
	QDir().mkpath(OutputPath);
	QString OutputFilename = OutputPath + "/" + Filename;
	QFile OutputFile(OutputFilename);
	if (!OutputFile.open(QIODevice::WriteOnly))
	{
		setError(CStyleManager::CssExportError, "Exporting stylesheet "
			+ Filename + " caused error: " + OutputFile.errorString());
		return false;
	}
	OutputFile.write(Stylesheet.toUtf8());
	OutputFile.close();
	return true;
}


//============================================================================
void StyleManagerPrivate::addFonts(QDir* Dir)
{
	// I dont't know, if this is the right way to detect, if there are any
	// widgets. The call to QFontDatabase::addApplicationFont() will crash, if
	// there are no widgets
	if (qApp->allWidgets().isEmpty())
	{
		return;
	}

	if (!Dir)
	{
		QDir FontsDir(_this->path(CStyleManager::FontsLocation));
		addFonts(&FontsDir);
	}
	else
	{
		auto Folders = Dir->entryList(QDir::Dirs | QDir::NoDotAndDotDot);
		for (auto Folder : Folders)
		{
			Dir->cd(Folder);
			addFonts(Dir);
			Dir->cdUp();
		}

		auto FontFiles = Dir->entryList({"*.ttf"}, QDir::Files);
		for (auto Font : FontFiles)
		{
            QString FontFilename = Dir->absoluteFilePath(Font);
			QFontDatabase::addApplicationFont(FontFilename);
		}
	}
}


//============================================================================
bool StyleManagerPrivate::parseVariablesFromXml(
	QXmlStreamReader& s, const QString& TagName, QMap<QString, QString>& Variables)
{
	while (s.readNextStartElement())
	{
		if (s.name() != TagName)
		{
			setError(CStyleManager::ThemeXmlError, "Malformed theme "
				"file - expected tag <" + TagName + "> instead of " + s.name());
			return false;
		}
		auto Name = s.attributes().value("name");
		if (Name.isEmpty())
		{
			setError(CStyleManager::ThemeXmlError, "Malformed theme file - "
				"name attribute missing in <" + TagName + "> tag");
			return false;
		}

		auto Value = s.readElementText(QXmlStreamReader::SkipChildElements);
		if (Value.isEmpty())
		{
			setError(CStyleManager::ThemeXmlError, "Malformed theme file - "
				"text of <" + TagName + "> tag is empty");
			return false;
		}

		Variables.insert(Name.toString(), Value);
	}

	return true;
}


//============================================================================
bool StyleManagerPrivate::parseThemeFile(const QString& Theme)
{
	QString ThemeFileName = _this->path(CStyleManager::ThemesLocation) + "/" + Theme;
	QFile ThemeFile(ThemeFileName);
	ThemeFile.open(QIODevice::ReadOnly);
	QXmlStreamReader s(&ThemeFile);
	s.readNextStartElement();
	if (s.name() != "resources")
	{
		setError(CStyleManager::ThemeXmlError, "Malformed theme file - "
			"expected tag <resources> instead of " + s.name());
		return false;
	}

    QMap<QString, QString> ColorVariables;
	parseVariablesFromXml(s, "color", ColorVariables);
	this->ThemeVariables = this->StyleVariables;
	this->ThemeVariables.insert(ColorVariables);
	this->ThemeColors = ColorVariables;
	return true;
}


//============================================================================
bool StyleManagerPrivate::parseStyleJsonFile()
{
	QDir Dir(_this->currentStylePath());
	auto JsonFiles = Dir.entryInfoList({"*.json"}, QDir::Files);
	if (JsonFiles.count() < 1)
	{
		setError(CStyleManager::StyleJsonError, "Stylesheet folder does "
			"not contain a style json file");
		return false;
	}

	if (JsonFiles.count() > 1)
	{
		setError(CStyleManager::StyleJsonError, "Stylesheet folder "
			"contains multiple theme json files");
		return false;
	}

	QFile StyleJsonFile(JsonFiles[0].absoluteFilePath());
	StyleJsonFile.open(QIODevice::ReadOnly);

	auto JsonData = StyleJsonFile.readAll();
	QJsonParseError ParseError;
	auto JsonDocument = QJsonDocument::fromJson(JsonData, &ParseError);
	if (JsonDocument.isNull())
	{
		setError(CStyleManager::StyleJsonError, "Loading style json file "
			"caused error: " + ParseError.errorString());
		return false;
	}

	auto json = JsonStyleParam = JsonDocument.object();
	StyleName = json.value("name").toString();
	if (StyleName.isEmpty())
	{
		setError(CStyleManager::StyleJsonError, "No key \"name\" found "
			"in style json file");
		return false;
	}

	QMap<QString, QString> Variables;
	auto jvariables = json.value("variables").toObject();
	for (const auto& key : jvariables.keys())
	{
		Variables.insert(key, jvariables.value(key).toString());
	}

	StyleVariables = Variables;
	IconFile = json.value("icon").toString();
	parsePaletteFromJson();

	return true;
}


//============================================================================
void StyleManagerPrivate::parsePaletteFromJson()
{
	PaletteBaseColor = QString();
	PaletteColors.clear();
	auto jPalette = JsonStyleParam.value("palette").toObject();
	if (jPalette.isEmpty())
	{
		return;
	}

	PaletteBaseColor = jPalette.value("base_color").toString();
	parsePaletteColorGroup(jPalette, QPalette::Active);
	parsePaletteColorGroup(jPalette, QPalette::Disabled);
	parsePaletteColorGroup(jPalette, QPalette::Inactive);
}


//============================================================================
void StyleManagerPrivate::parsePaletteColorGroup(QJsonObject& jPalette, QPalette::ColorGroup ColorGroup)
{
	auto jColorGroup = jPalette.value(colorGroupString(ColorGroup)).toObject();
	if (jColorGroup.isEmpty())
	{
		return;
	}

	for (auto itc = jColorGroup.constBegin(); itc != jColorGroup.constEnd(); ++itc)
	{
		auto ColorRole = colorRoleFromString(itc.key());
		if (QPalette::NoRole == ColorRole)
		{
			continue;
		}

		this->PaletteColors.append({ColorGroup, ColorRole, itc.value().toString()});
		if (ColorGroup != QPalette::Active)
		{
			continue;
		}
	}
}


//============================================================================
void StyleManagerPrivate::replaceColor(QByteArray& Content,
	const QString& TemplateColor, const QString& ThemeColor) const
{
	Content.replace(TemplateColor.toLatin1(), ThemeColor.toLatin1());
}


//============================================================================
bool StyleManagerPrivate::generateResourcesFor(const QString& SubDir,
	const QJsonObject& JsonObject, const QFileInfoList& Entries)
{
	const QString OutputDir = _this->currentStyleOutputPath() + "/" + SubDir;
	if (!QDir().mkpath(OutputDir))
	{
		setError(CStyleManager::ResourceGeneratorError, "Error "
			"creating resource output folder: " + OutputDir);
		return false;
	}

	// Fill the color replace list with the values read from style json file
	QVector<QStringPair> ColorReplaceList;
	for (auto it = JsonObject.constBegin(); it != JsonObject.constEnd(); ++it)
	{
		auto TemplateColor = it.key();
		auto ThemeColor = it.value().toString();
		// If the color starts with an hashtag, then we have a real color value
		// If it does not start with # then it is a theme variable
		if (!ThemeColor.startsWith('#'))
		{
			ThemeColor = _this->themeVariableValue(ThemeColor);
		}
		ColorReplaceList.append({TemplateColor, ThemeColor});
	}

	// Now loop through all resources svg files and replace the colors
	for (const auto& Entry : Entries)
	{
		QFile SvgFile(Entry.absoluteFilePath());
		SvgFile.open(QIODevice::ReadOnly);
		auto Content = SvgFile.readAll();
		SvgFile.close();

		for (const auto& Replace : ColorReplaceList)
		{
			replaceColor(Content, Replace.first, Replace.second);
		}

		QString OutputFilename = OutputDir + "/" + Entry.fileName();
		QFile OutputFile(OutputFilename);
		OutputFile.open(QIODevice::WriteOnly);
		OutputFile.write(Content);
		OutputFile.close();
	}

	return true;
}


//============================================================================
CStyleManager::CStyleManager(QObject* parent) :
	QObject(parent),
	d(new StyleManagerPrivate(this))
{

}


//============================================================================
CStyleManager::~CStyleManager()
{
	delete d;
}


//============================================================================
void CStyleManager::setStylesDirPath(const QString& DirPath)
{
	d->StylesDir = DirPath;
	QDir Dir(d->StylesDir);
	d->Styles = Dir.entryList(QDir::Dirs | QDir::NoDotAndDotDot);
}


//============================================================================
QString CStyleManager::stylesDirPath() const
{
	return d->StylesDir;
}


//============================================================================
bool CStyleManager::setCurrentStyle(const QString& Style)
{
	d->clearError();
	d->CurrentStyle = Style;
	QDir Dir(path(ThemesLocation));
	d->Themes = Dir.entryList({"*.xml"}, QDir::Files);
	for (auto& Theme : d->Themes)
	{
		Theme.replace(".xml", "");
	}
	auto Result = d->parseStyleJsonFile();
	QDir::addSearchPath("icon", currentStyleOutputPath());
	d->addFonts();
	emit currentStyleChanged(d->CurrentStyle);
	emit stylesheetChanged();
	return Result;
}


//============================================================================
QString CStyleManager::currentStyle() const
{
	return d->CurrentStyle;
}


//============================================================================
QString CStyleManager::currentStylePath() const
{
	return d->StylesDir + "/" + d->CurrentStyle;
}


//============================================================================
QString CStyleManager::outputDirPath() const
{
	return d->OutputDir;
}


//============================================================================
void CStyleManager::setOutputDirPath(const QString& Path)
{
	d->OutputDir = Path;
}


//============================================================================
QString CStyleManager::currentStyleOutputPath() const
{
	return outputDirPath() + "/" + d->CurrentStyle;
}


//============================================================================
QString CStyleManager::themeVariableValue(const QString& VariableId) const
{
	return d->ThemeVariables.value(VariableId, QString());
}


//============================================================================
QColor CStyleManager::themeColor(const QString& VariableId) const
{
	auto ColorString = d->ThemeColors.value(VariableId, QString());
	if (ColorString.isEmpty())
	{
		return QColor();
	}

	return QColor(ColorString);
}


//============================================================================
void CStyleManager::setThemeVariableValue(const QString& VariableId, const QString& Value)
{
	d->ThemeVariables.insert(VariableId, Value);
	auto it = d->ThemeColors.find(VariableId);
	if (it != d->ThemeColors.end())
	{
		it.value() = Value;
	}
}


//============================================================================
bool CStyleManager::setCurrentTheme(const QString& Theme)
{
	d->clearError();
	if (d->JsonStyleParam.isEmpty())
	{
		return false;
	}

	if (!d->parseThemeFile(Theme + ".xml"))
	{
		return false;
	}

	d->CurrentTheme = Theme;
	emit currentThemeChanged(d->CurrentTheme);
	return true;
}


//============================================================================
bool CStyleManager::updateStylesheet()
{
	if (!processStyleTemplate())
	{
		return false;
	}

	if (!d->generateStylesheet() && (error() != CStyleManager::NoError))
	{
		return false;
	}

	emit stylesheetChanged();
	return true;
}



//============================================================================
bool CStyleManager::processStyleTemplate()
{
	updateApplicationPaletteColors();
	return generateResources();
}


//============================================================================
QString CStyleManager::currentTheme() const
{
	return d->CurrentTheme;
}


//============================================================================
QString CStyleManager::styleSheet() const
{
	return d->Stylesheet;
}


//============================================================================
const QIcon& CStyleManager::styleIcon() const
{
	if (d->Icon.isNull() && !d->IconFile.isEmpty())
	{
		d->Icon = QIcon(currentStylePath() + "/" + d->IconFile);
	}

	return d->Icon;
}


//============================================================================
const QStringList& CStyleManager::styles() const
{
	return d->Styles;
}


//============================================================================
const QStringList& CStyleManager::themes() const
{
	return d->Themes;
}


//============================================================================
QString CStyleManager::processStylesheetTemplate(const QString& Template,
	const QString& OutputFile)
{
	auto Stylesheet = Template;
	d->replaceStylesheetVariables(Stylesheet);
	if (!OutputFile.isEmpty())
	{
		d->storeStylesheet(Stylesheet, OutputFile);
	}
	return Stylesheet;
}


//============================================================================
const QMap<QString, QString>& CStyleManager::themeColorVariables() const
{
	return d->ThemeColors;
}


//============================================================================
CStyleManager::eError CStyleManager::error() const
{
	return d->Error;
}


//============================================================================
QString CStyleManager::errorString() const
{
	return d->ErrorString;
}


//============================================================================
QString CStyleManager::path(eLocation Location) const
{
	switch (Location)
	{
	case ThemesLocation: return currentStylePath() + "/themes";
	case ResourceTemplatesLocation: return currentStylePath() + "/resources";
	case FontsLocation: return currentStylePath() + "/fonts";
	}

	return QString();
}


//============================================================================
bool CStyleManager::generateResources()
{
	QDir ResourceDir(path(CStyleManager::ResourceTemplatesLocation));
	auto Entries = ResourceDir.entryInfoList({"*.svg"}, QDir::Files);

	auto jresources = d->JsonStyleParam.value("resources").toObject();
	if (jresources.isEmpty())
	{
		d->setError(CStyleManager::StyleJsonError, "Key resources "
			"missing in style json file");
		return false;
	}

	// Process all resource generation variants
	bool Result = true;
	for (auto itc = jresources.constBegin(); itc != jresources.constEnd(); ++itc)
	{
		auto Param = itc.value().toObject();
		if (Param.isEmpty())
		{
			d->setError(CStyleManager::StyleJsonError, "Key resources "
				"missing in style json file");
			Result = false;
			continue;
		}
		if (!d->generateResourcesFor(itc.key(), Param, Entries))
		{
			Result = false;
		}
	}

	return Result;
}


//============================================================================
QPalette CStyleManager::generateThemePalette() const
{
	QPalette Palette = qApp->palette();
	if (!d->PaletteBaseColor.isEmpty())
	{
		auto Color = themeColor(d->PaletteBaseColor);
		if (Color.isValid())
		{
			Palette = QPalette(Color);
		}
	}

	for (const auto& Entry : d->PaletteColors)
	{
		auto Color = themeColor(Entry.ColorVariable);
		if (Color.isValid())
		{
			Palette.setColor(Entry.Group, Entry.Role, themeColor(Entry.ColorVariable));
		}
	}

	return Palette;
}


//============================================================================
void CStyleManager::updateApplicationPaletteColors()
{
	qApp->setPalette(generateThemePalette());
}


//============================================================================
const QJsonObject& CStyleManager::styleParameters() const
{
	return d->JsonStyleParam;
}

} // namespace acss

//---------------------------------------------------------------------------
// EOF AdvancedStylesheet.cpp
