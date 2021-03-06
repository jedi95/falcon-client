#include "StdAfx.h"
#include "NetPlayerInput.h"
#include "Player.h"
#include "Game.h"
#include "GameCVars.h"


static Vec3 HermiteInterpolate( float s, const Vec3& p1, const Vec3& t1, const Vec3& p2, const Vec3& t2 )
{
	float s2 = s*s;
	float s3 = s2*s;
	float h1 = 2*s3 - 3*s2 + 1.0f;
	float h2 = -2*s3 + 3*s2;
	float h3 = s3 - 2*s2 + s;
	float h4 = s3 - s2;
	return h1*p1 + h2*p2 + h3*t1 + h4*t2;
}

CNetPlayerInput::CNetPlayerInput( CPlayer * pPlayer ) : m_pPlayer(pPlayer)
{
}

void CNetPlayerInput::PreUpdate()
{
	IPhysicalEntity * pPhysEnt = m_pPlayer->GetEntity()->GetPhysics();
	if (!pPhysEnt)
		return;

	CMovementRequest moveRequest;
	SMovementState moveState;
	m_pPlayer->GetMovementController()->GetMovementState(moveState);
	Quat worldRot = m_pPlayer->GetBaseQuat();
	Vec3 deltaMovement = worldRot.GetInverted().GetNormalized() * m_curInput.deltaMovement;
	// absolutely ensure length is correct
	deltaMovement = deltaMovement.GetNormalizedSafe(ZERO) * m_curInput.deltaMovement.GetLength();
	moveRequest.AddDeltaMovement( deltaMovement );
	if(IsDemoPlayback())
	{
		Vec3 localVDir(m_pPlayer->GetViewQuatFinal().GetInverted() * m_curInput.lookDirection);
		Ang3 deltaAngles(asin(localVDir.z),0,cry_atan2f(-localVDir.x,localVDir.y));
		moveRequest.AddDeltaRotation(deltaAngles*gEnv->pTimer->GetFrameTime());
	}

	Vec3 distantTarget = moveState.eyePosition + 1000.0f * m_curInput.lookDirection;
	Vec3 lookTarget = distantTarget;
	if (gEnv->bClient && m_pPlayer->GetGameObject()->IsProbablyVisible())
	{
		// post-process aim direction
		ray_hit hit;
		static const int obj_types = ent_all;
		static const unsigned int flags = rwi_stop_at_pierceable|rwi_colltype_any;
		bool rayHitAny = 0 != gEnv->pPhysicalWorld->RayWorldIntersection( moveState.eyePosition, 150.0f * m_curInput.lookDirection, obj_types, flags, &hit, 1, pPhysEnt );
		if (rayHitAny)
		{
			lookTarget = hit.pt;
		}

		static float proneDist = 1.0f;
		static float crouchDist = 0.6f;
		static float standDist = 0.3f;

		float dist = standDist;
		if(m_pPlayer->GetStance() == STANCE_CROUCH)
			dist = crouchDist;
		else if(m_pPlayer->GetStance() == STANCE_PRONE)
			dist = proneDist;

		if((lookTarget - moveState.eyePosition).GetLength2D() < dist)
		{
			Vec3 eyeToTarget2d = lookTarget - moveState.eyePosition;
			eyeToTarget2d.z = 0.0f;
			eyeToTarget2d.NormalizeSafe();
			eyeToTarget2d *= dist;
			ray_hit newhit;
			bool rayHitAny = 0 != gEnv->pPhysicalWorld->RayWorldIntersection( moveState.eyePosition + eyeToTarget2d, 3 * Vec3(0,0,-1), obj_types, flags, &newhit, 1, pPhysEnt );
			if (rayHitAny)
			{
				lookTarget = newhit.pt;
			}
		}

		// SNH: new approach. Make sure the aimTarget is at least 1.5m away,
		//	if not, pick a point 1m down the vector instead.
		Vec3 dir = lookTarget - moveState.eyePosition;
		static float minDist = 1.5f;
		if(dir.GetLengthSquared() < minDist)
		{
			lookTarget = moveState.eyePosition + dir.GetNormalizedSafe();
		}
	}

	moveRequest.SetLookTarget( lookTarget );
	moveRequest.SetAimTarget( lookTarget );
	if (m_curInput.deltaMovement.GetLengthSquared() > sqr(0.2f)) // 0.2f is almost stopped
		moveRequest.SetBodyTarget( distantTarget );
	else
		moveRequest.ClearBodyTarget();

	moveRequest.SetAllowStrafing(true);

	float pseudoSpeed = 0.0f;
	if (m_curInput.deltaMovement.len2() > 0.0f)
	{
		pseudoSpeed = m_pPlayer->CalculatePseudoSpeed(m_curInput.sprint);
	}
	moveRequest.SetPseudoSpeed(pseudoSpeed);

	float lean=0.0f;
	if (m_curInput.leanl)
		lean-=1.0f;
	if (m_curInput.leanr)
		lean+=1.0f;

	if (fabsf(lean)>0.01f)
		moveRequest.SetLean(lean);
	else
		moveRequest.ClearLean();

	m_pPlayer->GetMovementController()->RequestMovement(moveRequest);

	if (m_curInput.sprint)
		m_pPlayer->m_actions |= ACTION_SPRINT;
	else
		m_pPlayer->m_actions &= ~ACTION_SPRINT;

	if (m_curInput.leanl)
		m_pPlayer->m_actions |= ACTION_LEANLEFT;
	else
		m_pPlayer->m_actions &= ~ACTION_LEANLEFT;

	if (m_curInput.leanr)
		m_pPlayer->m_actions |= ACTION_LEANRIGHT;
	else
		m_pPlayer->m_actions &= ~ACTION_LEANRIGHT;
}

