/*************************************************************************
  Crytek Source File.
  Copyright (C), Crytek Studios, 2001-2004.
 -------------------------------------------------------------------------
  $Id$
  $DateTime$
  
 -------------------------------------------------------------------------
  History:
  - 19:6:2006 : Created by Filippo De Luca

*************************************************************************/
#include "StdAfx.h"
#include "Game.h"
#include "Actor.h"
#include "ScriptBind_Actor.h"
#include "ISerialize.h"
#include "GameUtils.h"
#include "Player.h"
#include "GameRules.h"

void SGrabStats::Serialize(TSerialize ser)
{
	if (ser.BeginOptionalGroup("SGrabStats", true))
	{
		//when reading, reset the structure first.
		if (ser.IsReading())
			Reset();

		ser.Value("grabId", grabId);
		ser.Value("dropId", dropId);
		ser.Value("lHoldPos", lHoldPos);
		ser.Value("throwVector", throwVector);
		ser.Value("additionalRotation", additionalRotation);

		ser.Value("limbNum", limbNum);
		for (int i=0; i < limbNum; ++i)
		{
			char limbName[64];
			_snprintf (limbName, 64, "limbId%d", i);

			ser.Value(limbName, limbId[i]);
		}

		ser.Value("resetFlagsDelay", resetFlagsDelay);
		ser.Value("grabDelay", grabDelay);
		ser.Value("throwDelay", throwDelay);
		ser.Value("maxDelay", maxDelay);
		ser.Value("followSpeed", followSpeed);
		ser.Value("useIKRotation", useIKRotation);
		ser.Value("collisionFlags", collisionFlags);
		ser.Value("usingAnimation", usingAnimation);
		ser.Value("usingAnimationForGrab", usingAnimationForGrab);
		ser.Value("usingAnimationForDrop", usingAnimationForDrop);
		ser.ValueChar("carryAnimGraphInput", (char * )carryAnimGraphInput, s_maxAGInputNameLen);
		ser.ValueChar("grabAnimGraphSignal", (char * )grabAnimGraphSignal, s_maxAGInputNameLen);
		ser.ValueChar("dropAnimGraphSignal", (char * )dropAnimGraphSignal, s_maxAGInputNameLen);

		ser.Value("IKActive", IKActive);
		ser.Value("releaseIKTime", releaseIKTime);
		ser.Value("followBoneID", followBoneID);
		ser.Value("followBoneWPos", followBoneWPos);
		ser.Value("grabbedObjOfs", grabbedObjOfs);

		ser.Value("readIkInaccuracyCorrection", readIkInaccuracyCorrection);
		ser.Value("ikInaccuracyCorrection", ikInaccuracyCorrection);

		ser.Value("origRotation", origRotation);
		ser.Value("origEndBoneWorldRotTrans", origEndBoneWorldRot.t);
		ser.Value("origEndBoneWorldRotQuat", origEndBoneWorldRot.q);
		ser.Value("origRotationsValid", origRotationsValid);

		ser.Value("entityGrabSpot", entityGrabSpot);
		ser.Value("boneGrabOffset", boneGrabOffset);

		ser.EndGroup();
	}
}

bool CBaseGrabHandler::Grab(SmartScriptTable &rParams)
{
	SGrabParams params(rParams);
	return true;
}

bool CBaseGrabHandler::Drop(SmartScriptTable &rParams)
{
	SGrabParams params(rParams);
	return true;
}

