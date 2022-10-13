#include "Common.h"
#include "ThreadCache.h"

void* ThreadCache::Allocate(size_t size)
{
	//如果超过这个值将不可以申请
	assert(size <= MAX_BYTES);

	//计算该空间的对齐数
	size_t alignSize = SizeClass::RoundUp(size);
	//计算该空间对应的哈希桶下标
	size_t index = SizeClass::Index(size);

	//如果不为空说明里面还有空间，可以先使用自由链表里面的空间
	if (!_freeList[index].Empty())
	{
		return _freeList[index].Pop();
	}
	else
	{
		//如果自由链表里面没空间，那么说明 thread cache里面也没空间了，那么需要向 center cache 申请
		return FetchFromCentralCache(index, size);
	}
}

/**
 * @brief 释放线程缓存的内粗资源，把该资源放到自由链表里面链接起来
 * @param ptr 需要释放资源的地址
 * @param size 释放资源的空间大小
*/
void ThreadCache::Deallocate(void* ptr, size_t size)
{
	assert(ptr);
	assert(size <= MAX_BYTES);

	size_t index = SizeClass::Index(size);
	// 找对映射的自由链表桶，对象插入进入
	_freeList[index].Push(ptr);
}


/**
 * @brief 中心缓存是线程缓存的下一层，当线程缓存没内存空间的时候需要向下一层申请
 * @param index 要申请缓存的所在哈希桶位置
 * @param size 申请多大空间
 * @return 返回申请好的空间
*/
void* ThreadCache::FetchFromCentralCache(size_t index, size_t size)
{
	return nullptr;
}