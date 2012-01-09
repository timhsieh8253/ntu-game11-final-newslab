#include "Lyubu.h"
#include "utils.h"

#include "FX.h"

#include <queue>

using namespace std;

extern MyAttackQueue AttackList;
extern FX *fx;

Lyubu::Lyubu(OBJECTid id, OBJECTid cid, OBJECTid tid, OBJECTid aid, int bid) : FnActor()
{
	this->cameraID = cid;
	this->terrainID = tid;
	this->arrowID = aid;
	this->arrow_billboardID = bid;
	this->minHeight = 500.0f;
	this->maxDistance = 1000.0f;
	this->blood = 40.0f;
	this->selfID = id;
}

void Lyubu::init()
{

	float pos[3] = {3650.0f, -3345.0f, 0.f};
	float fdir[3] = {0.0f, 1.0f, 0.0f}, udir[3] = {0.0f, 0.0f, 1.0f};
	this->SetPosition(pos);
	this->SetDirection(fdir, udir);
	this->TurnRight(340);

	if(!this->PutOnTerrain(terrainID, FALSE, 0.0f))
	   exit(1);

	this->ResetCamera();

	// Load Actions
	idleID = this->GetBodyAction(NULL, "CombatIdle");
	runID = this->GetBodyAction(NULL, "Run");
	nAtt1ID = this->GetBodyAction(NULL, "NormalAttack1");
	nAtt2ID = this->GetBodyAction(NULL, "NormalAttack2");
	nAtt3ID = this->GetBodyAction(NULL, "NormalAttack3");
	nAtt4ID = this->GetBodyAction(NULL, "NormalAttack4");
	hittedID = this->GetBodyAction(NULL, "HeavyDamaged");
	dieID = this->GetBodyAction(NULL, "Die");

	// Default Action is Idle
	this->state.set(LYUBU_IDLE);
	this->MakeCurrentAction(0, NULL, this->idleID);
	this->Play(0, START, 0.0f, FALSE, TRUE);

	this->nextFrame = &Lyubu::IdleFunction;

	// create blood bar
	OBJECTid sID = this->GetScene();
	FnScene scene;
	scene.Object(sID);
	bloodID = scene.CreateObject(ROOT);
	FnObject blood;
	blood.Object(bloodID);
	blood.SetParent(this->GetBaseObject());
	float size[2], color[3];
	pos[0] = 0.0f;
	pos[1] = 0.0f;
	pos[2] = 100.0f;

	size[0] = this->blood*1;
	size[1] = 5.0f;
	//pos[0]-=fullblood-size[0];
	color[0] = 1.0f; color[1] = color[2] = 0.0f;

	blood_billboardID = blood.Billboard(pos, size, NULL, 0, color);

}

bool Lyubu::Isdead(){
	return (blood <= 0);
}

void Lyubu::changeState(BYTE code, BOOL value)
{
	if(state.contain(LYUBU_ATT))
	{
		switch(code)
		{
			case FY_W:
			case FY_UP:
				if(value)
					state.add(LYUBU_UP);
				else
					state.remove(LYUBU_UP);
				break;
			case FY_A:
			case FY_LEFT:
				if(value)
					state.add(LYUBU_LEFT);
				else
					state.remove(LYUBU_LEFT);
				break;
			case FY_S:
			case FY_DOWN:
				if(value)
					state.add(LYUBU_DOWN);
				else
					state.remove(LYUBU_DOWN);
				break;
			case FY_D:
			case FY_RIGHT:
				if(value)
					state.add(LYUBU_RIGHT);
				else
					state.remove(LYUBU_RIGHT);
				break;
		}
	}
	else
	{
		switch(code)
		{
			case FY_J:
				if(value)
				{
					fx->Attack1("lyubu_attack", *this);
					state.add(LYUBU_ATT);
					curAttID = nAtt1ID;

				}
				break;
			case FY_K:
				if(value)
				{
					fx->Attack2("lyubu_attack", *this);
					state.add(LYUBU_ATT);
					curAttID = nAtt2ID;
				}
				break;
			case FY_L:
				if(value)
				{
					state.add(LYUBU_ATT);
					curAttID = nAtt3ID;
				}
				break;
			case FY_I:
				if(value)
				{
					fx->Attack4("lyubu_attack", *this);
					state.add(LYUBU_ATT);
					curAttID = nAtt4ID;
				}
				break;
			case FY_W:
			case FY_UP:
				if(value)
					state.add(LYUBU_UP);
				else
					state.remove(LYUBU_UP);
				break;
			case FY_A:
			case FY_LEFT:
				if(value)
					state.add(LYUBU_LEFT);
				else
					state.remove(LYUBU_LEFT);
				break;
			case FY_S:
			case FY_DOWN:
				if(value)
					state.add(LYUBU_DOWN);
				else
					state.remove(LYUBU_DOWN);
				break;
			case FY_D:
			case FY_RIGHT:
				if(value)
					state.add(LYUBU_RIGHT);
				else
					state.remove(LYUBU_RIGHT);
				break;
		}
		if(!state.contain(LYUBU_HITTED))
			MakeAction();
	}


}

