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
/// \file   AdvancedStylesheet.cpp
/// \author Uwe Kindler
/// \date   13.12.2021
/// \brief  Implementation of CAdvancedStylesheet class
//============================================================================

//============================================================================
//                                   INCLUDES
//============================================================================
#include <QtAdvancedStylesheet.h>
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
#include <QIconEngine>
#include <QSvgRenderer>
#include <QPainter>
#include <QSet>


namespace acss
{
class CSVGIconEngine;
Q_GLOBAL_STATIC(QSet<CSVGIconEngine*>, IconEngineInstances)

/**
 * SvgIcon engine that supports loading from memory buffer
 */
class CSVGIconEngine : public QIconEngine
{
private:
	QByteArray m_SvgTemplate;
	QByteArray m_SvgContent; ///< memory buffer with SVG data load from file
	QtAdvancedStylesheet* m_AdvancedStyleheet = nullptr;

public:
	/**
	 * Creates an icon engine with the given SVG content an assigned
	 * AndvancedStylesheet object
	 */
	explicit CSVGIconEngine(const QByteArray &SvgContent, QtAdvancedStylesheet* Styleeheet)
		: m_SvgTemplate(SvgContent),
		  m_AdvancedStyleheet(Styleeheet)
	{
		update();
		IconEngineInstances->insert(this);
	}

	/**
	 * Removes itself from the set of instances
	 */
	virtual ~CSVGIconEngine()
	{
		if (!IconEngineInstances.isDestroyed())
		{
			IconEngineInstances->remove(this);
		}
	}

	/**
	 * Update the SVG content with the current theme icon colors
	 */
	void update()
	{
		m_SvgContent = m_SvgTemplate;
		m_AdvancedStyleheet->replaceSvgColors(m_SvgContent);
	}

	/**
	 * Calls update for all Icon engine instances
	 */
	static void updateAllIcons()
	{
		for (auto Engine : *IconEngineInstances)
		{
			Engine->update();
		}
	}

	virtual void paint(QPainter *painter, const QRect &rect, QIcon::Mode mode,
	    QIcon::State state) override
	{
		Q_UNUSED(mode);
		Q_UNUSED(state);

		QSvgRenderer renderer(m_SvgContent);
		renderer.render(painter, rect);
	}

	virtual QIconEngine* clone() const override
	{
		return new CSVGIconEngine(*this);
	}

