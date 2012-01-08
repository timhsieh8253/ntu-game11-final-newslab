#include "TheFlyWin32.h"
#include "npc.h"
#include "Lyubu.h"
#include "utils.h"

#include <queue>

#define MAX_CD 60

using namespace std;

extern Lyubu *lyubu; // 把呂布拿來用
extern MyAttackQueue AttackList;

/********************************************************************/
// 初始化 (actor = 0 董卓 , actor = 1 小兵)
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
		rightID    = GetBodyAction(NULL, "MoveRight");
		leftID     = GetBodyAction(NULL, "MoveLeft");
		attackID   = GetBodyAction(NULL, "NormalAttack1");
		damageID   = GetBodyAction(NULL, "Damage1");
		deadID     = GetBodyAction(NULL, "Dead");
	}


}

/********************************************************************/
// 改變 NPC fsm中的狀態
//
// 傳入參數:
// newState:    {wait, follow, attackPlayer, escape, hitted, die}
// Level:       在同一action有不同程度的動作時，傳入動作level (1~4)
//
/********************************************************************/

void NPC::changeState(int newState, int Level){
	switch(newState){
		case wait:           this->state = wait;			break;
		case follow:		 this->state = follow;			break;
		case attackPlayer:   
							 this->state = attackPlayer;	
							 this->attackLevel = Level; 
							 break;

		case escape:		 this->state = escape;			break;
		case die:			 this->state = die;				break;
		case hitted:
							 this->state = hitted;
							 this->hitLevel = Level; 
							 hitNum++;
							 break;
		default:    		 ;
	}
}

/********************************************************************/
// 改變 NPC 動作，在 fsm 中相對應state視需求呼叫
//
// 傳入參數:
// action:      {dle, run, right, left, attack, damage, defance, dead}
// ActionName:  在同一action有不同程度的動作時，傳入動作字串 (例如:attack, damage)
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
		case right:
			nextActID = rightID;
			break;
		case left:
			nextActID = leftID;
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
// 播放NPC動作
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


/********************************************************************/
// NPC 的 FSM
// 在一開始播放此時NPC的動作，之後依據目前狀態去決定下一個動作
// {wait, follow, attackPlayer, escape, hitted, die}
// follow 和 escape 部份需加最多 NPC AI 來與呂布互動(escape不一定要有)
//
/********************************************************************/
void NPC::fsm(int skip) {

	playAction(skip);
	FnBillBoard bb;

	float lyubu_pos[3], pos[3];
	this->GetPosition(pos);
	lyubu->GetPosition(lyubu_pos);

	float follow_dis = 1000;
	float attack_dis = 100;
	float dis = FVector::ComputeDistance(lyubu_pos, pos);

	if(cd < MAX_CD)
		cd += skip;

	// 計算方向
	float dir[3];
	FVector::Minus(lyubu_pos, pos, dir);
	float udir[3], fdir[3];
	this->GetDirection(fdir, udir);
	this->SetDirection(dir, udir);

	switch(this->state){

		case wait:
		{
			//檢查和呂布的距離，小於限制就變 follow
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

			break;
		}

		case follow:
		{
			  /*if(runState.contain(NPC_UP) || runState.contain(NPC_DOWN) 
				 || runState.contain(NPC_LEFT) || runState.contain(NPC_RIGHT) )
			  {
				test = 2;
				setNPCurAction(run,"");
				turn();
				MoveForward(5, TRUE, TRUE, 0, TRUE);
			  }*/

			
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

			break;
		}
		case attackPlayer:
		{
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

			break;
		}
		case escape: break;

		case hitted:
		{
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
				this->state = die;
			}
			else
			{
				if(actor == 0){
					if(hitLevel == 1)      setNPCurAction(damage,"DamageL");
					else if(hitLevel == 2) setNPCurAction(damage,"DamageH");
					else				   setNPCurAction(defance,"");
				}
				else{
					if(hitLevel == 1)      setNPCurAction(damage,"Damage1");
					else if(hitLevel == 2) setNPCurAction(damage,"Damage2");
					else				   setNPCurAction(damage,"Damage2");
				}
			}

			break;
		}
		case die:
		{
			setNPCurAction(dead,"");
			break;
		}
		default:;
	}

}

/********************************************************************/
// 改變NPC跑步的狀態 (跟呂布一樣) 按鍵TFGH
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
					runState.add(NPC_LEFT);
				else
					runState.remove(NPC_LEFT);
				break;
			case FY_G:
				if(value)
					runState.add(NPC_DOWN);
				else
					runState.remove(NPC_DOWN);
				break;
			case FY_H:
				if(value)
					runState.add(NPC_RIGHT);
				else
					runState.remove(NPC_RIGHT);
				break;
		}

		if(!value) changeState(wait,0);

		MakeAction();
	
}

/********************************************************************/
// NPC攻擊 按鍵1234 (程度)
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
// NPC轉向
/********************************************************************/

void NPC::MakeAction()
{
	
	switch(runState.get() & 15)
	{
		case NPC_UP:
			face_NPC = 0;
			break;
		case NPC_LEFT:
			face_NPC = 270;
			break;
		case NPC_DOWN:
			face_NPC = 180;
			break;
		case NPC_RIGHT:
			face_NPC = 90;
			break;
		case (NPC_UP | NPC_LEFT):
			face_NPC = 315;
			break;
		case (NPC_UP | NPC_RIGHT):
			face_NPC = 45;
			break;
		case (NPC_DOWN | NPC_RIGHT):
			face_NPC = 135;
			break;
		case (NPC_DOWN | NPC_LEFT):
			face_NPC = 225;
			break;
		case (NPC_LEFT | NPC_RIGHT):
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
// NPC 損血
/********************************************************************/
void NPC::decreaseLife(int value){
	if(life > 0) life = life - value;
}

/********************************************************************/
// NPC 是否死亡
/********************************************************************/
bool NPC::Isdead(){
	return (life <= 0);
}

/********************************************************************/
// NPC 測試資訊
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