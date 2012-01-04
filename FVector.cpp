#include "utils.h"
#include <math.h>

void FVector::Copy(float *src, float *dst)
{
	dst[0] = src[0];
	dst[1] = src[1];
	dst[2] = src[2];
}

void FVector::CrossProduct(float *src1, float *src2, float *dst)
{
	float tmp[3];
	tmp[0] = src1[1]*src2[2] - src2[1]*src1[2];
	tmp[1] = -src1[0]*src2[2] + src2[0]*src1[2];
	tmp[2] = src1[0]*src2[1] - src2[0]*src1[1];

	Copy(tmp, dst);
}

void FVector::Minus(float *src1, float *src2, float *dst)
{
	float tmp[3];
	tmp[0] = src1[0] - src2[0];
	tmp[1] = src1[1] - src2[1];
	tmp[2] = src1[2] - src2[2];

	Copy(tmp, dst);
}

void FVector::Add(float *src1, float *src2, float *dst)
{
	float tmp[3];
	tmp[0] = src1[0] + src2[0];
	tmp[1] = src1[1] + src2[1];
	tmp[2] = src1[2] + src2[2];

	Copy(tmp, dst);
}

float FVector::InnerProduct(float *src1, float *src2)
{
	return src1[0]*src2[0]+src1[1]*src2[1]+src1[2]*src2[2];
}

float FVector::Magnitude(float *src)
{
	return pow(InnerProduct(src, src), 0.5f);
}

void FVector::GetUnit(float *src, float *dst)
{
	float tmp[3];
	float magni = Magnitude(src);
	tmp[0] = src[0]/magni;
	tmp[1] = src[1]/magni;
	tmp[2] = src[2]/magni;

	Copy(tmp, dst);
}

void FVector::Project(float *src1, float *src2, float *dst)
{
	float tmp[3];
	float len = InnerProduct(src1, src2) / Magnitude(src2);
	GetUnit(src2, tmp);
	Scale(tmp, len, tmp);

	Copy(tmp, dst);
}

void FVector::Scale(float *src, float scale, float *dst)
{
	float tmp[3];
	tmp[0] = src[0]*scale;
	tmp[1] = src[1]*scale;
	tmp[2] = src[2]*scale;

	Copy(tmp, dst);
}

bool FVector::Equal(float *src1, float *src2)
{
	if(src1[0] != src2[0])
		return false;
	else if(src1[1] != src2[1])
		return false;
	else if(src1[2] != src2[2])
		return false;
	return true;
}

bool FVector::EqualZero(float *src)
{
	float tmp[3] = {0.0f, 0.0f, 0.0f};
	return Equal(src, tmp);
}

float FVector::Angle(float *src1, float *src2)
{
	float cosine = InnerProduct(src1, src2)/(Magnitude(src1)*Magnitude(src2));
	if(cosine>1.0f)
		cosine=1.0f;
	else if(cosine<-1.0f)
		cosine=-1.0f;
	return acos(cosine)*90.0f/acos(0.0f);
}

void FVector::Rotate(float *src, float *axis, float degree, float *dst)
{
	float rad = degree/180.0f*(2*acos(0.0));
	float p = 1 - cos(rad);
	float q = cos(rad);
	float s = sin(rad);

	GetUnit(axis, axis);

	float a = axis[0];
	float b = axis[1];
	float c = axis[2];

	float R[3][3];

	R[0][0] = p*a*a + q;
	R[0][1] = p*a*b-s*c;
	R[0][2] = p*a*c+s*b;

	R[1][0] = p*a*b+s*c;
	R[1][1] = p*b*b+q;
	R[1][2] = p*b*c-s*a;

	R[2][0] = p*a*c-s*b;
	R[2][1] = p*b*c+s*a;
	R[2][2] = p*c*c+q;

	float tmp[3], sum;
	int i, j;
	
	for(i=0;i<3;i++)
	{
		sum = 0;
		for(j=0;j<3;j++)
		{
			sum += R[i][j] * src[j];
		}
		tmp[i] = sum;
	}

	Copy(tmp, dst);
}

float FVector::ComputeDistance(float *v1, float *v2)
{
  float dist = pow((v1[0] - v2[0]), 2) + 
	           pow((v1[1] - v2[1]), 2) + 
			   pow((v1[2] - v2[2]), 2);
  dist = sqrt(dist);
  return dist;
}