void CNetPlayerInput::Update()
{
	if (gEnv->bServer && (g_pGameCVars->sv_input_timeout>0) && ((gEnv->pTimer->GetFrameStartTime()-m_lastUpdate).GetMilliSeconds()>=g_pGameCVars->sv_input_timeout))
	{
		m_curInput.deltaMovement.zero();
		m_curInput.sprint=m_curInput.leanl=m_curInput.leanr=false;
		m_curInput.stance=STANCE_NULL;

		m_pPlayer->GetGameObject()->ChangedNetworkState( INPUT_ASPECT );
	}
}

void CNetPlayerInput::PostUpdate()
{
}

void CNetPlayerInput::SetState( const SSerializedPlayerInput& input )
{
	DoSetState(input);

	m_lastUpdate = gEnv->pTimer->GetCurrTime();
}

void CNetPlayerInput::GetState( SSerializedPlayerInput& input )
{
	input = m_curInput;
}

void CNetPlayerInput::Reset()
{
	SSerializedPlayerInput i(m_curInput);
	i.leanl=i.leanr=i.sprint=false;
	i.deltaMovement.zero();

	DoSetState(i);

	m_pPlayer->GetGameObject()->ChangedNetworkState(IPlayerInput::INPUT_ASPECT);
}

void CNetPlayerInput::DisableXI(bool disabled)
{
}


void CNetPlayerInput::DoSetState(const SSerializedPlayerInput& input )
{
	m_curInput = input;
	m_pPlayer->GetGameObject()->ChangedNetworkState( INPUT_ASPECT );

	CMovementRequest moveRequest;
	moveRequest.SetStance( (EStance)m_curInput.stance );

	if(IsDemoPlayback())
	{
		Vec3 localVDir(m_pPlayer->GetViewQuatFinal().GetInverted() * m_curInput.lookDirection);
		Ang3 deltaAngles(asin(localVDir.z),0,cry_atan2f(-localVDir.x,localVDir.y));
		moveRequest.AddDeltaRotation(deltaAngles*gEnv->pTimer->GetFrameTime());
	}

	moveRequest.SetLookTarget( m_pPlayer->GetEntity()->GetWorldPos() + 10.0f * m_curInput.lookDirection );
	moveRequest.SetAimTarget(moveRequest.GetLookTarget());

	float pseudoSpeed = 0.0f;
	if (m_curInput.deltaMovement.len2() > 0.0f)
	{
		pseudoSpeed = m_pPlayer->CalculatePseudoSpeed(m_curInput.sprint);
	}
	moveRequest.SetPseudoSpeed(pseudoSpeed);
	moveRequest.SetAllowStrafing(true);

	float lean=0.0f;
	if (m_curInput.leanl)
		lean-=1.0f;
	if (m_curInput.leanr)
		lean+=1.0f;
	moveRequest.SetLean(lean);

	m_pPlayer->GetMovementController()->RequestMovement(moveRequest);
}
