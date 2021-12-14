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

#include "ResourceGenerator.h"

namespace acss
{
/**
 * Private data class of CAdvancedStylesheet class (pimpl)
 */
struct AdvancedStylesheetPrivate
{
	CAdvancedStylesheet *_this;
	QString StylesheetFolder;
	QString OutputDir;
	QMap<QString, QString> StyleVariables;
	QMap<QString, QString> ThemeVariables;
	QString Stylesheet;

	/**
	 * Private data constructor
	 */
	AdvancedStylesheetPrivate(CAdvancedStylesheet *_public);

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
	bool parseThemeVariables(QXmlStreamReader& s, const QString& VariableTagName,
		QMap<QString, QString>& Variable);

	/**
	 * Parse the theme file for
	 */
	bool parseThemeFile(const QString& ThemeFilename);

	/**
	 * Parse the style XML file
	 */
	bool parseStyleXmlFile();

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
};
// struct AdvancedStylesheetPrivate

//============================================================================
AdvancedStylesheetPrivate::AdvancedStylesheetPrivate(
    CAdvancedStylesheet *_public) :
	_this(_public)
{

}


//============================================================================
QString AdvancedStylesheetPrivate::rgbaColor(const QString& RgbColor, float Opacity)
{
	int Alpha = 255 * Opacity;
	auto RgbaColor = RgbColor;
	RgbaColor.insert(1, QString::number(Alpha, 16));
	return RgbaColor;
}


//============================================================================
void AdvancedStylesheetPrivate::replaceStylesheetVariables(QString& Content)
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
			ValueString = _this->themeVariable(Values[0].toString());
			auto OpacityStr = Values[1].mid(OpacityStrSize, Values[1].size() - OpacityStrSize - 1);
			bool Ok;
			auto Opacity = OpacityStr.toFloat(&Ok);
			ValueString = rgbaColor(ValueString, Opacity);
		}
		else
		{
			ValueString = _this->themeVariable(TemplateVariable.toString());
		}

		Content.replace(index, MatchString.size(), ValueString);
		index += ValueString.size();
	}
}


//============================================================================
bool AdvancedStylesheetPrivate::generateStylesheet()
{
	QDir Dir(StylesheetFolder);
	auto TemplateFiles = Dir.entryInfoList({"*.template"}, QDir::Files);
	if (TemplateFiles.count() < 1)
	{
		qDebug() << "Stylesheet folder does not contain a template file";
		return false;
	}

	if (TemplateFiles.count() > 1)
	{
		qDebug() << "Stylesheet folder contains multiple template files";
		return false;
	}

	QFile TemplateFile(TemplateFiles[0].absoluteFilePath());
	TemplateFile.open(QIODevice::ReadOnly);
	QString Content(TemplateFile.readAll());
	replaceStylesheetVariables(Content);
	//std::cout << Content.toStdString() << std::endl;
	Stylesheet = Content;
	exportStylesheet(TemplateFiles[0].baseName() + ".css");
	return true;
}


//============================================================================
void AdvancedStylesheetPrivate::exportStylesheet(const QString& Filename)
{
	QDir().mkpath(OutputDir);
	QString OutputFilename = OutputDir + "/" + Filename;
	QFile OutputFile(OutputFilename);
	OutputFile.open(QIODevice::WriteOnly);
	OutputFile.write(Stylesheet.toUtf8());
	OutputFile.close();
}


