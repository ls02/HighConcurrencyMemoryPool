#pragma once
#ifndef __THREAD_CACHE_H__
#define __THREAD_CACHE_H__

#include "Common.h"

// 每个线程独享一个 ThreadCache 对象所以不需要加锁因此在高并发下面性能会非常高
class ThreadCache
{
public:
	/**
	 * @brief 申请内存空间
	 * @param size 申请内存大小
	 * @return 返回申请好的内存空间
	*/
	void* Allocate(size_t size);

	/**
	 * @brief 释放内存空间
	 * @param ptr 要释放内存空间的对象
	 * @param size 释放内存空间的大小
	*/
	void Deallocate(void* ptr, size_t size);
private:

	// 用哈希桶映射每个自由链表的位置
	FreeList _freeList[];
};

#endif // !__THREAD_CACHE_H__