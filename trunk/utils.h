#ifndef UTILS
#define UTILS
#include "TheFlyWin32.h"
#include <vector>

class FVector
{
public:
	// true if src1 == src2
	static bool Equal(float *src1, float *src2);

	// true if src == {0, 0, 0}
	static bool EqualZero(float *src);

	// dst = src
	static void Copy(float *src, float *dst);

	// dst = src1 กั src2
	static void CrossProduct(float *src1, float *src2, float *dst);

	// dst = src1 กD src2
	static float InnerProduct(float *src1, float *src2);

	// dst = src1 - src2
	static void Minus(float *src1, float *src2, float *dst);

	// dst = src1 + src2
	static void Add(float *src1, float *src2, float *dst);

	// dst = scale*src
	static void Scale(float *src, float scale, float *dst);

	// return |src|
	static float Magnitude(float *src);

	// dst = ^src
	static void GetUnit(float *src, float *dst);

	// dst = src1 projects on src2
	static void Project(float *src1, float *src2, float *dst);

	// rotate vector about axis by degree
	static void Rotate(float *src, float *axis, float degree, float *dst);

	// return the angle between src1 and src2 in degree
	static float Angle(float *src1, float *src2);
		
	static float ComputeDistance(float *v1, float *v2);
};



class State
{
	private:
		int state;

	public:
		int get()
		{
			return state;
		}
		void set(int s)
		{
			state = s;
		}

		void add(int s)
		{
			state = state | s;
		}

		void remove(int s)
		{
			state = state & (~s);
		}
		
		bool contain(int s)
		{
			if((state & s) != 0)
				return true;
			return false;
		}
};

class AttackEvent
{
public:
	FnActor *actor;
	float length;
	float width;
	int damage;
	int delay;
};

class MyAttackQueue
{
private:
	std::vector<AttackEvent> queue;

public:

	void push(AttackEvent ae)
	{
		int num = queue.size();
		queue.resize(num + 1);
		int i = num;
		while(i > 0 && queue[i-1].delay > ae.delay)
		{
			queue[i] = queue[i-1];
			i--;
		}
		queue[i] = ae;
	}

	void reduceDelay(int skip)
	{
		int num = queue.size();
		int i;
		for(i=0;i<num;i++)
		{
			AttackEvent ae = queue[i];
			ae.delay -= skip;
			queue[i] = ae;
		}
	}

	void pop()
	{
		queue.erase(queue.begin());
	}

	AttackEvent front()
	{
		return queue[0];
	}

	int size()
	{
		return queue.size();
	}
};

#endif