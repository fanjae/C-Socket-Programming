#include <vector>
#include <iostream>
#include <chrono>
#include <thread>
#include <memory>
#include <mutex>

using namespace std;

const int MaxCount = 300000;
const int ThreadCount = 4;

bool IsPrimeNumber(int number)
{
	if (number == 1)
	{
		return false;
	}
	if (number == 2 || number == 3)
	{
		return true;
	}
	for (int i = 2; i < number - 1; i++)
	{
		if ((number % i) == 0)
		{
			return false;
		}
	}
	return true;
}

void PrintNumbers(const vector<int>& primes)
{
	for (int v : primes)
	{
		cout << v << endl;
	}
}

int main()
{
	int num = 1;
	recursive_mutex num_mutex;
	vector<int> primes;
	recursive_mutex primes_mutex;

	auto t0 = chrono::system_clock::now();

	vector<shared_ptr<thread> > threads;

	for (int i = 0; i < ThreadCount; i++)
	{
		shared_ptr<thread> thread(new thread([&]() {
			// 스레드의 메인함수
			// 값을 가져올 수 있음녀 루프를 돈다.
			while (true)
			{
				int n;
				{ // 락(lock)을 하여 다른 스레드가 대기 상태로 전환.
					lock_guard<recursive_mutex> num_lock(num_mutex);
					n = num;
					num++;
				}
				if (n >= MaxCount)
				{
					break;
				}

				if (IsPrimeNumber(n))
				{
					lock_guard<recursive_mutex> primes_lock(primes_mutex);
					primes.push_back(n);
				}
			}
		}));
		// 스레드 객체를 일단 갖고 있는다.
		threads.push_back(thread);
	}

	// 모든 쓰레드가 일을 마칠때 까지 기다린다.
	for (auto thread : threads)
	{
		thread->join();
	}

	auto t1 = chrono::system_clock::now();
	auto duration = chrono::duration_cast<chrono::milliseconds>(t1 - t0).count();
	cout << "Took " << duration << " milliseconds." << endl;

}