bool CBaseGrabHandler::SetGrab(SmartScriptTable &rParams)
{
	ScriptHandle id;
	id.n = 0;

	rParams->GetValue("entityId",id);
	rParams->GetValue("holdPos",m_grabStats.lHoldPos);
	rParams->GetValue("followSpeed",m_grabStats.followSpeed);
	
	m_grabStats.collisionFlags = 0;
	rParams->GetValue("collisionFlags",m_grabStats.collisionFlags);

	m_grabStats.grabDelay = 0.0f;
	rParams->GetValue("grabDelay",m_grabStats.grabDelay);

	m_grabStats.throwDelay = 0.0f;
	m_grabStats.maxDelay = m_grabStats.grabDelay;

	m_grabStats.entityGrabSpot = Vec3 (0.0f, 0.0f, 0.0f);
	rParams->GetValue("entityGrabSpot",m_grabStats.entityGrabSpot);

	m_grabStats.boneGrabOffset = Vec3 (0.0f, 0.0f, 0.0f);
	rParams->GetValue("boneGrabOffset",m_grabStats.boneGrabOffset);

	// NOTE Dez 12, 2006: <pvl> the following code was formerly in CBaseGrabHandler::StartGrab().
	if (m_grabStats.grabId<1)
	{
		IEntity *pGrab = gEnv->pEntitySystem->GetEntity(id.n);

		if (pGrab)
		{
			float heavyness(0.0f);
			float volume(0.0f);
			if (!m_pActor->CanPickUpObject(pGrab,heavyness, volume))
				return false;

			if(heavyness == 1.0f) // nanosuit has to be used
			{
				if(m_pActor->GetActorClass() == CPlayer::GetActorClassType())
				{
					CNanoSuit *pSuit = ((CPlayer*)m_pActor)->GetNanoSuit();
					if(pSuit)
						pSuit->SetSuitEnergy(pSuit->GetSuitEnergy()-25.0f);
				}
			}

			m_grabStats.grabId = id.n;
			m_grabStats.additionalRotation = (m_pActor->GetEntity()->GetRotation().GetInverted() * pGrab->GetRotation()).GetNormalized();
			
			//if there is still a link to remove
			if (m_grabStats.dropId>0)
			{
				// NOTE Dez 14, 2006: <pvl> switch collision detection back
				// on for the object to be dropped
				IgnoreCollision(m_grabStats.dropId,m_grabStats.collisionFlags,false);
				m_grabStats.dropId = 0;
			}

			IgnoreCollision(m_grabStats.grabId,m_grabStats.collisionFlags,true);

			return true;
		}
	}
	return false;
}

bool CBaseGrabHandler::SetDrop(SmartScriptTable &rParams)
{
	rParams->GetValue("throwVec",m_grabStats.throwVector);

	int throwImmediately(0);
	rParams->GetValue("throwImmediately",throwImmediately);

	//FIXME:
	if (throwImmediately)
	{
		StartDrop();
	}
	else if (m_grabStats.throwDelay<0.001f)
	{
		rParams->GetValue("throwDelay",m_grabStats.throwDelay);

		m_grabStats.grabDelay = 0.0f;
		m_grabStats.maxDelay = m_grabStats.throwDelay;

		StartDrop();
	}

	return true;
}

bool CBaseGrabHandler::StartGrab(/*EntityId objectId*/)
{
	// NOTE Dez 12, 2006: <pvl> a placeholder
	return true;
}

void CBaseGrabHandler::IgnoreCollision(EntityId eID,unsigned int flags,bool ignore)
{
	IEntity *pGrab = gEnv->pEntitySystem->GetEntity(eID);
	IPhysicalEntity *ppGrab = pGrab ? pGrab->GetPhysics() : NULL;

	if ( !ppGrab) return;

	if (ignore)
	{
		// NOTE Dez 14, 2006: <pvl> this whole block just fills in
		// a request structure and passes it to physics
		IEntity *pEnt = m_pActor->GetEntity();

		pe_action_add_constraint ac;
		ac.flags = constraint_inactive|constraint_ignore_buddy;
		ac.pBuddy = pEnt->GetPhysics();
		ac.pt[0].Set(0,0,0);

		ICharacterInstance *pCharacter = pEnt->GetCharacter(0);
		IPhysicalEntity *pPhysEnt = pCharacter?pCharacter->GetISkeletonPose()->GetCharacterPhysics(-1):NULL;

		if (pPhysEnt)
		{
			pe_simulation_params sp;
			pPhysEnt->GetParams(&sp);
			if (sp.iSimClass <= 2)
				ac.pBuddy = pPhysEnt;
		}

		ppGrab->Action(&ac);
	}
	else
	{
		// NOTE Dez 14, 2006: <pvl> the same as the other branch - just
		// fill in a request and pass it to the physics engine
		pe_action_update_constraint uc;
		uc.bRemove = 1;

		ppGrab->Action(&uc);
	}

	// NOTE Dez 14, 2006: <pvl> flag manipulation is basically a legacy
	// code - probably not used anymore, scheduled for removal.
	if (flags)
	{
		pe_params_part pp;
		pp.flagsAND = pp.flagsColliderAND = ~flags;
		pp.flagsOR = pp.flagsColliderOR = flags * (ignore?0:1);

		pe_status_nparts status_nparts;
		for(pp.ipart = ppGrab->GetStatus(&status_nparts)-1; pp.ipart>=0; pp.ipart--)
			ppGrab->SetParams(&pp);
	}
}

