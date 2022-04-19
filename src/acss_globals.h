#ifndef acss_globalsH
#define acss_globalsH
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
/// \file   ACSS_globals.h
/// \author Uwe Kindler
/// \date   19.04.2022
//============================================================================


//============================================================================
//                                   INCLUDES
//============================================================================
#include <QtCore/QtGlobal>
#include <QDebug>


#ifndef ACSS_STATIC
#ifdef ACSS_SHARED_EXPORT
#define ACSS_EXPORT Q_DECL_EXPORT
#else
#define ACSS_EXPORT Q_DECL_IMPORT
#endif
#else
#define ACSS_EXPORT
#endif

// Define ACSS_DEBUG_PRINT to enable a lot of debug output
#ifdef ACSS_DEBUG_PRINT
#define ACSS_PRINT(s) qDebug() << s
#else
#define ACSS_PRINT(s)
#endif

// Set ACSS_DEBUG_LEVEL to enable additional debug output and to enable layout
// dumps to qDebug and std::cout after layout changes
#define ACSS_DEBUG_LEVEL 0



//---------------------------------------------------------------------------
#endif // acss_globalsH
