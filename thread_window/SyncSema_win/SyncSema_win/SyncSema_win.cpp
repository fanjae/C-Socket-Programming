#include <stdio.h>
#include <windows.h>
#include <process.h>

#pragma warning (disable:4996)
unsigned WINAPI Read(void *arg);
unsigned WINAPI Accu(void *arg);

static HANDLE semOne;
static HANDLE semTwo;
static int num;

int main(int argc, char *argv[])
{
	HANDLE hThread1, hThread2;
	semOne = CreateSemaphore(NULL, 0, 1, NULL); // 세마포어 생성
	semTwo = CreateSemaphore(NULL, 1, 1, NULL); // 세마포어 생성

	hThread1 = (HANDLE)_beginthreadex(NULL, 0, Read, NULL, 0, NULL); // 쓰레드1
	hThread2 = (HANDLE)_beginthreadex(NULL, 0, Accu, NULL, 0, NULL); // 쓰레드2

	WaitForSingleObject(hThread1, INFINITE);
	WaitForSingleObject(hThread2, INFINITE);

	CloseHandle(semOne);
	CloseHandle(semTwo);
	return 0;
}
unsigned WINAPI Read(void *arg)
{
	int i;
	for (i = 0; i < 5; i++)
	{
		fputs("Input num : ", stdout);
		WaitForSingleObject(semTwo, INFINITE); // 임계영역 시작
		scanf("%d", &num);
		ReleaseSemaphore(semOne, 1, NULL); // 임계영역 끝
	}
	return 0;
}
unsigned WINAPI Accu(void *arg)
{
	int sum = 0;
	int i;
	for (i = 0; i < 5; i++)
	{
		WaitForSingleObject(semOne, INFINITE); // 임계영역 시작
		sum += num;
		ReleaseSemaphore(semTwo, 1, NULL); // 임계영역 끝
	}
	printf("Result: %d\n", sum);
	return 0;
}