bool CBaseGrabHandler::StartDrop()
{
	IEntity *pGrab = gEnv->pEntitySystem->GetEntity(m_grabStats.grabId);
	IPhysicalEntity *pGrabPhys = pGrab ? pGrab->GetPhysics() : NULL;

	if (pGrabPhys && m_grabStats.throwVector.len2()>0.01f)
	{	
		pe_action_set_velocity asv;
		asv.v = m_grabStats.throwVector;

		asv.w.x = -1.0f + (cry_rand()/(float)RAND_MAX*(float)(2.0f));
		asv.w.y = -1.0f + (cry_rand()/(float)RAND_MAX*(float)(2.0f));
		asv.w.z = -1.0f + (cry_rand()/(float)RAND_MAX*(float)(2.0f));
		asv.w.NormalizeSafe();
		asv.w *= 2.0f;

		pGrabPhys->Action(&asv);
	}

	if (m_grabStats.grabId>0)
	{
		m_grabStats.dropId = m_grabStats.grabId;
		m_grabStats.resetFlagsDelay = 1.0f;

		m_pActor->CreateScriptEvent("droppedObject",m_grabStats.grabId);

		if (pGrab) DisableGrabbedAnimatedCharacter (false);
	}

	m_grabStats.Reset();

	return true;
}

void CBaseGrabHandler::Reset()
{
	m_grabStats.Reset();
}

void CBaseGrabHandler::UpdatePosVelRot(float frameTime)
{
	IEntity *pGrab = gEnv->pEntitySystem->GetEntity(m_grabStats.grabId);
	
	if ( !pGrab) return;

	IEntity *pEnt = m_pActor->GetEntity();

	// NOTE Dez 14, 2006: <pvl> fade away the initial difference between
	// orientations of grabber and grabbed entities, so that they're
	// the same finally.
	m_grabStats.additionalRotation = Quat::CreateSlerp( m_grabStats.additionalRotation, IDENTITY, frameTime * 3.3f );
	pGrab->SetRotation(pEnt->GetRotation() * m_grabStats.additionalRotation,ENTITY_XFORM_USER);

	AABB bbox;
	pGrab->GetLocalBounds(bbox);
	Vec3 grabCenter(pGrab->GetWorldTM() * ((bbox.max + bbox.min) * 0.5f));

	Vec3 grabWPos(GetGrabWPos());
	Vec3 setGrabVel(0,0,0);

	// NOTE Dez 14, 2006: <pvl> grabCenter is where the grabbed object's
	// AABB's center is, grabWPos is where it should be.  Use physics
	// to set the grabbed object's speed towards grabWPos.
	if (pEnt->GetPhysics() && pGrab->GetPhysics())
	{
		pe_status_dynamics dyn;
		pEnt->GetPhysics()->GetStatus(&dyn);

		pe_action_set_velocity asv;
		if (setGrabVel.len2()>0.01f)
			asv.v = dyn.v + setGrabVel;
		else
			asv.v = dyn.v + (grabWPos - grabCenter)*m_grabStats.followSpeed;
		asv.w.Set(0,0,0);
		
		pGrab->GetPhysics()->Action(&asv);
	}

}

void CBaseGrabHandler::Update(float frameTime)
{

	//we have to restore the grabbed object collision flag at some point after the throw.
	if (m_grabStats.dropId>0)
	{
		m_grabStats.resetFlagsDelay -= frameTime;
		if (m_grabStats.resetFlagsDelay<0.001f)
		{
			IgnoreCollision(m_grabStats.dropId,m_grabStats.collisionFlags,false);
			m_grabStats.dropId = 0;
		}
	}

	if (m_grabStats.grabId<1)
		return;

	if (m_pActor->GetHealth()<=0)
	{
		StartDrop();
		return;
	}

	IEntity *pGrab = gEnv->pEntitySystem->GetEntity(m_grabStats.grabId);
		
	if (pGrab)
	{
		bool grabbing(m_grabStats.grabDelay>0.001f);

		if (!grabbing || m_grabStats.usingAnimation)
			UpdatePosVelRot(frameTime);
	}
	else
	{
		//in case the grabber lost the grabbed object, reset the ignore collision flags

		// FIXME Dez 14, 2006: <pvl> the following should be done by
		// calling IgnoreCollisions() but that function requires
		// id of the grabbed object => needs refactoring
		IPhysicalEntity *pActorPhys = m_pActor->GetEntity()->GetPhysics();
		if (pActorPhys)
		{
			pe_action_update_constraint uc;
			uc.bRemove = 1;

			pActorPhys->Action(&uc);
		}
	}

	// TODO Sep 13, 2007: <pvl> I strongly suspect releaseIKTime is redundant now.
	// NOTE Sep 13, 2007: <pvl> note that all timeouts are dodgy here since there's
	// no guarantee that the grabbing/throwing animation starts playing immediately
	// after the grabbing command is issued.
	if (m_grabStats.releaseIKTime>0.001f && m_grabStats.grabDelay < 0.001f)
		m_grabStats.releaseIKTime -= frameTime;
}

