#include "TheFlyWin32.h"
#include "npc.h"
#include "Lyubu.h"
#include "utils.h"
#include "FX.h"

#include <queue>

#define MAX_CD 60

using namespace std;

extern Lyubu *lyubu; // ��f�����ӥ�
extern MyAttackQueue AttackList;
extern FX *fx;

/********************************************************************/
// ��l�� (actor = 0 ���� , actor = 1 �p�L)
/********************************************************************/

NPC::NPC(){
	npc_id = NULL;
}

NPC::~NPC(){
} 

bool NPC::init(OBJECTid id, int actorType, OBJECTid cameraid)
{
	npc_id  = id;
	actor   = actorType;
	Object(npc_id);
	setNPCActionID();
	curActID  = idleID;
	nextActID = idleID;
	MakeCurrentAction(0, NULL, idleID);
	flag = TRUE;
	life = 20;
	hitLevel = 0;
	attackLevel = 0;
	hitNum = 0;
	test = 0;
	cd = MAX_CD;

	this->runState.set(NPC_IDLE);
	this->nextRunState.set(NPC_IDLE);
	this->cameraID = cameraid;
	
	this->changeState(wait, 0);
	// create blood bar
	OBJECTid sID = this->GetScene();
	FnScene scene;
	scene.Object(sID);
	bloodID = scene.CreateObject(ROOT);
	FnObject blood;
	blood.Object(bloodID);
	blood.SetParent(this->GetBaseObject());
	float size[2], color[3], pos[3];
	pos[0] = 0.0f;
	pos[1] = 0.0f;
	pos[2] = 100.0f;
	
	size[0] = 3.0f*life;
	size[1] = 5.0f;
	color[0] = 1.0f; color[1] = color[2] = 0.0f;
	
	blood_billboardID = blood.Billboard(pos, size, NULL, 0, color);
	// blood bar end

	return TRUE;
}

void NPC::setNPCActionID(){

	if(actor == 0)
	{
		idleID     = GetBodyAction(NULL, "CombatIdle");
		runID      = GetBodyAction(NULL, "Run");
		attackID   = GetBodyAction(NULL, "AttackL1");
		damageID   = GetBodyAction(NULL, "DamageL");
		defanceID  = GetBodyAction(NULL, "Defance");
		deadID     = GetBodyAction(NULL, "Die");
	}
	else if(actor == 1)
	{
		idleID     = GetBodyAction(NULL, "CombatIdle");
		runID      = GetBodyAction(NULL, "Run");
		move_rightID    = GetBodyAction(NULL, "Movemove_right");
		move_leftID     = GetBodyAction(NULL, "Movemove_left");
		attackID   = GetBodyAction(NULL, "NormalAttack1");
		damageID   = GetBodyAction(NULL, "Damage1");
		deadID     = GetBodyAction(NULL, "Dead");
	}


}

/********************************************************************/
// ���� NPC fsm�������A
//
// �ǤJ�Ѽ�:
// newState:    {wait, follow, attackPlayer, escape, hitted, die}
// Level:       �b�P�@action�����P�{�ת��ʧ@�ɡA�ǤJ�ʧ@level (1~4)
//
/********************************************************************/

void NPC::changeState(int newState, int Level){
	switch(newState){
		case wait:           this->state = wait;			this->action=&NPC::waitAction;		break;
		case follow:		 this->state = follow;			this->action=&NPC::followAction;	break;
		case attackPlayer:   
							 this->state = attackPlayer;	
							 this->attackLevel = Level; 
							 this->action=&NPC::attackAction;
							 break;

		//case escape:		 this->state = escape;			break;
		case die:			 this->state = die;				this->action=&NPC::dieAction;		break;
		case hitted:
							 this->state = hitted;
							 this->hitLevel = Level; 
							 hitNum++;
							 this->action=&NPC::hittedAction;
							 break;
		default:    		 ;
	}
}

/********************************************************************/
// ���� NPC �ʧ@�A�b fsm ���۹���state���ݨD�I�s
//
// �ǤJ�Ѽ�:
// action:      {dle, run, move_right, move_left, attack, damage, defance, dead}
// ActionName:  �b�P�@action�����P�{�ת��ʧ@�ɡA�ǤJ�ʧ@�r�� (�Ҧp:attack, damage)
//
/********************************************************************/
void NPC::setNPCurAction(int action, char *ActionName){

	switch(action){
		case idle:
			nextActID = idleID;
			break;
		case run:
			nextActID = runID;
			break;
		case move_right:
			nextActID = move_rightID;
			break;
		case move_left:
			nextActID = move_leftID;
			break;
		case attack:
			attackID = GetBodyAction(NULL, ActionName);
			nextActID = attackID;
			break;
		case damage:
			damageID   = GetBodyAction(NULL, ActionName);
			nextActID  = damageID;
			break;
		case defance:
			nextActID = defanceID;
			break;
		case dead:
			nextActID = deadID;
			break;
		default: ;
	}
}

