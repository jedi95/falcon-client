/*************************************************************************
Crytek Source File.
Copyright (C), Crytek Studios, 2001-2006.
-------------------------------------------------------------------------
$Id$
$DateTime$

-------------------------------------------------------------------------
History:
- 5:7:2006   16:01 : Created by Márcio Martins

*************************************************************************/
#include "StdAfx.h"
#include "SynchedStorage.h"
#include <IEntitySystem.h>


//------------------------------------------------------------------------
void CSynchedStorage::Reset()
{
	m_globalStorage.clear();
	m_entityStorage.clear();
	m_channelStorageMap.clear();
	m_channelStorage.clear();
}

//------------------------------------------------------------------------
void CSynchedStorage::SerializeValue(TSerialize ser, TSynchedKey &key, TSynchedValue &value, int type)
{
	ser.Value("key", key, 'ssk');

	switch (type)
	{
	case eSVT_Bool:
		{
			bool b;
			if (ser.IsWriting())
				b=*value.GetPtr<bool>();
			ser.Value("value", b, 'bool');
			if (ser.IsReading())
				SetGlobalValue(key, b);
		}
		break;
	case eSVT_Float:
		{
			float f;
			if (ser.IsWriting())
				f=*value.GetPtr<float>();
			ser.Value("value", f, 'ssfl');
			if (ser.IsReading())
				SetGlobalValue(key, f);
		}
		break;
	case eSVT_Int:
		{
			int i;
			if (ser.IsWriting())
				i=*value.GetPtr<int>();
			ser.Value("value", i, 'ssi');
			if (ser.IsReading())
				SetGlobalValue(key, i);
		}
		break;
	case eSVT_EntityId:
		{
			EntityId e;
			if (ser.IsWriting())
				e=*value.GetPtr<EntityId>();
			ser.Value("value", e, 'eid');
			if (ser.IsReading())
				SetGlobalValue(key, e);
		}
	case eSVT_String:
		{
			static string s;
			s.resize(0);
			if (ser.IsWriting())
				s=*value.GetPtr<string>();
			ser.Value("value", s);
			if (ser.IsReading())
				SetGlobalValue(key, s);
		}
	break;
	default:
		break;
	}
}

//------------------------------------------------------------------------
void CSynchedStorage::SerializeEntityValue(TSerialize ser, EntityId id, TSynchedKey &key, TSynchedValue &value, int type)
{
	ser.Value("key", key, 'ssk');

	switch (type)
	{
	case eSVT_Bool:
		{
			bool b;
			if (ser.IsWriting())
				b=*value.GetPtr<bool>();
			ser.Value("value", b, 'bool');
			if (ser.IsReading())
				SetEntityValue(id, key, b);
		}
		break;
	case eSVT_Float:
		{
			float f;
			if (ser.IsWriting())
				f=*value.GetPtr<float>();
			ser.Value("value", f, 'ssfl');
			if (ser.IsReading())
				SetEntityValue(id, key, f);
		}
		break;
	case eSVT_Int:
		{
			int i;
			if (ser.IsWriting())
				i=*value.GetPtr<int>();
			ser.Value("value", i, 'ssi');
			if (ser.IsReading())
				SetEntityValue(id, key, i);
		}
		break;
	case eSVT_EntityId:
		{
			EntityId e;
			if (ser.IsWriting())
				e=*value.GetPtr<EntityId>();
			ser.Value("value", e, 'eid');
			if (ser.IsReading())
				SetEntityValue(id, key, e);
		}
	case eSVT_String:
		{
			static string s;
			s.resize(0);
			if (ser.IsWriting())
				s=*value.GetPtr<string>();
			ser.Value("value", s);
			if (ser.IsReading())
				SetEntityValue(id, key, s);
		}
		break;
	default:
		break;
	}
}

//------------------------------------------------------------------------
CSynchedStorage::TStorage *CSynchedStorage::GetEntityStorage(EntityId id, bool create)
{
	TEntityStorageMap::iterator it=m_entityStorage.find(id);
	if (it!=m_entityStorage.end())
		return &it->second;
	else if (create)
	{
		std::pair<TEntityStorageMap::iterator, bool> result=m_entityStorage.insert(
			TEntityStorageMap::value_type(id, TStorage()));
		return &result.first->second;
	}

	return 0;
}

//------------------------------------------------------------------------
CSynchedStorage::TStorage *CSynchedStorage::GetChannelStorage(int channelId, bool create)
{
	TChannelStorageMap::iterator it=m_channelStorageMap.find(channelId);
	if (it!=m_channelStorageMap.end())
		return &it->second;
	else
	{
		INetChannel *pNetChannel=m_pGameFramework->GetNetChannel(channelId);
		if ((!gEnv->bServer && !pNetChannel) || (gEnv->bServer && pNetChannel->IsLocal()))
			return &m_channelStorage;

		if (gEnv->bServer && create)
		{
			std::pair<TChannelStorageMap::iterator, bool> result=m_channelStorageMap.insert(
				TEntityStorageMap::value_type(channelId, TStorage()));
			return &result.first->second;
		}
	}

	return 0;
}


static void AddStorageTo( const CSynchedStorage::TStorage& stor, ICrySizer * s )
{
	for (CSynchedStorage::TStorage::const_iterator iter = stor.begin(); iter != stor.end(); ++iter)
	{
		s->Add(iter->first);
		int nSize = iter->second.GetMemorySize();
		s->AddObject( ((const char*)(iter->first)+1),nSize );
	}
}

void CSynchedStorage::GetStorageMemoryStatistics(ICrySizer * s)
{
	AddStorageTo(m_globalStorage, s);
	for (TEntityStorageMap::const_iterator iter = m_entityStorage.begin(); iter != m_entityStorage.end(); ++iter)
	{
		s->Add(*iter);
		AddStorageTo(iter->second, s);
	}
}