Vec3 CBaseGrabHandler::GetGrabWPos()
{
	return m_pActor->GetEntity()->GetSlotWorldTM(0) * m_grabStats.lHoldPos;
}

void CBaseGrabHandler::Serialize(TSerialize ser)
{
	if (ser.BeginOptionalGroup("CBaseGrabHandler", true))
	{
		m_grabStats.Serialize(ser);
		ser.EndGroup();
	}
}

/**
 * Turn the grabbed AnimatedCharacter on/off if necessary.  If the grabbed thing
 * has an AC, it's necessary to turn it off during the grab to prevent it from
 * interfering and messing with its Entity's transformation.
 */
void CBaseGrabHandler::DisableGrabbedAnimatedCharacter (bool enable) const
{
	CActor *pGrabbedActor = (CActor *)g_pGame->GetIGameFramework()->GetIActorSystem()->GetActor(m_grabStats.grabId);
	if ( ! pGrabbedActor) return;

	IAnimatedCharacter * pGrabbedAC = pGrabbedActor->GetAnimatedCharacter();
	if (pGrabbedAC)
		pGrabbedAC->SetNoMovementOverride (enable);

	if(pGrabbedActor->IsClient())
	{
		if(SActorStats *stats = static_cast<SActorStats*>(pGrabbedActor->GetActorStats()))
			stats->isGrabbed = enable;
	}
}

