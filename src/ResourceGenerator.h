#ifndef ResourceGeneratorH
#define ResourceGeneratorH
//============================================================================
/// \file   ResourceGenerator.h
/// \author Uwe Kindler
/// \date   13.12.2021
/// \brief  Declaration of CResourceGenerator
//============================================================================

//============================================================================
//                                   INCLUDES
//============================================================================
namespace acss
{
class CAdvancedStylesheet;
struct ResourceGeneratorPrivate;

/**
 * The resource generator creates icons with the right colors from a given
 * set of icons
 */
class CResourceGenerator
{
private:
	ResourceGeneratorPrivate* d;
	friend struct ResourceGeneratorPrivate;

public:
	CResourceGenerator(CAdvancedStylesheet* Stylesheet);

	virtual ~CResourceGenerator();

	/**
	 * Generate the resource for the assigned stylesheet
	 */
	void generate();
};

} // namespace acss

//---------------------------------------------------------------------------
#endif // ResourceGeneratorH