/********************************************************************/
// ����NPC�ʧ@
/********************************************************************/

void NPC::playAction(int skip){

	if(curActID != nextActID)
	{
		MakeCurrentAction(0, NULL, nextActID);
		Play(0, START, 0.0f, FALSE, TRUE);
		curActID = nextActID;
	}
	else{
		flag = TRUE;
		if(this->state == hitted)        
			flag = Play(0, ONCE, (float)skip, FALSE, TRUE);
		else if(this->state == attackPlayer)
			flag = Play(0, ONCE, (float)skip, FALSE, TRUE);
		else if(this->state == die)		 
			Play(0, ONCE, (float)skip, FALSE, TRUE);
		else							 
			Play(0, LOOP, (float)skip, FALSE, TRUE);

		if(!flag){
			if(this->state == hitted)
				fx->Delete("donzo_damage");
			else if(this->state == attackPlayer)
				fx->Delete("donzo_attack");
			changeState(wait,0);
			flag = TRUE;
		}
		else{
			if(this->state == hitted && hitNum > 1){
				MakeCurrentAction(0, NULL, nextActID);
				Play(0, START, (float)skip, FALSE, TRUE);
				curActID = nextActID;
				hitNum--;
			}
		}
	}

}

void NPC::waitAction()
{
	float lyubu_pos[3], pos[3];
	this->GetPosition(pos);
	lyubu->GetPosition(lyubu_pos);

	float follow_dis = 1000;
	float attack_dis = 100;
	float dis = FVector::ComputeDistance(lyubu_pos, pos);

	//�ˬd�M�f�����Z���A�p�󭭨�N�� follow
	if(dis < attack_dis && !lyubu->Isdead())
	{
		if(cd >= MAX_CD)
		{
			cd = 0;
			this->changeState(attackPlayer, 1);
			AttackEvent ae;
			ae.actor = this;
			ae.length = 150;
			ae.width = 50;
			ae.damage = 2;
			ae.delay = 15;
			AttackList.push(ae);
		}
	}
	else if(dis < follow_dis && !lyubu->Isdead())
		this->changeState(follow, 0);
	setNPCurAction(idle,"");
}

void NPC::followAction()
{

	float lyubu_pos[3], pos[3];
	this->GetPosition(pos);
	lyubu->GetPosition(lyubu_pos);

	float follow_dis = 1000;
	float attack_dis = 100;
	float dis = FVector::ComputeDistance(lyubu_pos, pos);

	// �p���V
	float dir[3];
	if(!this->Isdead())
	{
		FVector::Minus(lyubu_pos, pos, dir);
		float udir[3], fdir[3];
		this->GetDirection(fdir, udir);
		this->SetDirection(dir, udir);
	}

	if(dis < attack_dis && !lyubu->Isdead())
	{
		if(cd >= MAX_CD)
		{
			cd = 0;
			this->changeState(attackPlayer, 1);
			AttackEvent ae;
			ae.actor = this;
			ae.length = 150;
			ae.width = 50;
			ae.damage = 2;
			ae.delay = 15;
			AttackList.push(ae);
		}
	}
	else if(dis >= follow_dis || lyubu->Isdead())
		this->changeState(wait, 0);

	setNPCurAction(run,"");
	
	this->MoveForward(5, TRUE, TRUE, 0, TRUE);
}

void NPC::attackAction()
{
	float lyubu_pos[3], pos[3];
	this->GetPosition(pos);
	lyubu->GetPosition(lyubu_pos);

	float follow_dis = 1000;
	float attack_dis = 100;
	float dis = FVector::ComputeDistance(lyubu_pos, pos);
	// �p���V
	float dir[3];
	if(!this->Isdead())
	{
		FVector::Minus(lyubu_pos, pos, dir);
		float udir[3], fdir[3];
		this->GetDirection(fdir, udir);
		this->SetDirection(dir, udir);
	}

	if(actor == 0){
		if(attackLevel == 1)      setNPCurAction(attack,"AttackL1");
		else if(attackLevel == 2) setNPCurAction(attack,"AttackL2");
		else if(attackLevel == 3) setNPCurAction(attack,"AttackH");
		else					  setNPCurAction(attack,"HeavyAttack");
	}
	else{
		if(attackLevel == 1)      setNPCurAction(attack,"NormalAttack1");
		else if(attackLevel == 2) setNPCurAction(attack,"NormalAttack2");
		else if(attackLevel == 3) setNPCurAction(attack,"HeavyAttack1");
	}
}

