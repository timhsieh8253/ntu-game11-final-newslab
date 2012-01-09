#include "FX.h"
/*
char *newfxname[] = {"Attack01",
					"Attack03",
					"blood2",
					"Defense",
					"Defense04",
					"dust3",
					"Eat",
					"fire",
					"fire_1",
					"Hurt",
					"Hurt2",
					"NoPigeon",
					"NoPigeon1",
					"Pose13",
					"smoke",
					"Smoke_01",
					"Thunder"};
*/

FX::FX(SCENEid sID)
{
	this->sID = sID;
	at = string("lyubu_attack2");
}

void FX::NewFX(char* fxname, float pos[3], char* key)
{
	eF3DFX *newfx = new eF3DFX(sID);
	newfx->SetWorkPath("NTU4\\FXs");
	newfx->Load(fxname);
	AddFX(newfx, pos, key);
}

void FX::NewFX(char* fxname, float pos[3], char* key, FnActor actor)
{
	eF3DFX *newfx = new eF3DFX(sID);
	newfx->SetWorkPath("NTU4\\FXs");
	newfx->Load(fxname);
	AddFX(newfx, pos, key, actor);
}

void FX::Attack1(char* key, FnActor actor)
{
	float pos[3] = {0.f, -125.f, 50.f};
	eF3DFX *newfx = new eF3DFX(sID);
	newfx->SetWorkPath("NTU4\\FXs");
	newfx->Load("Attack01");
	AddFX(newfx, pos, key, actor);
}

void FX::Attack2(char* key, FnActor actor)
{
	float pos[3] = {0.f, -125.f, 50.f};
	eF3DFX *newfx = new eF3DFX(sID);
	newfx->SetWorkPath("NTU4\\FXs");
	newfx->Load("Attack03");
	AddFX(newfx, pos, key, actor);
}

void FX::Attack4(char* key, FnActor actor)
{
	iter = playlist.find(at);
	if(iter != playlist.end())
	{
		iter->second->Reset();
	}
	else
	{
		float pos[3] = {0.f, -125.f, 50.f};
		eF3DFX *newfx = new eF3DFX(sID);
		newfx->SetWorkPath("NTU4\\FXs");
		newfx->Load("Pose13");
		AddFX(newfx, pos, key, actor);
	}
}

void FX::Damage1(char* key, FnActor actor)
{
	float pos[3] = {0.f, 0.f, 50.f};
	eF3DFX *newfx = new eF3DFX(sID);
	newfx->SetWorkPath("NTU4\\FXs");
	newfx->Load("Hurt");
	AddFX(newfx, pos, key, actor);
}

void FX::Damage2(char* key, FnActor actor)
{
	float pos[3] = {0.f, 0.f, 50.f};
	eF3DFX *newfx = new eF3DFX(sID);
	newfx->SetWorkPath("NTU4\\FXs");
	newfx->Load("Hurt2");
	AddFX(newfx, pos, key, actor);
}

void FX::Defense1(char* key, FnActor actor)
{
	float pos[3] = {0.f, 0.f, 50.f};
	eF3DFX *newfx = new eF3DFX(sID);
	newfx->SetWorkPath("NTU4\\FXs");
	newfx->Load("Defense");
	AddFX(newfx, pos, key, actor);
}

void FX::Defense2(char* key, FnActor actor)
{
	float pos[3] = {0.f, 0.f, 50.f};
	eF3DFX *newfx = new eF3DFX(sID);
	newfx->SetWorkPath("NTU4\\FXs");
	newfx->Load("Defense04");
	AddFX(newfx, pos, key, actor);
}

void FX::Eat(char* key, FnActor actor)
{
	float pos[3] = {0.f, 0.f, 50.f};
	eF3DFX *newfx = new eF3DFX(sID);
	newfx->SetWorkPath("NTU4\\FXs");
	newfx->Load("Eat");
	AddFX(newfx, pos, key, actor);
}

void FX::Heal(char* key, FnActor actor)
{
	float pos[3] = {0.f, 0.f, 50.f};
	eF3DFX *newfx = new eF3DFX(sID);
	newfx->SetWorkPath("NTU4\\FXs");
	newfx->Load("NoPigeon1");
	AddFX(newfx, pos, key, actor);
}

void FX::Delete(char *key)
{
	iter = playlist.find(string(key));
	if(iter != playlist.end())
	{
		delete iter->second;
		playlist.erase(iter);
	}
}

void FX::AddFX(eF3DFX* newfx, float pos[3], char* key, FnActor actor)
{
	eF3DBaseFX* base;
	int i, numFXs = newfx->NumberFXs();
	for(i=0; i<numFXs; i++)
	{
		base = newfx->GetFX(i);
		base->SetParent(actor.GetBaseObject());
		base->InitPosition(pos);
	}
	playlist.insert(pair<string, eF3DFX*>(string(key), newfx));
}

void FX::AddFX(eF3DFX* newfx, float pos[3], char* key)
{
	eF3DBaseFX* base;
	int i, numFXs = newfx->NumberFXs();
	for(i=0; i<numFXs; i++)
	{
		base = newfx->GetFX(i);
		//base->SetParent(actor.GetBaseObject());
		base->InitPosition(pos);
	}
	playlist.insert(pair<string, eF3DFX*>(string(key), newfx));
}

void FX::Play(float skip)
{
	for(iter = playlist.begin(); iter != playlist.end(); iter++)
	{
		if(!(iter->second->Play(skip)) && iter->first != at)
		{
			iter->second->Reset();
		}
	}
}