void Lyubu::MakeAction()
{
	switch(state.get() & 15)
	{
		case LYUBU_UP:
			face_target = 0;
			break;
		case LYUBU_LEFT:
			face_target = 270;
			break;
		case LYUBU_DOWN:
			face_target = 180;
			break;
		case LYUBU_RIGHT:
			face_target = 90;
			break;
		case (LYUBU_UP | LYUBU_LEFT):
			face_target = 315;
			break;
		case (LYUBU_UP | LYUBU_RIGHT):
			face_target = 45;
			break;
		case (LYUBU_DOWN | LYUBU_RIGHT):
			face_target = 135;
			break;
		case (LYUBU_DOWN | LYUBU_LEFT):
			face_target = 225;
			break;
		case (LYUBU_LEFT | LYUBU_RIGHT):
			face_target = 270;
			break;
	}

	if(state.contain(LYUBU_ATT))
	{
		if(this->curActID != this->curAttID)
		{
			this->MakeCurrentAction(0, NULL, this->curAttID);
			this->curActID = this->curAttID;
			this->Play(0, START, 0.0f, FALSE, TRUE);

			AttackEvent ae;
			ae.actor = this;
			if(curAttID == nAtt1ID)
			{
				ae.length = 150;
				ae.width = 30;
				ae.damage = 1;
				ae.delay = 7;
			}
			else if(curAttID == nAtt4ID)
			{
				ae.length = 150;
				ae.width = 30;
				ae.damage = 4;
				ae.delay = 15;
			}
			else
			{
				ae.length = 150;
				ae.width = 150;
				ae.damage = 2;
				ae.delay = 20;
			}


			AttackList.push(ae);

		}
		this->nextFrame = &Lyubu::AttackFunction;
	}
	else if(state.contain(LYUBU_UP))
	{
		if(this->curActID != this->runID)
		{
			this->MakeCurrentAction(0, NULL, this->runID);
			this->curActID = this->runID;
			this->Play(0, START, 0.0f, FALSE, TRUE);
		}
		this->nextFrame = &Lyubu::UpFunction;
	}
	else if( state.contain(LYUBU_DOWN) )
	{
		if(this->curActID != this->runID)
		{
			this->MakeCurrentAction(0, NULL, this->runID);
			this->curActID = this->runID;
			this->Play(0, START, 0.0f, FALSE, TRUE);
		}
		this->nextFrame = &Lyubu::DownFunction;
	}
	else if( state.contain(LYUBU_LEFT) || state.contain(LYUBU_RIGHT) )
	{
		if(this->curActID != this->runID)
		{
			this->MakeCurrentAction(0, NULL, this->runID);
			this->curActID = this->runID;
			this->Play(0, START, 0.0f, FALSE, TRUE);
		}
		this->nextFrame = &Lyubu::LeftRightFunction;
	}
	else if( state.get() == LYUBU_IDLE )
	{
		this->MakeCurrentAction(0, NULL, this->idleID);
		this->curActID = this->idleID;
		this->Play(0, START, 0.0f, FALSE, TRUE);
		this->nextFrame = &Lyubu::IdleFunction;
	}
}

