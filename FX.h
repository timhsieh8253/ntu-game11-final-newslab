#ifndef DEFINEFX
#define DEFINEFX
#include "TheFlyWin32.h"
#include "FyFX.h"

#include <map>
#include <string>

using namespace std;

class FX
{
	public:
		FX(SCENEid sID);
		void NewFX(char* fxname, float pos[3], char* key);
		void NewFX(char* fxname, float pos[3], char* key, FnActor actor);
		void Attack1(char* key, FnActor actor);
		void Attack2(char* key, FnActor actor);
		void Attack4(char* key, FnActor actor);
		void Damage1(char* key, FnActor actor);
		void Damage2(char* key, FnActor actor);
		void Defense1(char* key, FnActor actor);
		void Defense2(char* key, FnActor actor);
		void Eat(char* key, FnActor actor);
		void Heal(char* key, FnActor actor); //play NoPigeon
		void Play(float skip);
		void Delete(char* key);
	private:
		string at;
		SCENEid sID;
		map<string, eF3DFX*> playlist;
		map<string, eF3DFX*>::iterator iter;
		void AddFX(eF3DFX* newfx, float pos[3], char* key, FnActor actor);
		void AddFX(eF3DFX* newfx, float pos[3], char* key);
		
};
#endif