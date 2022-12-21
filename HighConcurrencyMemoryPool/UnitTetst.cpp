#include "Common.h"
#include "ConcurrentAlloc.h"
#include "ThreadCache.h"

void Alloc1()
{
	for (int i = 0; i < 5; i++)
	{
		void* ptr = ConcurrentAlloc(5000000);
	}
}

void Alloc2()
{
	for (int i = 0; i < 5; i++)
	{
		void* ptr = ConcurrentAlloc(7);
	}
}

void TLSTest()
{
	std::thread t1(Alloc1);
	t1.join();

	std::thread t2(Alloc2);
	t2.join();
}

void TestConcurrentAlloc1()
{
	void* p1 = ConcurrentAlloc(6);
	void* p2 = ConcurrentAlloc(8);
	void* p3 = ConcurrentAlloc(1);
	void* p4 = ConcurrentAlloc(7);
	void* p5 = ConcurrentAlloc(8);

	std::cout << p1 << std::endl;
	std::cout << p2 << std::endl;
	std::cout << p3 << std::endl;
	std::cout << p4 << std::endl;
	std::cout << p5 << std::endl;

}

void TestConcurrentAlloc2()
{
	//for (size_t i = 0; i < 1024; ++i)
	//{
	//	void* p1 = ConcurrentAlloc(6);
	//	std::cout << p1 << std::endl;
	//}

	void* p2 = ConcurrentAlloc(200 * 1024);
	std::cout << p2 << std::endl;
}

void TestConcurrentAlloc3()
{
	void* p1 = ConcurrentAlloc(6);
	void* p2 = ConcurrentAlloc(3);
	void* p3 = ConcurrentAlloc(5);
	void* p4 = ConcurrentAlloc(1);
	void* p5 = ConcurrentAlloc(8);
}

int main(void)
{
	//TLSTest();
	//TestConcurrentAlloc1();
	//TestConcurrentAlloc2();
	TestConcurrentAlloc3();
}