	virtual QPixmap pixmap(const QSize &size, QIcon::Mode mode,
	    QIcon::State state) override
	{
		// This function is necessary to create an EMPTY pixmap. It's called always
		// before paint()
		QImage img(size, QImage::Format_ARGB32);
		img.fill(qRgba(0, 0, 0, 0));
		QPixmap pix = QPixmap::fromImage(img, Qt::NoFormatConversion);
		{
			QPainter painter(&pix);
			QRect r(QPoint(0.0, 0.0), size);
			this->paint(&painter, r, mode, state);
		}
		return pix;
	}
};


/**
 * Groups the data the build a parsed palette color entry
 */
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


template <class Key, class T>
static void insertIntoMap(QMap<Key, T>& Map, const QMap<Key, T> &map)
{
#if QT_VERSION >= 0x050F00
    Map.insert(map);
#else
    for (auto itc = map.constBegin(); itc != map.constEnd(); ++itc)
    {
        Map.insert(itc.key(), itc.value());
    }
#endif
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
struct QtAdvancedStylesheetPrivate
{
	QtAdvancedStylesheet *_this;
	QString StylesDir;
	QString OutputDir;
	QMap<QString, QString> StyleVariables;
	QMap<QString, QString> ThemeColors;
	QMap<QString, QString> ThemeVariables;// theme variables contains StyleVariables and ThemeColors
	QString Stylesheet;
	QString CurrentStyle;
	QString CurrentTheme;
	QString DefaultTheme;
	QString StyleName;
	QString IconFile;
	QVector<QStringPair> ResourceReplaceList;
	QVector<PaletteColorEntry> PaletteColors;
	QString PaletteBaseColor;
	QJsonObject JsonStyleParam;
	QString ErrorString;
	QtAdvancedStylesheet::eError Error;
	mutable QIcon Icon;
	QStringList Styles;
	QStringList Themes;
	bool IsDarkTheme = false;
	mutable tColorReplaceList IconColorReplaceList;

	/**
	 * Private data constructor
	 */
	QtAdvancedStylesheetPrivate(QtAdvancedStylesheet *_public);

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
	void setError(QtAdvancedStylesheet::eError Error, const QString& ErrorString);

	/**
	 * Convenience function to ease clearing the error
	 */
	void clearError()
	{
		setError(QtAdvancedStylesheet::NoError, QString());
	}

	/**
	 * Parse palette from JSON file
	 */
	void parsePaletteFromJson();

	/**
	 * Parse palette color group from the given palette json parameters
	 */
	void parsePaletteColorGroup(QJsonObject& jPalette, QPalette::ColorGroup ColorGroup);

	/**
	 * Use this function to access the icon color replace list, to ensure, that
	 * is is properly initialized
	 */
	const tColorReplaceList& iconColorReplaceList() const;

	/**
	 * Parse a color replace list from the given JsonObject
	 */
	tColorReplaceList parseColorReplaceList(const QJsonObject& JsonObject) const;
};// struct AdvancedStylesheetPrivate


//============================================================================
QtAdvancedStylesheetPrivate::QtAdvancedStylesheetPrivate(
    QtAdvancedStylesheet *_public) :
	_this(_public)
{

}


//============================================================================
void QtAdvancedStylesheetPrivate::setError(QtAdvancedStylesheet::eError Error,
	const QString& ErrorString)
{
	this->Error = Error;
	this->ErrorString = ErrorString;
	if (Error != QtAdvancedStylesheet::NoError)
	{
		qDebug() << "CAdvancedStylesheet Error: " << Error << " " << ErrorString;
	}
}


//============================================================================
QString QtAdvancedStylesheetPrivate::rgbaColor(const QString& RgbColor, float Opacity)
{
	int Alpha = 255 * Opacity;
	auto RgbaColor = RgbColor;
	RgbaColor.insert(1, QString("%1").arg(Alpha, 2, 16, QChar('0')));
	return RgbaColor;
}


//============================================================================
void QtAdvancedStylesheetPrivate::replaceStylesheetVariables(QString& Content)
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
                auto TemplateVariable = MatchString.mid(2, MatchString.size() - 4);
		bool HasOpacity = TemplateVariable.endsWith(')');

		if (HasOpacity)
		{
			auto Values = TemplateVariable.split("|");
                        ValueString = _this->themeVariableValue(Values[0]);
			auto OpacityStr = Values[1].mid(OpacityStrSize, Values[1].size() - OpacityStrSize - 1);
			bool Ok;
			auto Opacity = OpacityStr.toFloat(&Ok);
			ValueString = rgbaColor(ValueString, Opacity);
		}
		else
		{
                        ValueString = _this->themeVariableValue(TemplateVariable);
		}

		Content.replace(index, MatchString.size(), ValueString);
		index += ValueString.size();
	}
}