void Lyubu::ResetCamera()
{
	float fdir[3], udir[3];
	float pos[3], pos2[3], cdir[3];
	float cross1[3], cross2[3];

	FnCamera camera;
	camera.Object(cameraID);

	this->GetPosition(pos);
	this->GetDirection(fdir, udir);
	FVector::CrossProduct(udir, fdir, cross1);
	pos2[0] = pos[0] - fdir[0]*(this->maxDistance) + udir[0]*(this->minHeight);
	pos2[1] = pos[1] - fdir[1]*(this->maxDistance) + udir[1]*(this->minHeight);
	pos2[2] = pos[2] - fdir[2]*(this->maxDistance) + udir[2]*(this->minHeight);
	this->Distance = this->maxDistance;
	this->Height = this->minHeight;
	FVector::Minus(pos, pos2, cdir);
	FVector::GetUnit(cdir, cdir);
	FVector::CrossProduct(cdir, cross1, cross2);
	camera.SetPosition(pos2);
	camera.SetDirection(cdir, cross2);
}

void Lyubu::IdleFunction(int skip)
{
	this->Play(0, LOOP, (float)skip, FALSE, TRUE);
}


void Lyubu::UpFunction(int skip)
{
	this->turn();
	this->Play(0, LOOP, (float)skip, FALSE, TRUE);

	FnCamera camera;
	camera.Object(this->cameraID);

	float lfdir[3], ludir[3];
	float cfdir[3], cudir[3];
	this->GetDirection(lfdir, ludir);
	camera.GetDirection(cfdir, cudir);

	float pos_lyubu[3], pos_camera[3], walk[3], walkdist;
	float newpos_lyubu[3];
	float height, dist, angle;

	this->GetPosition(pos_lyubu);
	camera.GetPosition(pos_camera);
	height = fabs(pos_camera[2] - pos_lyubu[2]);
	dist = sqrt(pow(pos_camera[0] - pos_lyubu[0],2) + pow(pos_camera[1] - pos_lyubu[1],2));
	angle = acos(height / sqrt(pow(maxDistance,2) + pow(minHeight, 2)));

	this->MoveForward(10, TRUE, TRUE, 0, TRUE);
	this->GetPosition(newpos_lyubu);
	walkdist = FVector::ComputeDistance(pos_lyubu,newpos_lyubu);
	FVector::Minus(newpos_lyubu, pos_lyubu, walk);


	/* 將抬高的camera降下來 */
	FnTerrain terrain;
	float origin[3] = {pos_camera[0], pos_camera[1], newpos_lyubu[2]};
	terrain.Object(this->terrainID);
	if(height > minHeight)
	{

		Distance = dist + walkdist;
		float new_height = sqrt(pow(maxDistance,2) + pow(minHeight, 2) - pow(Distance, 2));
		float new_angle = acos(new_height / sqrt(pow(maxDistance,2) + pow(minHeight, 2)));
		float axis[3];

		pos_camera[2] = new_height + newpos_lyubu[2];
		FVector::CrossProduct(ludir, cfdir, axis);
		FVector::Minus(newpos_lyubu, pos_camera, cfdir);
		FVector::CrossProduct(cfdir, axis, cudir);
		camera.SetPosition(pos_camera);
		camera.SetDirection(cfdir, cudir);
	}
	else
	{
		FVector::Add(walk, pos_camera, pos_camera);
		camera.SetPosition(pos_camera);
	}
}

