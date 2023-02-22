#pragma once

#define STR_HELPER(x) #x
#define STR(x) STR_HELPER(x)

#include "gitparams.h"

#define _Falcon_Version_Major 0
#define _Falcon_Version_Minor 814

#define _Falcon_Commit STR(GIT_CUR_COMMIT)
#define _Falcon_ISA "SSE2"

#define _Falcon_Version STR(_Falcon_Version_Major) "." STR(_Falcon_Version_Minor)
#define _Falcon_Version_rc "1.0." _Falcon_Version
#define _Falcon_Build_version _Falcon_Version " " _Falcon_ISA " " _Falcon_Commit
#define _Falcon_FileVersion 1,0,_Falcon_Version_Major,_Falcon_Version_Minor
