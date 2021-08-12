#include <stdio.h>
#include <windows.h>
#include <process.h>
unsigned WINAPI ThreadFunc(void *arg);

int main(int argc, char *argv[])
{
	HANDLE hThread;
	DWORD wr;
	unsigned threadID;
	int param = 5;

	hThread = (HANDLE)_beginthreadex(NULL, 0, ThreadFunc, (void *)&param, 0, &threadID); // 쓰레드 생성
	if (hThread == 0)
	{
		puts("_beginthreadex() error");
		return -1;
	}

	if ((wr = WaitForSingleObject(hThread, INFINITE)) == WAIT_FAILED) // 커널 오브젝트 signaled 상태 확인 (스레드의 종료를 기다림)
	{
		puts("thread wait error");
		return -1;
	}

	printf("wait result : %s \n", (wr == WAIT_OBJECT_0) ? "signaled" : "time-out");
	puts("end of main");
	return 0;
}

unsigned WINAPI ThreadFunc(void *arg)
{
	int i;
	int cnt = *((int *)arg);
	for (i = 0; i < cnt; i++)
	{
		Sleep(1000);
		puts("running thread");
	}
	return 0;
}