void Lyubu::DownFunction(int skip)
{
	this->turn();
	this->Play(0, LOOP, (float)skip, FALSE, TRUE);

	FnCamera camera;
	camera.Object(this->cameraID);

	float lfdir[3], ludir[3];
	float cfdir[3], cudir[3];
	this->GetDirection(lfdir, ludir);
	camera.GetDirection(cfdir, cudir);

	float pos_lyubu[3], pos_camera[3], walk[3], walkdist;
	float newpos_lyubu[3];
	float height, dist, angle;

	this->GetPosition(pos_lyubu);
	camera.GetPosition(pos_camera);
	height = fabs(pos_camera[2] - pos_lyubu[2]);
	dist = sqrt(pow(pos_camera[0] - pos_lyubu[0],2) + pow(pos_camera[1] - pos_lyubu[1],2));
	angle = acos(height / sqrt(pow(maxDistance,2) + pow(minHeight, 2)));

	this->MoveForward(10, TRUE, TRUE, 0, TRUE);
	this->GetPosition(newpos_lyubu);
	walkdist = FVector::ComputeDistance(pos_lyubu,newpos_lyubu);
	FVector::Minus(newpos_lyubu, pos_lyubu, walk);

	/* camera撞牆，將camera向上抬 */

	FnTerrain terrain;
	float hitPos[3], origin[3] = {pos_camera[0], pos_camera[1], newpos_lyubu[2]};
	float hitDis;
	terrain.Object(this->terrainID);
	float hitNorm[3];
	if(terrain.HitTest(origin,lfdir,hitPos,FALSE, NULL, hitNorm))
	{

		hitDis = FVector::ComputeDistance(origin, hitPos);
		// camera up
		if( hitDis < walkdist){
			//hit_tag = 1;
			float axis[3];

			Distance = sqrt(pow(pos_camera[0] - pos_lyubu[0],2) + pow(pos_camera[1] - pos_lyubu[1],2));
			float new_height = sqrt(pow(maxDistance,2) + pow(minHeight, 2) - pow(Distance, 2));
			float new_angle = acos(new_height / sqrt(pow(maxDistance,2) + pow(minHeight, 2)));

			pos_camera[2] = new_height + newpos_lyubu[2];
			FVector::CrossProduct(ludir, cfdir, axis);
			FVector::Minus(newpos_lyubu, pos_camera, cfdir);
			FVector::CrossProduct(cfdir, axis, cudir);
			camera.SetPosition(pos_camera);
			camera.SetDirection(cfdir, cudir);
		}
		else
		{
			//hit_tag = 0;
			FVector::Add(walk, pos_camera, pos_camera);
			camera.SetPosition(pos_camera);
		}
	}
	else
	{
		//hit_tag = 0;
		FVector::Add(walk, pos_camera, pos_camera);
		camera.SetPosition(pos_camera);
	}
}

void Lyubu::LeftRightFunction(int skip)
{
	this->turn();
	this->Play(0, LOOP, (float)skip, FALSE, TRUE);

	FnCamera camera;
	camera.Object(this->cameraID);

	float lfdir[3], ludir[3];
	float cfdir[3], cudir[3];
	this->GetDirection(lfdir, ludir);
	camera.GetDirection(cfdir, cudir);

	int mf_state;
	int hitRotate = 0;

	float dis = 10.0f;
	float rad = (float) ((dis/2/Distance) * 180.0/3.1415926 * 2);
	float lastRad = rad;
	if((state.get() & LYUBU_LEFT) == 0)
		rad = -rad;

	// camera 沒被擋住，或camera被擋住但呂布改向另一邊跑才可以改變呂布臉方向
	FVector::Rotate(lfdir, ludir, rad/2, lfdir); // Rotate(src, axis, angle, dst)
	if(hitRotate != 2 || lastRad != rad)
		this->SetDirection(lfdir, ludir);

	mf_state = this->MoveForward(dis, TRUE, TRUE, 0, TRUE);

	// 旋轉camera
	FVector::Rotate(cfdir, ludir, rad, cfdir);
	FVector::Rotate(cudir, ludir, rad, cudir);

	FVector::Rotate(lfdir, ludir, rad/2, lfdir);
	if(hitRotate != 2 || lastRad != rad)
		this->SetDirection(lfdir, ludir);

	if(mf_state < 0)
	{
		float pos1[3], pos2[3];
		float camera_old[3], camera_new[3], camera_dir[3];

		this->GetPosition(pos1);
		camera.GetPosition(pos2);
		FVector::Copy(pos2,camera_old);

		FVector::Minus(pos2, pos1, pos2);

		// 繞著呂布頭頂轉camera
		FVector::Rotate(pos2, ludir, rad, pos2);
		FVector::Add(pos1, pos2, pos2);
		FVector::Copy( pos2, camera_new);
		FVector::Minus( camera_new, camera_old , camera_dir);

		// camera旋轉時碰撞偵測與處理
		/*float hitPos[3], origin[3] = {camera_old[0], camera_old[1], pos1[2]+180};
		float hitDis;

		FnTerrain terrain;
		terrain.Object(tID);

		if(terrain.HitTest(origin, camera_dir,hitPos,FALSE))
		{
			hitLen = FVector::ComputeDistance(hitPos,camera_old);

			origin[0] = camera_new[0];
			origin[1] = camera_new[1];
			origin[2] = pos1[2];
			cameraLen = FVector::ComputeDistance(pos1, origin);

			if( hitLen < cameraLen) hitRotate = 2;
			else{
				hitRotate = 1;
				camera.SetDirection(cfdir, cudir);
				camera.SetPosition(pos2);
			}

		}
		else
		{*/
			camera.SetDirection(cfdir, cudir);
			camera.SetPosition(pos2);
		//}

	}
	else{
			hitRotate = 0;
			camera.SetDirection(cfdir, cudir);
	}

	lastRad = rad;
}

