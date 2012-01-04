#include "TheFlyWin32.h"
#include "npc.h"

NPC::NPC(){
	npc_id = NULL;
}

NPC::~NPC(){
} 

bool NPC::init(OBJECTid id, int actorType)
{
	npc_id  = id;
	actor   = actorType;
	Object(npc_id);
	setNPCActionID();
	curActID  = idleID;
	nextActID = idleID;
	MakeCurrentAction(0, NULL, idleID);
	flag = TRUE;
	life = 10;
	hitLevel = 0;

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

void NPC::changeState(int newState, int hitLevel){
	switch(newState){
		case wait:           this->state = wait;			break;
		case follow:		 this->state = follow;			break;
		case attackPlayer:   this->state = attackPlayer;	break;
		case escape:		 this->state = escape;			break;
		case die:			 this->state = die;				break;
		case hitted:
							 this->state = hitted;
							 this->hitLevel = hitLevel; 
							 break;
		default:    		 ;
	}
}

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
		else if(this->state == die)		 
			Play(0, ONCE, (float)skip, FALSE, TRUE);
		else							 
			Play(0, LOOP, (float)skip, FALSE, TRUE);

		if(!flag){
			changeState(wait,0);
			flag = TRUE;
		}
	}

}


/*
 * NPCªºFinite state machine
 */
void NPC::fsm(int skip) {

	playAction(skip);
	FnBillBoard bb;

	switch(this->state){

		case wait:
			setNPCurAction(idle,"");
			break;

		case follow:
			break;

		case attackPlayer:
			break;

		case escape:

			break;

		case hitted:

			if(curActID != damageID && curActID != defanceID) life-=hitLevel;

			
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
				}
			}

			break;

		case die:
			setNPCurAction(dead,"");
			break;

		default:
			;

	}

}

void NPC::decreaseLife(int value){
	if(life > 0) life = life - value;
}


bool NPC::Isdead(){
	return (life <= 0);
}

int NPC::getlife(){
	return life;
}

int NPC::getcurAction(){
	return curActID;
}

int NPC::getState(){
	return this->state;
}

OBJECTid NPC::getid()
{
	return this->npc_id;
}