//
bool CAnimatedGrabHandler::SetGrab(SmartScriptTable &rParams)
{
	// NOTE Aug 16, 2007: <pvl> if there's another grab action under way, this one fails
	// first the cheaper check (should also cover the case when output is not yet set because of longer transition time)
	if (m_grabStats.IKActive == true)
		return false;
	// then the more expensive check
	if (IAnimationGraphState* pAGState = m_pActor->GetAnimationGraphState())
	{
		const char* grabActive = pAGState->QueryOutput("GrabActive");
		if (grabActive != NULL && grabActive[0] != 0)
			return false;
	}
	if (m_grabStats.grabId > 0)
		Reset();

	m_grabStats.useIKRotation = false;
	rParams->GetValue("useIKRotation",m_grabStats.useIKRotation);

	m_grabStats.limbNum = 0;

	SmartScriptTable limbsTable;
	if (rParams->GetValue("limbs",limbsTable))
	{
		IScriptTable::Iterator iter = limbsTable->BeginIteration();
	
		while(limbsTable->MoveNext(iter))
		{
			const char *pLimb;
			iter.value.CopyTo(pLimb);

			int limbIdx = m_pActor->GetIKLimbIndex(pLimb);
			if (limbIdx > -1 && m_grabStats.limbNum<GRAB_MAXLIMBS)
				m_grabStats.limbId[m_grabStats.limbNum++] = limbIdx;
		}

		limbsTable->EndIteration(iter);
	}

	m_grabStats.usingAnimation = false;

	const char * pCarryAnimGraphInput = 0;
	if (rParams->GetValue("carryAnimGraphInput",pCarryAnimGraphInput))
	{
		const int maxNameLen = SGrabStats::s_maxAGInputNameLen;
		strncpy(m_grabStats.carryAnimGraphInput,pCarryAnimGraphInput,maxNameLen);
		m_grabStats.carryAnimGraphInput[maxNameLen-1] = 0;
	}
	
	SmartScriptTable animationTable;
	if (rParams->GetValue("animation",animationTable))
	{
		const char *pAnimGraphSignal = NULL;

		if (animationTable->GetValue("animGraphSignal",pAnimGraphSignal))
		{
			const int maxNameLen = SGrabStats::s_maxAGInputNameLen;
			strncpy(m_grabStats.grabAnimGraphSignal,pAnimGraphSignal,maxNameLen);
			m_grabStats.grabAnimGraphSignal[maxNameLen-1] = 0;
		}

		// TODO Dez 15, 2006: <pvl> if there's no graph signal, consider
		// returning false - won't work without graph signal anyway
		if (pAnimGraphSignal)
		{
			m_grabStats.usingAnimation = true;
			m_grabStats.usingAnimationForDrop = true;
			m_grabStats.usingAnimationForGrab = true;
		}

		if (animationTable->GetValue("forceThrow",m_grabStats.throwDelay))
		{
			//m_grabStats.grabDelay = 0.0f;
			m_grabStats.maxDelay = m_grabStats.throwDelay;
		}

		m_grabStats.grabbedObjOfs.zero();
		animationTable->GetValue("grabbedObjOfs",m_grabStats.grabbedObjOfs);

		m_grabStats.releaseIKTime = 0.0f;
		animationTable->GetValue("releaseIKTime",m_grabStats.releaseIKTime);
	} else {
		if (rParams->GetValue("grabAnim",animationTable))
		{
			m_grabStats.usingAnimationForGrab = true;

			m_grabStats.releaseIKTime = 0.0f;
			animationTable->GetValue("releaseIKTime",m_grabStats.releaseIKTime);

			m_grabStats.grabbedObjOfs.zero();
			animationTable->GetValue("grabbedObjOfs",m_grabStats.grabbedObjOfs);

			const char *pAnimGraphSignal = NULL;
			if (animationTable->GetValue("animGraphSignal",pAnimGraphSignal))
			{
				const int maxNameLen = SGrabStats::s_maxAGInputNameLen;
				strncpy(m_grabStats.grabAnimGraphSignal,pAnimGraphSignal,maxNameLen);
				m_grabStats.grabAnimGraphSignal[maxNameLen-1] = 0;
			}
		}
		if (rParams->GetValue("dropAnim",animationTable))
		{
			m_grabStats.usingAnimationForDrop = true;

			// NOTE Feb 10, 2007: <pvl> this is just to get around the
			// condition in CBaseGrabHandler::SetDrop().  If we don't set
			// throwDelay to something bigger than zero SetDrop() executes
			// StartDrop() immediately.
			// All of this stuff around maxDelay, throwDelay etc. should be
			// rewritten but in a way that doesn't break existing behaviour.
			m_grabStats.throwDelay = 1000.0f;

			const char *pAnimGraphSignal = NULL;
			if (animationTable->GetValue("animGraphSignal",pAnimGraphSignal))
			{
				const int maxNameLen = SGrabStats::s_maxAGInputNameLen;
				strncpy(m_grabStats.dropAnimGraphSignal,pAnimGraphSignal,SGrabStats::s_maxAGInputNameLen);
				m_grabStats.dropAnimGraphSignal[maxNameLen] = 0;
			}
		}
		m_grabStats.usingAnimation = m_grabStats.usingAnimationForDrop || m_grabStats.usingAnimationForGrab;
	}

	m_grabStats.followBoneID = -1;

	const char *followBone;
	if (rParams->GetValue("followBone",followBone))
	{
		ICharacterInstance *pCharacter = m_pActor->GetEntity()->GetCharacter(0);
		if (pCharacter)
			m_grabStats.followBoneID = pCharacter->GetISkeletonPose()->GetJointIDByName(followBone);
	}
	// TODO Dez 15, 2006: <pvl> consider returning false if bone ID is -1
	// at this point - it won't work anyway without bone ID
	

	//FIXME:remove this garbage when the grabbing setup is refactored too
	float savedThrowDelay(m_grabStats.throwDelay);
	if ( ! CBaseGrabHandler::SetGrab(rParams))
		return false;

	m_grabStats.additionalRotation.SetIdentity();
	m_grabStats.origRotation.SetIdentity();

	if (m_grabStats.carryAnimGraphInput[0])
	{
		m_pActor->SetAnimationInput("CarryItem",m_grabStats.carryAnimGraphInput);	
	}
	if (m_grabStats.grabAnimGraphSignal[0])
	{
		m_pActor->SetAnimationInput("Signal",m_grabStats.grabAnimGraphSignal);
	}
	if ( ! m_grabStats.usingAnimationForGrab)
	{
		StartGrab();
	}

	m_grabStats.maxDelay = m_grabStats.throwDelay = savedThrowDelay;

	return true;
}

bool CAnimatedGrabHandler::StartGrab()
{
	if (m_grabStats.grabAnimGraphSignal)
	{
		m_pActor->SetAnimationInput("Signal",m_grabStats.grabAnimGraphSignal);
	}
	m_grabStats.grabDelay = 0.0f;
	return true;
}

