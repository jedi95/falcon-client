/*************************************************************************
Crytek Source File.
Copyright (C), Crytek Studios, 2001-2007.
-------------------------------------------------------------------------
$Id$
$DateTime$
Description: 

-------------------------------------------------------------------------
History:
- 4:23:2007   : Created by Stas Spivakov

*************************************************************************/

#ifndef __VOTING_H__
#define __VOTING_H__

#pragma once

// Summary
//  Types for the different vote states
enum EVotingState
{
  eVS_none=0,//no voting is currently held
  eVS_kick,//kick vote
  eVS_nextMap,//next map vote
  eVS_changeMap,//change map vote
  eVS_consoleCmd,//execute arbitrary console cmd
  eVS_last//this should be always last!
};

struct SVotingParams
{
  SVotingParams();
};

#endif // #ifndef __VOTING_H__