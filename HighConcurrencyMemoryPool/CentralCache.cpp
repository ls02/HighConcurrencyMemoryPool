#include "CentralCache.h"
#include "PageCache.h"

CentralCache CentralCache::_sInst;

// 获取一个非空的span
Span* CentralCache::GetOneSpan(SpanList& list, size_t size)
{
	// 查看当前的spanlist中是否有还有未分配对象的span
	Span* it = list.Begin();
	while (it != list.End())
	{
		if (it->_freeList != nullptr)
		{
			return it;
		}
		else
		{
			it = it->_next;
		}
	}

	// 先把central cache的桶锁解掉，这样如果其他线程释放内存对象回来，不会阻塞
	list._mtx.unlock();

	// 走到这里说没有空闲span了，只能找page cache要
	PageCache::GetInstance()->_pageMtx.lock();
	Span* span = PageCache::GetInstance()->NewSpan(SizeClass::NumMovePage(size));
	PageCache::GetInstance()->_pageMtx.unlock();

	// 对获取span进行切分，不需要加锁，因为这会其他线程访问不到这个span

	// 计算span的大块内存的起始地址和大块内存的大小(字节数)
	char* start = (char*)(span->_pageId << PAGE_SHIFT);
	size_t bytes = span->_n << PAGE_SHIFT;
	char* end = start + bytes;

	// 把大块内存切成自由链表链接起来
	// 1、先切一块下来去做头，方便尾插
	span->_freeList = start;
	start += size;
	void* tail = span->_freeList;
	int i = 1;
	while (start < end)
	{
		++i;
		NextObj(tail) = start;
		tail = NextObj(tail); // tail = start;
		start += size;
	}

	// 切好span以后，需要把span挂到桶里面去的时候，再加锁
	list._mtx.lock();
	list.PushFront(span);

	return span;
}

// 从中心缓存获取一定数量的对象给thread cache
size_t CentralCache::FetchRangeObj(void*& start, void*& end, size_t batchNum, size_t size)
{
	//算出是那个 Thread Cache 的桶没空间了
	size_t index = SizeClass::Index(size);
	//对该桶加锁
	_spanLists[index]._mtx.lock();

	//找到空的span空间
	Span* span = GetOneSpan(_spanLists[index], size);
	assert(span);
	assert(span->_freeList);

	// 从span中获取batchNum个对象
	// 如果不够batchNum个，有多少拿多少
	start = span->_freeList;
	end = start;
	size_t i = 0;
	//实际获取到span的个数
	size_t actualNum = 1;

	//如果span没了或拿够 span就结束
	while (i < batchNum && nullptr != NextObj(end))
	{
		end = NextObj(end);
		i++;
		//每拿一个 span 就++
		++actualNum;
	}

	//断开链接，重新建立新的链接
	span->_freeList = NextObj(end);
	NextObj(end) = nullptr;

	//对该桶进行解锁
	_spanLists[index]._mtx.unlock();

	return actualNum;
}