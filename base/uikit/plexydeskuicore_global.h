#ifndef PLEXYDESKUICORE_GLOBAL_H
#define PLEXYDESKUICORE_GLOBAL_H

#include <QtCore/qglobal.h>

#if defined(plexydeskuicore_EXPORTS)
#define PLEXYDESKUICORE_EXPORT Q_DECL_EXPORT
#else
#define PLEXYDESKUICORE_EXPORT Q_DECL_IMPORT
#endif

#endif // PLEXYDESKUICORE_GLOBAL_H