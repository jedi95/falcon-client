#ifndef __FALCON_H__
#define __FALCON_H__

#include "FalconVersion.h"

#pragma once

#define FalconConsoleText(text) "$5[Falcon]$9 " ## text
#define FalconLogInfo(format, ...) CryLogAlways(FalconConsoleText(format), ##__VA_ARGS__)
#define FalconLogWarning(format, ...) CryLogAlways("$5[Falcon]$6[Warning]$9 " ## format, ##__VA_ARGS__)
#define FalconLogError(format, ...) CryLogAlways("$5[Falcon]$7[Error]$9 " ## format, ##__VA_ARGS__)

class Falcon
{
public:
	static string GetVersion() { return _Falcon_Version; }
	static string GetBuildVersion() { return _Falcon_Build_version; }
};

#endif // __FALCON_H__
