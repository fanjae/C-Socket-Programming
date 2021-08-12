#include <stdio.h>
#include <Windows.h>
#include <process.h>

/* 본 예제는 동기화가 되어있지 않아
의도한 대로 프로그램이 실행되지 않음. */
#define NUM_THREAD 50
unsigned WINAPI threadInc(void * arg);
unsigned WINAPI threadDes(void * arg);
long long int num = 0;

int main(int argc, char * argv[])
{
	HANDLE tHandle[NUM_THREAD];
	int i;

	printf("sizeof long long : %d\n", sizeof(long long));
	for (i = 0; i < NUM_THREAD; i++)
	{
		if (i % 2 == 0)
		{
			tHandle[i] = (HANDLE)_beginthreadex(NULL, 0, threadInc, NULL, 0, NULL);
		}
		else
		{
			tHandle[i] = (HANDLE)_beginthreadex(NULL, 0, threadDes, NULL, 0, NULL);
		}
	}
	// 각 쓰레드마다 5천만번의 덧셈과 뺄셈의 과정을 끝내길 기다림.
	WaitForMultipleObjects(NUM_THREAD, tHandle, TRUE, INFINITE);
	printf("result : %lld \n", num);
	return 0;
}

unsigned WINAPI threadInc(void *arg)
{
	int i;
	for (i = 0; i < 50000000; i++)
	{
		num += i;
	}
	return 0;
}
unsigned WINAPI threadDes(void *arg)
{
	int i;
	for (i = 0; i < 50000000; i++)
	{
		num -= i;
	}
	return 0;
}