//============================================================================
bool QtAdvancedStylesheetPrivate::generateStylesheet()
{
	auto CssTemplateFileName = JsonStyleParam.value("css_template").toString();
	if (CssTemplateFileName.isEmpty())
	{
		return false;
	}

	QString TemplateFilePath = _this->currentStylePath() + "/" + CssTemplateFileName;
	if (!QFile::exists(TemplateFilePath))
	{
		setError(QtAdvancedStylesheet::CssTemplateError, "Stylesheet folder "
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
bool QtAdvancedStylesheetPrivate::exportInternalStylesheet(const QString& Filename)
{
	return storeStylesheet(this->Stylesheet, Filename);
}


//============================================================================
bool QtAdvancedStylesheetPrivate::storeStylesheet(const QString& Stylesheet, const QString& Filename)
{
	auto OutputPath = _this->currentStyleOutputPath();
	QDir().mkpath(OutputPath);
	QString OutputFilename = OutputPath + "/" + Filename;
	QFile OutputFile(OutputFilename);
	if (!OutputFile.open(QIODevice::WriteOnly))
	{
		setError(QtAdvancedStylesheet::CssExportError, "Exporting stylesheet "
			+ Filename + " caused error: " + OutputFile.errorString());
		return false;
	}
	OutputFile.write(Stylesheet.toUtf8());
	OutputFile.close();
	return true;
}


//============================================================================
void QtAdvancedStylesheetPrivate::addFonts(QDir* Dir)
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
		QDir FontsDir(_this->path(QtAdvancedStylesheet::FontsLocation));
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
bool QtAdvancedStylesheetPrivate::parseVariablesFromXml(
	QXmlStreamReader& s, const QString& TagName, QMap<QString, QString>& Variables)
{
	while (s.readNextStartElement())
	{
		if (s.name() != TagName)
		{
			setError(QtAdvancedStylesheet::ThemeXmlError, "Malformed theme "
                                "file - expected tag <" + TagName + "> instead of " + s.name().toString());
			return false;
		}
		auto Name = s.attributes().value("name");
		if (Name.isEmpty())
		{
			setError(QtAdvancedStylesheet::ThemeXmlError, "Malformed theme file - "
				"name attribute missing in <" + TagName + "> tag");
			return false;
		}

		auto Value = s.readElementText(QXmlStreamReader::SkipChildElements);
		if (Value.isEmpty())
		{
			setError(QtAdvancedStylesheet::ThemeXmlError, "Malformed theme file - "
				"text of <" + TagName + "> tag is empty");
			return false;
		}

		Variables.insert(Name.toString(), Value);
	}

	return true;
}


//============================================================================
bool QtAdvancedStylesheetPrivate::parseThemeFile(const QString& Theme)
{
	QString ThemeFileName = _this->path(QtAdvancedStylesheet::ThemesLocation) + "/" + Theme;
	QFile ThemeFile(ThemeFileName);
	ThemeFile.open(QIODevice::ReadOnly);
	QXmlStreamReader s(&ThemeFile);
        s.readNextStartElement();
        if (s.name() != QString("resources"))
	{
		setError(QtAdvancedStylesheet::ThemeXmlError, "Malformed theme file - "
                        "expected tag <resources> instead of " + s.name().toString());
		return false;
	}

	IsDarkTheme = (s.attributes().value("dark").toInt() == 1);
    QMap<QString, QString> ColorVariables;
	parseVariablesFromXml(s, "color", ColorVariables);
	this->ThemeVariables = this->StyleVariables;
	insertIntoMap(this->ThemeVariables, ColorVariables);
	this->ThemeColors = ColorVariables;
	return true;
}


//============================================================================
bool QtAdvancedStylesheetPrivate::parseStyleJsonFile()
{
	QDir Dir(_this->currentStylePath());
	auto JsonFiles = Dir.entryInfoList({"*.json"}, QDir::Files);
	if (JsonFiles.count() < 1)
	{
		setError(QtAdvancedStylesheet::StyleJsonError, "Stylesheet folder does "
			"not contain a style json file");
		return false;
	}

	if (JsonFiles.count() > 1)
	{
		setError(QtAdvancedStylesheet::StyleJsonError, "Stylesheet folder "
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
		setError(QtAdvancedStylesheet::StyleJsonError, "Loading style json file "
			"caused error: " + ParseError.errorString());
		return false;
	}

	auto json = JsonStyleParam = JsonDocument.object();
	StyleName = json.value("name").toString();
	if (StyleName.isEmpty())
	{
		setError(QtAdvancedStylesheet::StyleJsonError, "No key \"name\" found "
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

	DefaultTheme = json.value("default_theme").toString();
	if (DefaultTheme.isEmpty())
	{
		setError(QtAdvancedStylesheet::StyleJsonError, "No key \"default_theme\" found "
			"in style json file");
		return false;
	}

	return true;
}


//============================================================================
void QtAdvancedStylesheetPrivate::parsePaletteFromJson()
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
void QtAdvancedStylesheetPrivate::parsePaletteColorGroup(QJsonObject& jPalette, QPalette::ColorGroup ColorGroup)
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
void QtAdvancedStylesheetPrivate::replaceColor(QByteArray& Content,
	const QString& TemplateColor, const QString& ThemeColor) const
{
	Content.replace(TemplateColor.toLatin1(), ThemeColor.toLatin1());
}


//============================================================================
tColorReplaceList QtAdvancedStylesheetPrivate::parseColorReplaceList(const QJsonObject& JsonObject) const
{
	// Fill the color replace list with the values read from style json file
	tColorReplaceList ColorReplaceList;
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

	return ColorReplaceList;
}


//============================================================================
bool QtAdvancedStylesheetPrivate::generateResourcesFor(const QString& SubDir,
	const QJsonObject& JsonObject, const QFileInfoList& Entries)
{
	const QString OutputDir = _this->currentStyleOutputPath() + "/" + SubDir;
	if (!QDir().mkpath(OutputDir))
	{
		setError(QtAdvancedStylesheet::ResourceGeneratorError, "Error "
			"creating resource output folder: " + OutputDir);
		return false;
	}

	auto ColorReplaceList = parseColorReplaceList(JsonObject);

	// Now loop through all resources svg files and replace the colors
	for (const auto& Entry : Entries)
	{
		QFile SvgFile(Entry.absoluteFilePath());
		SvgFile.open(QIODevice::ReadOnly);
		auto Content = SvgFile.readAll();
		SvgFile.close();
		_this->replaceSvgColors(Content, ColorReplaceList);
		QString OutputFilename = OutputDir + "/" + Entry.fileName();
		QFile OutputFile(OutputFilename);
		OutputFile.open(QIODevice::WriteOnly);
		OutputFile.write(Content);
		OutputFile.close();
	}

	return true;
}


//============================================================================
const tColorReplaceList& QtAdvancedStylesheetPrivate::iconColorReplaceList() const
{
	if (IconColorReplaceList.count())
	{
		return IconColorReplaceList;
	}

	auto jicon_colors = JsonStyleParam.value("icon_colors").toObject();
	if (jicon_colors.isEmpty())
	{
		return IconColorReplaceList;
	}

	IconColorReplaceList = parseColorReplaceList(jicon_colors);
	return IconColorReplaceList;
}


//============================================================================
void QtAdvancedStylesheet::replaceSvgColors(QByteArray& SvgContent,
	const tColorReplaceList& ColorReplaceList)
{
	const tColorReplaceList& ReplaceList = ColorReplaceList.isEmpty() ?
		d->iconColorReplaceList() : ColorReplaceList;
	for (const auto& Replace : ReplaceList)
	{
		d->replaceColor(SvgContent, Replace.first, Replace.second);
	}
}


//============================================================================
QIcon QtAdvancedStylesheet::loadThemeAwareSvgIcon(const QString& Filename)
{
	QFile SvgFile(Filename);
	SvgFile.open(QIODevice::ReadOnly);
	auto Content = SvgFile.readAll();
	return QIcon(new CSVGIconEngine(Content, this));
}


//============================================================================
QtAdvancedStylesheet::QtAdvancedStylesheet(QObject* parent) :
	QObject(parent),
	d(new QtAdvancedStylesheetPrivate(this))
{

}


//============================================================================
QtAdvancedStylesheet::~QtAdvancedStylesheet()
{
	delete d;
}


//============================================================================
void QtAdvancedStylesheet::setStylesDirPath(const QString& DirPath)
{
	d->StylesDir = DirPath;
	QDir Dir(d->StylesDir);
	d->Styles = Dir.entryList(QDir::Dirs | QDir::NoDotAndDotDot);
}


//============================================================================
QString QtAdvancedStylesheet::stylesDirPath() const
{
	return d->StylesDir;
}


//============================================================================
bool QtAdvancedStylesheet::setCurrentStyle(const QString& Style)
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
QString QtAdvancedStylesheet::currentStyle() const
{
	return d->CurrentStyle;
}


//============================================================================
QString QtAdvancedStylesheet::currentStylePath() const
{
	return d->StylesDir + "/" + d->CurrentStyle;
}


//============================================================================
QString QtAdvancedStylesheet::outputDirPath() const
{
	return d->OutputDir;
}


//============================================================================
void QtAdvancedStylesheet::setOutputDirPath(const QString& Path)
{
	d->OutputDir = Path;
}


//============================================================================
QString QtAdvancedStylesheet::currentStyleOutputPath() const
{
	return outputDirPath() + "/" + d->CurrentStyle;
}


//============================================================================
QString QtAdvancedStylesheet::themeVariableValue(const QString& VariableId) const
{
	return d->ThemeVariables.value(VariableId, QString());
}


//============================================================================
QColor QtAdvancedStylesheet::themeColor(const QString& VariableId) const
{
	auto ColorString = d->ThemeColors.value(VariableId, QString());
	if (ColorString.isEmpty())
	{
		return QColor();
	}

	return QColor(ColorString);
}


//============================================================================
void QtAdvancedStylesheet::setThemeVariableValue(const QString& VariableId, const QString& Value)
{
	d->ThemeVariables.insert(VariableId, Value);
	auto it = d->ThemeColors.find(VariableId);
	if (it != d->ThemeColors.end())
	{
		it.value() = Value;
	}
}


//============================================================================
bool QtAdvancedStylesheet::setCurrentTheme(const QString& Theme)
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
void QtAdvancedStylesheet::setDefaultTheme()
{
	setCurrentTheme(d->DefaultTheme);
}


//============================================================================
bool QtAdvancedStylesheet::updateStylesheet()
{
	if (!processStyleTemplate())
	{
		return false;
	}

	d->IconColorReplaceList.clear();
	CSVGIconEngine::updateAllIcons();
	if (!d->generateStylesheet() && (error() != QtAdvancedStylesheet::NoError))
	{
		return false;
	}

	emit stylesheetChanged();
	return true;
}



//============================================================================
bool QtAdvancedStylesheet::processStyleTemplate()
{
	updateApplicationPaletteColors();
	return generateResources();
}


//============================================================================
QString QtAdvancedStylesheet::currentTheme() const
{
	return d->CurrentTheme;
}


//============================================================================
QString QtAdvancedStylesheet::styleSheet() const
{
	return d->Stylesheet;
}


//============================================================================
const QIcon& QtAdvancedStylesheet::styleIcon() const
{
	if (d->Icon.isNull() && !d->IconFile.isEmpty())
	{
		d->Icon = QIcon(currentStylePath() + "/" + d->IconFile);
	}

	return d->Icon;
}


//============================================================================
const QStringList& QtAdvancedStylesheet::styles() const
{
	return d->Styles;
}


//============================================================================
const QStringList& QtAdvancedStylesheet::themes() const
{
	return d->Themes;
}


//============================================================================
QString QtAdvancedStylesheet::processStylesheetTemplate(const QString& Template,
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
const QMap<QString, QString>& QtAdvancedStylesheet::themeColorVariables() const
{
	return d->ThemeColors;
}


//============================================================================
QtAdvancedStylesheet::eError QtAdvancedStylesheet::error() const
{
	return d->Error;
}


//============================================================================
QString QtAdvancedStylesheet::errorString() const
{
	return d->ErrorString;
}


//============================================================================
QString QtAdvancedStylesheet::path(eLocation Location) const
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
bool QtAdvancedStylesheet::generateResources()
{
	QDir ResourceDir(path(QtAdvancedStylesheet::ResourceTemplatesLocation));
	auto Entries = ResourceDir.entryInfoList({"*.svg"}, QDir::Files);

	auto jresources = d->JsonStyleParam.value("resources").toObject();
	if (jresources.isEmpty())
	{
		d->setError(QtAdvancedStylesheet::StyleJsonError, "Key resources "
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
			d->setError(QtAdvancedStylesheet::StyleJsonError, "Key resources "
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
QPalette QtAdvancedStylesheet::generateThemePalette() const
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
void QtAdvancedStylesheet::updateApplicationPaletteColors()
{
	qApp->setPalette(generateThemePalette());
}


//============================================================================
const QJsonObject& QtAdvancedStylesheet::styleParameters() const
{
	return d->JsonStyleParam;
}


//============================================================================
bool QtAdvancedStylesheet::isCurrentThemeDark() const
{
	return d->IsDarkTheme;
}


} // namespace acss

//---------------------------------------------------------------------------
// EOF AdvancedStylesheet.cpp
