#include <stdio.h>
#include <tchar.h>
#include <windows.h>
#include <iostream>
#include <thread>

using namespace std;
class CriticalSection
{
	CRITICAL_SECTION m_critSec;
public:
	CriticalSection();
	~CriticalSection();

	void Lock();
	void Unlock();
};

class CriticalSectionLock
{
	CriticalSection* m_pCritSec;
public:
	CriticalSectionLock(CriticalSection& critSec);
	~CriticalSectionLock();
};

CriticalSection::CriticalSection()
{
	InitializeCriticalSectionEx(&m_critSec, 0, 0);
}

CriticalSection::~CriticalSection()
{
	DeleteCriticalSection(&m_critSec);
}

void CriticalSection::Lock()
{
	EnterCriticalSection(&m_critSec);
}

void CriticalSection::Unlock()
{
	LeaveCriticalSection(&m_critSec);
}

CriticalSectionLock::CriticalSectionLock(CriticalSection& critSec)
{
	m_pCritSec = &critSec;
	m_pCritSec->Lock();
}

CriticalSectionLock::~CriticalSectionLock()
{
	m_pCritSec->Unlock();
}

int a, b;
CriticalSection a_mutex;
CriticalSection b_mutex;

int main()
{
	thread t1([]()
	{
		while (1)
		{
			CriticalSectionLock lock(a_mutex);
			a++;
			CriticalSectionLock lock2(b_mutex);
			b++;
			cout << "t1 done.\n";
		}
	});

	thread t2([]()
	{
		while (1)
		{
			CriticalSectionLock lock(b_mutex);
			b++;
			CriticalSectionLock lock2(a_mutex);
			a++;
			cout << "t2 done.\n";
		}
	});

	// 스레드가 끝나길 기다린다.
	// 무한 반복이므로, 끝나지 않는다.
	// 잠금 및 잠금해제를 계속 반복함.
	t1.join();
	t2.join();

	return 0;
}