void CAnimatedGrabHandler::UpdatePosVelRot(float frameTime)
{
	IEntity *pGrab = gEnv->pEntitySystem->GetEntity(m_grabStats.grabId);
	
	if ( !pGrab) return;

	IEntity *pEnt = m_pActor->GetEntity();

	if (m_grabStats.grabDelay<0.001f)
	{
		Vec3 grabWPos (GetGrabBoneWorldTM ().t);

		// NOTE Aug 3, 2007: <pvl> the second part of this test means don't enable
		// the correction if animation/ik wasn't used for grabbing in the first place
		if (m_grabStats.readIkInaccuracyCorrection && m_grabStats.grabAnimGraphSignal[0])
		{
			// NOTE Aug 2, 2007: <pvl> executed the first time this function is called
			// for a particular grabbing action
			m_grabStats.ikInaccuracyCorrection = grabWPos - (pGrab->GetWorldTM().GetTranslation() + m_grabStats.entityGrabSpot);
			m_grabStats.readIkInaccuracyCorrection = false;

			// FIXME Sep 13, 2007: <pvl> only putting it here because it's called just
			// once, at the instant when the object is grabbed - rename readIkInaccuracyCorrection
			// to make this clearer, or put this somewhere else
			DisableGrabbedAnimatedCharacter (true);
		}
		else
		{
			// NOTE Aug 2, 2007: <pvl> phase it out gradually
			m_grabStats.ikInaccuracyCorrection *= 0.9f;
			if (m_grabStats.ikInaccuracyCorrection.len2 () < 0.01f)
				m_grabStats.ikInaccuracyCorrection = Vec3 (0.0f, 0.0f, 0.0f);
		}
		// NOTE Sep 13, 2007: <pvl> this should prevent us from calling SetWPos()
		// later so that the IK "release" phase can take over
		m_grabStats.IKActive = false;

		Matrix34 tm(pGrab->GetWorldTM());
		tm.SetTranslation(grabWPos - (m_grabStats.ikInaccuracyCorrection + pGrab->GetRotation() * m_grabStats.entityGrabSpot));
		pGrab->SetWorldTM(tm,ENTITY_XFORM_USER);
	}

	//update IK
	for (int i=0;i<m_grabStats.limbNum;++i)
	{
		SIKLimb *pLimb = m_pActor->GetIKLimb(m_grabStats.limbId[i]);

		// NOTE Dez 14, 2006: <pvl> this class is always supposed to have
		// m_grabStats.usingAnimation == true
		if (m_grabStats.usingAnimation && m_grabStats.releaseIKTime>0.001f && m_grabStats.IKActive)
		{
			// NOTE Dez 15, 2006: <pvl> use IK to constantly offset the
			// animation so that the difference between where the animation
			// expects the object to be and where the object really is is taken
			// into account.
			Vec3 animPos = pEnt->GetSlotWorldTM(0) * pLimb->lAnimPos;
			Vec3 assumedGrabPos = pEnt->GetSlotWorldTM(0) * m_grabStats.grabbedObjOfs;
			Vec3 actualGrabPos = pGrab->GetWorldPos() + m_grabStats.entityGrabSpot;
			Vec3 adjustment = actualGrabPos - assumedGrabPos;
			pLimb->SetWPos(pEnt,animPos + adjustment,ZERO,0.5f,2.0f,1000);
		}
		
		//if there are multiple limbs, only the first one sets the rotation of the object.
		if (m_grabStats.useIKRotation && i == 0 && m_grabStats.grabDelay<0.001f)
		{
			// NOTE Aug 8, 2007: <pvl> the idea here is to store current world
			// rotations of both the object being grabbed and the end bone of
			// a grabbing limb.  Then track how the end bone rotates with respect
			// to the stored original rotation and rotate the grabbed object
			// the same way.  That way, the grabbed object rotates the same as
			// the limb and appears to be "stabbed" by it.
			QuatT endBoneWorldRot = GetGrabBoneWorldTM ();
			endBoneWorldRot.q.Normalize();	// may not be necessary - just to be safe

			if ( ! m_grabStats.origRotationsValid)
			{
				m_grabStats.origRotation = pGrab->GetRotation();
				m_grabStats.origRotation.Normalize();			// may not be necessary - just to be safe
				m_grabStats.origEndBoneWorldRot = endBoneWorldRot;
				m_grabStats.origRotationsValid = true;
			}

			Quat grabQuat( (endBoneWorldRot*m_grabStats.origEndBoneWorldRot.GetInverted()).q * m_grabStats.origRotation);
			grabQuat.Normalize();
			pGrab->SetRotation(grabQuat,ENTITY_XFORM_USER);
		}
	}

	if (m_grabStats.grabDelay<0.001f)
	{
		// NOTE Sep 16, 2007: <pvl> now that grabbed entity rotation coming from
		// a grabbing bone (if any) is computed, bone-space offset can be applied
		Matrix34 tm(pGrab->GetWorldTM());
		tm.AddTranslation(GetGrabBoneWorldTM().q * m_grabStats.boneGrabOffset);
		pGrab->SetWorldTM(tm,ENTITY_XFORM_USER);
	}
}