void NPC::hittedAction()
{
	FnBillBoard bb;
	if(actor == 0){
		if(curActID != damageID && curActID != defanceID) decreaseLife(hitLevel);
	}
	else{
		if(curActID != damageID) decreaseLife(hitLevel);
	}
	
	bb.Object(this->bloodID, this->blood_billboardID);
	float size[2];
	size[0] = 3.0f*life;
	size[1] = 5.0f;
	bb.SetSize(size);

	if(life <= 0)
	{
		this->changeState(die, 0);
	}
	else
	{
		if(actor == 0){
			if(hitLevel == 1){      setNPCurAction(damage,"DamageL");
									fx->Damage1("donzo_damage", *this);
			}
			else if(hitLevel == 2){ setNPCurAction(damage,"DamageH");
									fx->Damage2("donzo_damage", *this);
			}
			else{					setNPCurAction(defance,"");
									fx->Defense1("donzo_defense", *this);
			}
		}
		else{
			if(hitLevel == 1)      setNPCurAction(damage,"Damage1");
			else if(hitLevel == 2) setNPCurAction(damage,"Damage2");
			else				   setNPCurAction(damage,"Damage2");
		}
	}
}

void NPC::dieAction()
{
	setNPCurAction(dead,"");
}

/********************************************************************/
// NPC �� FSM
// �b�@�}�l���񦹮�NPC���ʧ@�A����̾ڥثe���A�h�M�w�U�@�Ӱʧ@
// {wait, follow, attackPlayer, escape, hitted, die}
// follow �M escape �����ݥ[�̦h NPC AI �ӻP�f������(escape���@�w�n��)
//
/********************************************************************/
void NPC::fsm(int skip) {

	playAction(skip);
	if(cd < MAX_CD)
		cd += skip;

	(this->*(this->action))();

}

/********************************************************************/
// ����NPC�]�B�����A (��f���@��) ����TFGH
//
/********************************************************************/

void NPC::changeRunState(BYTE code, BOOL value)
{
	test = 1;
		switch(code)
		{
			case FY_T:
				if(value)
					runState.add(NPC_UP);
				else
					runState.remove(NPC_UP);
				break;
			case FY_F:
				if(value)
					runState.add(NPC_move_left);
				else
					runState.remove(NPC_move_left);
				break;
			case FY_G:
				if(value)
					runState.add(NPC_DOWN);
				else
					runState.remove(NPC_DOWN);
				break;
			case FY_H:
				if(value)
					runState.add(NPC_move_right);
				else
					runState.remove(NPC_move_right);
				break;
		}

		if(!value) changeState(wait,0);

		MakeAction();
	
}

/********************************************************************/
// NPC���� ����1234 (�{��)
//
/********************************************************************/
void NPC::changeAttackState(BYTE code, BOOL value)
{
		switch(code)
		{
			case FY_1:
				if(value)  changeState(attackPlayer,1);
				break;
			case FY_2:
				if(value)  changeState(attackPlayer,2);
				break;
			case FY_3:
				if(value)  changeState(attackPlayer,3);
				break;
			case FY_4:
				if(value)  changeState(attackPlayer,4);
				break;
		}

}

/********************************************************************/
// NPC��V
/********************************************************************/

void NPC::MakeAction()
{
	
	switch(runState.get() & 15)
	{
		case NPC_UP:
			face_NPC = 0;
			break;
		case NPC_move_left:
			face_NPC = 270;
			break;
		case NPC_DOWN:
			face_NPC = 180;
			break;
		case NPC_move_right:
			face_NPC = 90;
			break;
		case (NPC_UP | NPC_move_left):
			face_NPC = 315;
			break;
		case (NPC_UP | NPC_move_right):
			face_NPC = 45;
			break;
		case (NPC_DOWN | NPC_move_right):
			face_NPC = 135;
			break;
		case (NPC_DOWN | NPC_move_left):
			face_NPC = 225;
			break;
		case (NPC_move_left | NPC_move_right):
			face_NPC = 270;
			break;
	}


}

void NPC::turn()
{
	FnCamera camera;
	camera.Object(this->cameraID);

	float fdir[3], udir[3];
	float cfdir[3], cudir[3];
	this->GetDirection(fdir, udir);
	camera.GetDirection(cfdir, cudir);

	float tmp[3];
	FVector::CrossProduct(udir, cfdir, tmp);
	FVector::GetUnit(tmp, tmp);
	FVector::CrossProduct(tmp, udir, tmp);
	FVector::GetUnit(tmp, tmp);
	FVector::Rotate(tmp, udir, -face_NPC, tmp);
	this->SetDirection(tmp, udir);

}

/********************************************************************/
// NPC �l��
/********************************************************************/
void NPC::decreaseLife(int value){
	if(life > 0) life = life - value;
}

/********************************************************************/
// NPC �O�_���`
/********************************************************************/
bool NPC::Isdead(){
	return (life <= 0);
}

/********************************************************************/
// NPC ���ո�T
/********************************************************************/
int NPC::getlife(){
	return life;
}

int NPC::getcurAction(){
	return curActID;
}

int NPC::getState(){
	return this->state;
}

int NPC::getHitNum(){
	return this->hitNum;
}

int NPC::getTest(){
	return this->test;
}

OBJECTid NPC::getid()
{
	return this->npc_id;
}