//============================================================================
void AdvancedStylesheetPrivate::addFonts(QDir* Dir)
{
	if (!Dir)
	{
		QDir FontsDir(_this->fontsFolder());
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
bool AdvancedStylesheetPrivate::parseThemeVariables(
	QXmlStreamReader& s, const QString& TagName, QMap<QString, QString>& Variables)
{
	while (s.readNextStartElement())
	{
		if (s.name() != TagName)
		{
			qDebug() << "Malformed theme file - expected tag <" << TagName << "> instead of " << s.name();
			return false;
		}
		auto Name = s.attributes().value("name");
		if (Name.isEmpty())
		{
			qDebug() << "Malformed theme file - name attribute missing in <" << TagName << "> tag";
			return false;
		}

		auto Value = s.readElementText(QXmlStreamReader::SkipChildElements);
		if (Value.isEmpty())
		{
			qDebug() << "Malformed theme file - text of <" << TagName << "> tag is empty";
			return false;
		}

		Variables.insert(Name.toString(), Value);
	}

	return true;
}


//============================================================================
bool AdvancedStylesheetPrivate::parseThemeFile(const QString& Theme)
{
	QString ThemeFileName = _this->themesFolder() + "/" + Theme;
	QFile ThemeFile(ThemeFileName);
	ThemeFile.open(QIODevice::ReadOnly);
	QXmlStreamReader s(&ThemeFile);
	s.readNextStartElement();
	if (s.name() != "resources")
	{
		qDebug() << "Malformed theme file - expected tag <resources> instead of " << s.name();
		return false;
	}

    auto Variables = StyleVariables;
	parseThemeVariables(s, "color", Variables);
	this->ThemeVariables = Variables;
	return true;
}


//============================================================================
bool AdvancedStylesheetPrivate::parseStyleXmlFile()
{
	QDir Dir(StylesheetFolder);
	auto TemplateFiles = Dir.entryInfoList({"*.xml"}, QDir::Files);
	if (TemplateFiles.count() < 1)
	{
		qDebug() << "Stylesheet folder does not contain a theme xml file";
		return false;
	}

	if (TemplateFiles.count() > 1)
	{
		qDebug() << "Stylesheet folder contains multiple theme xml files";
		return false;
	}

	QFile ThemeXmlFile(TemplateFiles[0].absoluteFilePath());
	ThemeXmlFile.open(QIODevice::ReadOnly);
	QXmlStreamReader s(&ThemeXmlFile);
	s.readNextStartElement();
	if (s.name() != "style")
	{
		qDebug() << "Malformed style file - expected tag <style> instead of " << s.name();
		return false;
	}

	s.readNextStartElement();
	if (s.name() != "variables")
	{
		qDebug() << "Malformed style file - expected tag <variables> instead of " << s.name();
		return false;
	}


	QMap<QString, QString> Variables;
	auto Result = parseThemeVariables(s, "variable", Variables);
	if (Result)
	{
		StyleVariables = Variables;
	}

	for (auto Var : StyleVariables)
	{
		std::cout << Var.toStdString() << std::endl;
	}

	return true;
}


//============================================================================
CAdvancedStylesheet::CAdvancedStylesheet(const QString& StylesheetFolder) :
	d(new AdvancedStylesheetPrivate(this))
{
	d->StylesheetFolder = StylesheetFolder;
	d->ThemeVariables.insert("primaryColor", "#ffd740");
	d->ThemeVariables.insert("secondaryColor", "#e3e3e3");
	d->ThemeVariables.insert("secondaryLight", "#4f5b62");
}

//============================================================================
CAdvancedStylesheet::~CAdvancedStylesheet()
{
	delete d;
}


//============================================================================
QString CAdvancedStylesheet::resourcesTemplatesFolder() const
{
	return d->StylesheetFolder + "/resources";
}


//============================================================================
QString CAdvancedStylesheet::themesFolder() const
{
	return d->StylesheetFolder + "/themes";
}


//============================================================================
QString CAdvancedStylesheet::fontsFolder() const
{
	return d->StylesheetFolder + "/fonts";
}


//============================================================================
QVector<QStringPair> CAdvancedStylesheet::resourceColorReplaceList(eResourceState State) const
{
	QVector<QStringPair> Result;
	if (ResourceDisabled == State)
	{
		Result << QStringPair("#0000ff", themeVariable("secondaryLightColor"))
			   << QStringPair("#ff0000", themeVariable("secondaryColor"))
			   << QStringPair("#000000", "#ffffff00"); // replace black with transparent
	}
	else
	{
		Result << QStringPair("#0000ff", themeVariable("primaryColor"))
			   << QStringPair("#ff0000", themeVariable("secondaryColor"))
			   << QStringPair("#000000", "#ffffff00"); // replace black with transparent
	}
	return Result;
}


//============================================================================
QString CAdvancedStylesheet::outputDirPath() const
{
	return d->OutputDir;
}


//============================================================================
void CAdvancedStylesheet::setOutputDirPath(const QString& Path)
{
	d->OutputDir = Path;
}


//============================================================================
QString CAdvancedStylesheet::themeVariable(const QString& VariableId) const
{
	return d->ThemeVariables.value(VariableId, QString());
}


//============================================================================
void CAdvancedStylesheet::setThemeVariabe(const QString& VariableId, const QString& Value)
{
	d->ThemeVariables.insert(VariableId, Value);
}


//============================================================================
bool CAdvancedStylesheet::setTheme(const QString& Theme)
{
	auto Result = d->parseStyleXmlFile();
	if (!Result)
	{
		return false;
	}

	Result = d->parseThemeFile(Theme);
	if (!Result)
	{
		return false;
	}

	if (!d->generateStylesheet())
	{
		return false;
	}

	d->addFonts();
	CResourceGenerator ResourceGenerator(this);
	ResourceGenerator.generate();

	QDir::addSearchPath("icon", d->OutputDir);
	return Result;
}


//============================================================================
QString CAdvancedStylesheet::styleSheet() const
{
	return d->Stylesheet;
}
} // namespace acss

//---------------------------------------------------------------------------
// EOF AdvancedStylesheet.cpp
