#include "Common.h"
#include "ThreadCache.h"
#include "CentralCache.h"

void* ThreadCache::Allocate(size_t size)
{
	//如果超过这个值将不可以申请
	assert(size <= MAX_BYTES);

	//计算该空间的对齐数
	size_t alignSize = SizeClass::RoundUp(size);
	//计算该空间对应的哈希桶下标
	size_t index = SizeClass::Index(size);

	//如果不为空说明里面还有空间，可以先使用自由链表里面的空间
	if (!_freeLists[index].Empty())
	{
		return _freeLists[index].Pop();
	}
	else
	{
		//如果自由链表里面没空间，那么说明 thread cache里面也没空间了，那么需要向 center cache 申请
		return FetchFromCentralCache(index, size);
	}
}

void ThreadCache::Deallocate(void* ptr, size_t size)
{
	assert(ptr);
	assert(size <= MAX_BYTES);

	size_t index = SizeClass::Index(size);
	// 找对映射的自由链表桶，对象插入进入
	_freeLists[index].Push(ptr);
}


void* ThreadCache::FetchFromCentralCache(size_t index, size_t size)
{
	// 慢开始反馈调节算法
// 1、最开始不会一次向central cache一次批量要太多，因为要太多了可能用不完
// 2、如果你不要这个size大小内存需求，那么batchNum就会不断增长，直到上限
// 3、size越大，一次向central cache要的batchNum就越小
// 4、size越小，一次向central cache要的batchNum就越大
	size_t batchNum = min(_freeLists[index].MaxSize(), SizeClass::NumMoveSize(size));

	if (_freeLists[index].MaxSize() == batchNum)
	{
		_freeLists[index].MaxSize() += 1;
	}

	//输出型参数，用来保存获取到的地址
	void* start = nullptr;
	void* end = nullptr;

	size_t actualNum = CentralCache::GetInstance()->FetchRangeObj(start, end, batchNum, size);
	assert(actualNum > 0);

	if (actualNum == 1)
	{
		assert(start == end);

		return start;
	}
	else
	{
		_freeLists[index].PushRange(NextObj(start), end);

		return start;
	}

	return nullptr;
}