#include<iostream>
#include<ctime>
#include<stdlib.h>
using namespace std;

typedef unsigned long long UINT64;
#define nrround 10
#define testdeg 35
#define getbit(x,n)   ((x>>n)&(0x1))

UINT64 state[384] = {0};
UINT64 key[128] = {0};
UINT64 RC[32] = {0};
UINT64 temp[128] = {0};
UINT64 uconst[6]={0xAAAAAAAAAAAAAAAA,0xCCCCCCCCCCCCCCCC,0xF0F0F0F0F0F0F0F0,0xFF00FF00FF00FF00,0xFFFF0000FFFF0000,0xFFFFFFFF00000000};
int Invalist[65]={0, 1, 3, 8, 9, 10, 11, 13, 18, 19, 20, 21, 23, 29, 30, 31, 33, 39, 40, 41, 43, 46, 49, 50, 51, 53, 56, 59, 60, 61, 63, 66, 69, 70, 71, 73, 76, 79, 80, 81, 83, 86, 87, 89, 90, 91, 93, 96, 97, 99, 100, 101, 103, 106, 107, 109, 111, 113, 116, 117, 119, 121, 123, 126, 127};
int Invclist[33]={2, 4, 6, 10, 12, 18, 22, 24, 26, 28, 32, 34, 44, 46, 48, 50, 54, 56, 66, 68, 72, 76, 78, 80, 88, 90, 94, 98, 102, 110, 112, 116, 124};


void Frit(int rounds);
void InitializeRC();
int checksum(int* cube, int size);
int allcube(int num, int deg, int* cube, int a0, int a1);
void Invcube();

int main()
{
	int a0, a1, vnum, i = 0, j = 0;
	int guesskey[128] = {0};
	clock_t start, finish;
	double  duration;
	start = clock();  

	InitializeRC();
	 
	srand(unsigned(time(0)));
	//random key
	/*cout << "The random key is:" << endl;
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
	cout << endl;*/
	
	//cout << checksum(cube9r[i],testdeg) << endl;
	
	//for (i = 0; i < 128; i ++) //recover 128 bit key
	{
		//cout << i << endl;
		for (j = 0; j < 128; j ++)
			key[j]=0;
		key[(i+1)%128] = 0xFFFFFFFFFFFFFFFF;
		a0 = i%128;
		a1 = (i+1)%128;
		int vindex[128] = {0};
		int varc[6] = {a0%128,(a0+18)%128,(a0+41)%128,a1,(a1+18)%128,(a1+41)%128};
		for (j = 0; j < 6; j ++)
			vindex[varc[j]] = 1;
		int cube[122];
		vnum = 0;
		for (j = 0; j < 128; j ++)
		{
			if (vindex[j] == 0)
			{
				cube[vnum] = j+256;
					vnum ++;
			}
		}
		
		if (allcube(vnum,testdeg-2,cube,a0,a1)==1)
			guesskey[(i+1)%128] = 1;
		else
			guesskey[(i+1)%128] = 0;
		if (guesskey[(i+1)%128] != (key[(i+1)%128]&0x1)) //check 128 bit guessed-key
		{
			cout << "The guessed key bit[" << (i+1)%128 << "] is wrong." << endl;
			//break;
		}
	}

	if (i == 128) //The gusseed key is right
	{
		for (i = 0; i < 128; i ++)
			cout << guesskey[i];
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
			temp[j] ^= state[(j+Invalist[k])%128+256];
	}
	for (int j =0; j < 128; j ++)
		state[j+256] = temp[j];
	//sigma-1 c
	for (int j =0; j < 128; j ++)
	{
		temp[j] = 0;
		for (int k =0; k < 33; k ++)
			temp[j] ^= state[(j+Invclist[k])%128+256];
	}
	for (int j =0; j < 128; j ++)
		state[j+256] = temp[j];
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
			state[m+128] = key[m];
			state[m+256] = 0;
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
			state[m+128] = key[m];
			state[m+256] = 0;
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

int allcube(int n,int k, int* cube, int a0, int a1)
{
	int indx[testdeg];
	//true is selected
	bool *nselect = (bool *)malloc(n*sizeof(bool));
	
	int i = 0, j = 0;
	int testnum = 0;
	//初始化第一个组合，即选择前k个数 
	while(i<n)
	{
		if(i<k)
		 nselect[i] = true;
	    else
		 nselect[i] = false;
		i++;
	}
	
	i = 0;
	indx[0] = a0;
	indx[1] = a1;
	while(i<n)
	{
		if(nselect[i]) 
		{
			indx[j+2] = cube[i];
			j++;
			printf("%d,",cube[i]);
		}
		i++;
	}
	printf("\n");
	int judge = checksum(indx, testdeg);
	cout << judge << endl;
	if (judge == 0)
		testnum ++;
	else
	{
		for(j=0;j<testdeg;j++)
			cout << indx[j] << ",";
		cout << endl;
		return 1;
	}
	
	i=0;
	//while(i<n-1 && testnum < 20)
	while(i<n-1)
	{
		//找到第一个10组合，并交换10的位置 
		if(nselect[i] && !nselect[i+1])
		{
			int temp = nselect[i];
			int x=0,y=0;
			nselect[i] = nselect[i+1];
			nselect[i+1] = temp;
			//将该组合左边所有的1移至数组最左边 
			while(y<i)
			{
				if(nselect[y])
				{
					temp = nselect[y];
					nselect[y] = nselect[x];
					nselect[x] = temp;
					x++;
				}
				y++;
			}
			
			i = 0;
			j = 0;
			indx[0] = a0;
			indx[1] = a1;
			while(i<n)
			{
				if(nselect[i]) 
				{
					indx[j+2] = cube[i];
					j++;
					printf("%d,",cube[i]);
				}
				i++;
			}
			printf("\n");
			int judge = checksum(indx, testdeg);
			cout << judge << endl;
			if (judge == 0)
				testnum ++;
			else
			{
				for(j=0;j<testdeg;j++)
					cout << indx[j] << ",";
				cout << endl;
				return 1;
			}
			
			i = (x==0?0:x-1);
		}
		else i++; 
	}
	free(nselect);
	return 0;
}