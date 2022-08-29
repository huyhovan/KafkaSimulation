#pragma once
#pragma once
#ifdef SCRIPTMODULE_EXPORTS
#	ifndef SCRIPTMODULE_LIB_API
#		define SCRIPTMODULE_LIB_API  __declspec(dllexport)
#	endif
#else
#	ifndef SCRIPTMODULE_LIB_API
#		define SCRIPTMODULE_LIB_API  __declspec(dllimport)
#	endif
#endif
 