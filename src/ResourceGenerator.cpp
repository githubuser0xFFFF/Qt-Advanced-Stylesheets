//============================================================================
/// \file   ResourceGenerator.cpp
/// \author Uwe Kindler
/// \date   13.12.2021
/// \brief  Implementation of CResourceGenerator
//============================================================================

//============================================================================
//                                   INCLUDES
//============================================================================
#include "ResourceGenerator.h"
#include "AdvancedStylesheet.h"

#include <iostream>

#include <QDir>
#include <QStringList>
#include <QFileInfo>

namespace acss
{
/**
 * Private data class pimpl
 */
struct ResourceGeneratorPrivate
{
	CResourceGenerator* _this;
	CAdvancedStylesheet* Stylesheet;

	ResourceGeneratorPrivate(CResourceGenerator* _public) : _this(_public) {}

	/**
	 * Replace the in the given content the template color string with the
	 * theme color string
	 */
	void replaceColor(QByteArray& Content, const QString& TemplateColor,
		const QString& ThemeColor) const;

	/**
	 * Generate the output for normal or disabled state
	 */
	void generate(const QString& SubDir, CAdvancedStylesheet::eResourceState State,
		const QFileInfoList& Entries);
};


//============================================================================
void ResourceGeneratorPrivate::replaceColor(QByteArray& Content,
	const QString& TemplateColor, const QString& ThemeColor) const
{
	//std::cout << "replaceColor: " << TemplateColor.toStdString()
	//	<< " with " << ThemeColor.toStdString() << std::endl;
	Content.replace(TemplateColor.toLatin1(), ThemeColor.toLatin1());
}


//============================================================================
void ResourceGeneratorPrivate::generate(const QString& SubDir,
	CAdvancedStylesheet::eResourceState State, const QFileInfoList& Entries)
{
	auto ColorReplaceList = Stylesheet->resourceColorReplaceList(State);
	const QString OutputDir = Stylesheet->outputDirPath() + "/" + SubDir;
	QDir().mkpath(OutputDir);
	for (const auto& Entry : Entries)
	{
		//std::cout << "File: " << Entry.absoluteFilePath().toStdString() << std::endl;
		QFile SvgFile(Entry.absoluteFilePath());
		SvgFile.open(QIODevice::ReadOnly);
		auto Content = SvgFile.readAll();
		SvgFile.close();


		for (const auto& Replace : ColorReplaceList)
		{
			replaceColor(Content, Replace.first, Replace.second);
		}

		QString OutputFilename = OutputDir + "/" + Entry.fileName();
		//std::cout << "OutputFilename: " << OutputFilename.toStdString() << std::endl;
		QFile OutputFile(OutputFilename);
		OutputFile.open(QIODevice::WriteOnly);
		OutputFile.write(Content);
		OutputFile.close();
	}
}


//============================================================================
CResourceGenerator::CResourceGenerator(CAdvancedStylesheet* Stylesheet)
	: d(new ResourceGeneratorPrivate(this))
{
	d->Stylesheet = Stylesheet;
}


//============================================================================
CResourceGenerator::~CResourceGenerator()
{
	delete d;
}


//============================================================================
void CResourceGenerator::generate()
{
	QDir ResourceDir(d->Stylesheet->resourcesTemplatesFolder());
	auto Entries = ResourceDir.entryInfoList({"*.svg"}, QDir::Files, QDir::NoSort);
	d->generate("disabled", CAdvancedStylesheet::ResourceDisabled, Entries);
	d->generate("primary", CAdvancedStylesheet::ResourceNormal, Entries);
}

} // namespace acss

//---------------------------------------------------------------------------
// EOF ResourceGenerator.cpp
