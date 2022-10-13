#include "CentralCache.h"

CentralCache CentralCache::_sInst;

// 获取一个非空的span
Span* CentralCache::GetOneSpan(SpanList& list, size_t size)
{
	// ...
	return nullptr;
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