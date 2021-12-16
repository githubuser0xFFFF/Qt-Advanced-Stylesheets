//============================================================================
/// \file   AdvancedStylesheet.cpp
/// \author Uwe Kindler
/// \date   13.12.2021
/// \brief  Implementation of CAdvancedStylesheet class
//============================================================================

//============================================================================
//                                   INCLUDES
//============================================================================
#include "AdvancedStylesheet.h"

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

namespace acss
{
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
	void exportStylesheet(const QString& Filename);

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
	 * Generate the required icons for this theme
	 */
	bool generateResources();

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
	QDir Dir(_this->currentStylePath());
	auto TemplateFiles = Dir.entryInfoList({"*.template"}, QDir::Files);
	if (TemplateFiles.count() < 1)
	{
		setError(CStyleManager::CssTemplateError, "Stylesheet folder "
			"does not contain a *.template file");
		return false;
	}

	if (TemplateFiles.count() > 1)
	{
		setError(CStyleManager::CssTemplateError, "Stylesheet folder "
			"contains multiple *.template files");
		return false;
	}

	QFile TemplateFile(TemplateFiles[0].absoluteFilePath());
	TemplateFile.open(QIODevice::ReadOnly);
	QString Content(TemplateFile.readAll());
	replaceStylesheetVariables(Content);
	Stylesheet = Content;
	exportStylesheet(TemplateFiles[0].baseName() + ".css");
	return true;
}


//============================================================================
void StyleManagerPrivate::exportStylesheet(const QString& Filename)
{
	QDir().mkpath(OutputDir);
	QString OutputFilename = OutputDir + "/" + Filename;
	QFile OutputFile(OutputFilename);
	if (!OutputFile.open(QIODevice::WriteOnly))
	{
		setError(CStyleManager::CssExportError, "Exporting stylesheet "
			+ Filename + " caused error: " + OutputFile.errorString());
		return;
	}
	OutputFile.write(Stylesheet.toUtf8());
	OutputFile.close();
}


//============================================================================
void StyleManagerPrivate::addFonts(QDir* Dir)
{
	if (!Dir)
	{
		QDir FontsDir(_this->fontsPath());
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
	QString ThemeFileName = _this->themesPath() + "/" + Theme;
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

	return true;
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
	const QString OutputDir = _this->outputDirPath() + "/" + SubDir;
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
bool StyleManagerPrivate::generateResources()
{
	QDir ResourceDir(_this->resourceTemplatesPath());
	auto Entries = ResourceDir.entryInfoList({"*.svg"}, QDir::Files);

	auto jresources = JsonStyleParam.value("resources").toObject();
	if (jresources.isEmpty())
	{
		setError(CStyleManager::StyleJsonError, "Key resources "
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
			setError(CStyleManager::StyleJsonError, "Key resources "
				"missing in style json file");
			Result = false;
			continue;
		}
		if (!generateResourcesFor(itc.key(), Param, Entries))
		{
			Result = false;
		}
	}

	return Result;
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
	QDir Dir(themesPath());
	d->Themes = Dir.entryList({"*.xml"}, QDir::Files);
	for (auto& Theme : d->Themes)
	{
		Theme.replace(".xml", "");
	}
	auto Result = d->parseStyleJsonFile();
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
QString CStyleManager::resourceTemplatesPath() const
{
	return currentStylePath() + "/resources";
}


//============================================================================
QString CStyleManager::themesPath() const
{
	return currentStylePath() + "/themes";
}


//============================================================================
QString CStyleManager::fontsPath() const
{
	return currentStylePath() + "/fonts";
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
void CStyleManager::setThemeVariabeValue(const QString& VariableId, const QString& Value)
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
	d->addFonts();
	if (!d->generateStylesheet())
	{
		return false;
	}
	d->generateResources();

	QDir::addSearchPath("icon", d->OutputDir);
	emit currentThemeChanged(d->CurrentTheme);
	emit stylesheetChanged();
	return true;
}


//============================================================================
bool CStyleManager::updateStylesheet()
{
	if (!d->generateStylesheet())
	{
		return false;
	}

	d->generateResources();
	emit stylesheetChanged();
	return true;
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
QString CStyleManager::processStylesheetTemplate(const QString& Template)
{
	auto Stylesheet = Template;
	d->replaceStylesheetVariables(Stylesheet);
	return Stylesheet;
}


//============================================================================
const QMap<QString, QString>& CStyleManager::themeColors() const
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

} // namespace acss

//---------------------------------------------------------------------------
// EOF AdvancedStylesheet.cpp