bool CAnimatedGrabHandler::SetDrop(SmartScriptTable &rParams)
{
	bool retval = CBaseGrabHandler::SetDrop(rParams);

	if (m_grabStats.carryAnimGraphInput[0])
	{
		m_pActor->SetAnimationInput("CarryItem","none");
	}
	if (m_grabStats.dropAnimGraphSignal[0])
	{
		m_pActor->SetAnimationInput("Signal",m_grabStats.dropAnimGraphSignal);
	}

	if ( ! m_grabStats.usingAnimationForDrop)
	{
		StartDrop();
	}
	return retval;
}


bool CAnimatedGrabHandler::StartDrop()
{
	return CBaseGrabHandler::StartDrop();
}

Vec3 CAnimatedGrabHandler::GetGrabWPos()
{
	return m_grabStats.followBoneWPos;
}

QuatT CAnimatedGrabHandler::GetGrabBoneWorldTM() const
{
	IEntity *pEnt = m_pActor->GetEntity();
	SIKLimb *pLimb = m_pActor->GetIKLimb(m_grabStats.limbId[0]);
	ICharacterInstance *pCharacter = pEnt->GetCharacter(pLimb->characterSlot);

	QuatT followBoneLocalRot = pCharacter->GetISkeletonPose()->GetAbsJointByID(m_grabStats.followBoneID);
	QuatT rootBoneWorldRot = m_pActor->GetAnimatedCharacter()->GetAnimLocation();
	QuatT followBoneWorldRot = rootBoneWorldRot * followBoneLocalRot;
	return followBoneWorldRot;
}

// NOTE Dez 15, 2006: <pvl> not called ATM. Should be moved to BaseGrabHandler.
Vec3 CAnimatedGrabHandler::GetGrabIKPos(IEntity *pGrab,int limbIdx)
{
	AABB bbox;
	pGrab->GetLocalBounds(bbox);

	Vec3 grabCenter((bbox.max + bbox.min) * 0.5f);

	if (m_grabStats.limbNum<2)
	{
	}
	else
	{
		IEntity *pEnt = m_pActor->GetEntity();
		SIKLimb *pLimb = m_pActor->GetIKLimb(m_grabStats.limbId[limbIdx]);
		ICharacterInstance *pCharacter = pEnt->GetCharacter(pLimb->characterSlot);

		if (pCharacter)
		{
			Vec3 posPool[2]={grabCenter,grabCenter};
			posPool[0].x = bbox.min.x;
			posPool[1].x = bbox.max.x;

			Vec3 limbPosInLocal(pGrab->GetWorldTM().GetInverted()*(pEnt->GetSlotWorldTM(pLimb->characterSlot) * pCharacter->GetISkeletonPose()->GetAbsJointByID(pLimb->rootBoneID).t ));

			float minDist(9999.9f);
			for (int i=0;i<2;++i)
			{
				float len2((posPool[i] - limbPosInLocal).len2());
				if (len2<minDist)
				{
					minDist = len2;
					grabCenter = posPool[i];
				}
			}
		}
	}

	return (pGrab->GetWorldTM() * grabCenter);
}

void CAnimatedGrabHandler::ProcessIKLimbs (ICharacterInstance * pCharacter)
{
	if (m_grabStats.grabId>0 && pCharacter && m_grabStats.followBoneID>-1)
	{
		Matrix34 entMtx(m_pActor->GetEntity()->GetSlotWorldTM(0));

		QuatT boneQ(pCharacter->GetISkeletonPose()->GetAbsJointByID(m_grabStats.followBoneID));
		Vec3 bonePos(boneQ.t );
		m_grabStats.followBoneWPos = entMtx * bonePos;
	}
	else
		m_grabStats.followBoneWPos.zero();
}

void CAnimatedGrabHandler::ActivateIK ()
{
	m_grabStats.IKActive = true;
}



