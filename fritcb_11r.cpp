//key放在c处；11轮 在第三轮sigmaa后选择cube变量v，共有29个不相乘位置可以选择；算法采用一次加密64组；allcube可计算所有可能组合
//测试v39v127相关密钥k46+k69+k76+k86+k87+k99+k109+k117+k127
#include<iostream>
#include<ctime>
#include<stdlib.h>
#include"Frit3key.h"
using namespace std;


int main()
{
	int i = 0, j = 0;
	clock_t start, finish;
	double  duration;
	start = clock();  

	InitializeRC();
	 
	srand(unsigned(time(0)));
	//random key
	cout << "The random key is:" << endl;
	for (i = 0; i < 128; i ++)
	{
		if ((rand()%2) == 0)
		{
			key[i] = 0;	
			cout << 0;
		}
		else
		{
			key[i]= 0xFFFFFFFFFFFFFFFF;
			cout << 1;
		}
	}
	cout << endl;
	
	for (i = 0; i < 128; i ++) //get 128 bit conditions
	{
		if (checksum(cube[i],testdeg)==1)
			equkey[i] = 1;
		else 
			equkey[i] = 0;
	}
	getkey(); //recover 128-bit key
	
	for (i = 0; i < 128; i ++)
	{
		if ((key[i]&0x1) != equkey[i])
		{
			cout << "The guessed key bit[" << i << "] is wrong." << endl;
			break;
		}
	}
	if (i == 128) //The gusseed key is right
	{
		for (i = 0; i < 128; i ++)
			cout << equkey[i];
		cout << endl;
	}
	
	finish = clock();
	duration = (double)(finish - start) / CLOCKS_PER_SEC;
    cout << duration << 's' << endl;
	system("pause");
	return 0;
}

void Frit(int n) //n-round Frit permutation
{
	int i, j;
	for (i = 0; i< n; i ++)
	{
		//sigma(a)
		for (j =0; j < 128; j ++)
			temp[j] = state[j] ^ state[(j+110)%128] ^ state[(j+87)%128];
		for (j =0; j < 128; j ++)
			state[j] = temp[j];
		//c xor RC
		for (j = i; j < 32; j ++)
			state[352+j] ^= RC[j-i];
		//c xor a&b
		for (j =0; j < 128; j ++)
			state[j+256] ^= state[j]&state[j+128];
		//sigma(c)
		for (j =0; j < 128; j ++)
			temp[j] = state[j+256] ^ state[((j+118)%128)+256] ^ state[((j+88)%128)+256];
		for (j =0; j < 128; j ++)
			state[j+256] = temp[j];
		//b = a xor b xor c
		for (j =0; j < 128; j ++)
			state[j+128] ^= state[j]^state[j+256];
		//(a,b,c)<--(c,a,b)
		for (j =0; j < 128; j ++)
		{
			temp[j]=state[j];
			state[j]=state[j+256];
			state[j+256]=state[j+128];
			state[j+128]=temp[j];
		}
	}
}
void InitializeRC()
{
	int roundconst[32]={1,1,1,1,1,0,0,1,1,0,1,0,0,1,0,0,0,0,1,0,1,0,1,1,1,0,1,1,0,0,0,1};
	for (int i = 0; i < 32; i ++)
	{
		if (roundconst[i] == 1)
			RC[i] = 0xFFFFFFFFFFFFFFFF;
		else 
			RC[i] = 0;
	}
}

void Invcube()
{
	//sigma-1 a
	for (int j =0; j < 128; j ++)
	{
		temp[j] = 0;
		for (int k =0; k < 65; k ++)
			temp[j] ^= state[(j+Invalist[k])%128+128];
	}
	for (int j =0; j < 128; j ++)
		state[j+128] = temp[j];
	//sigma-1 c
	for (int j =0; j < 128; j ++)
	{
		temp[j] = 0;
		for (int k =0; k < 33; k ++)
			temp[j] ^= state[(j+Invclist[k])%128+128];
	}
	for (int j =0; j < 128; j ++)
		state[j+128] = temp[j];
}
int checksum(int* index, int size)
{
	UINT64 num = 1;
	num = (num << size);
	
	UINT64 j, n=0;
	UINT64 sum[128]={0};
	// <= 2^6 permutation
	if (size <= 6) 
	{
		for (int m = 0; m < 128; m ++)
		{
			state[m] = 0;
			state[m+128] = 0;
			state[m+256] =  key[m];
		}
		//set cube variables
		for (int q = 0; q < size; q ++)
			state[index[q]] = uconst[q];
	
		Invcube();

		Frit(nrround);
		for (int m = 0; m < 128; m ++)
			sum[m] ^= state[m+128];
		int res[128] = {0};
		for (int m = 0; m < 128; m ++)
		{
			for(int i = 0; i < num; i ++)
				res[m] ^= getbit(sum[m],i);
			if(res[m] == 1)
				return 1;
		}
		return 0;		
	}
	// num/64 times permutation
	for (j = 0;j < (num/64); j ++)
	{	
		for (int m = 0; m < 128; m ++)
		{
			state[m] = 0;
			state[m+128] = 0;
			state[m+256] = key[m];
		}
		//set cube variables
		for (int q = 0; q < 6; q ++)
			state[index[q]] = uconst[q];
	
		for (int q = 0; q < size-6; q ++)
		{
			if (getbit(j,q) == 0)
				state[index[q+6]] = 0;
			else 
				state[index[q+6]] = 0xFFFFFFFFFFFFFFFF;
		}
		//state256-383 inva invb
		Invcube();

		Frit(nrround);

		for (int m = 0; m < 128; m ++)
			sum[m] ^= state[m+128];
	}
	int res[128] = {0};
	for (int m = 0; m < 128; m ++)
	{
		for(int i = 0; i < 64; i ++)
			res[m] ^= getbit(sum[m],i);
		if(res[m] == 1)
			return 1;
	}
	return 0;
}
void getkey()
{
	for (int i = 0; i < 128; i ++)
		martrixb[i] = (equkey[i]+martrixb[i])%2;
	for (int i = 0; i < 128; i ++)
	{
		int guessk = 0;
		for (int j = 0; j < 128; j ++)
			guessk += martrixA[i][j]*martrixb[j];
		equkey[i] = (guessk)%2;
	}
}