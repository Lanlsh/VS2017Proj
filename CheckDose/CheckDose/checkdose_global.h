#pragma once

#include <QtCore/qglobal.h>

#ifndef BUILD_STATIC
# if defined(CHECKDOSE_LIB)
#  define CHECKDOSE_EXPORT Q_DECL_EXPORT
# else
#  define CHECKDOSE_EXPORT Q_DECL_IMPORT
# endif
#else
# define CHECKDOSE_EXPORT
#endif
