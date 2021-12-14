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
	 * Parse the theme file for
	 */
	bool parseThemeFile(const QString& ThemeFilename);

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
			std::cout << "Opacity " << Opacity << std::endl;
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
	std::cout << Content.toStdString() << std::endl;
	Stylesheet = Content;
	return true;
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
		std::cout << "Folder " << Dir->absolutePath().toStdString() << std::endl;
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
            std::cout << "Font " << FontFilename.toStdString() << std::endl;
			QFontDatabase::addApplicationFont(FontFilename);
		}
	}
}


//============================================================================
bool AdvancedStylesheetPrivate::parseThemeFile(const QString& Theme)
{
	QString ThemeFileName = _this->themesFolder() + "/" + Theme;
	QFile ThemeFile(ThemeFileName);
	ThemeFile.open(QIODevice::ReadOnly);
	QXmlStreamReader s(&ThemeFile);
	QMap<QString, QString> NamedColors;
	s.readNextStartElement();
	if (s.name() != "resources")
	{
		qDebug() << "Malformed theme file - expected tag <resources> instead of " << s.name();
		return false;
	}

	while (s.readNextStartElement())
	{
		if (s.name() != "color")
		{
			qDebug() << "Malformed theme file - expected tag <color> instead of " << s.name();
			return false;
		}
		auto ColorName = s.attributes().value("name");
		if (ColorName.isEmpty())
		{
			qDebug() << "Malformed theme file - name attribute missing in <color> tag";
			return false;
		}

		auto ColorValue = s.readElementText(QXmlStreamReader::SkipChildElements);
		if (ColorValue.isEmpty())
		{
			qDebug() << "Malformed theme file - text of <color> tag is empty";
			return false;
		}

		NamedColors.insert(ColorName.toString(), ColorValue);
	}

	this->ThemeVariables = NamedColors;
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
bool CAdvancedStylesheet::setTheme(const QString& Theme)
{
	d->addFonts();
	auto Result = d->parseThemeFile(Theme);
	if (!d->generateStylesheet())
	{
		return false;
	}

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
