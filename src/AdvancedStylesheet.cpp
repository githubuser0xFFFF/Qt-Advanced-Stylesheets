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
bool AdvancedStylesheetPrivate::generateStylesheet()
{
	static const int OpacityStrSize = QString("opacity(").size();


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

	QFile Template(TemplateFiles[0].absoluteFilePath());
	Template.open(QIODevice::ReadOnly);
	QString Content(Template.readAll());
	std::cout << Content.toStdString() << std::endl;

	QRegularExpression re("\\{\\{.*\\}\\}");
	QRegularExpressionMatch match;
	int index = 0;
	while ((index = Content.indexOf(re, index, &match)) != -1)
	{
		QString ValueString;
		QString MatchString = match.captured();
		// Use only the value inside of the brackets {{ }} without the brackets
		auto Value = MatchString.midRef(2, MatchString.size() - 4);
		bool HasOpacity = Value.endsWith(')');

		if (HasOpacity)
		{
			auto Values = Value.split("|");
			ValueString = _this->themeVariable(Values[0].toString());
			auto OpacityStr = Values[1].mid(OpacityStrSize, Values[1].size() - OpacityStrSize - 1);
			bool Ok;
			auto Opacity = OpacityStr.toFloat(&Ok);
			std::cout << "Opacity " << Opacity << std::endl;
			ValueString = rgbaColor(ValueString, Opacity);
		}
		else
		{
			ValueString = _this->themeVariable(Value.toString());
		}

		std::cout << index << "  " << Value.toString().toStdString()
			<< " " << HasOpacity << " " << ValueString.toStdString() << std::endl;

		Content.replace(index, MatchString.size(), ValueString);
		index += ValueString.size();
	}

	std::cout << Content.toStdString() << std::endl;
	return true;
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
	auto Result = d->parseThemeFile(Theme);
	d->generateStylesheet();
	return Result;
}
} // namespace acss

//---------------------------------------------------------------------------
// EOF AdvancedStylesheet.cpp