void Lyubu::AttackFunction(int skip)
{

	BOOL flag = this->Play(0, ONCE, (float)skip, FALSE, TRUE);

	if(flag == FALSE)
	{
		this->state.remove(LYUBU_ATT);
		fx->Delete("lyubu_attack");
		MakeAction();
	}
}

void Lyubu::HittedFunction(int skip)
{
	BOOL flag = this->Play(0, ONCE, (float)skip, FALSE, TRUE);

	if(flag == FALSE)
	{
		this->state.remove(LYUBU_HITTED);
		fx->Delete("lyubu_damage");
		MakeAction();
	}
}

void Lyubu::DieFunction(int skip)
{
	this->Play(0, ONCE, (float)skip, FALSE, TRUE);
}

void Lyubu::turn()
{
	FnCamera camera;
	camera.Object(this->cameraID);

	float lfdir[3], ludir[3];
	float cfdir[3], cudir[3];
	this->GetDirection(lfdir, ludir);
	camera.GetDirection(cfdir, cudir);

	float tmp[3];
	FVector::CrossProduct(ludir, cfdir, tmp);
	FVector::GetUnit(tmp, tmp);
	FVector::CrossProduct(tmp, ludir, tmp);
	FVector::GetUnit(tmp, tmp);
	FVector::Rotate(tmp, ludir, -face_target, tmp);
	this->SetDirection(tmp, ludir);

	// set arrow angle
	float up[3] = {0.0f, -1.0f, 0.0f};
	float angle = FVector::Angle(tmp, up);
	FnBillBoard bb;
	bb.Object(arrowID, arrow_billboardID);
	if(tmp[0]>0)
		bb.RollTo(angle);
	else
		bb.RollTo(360-angle);
}

void Lyubu::hit(int damage)
{
	fx->Delete("lyubu_attack");
	this->state.remove(LYUBU_ATT);
	this->blood -= damage;

	redrawBloodBar();
	

	this->state.add(LYUBU_HITTED);
	if(this->blood>0)
	{
		this->MakeCurrentAction(0, NULL, this->hittedID);
		this->curActID = this->hittedID;
		this->Play(0, START, 0.0f, FALSE, TRUE);
		this->nextFrame = &Lyubu::HittedFunction;
		fx->Damage1("lyubu_damage", *this);
	}
	else
	{
		this->MakeCurrentAction(0, NULL, this->dieID);
		this->curActID = this->dieID;
		this->Play(0, START, 0.0f, FALSE, TRUE);
		this->nextFrame = &Lyubu::DieFunction;
	}
}

int Lyubu::getState()
{
	return this->state.get();
}

float Lyubu::getBlood()
{
	return this->blood;
}

void Lyubu::addBlood(float amount)
{
	this->blood += amount;
	this->redrawBloodBar();
}

void Lyubu::redrawBloodBar()
{
	FnBillBoard bb;
	bb.Object(this->bloodID, this->blood_billboardID);
	float size[2];
	size[0] = 1.0f*blood;
	size[1] = 5.0f;
	bb.SetSize(size);
}