bool CMultipleGrabHandler::SetGrab(SmartScriptTable &rParams)
{
	// NOTE Mrz 20, 2007: <pvl> if we don't find 'params' param in the table,
	// we assume that this is an old-style grab param table that's not aware of
	// the possibility of multiple objects grabbed simultaneously.  

	SmartScriptTable grabParamsTable;
	if (rParams->GetValue("params",grabParamsTable))
	{
		bool result = true;

		IScriptTable::Iterator iter = grabParamsTable->BeginIteration();

		while(grabParamsTable->MoveNext(iter))
		{
			CAnimatedGrabHandler * handler = new CAnimatedGrabHandler (m_pActor);
			SmartScriptTable params;
			iter.value.CopyTo (params);
			result = handler->SetGrab(params) && result;
			m_handlers.push_back (handler);
		}

		grabParamsTable->EndIteration(iter);

		return result;
	}
	else
	{
		CAnimatedGrabHandler * handler = new CAnimatedGrabHandler (m_pActor);
		m_handlers.push_back (handler);
		return handler->SetGrab(rParams);
	}
}

bool CMultipleGrabHandler::StartGrab()
{
	bool result = true;

	std::vector <CAnimatedGrabHandler*>::iterator it = m_handlers.begin();
	std::vector <CAnimatedGrabHandler*>::iterator end = m_handlers.end();
	for ( ; it != end; ++it)
		result = (*it)->StartGrab() && result;

	return result;
}

// NOTE Mrz 21, 2007: <pvl> might need to handle the params the way SetGrab()
// does (separate param table for each of the grabbed objects).
// UPDATE Mrz 26, 2007: <pvl> done
bool CMultipleGrabHandler::SetDrop(SmartScriptTable &rParams)
{
	SmartScriptTable dropParamsTable;
	if (rParams->GetValue("params",dropParamsTable))
	{
		bool result = true;

		IScriptTable::Iterator iter = dropParamsTable->BeginIteration();
		int numGrabHandlers = m_handlers.size();

		for (int i=0; dropParamsTable->MoveNext(iter) && i < numGrabHandlers; ++i)
		{
			SmartScriptTable params;
			iter.value.CopyTo (params);
			result = m_handlers[i]->SetDrop(params) && result;
		}

		dropParamsTable->EndIteration(iter);

		return result;
	}
	else
	{
		bool result = true;

		std::vector <CAnimatedGrabHandler*>::iterator it = m_handlers.begin();
		std::vector <CAnimatedGrabHandler*>::iterator end = m_handlers.end();
		for ( ; it != end; ++it)
			result = (*it)->SetDrop(rParams) && result;

		return result;
	}
}

bool CMultipleGrabHandler::StartDrop()
{
	bool result = true;

	std::vector <CAnimatedGrabHandler*>::iterator it = m_handlers.begin();
	std::vector <CAnimatedGrabHandler*>::iterator end = m_handlers.end();
	for ( ; it != end; ++it)
		result = (*it)->StartDrop() && result;

	return result;
}

void CMultipleGrabHandler::Update(float frameTime)
{
	std::vector <CAnimatedGrabHandler*>::iterator it = m_handlers.begin();
	std::vector <CAnimatedGrabHandler*>::iterator end = m_handlers.end();
	for ( ; it != end; ++it)
		(*it)->Update (frameTime);
}

void CMultipleGrabHandler::Reset()
{
	std::for_each (m_handlers.begin(), m_handlers.end(), std::mem_fun (&CBaseGrabHandler::Reset));

	std::vector <CAnimatedGrabHandler*>::iterator it = m_handlers.begin();
	std::vector <CAnimatedGrabHandler*>::iterator end = m_handlers.end();
	for ( ; it != end; ++it)
		delete *it;

	m_handlers.erase (m_handlers.begin (), m_handlers.end ());
}

void CMultipleGrabHandler::Serialize(TSerialize ser)
{
	int numHandlers;
	if (ser.IsWriting())
		numHandlers = m_handlers.size();

	ser.Value("numHandlers", numHandlers);

	if (ser.IsReading())
	{
		for (int i=0; i < numHandlers; ++i)
		{
			m_handlers.push_back (new CAnimatedGrabHandler (m_pActor));
		}
	}

	std::vector <CAnimatedGrabHandler*>::iterator it = m_handlers.begin();
	std::vector <CAnimatedGrabHandler*>::iterator end = m_handlers.end();
	for ( ; it != end; ++it)
		(*it)->Serialize (ser);
}

void CMultipleGrabHandler::ProcessIKLimbs(ICharacterInstance * pCharacter)
{
	std::vector <CAnimatedGrabHandler*>::iterator it = m_handlers.begin();
	std::vector <CAnimatedGrabHandler*>::iterator end = m_handlers.end();
	for ( ; it != end; ++it)
		(*it)->ProcessIKLimbs (pCharacter);
}
