#include <stdio.h>
#include <windows.h>
#include <process.h>
#define STR_LEN 100

unsigned WINAPI NumberOfA(void *arg);
unsigned WINAPI NumberOfOthers(void *arg);

static char str[STR_LEN];
static HANDLE hEvent;

int main(int argc, char *argv[])
{
	HANDLE hThread1, hThread2;
	hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
	// 2번째 인자 : TRUE : manual-reset FALSE auto-reset 모드
	// 3번째 인자 : TRUE : signaled Event 오브젝트 생성, FLASE : non-signaled Event 오브젝트 생성.

	hThread1 = (HANDLE)_beginthreadex(NULL, 0, NumberOfA, NULL, 0, NULL);
	hThread2 = (HANDLE)_beginthreadex(NULL, 0, NumberOfOthers, NULL, 0, NULL);

	fputs("Input string : ", stdout); // 위 두 쓰레드가 해당 입력을 기다림
	fgets(str, STR_LEN, stdin);
	SetEvent(hEvent); // Event 오브젝트 상태 변경. // 이후 쓰레드 대기 상태에서 실행 이어감.
	WaitForSingleObject(hThread1, INFINITE);
	WaitForSingleObject(hThread2, INFINITE);
	ResetEvent(hEvent);
	CloseHandle(hEvent);
	return 0;
}

unsigned WINAPI NumberOfA(void *arg)
{
	int i, cnt = 0;
	WaitForSingleObject(hEvent, INFINITE);
	for (i = 0; str[i] != 0; i++)
	{
		if (str[i] == 'A')
		{
			cnt++;
		}
	}
	printf("Num of A : %d\n", cnt);
	return 0;
}
unsigned WINAPI NumberOfOthers(void *arg)
{
	int i, cnt = 0;
	WaitForSingleObject(hEvent, INFINITE);
	for (i = 0; str[i] != 0; i++)
	{
		if (str[i] != 'A')
		{
			cnt++;
		}
	}
	printf("Num of others: %d \n", cnt - 1);
	return 0;
}