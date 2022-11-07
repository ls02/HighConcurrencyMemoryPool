#pragma once
#ifndef __THREAD_CACHE_H__
#define __THREAD_CACHE_H__

#include "Common.h"

// 每个线程独享一个 ThreadCache 对象所以不需要加锁因此在高并发下面性能会非常高
class ThreadCache
{
public:
	/**
	 * @brief 当前线程申请内存空间
	 * @param size 申请内存大小
	 * @return 返回申请好的内存空间
	*/
	void* Allocate(size_t size);

	/**
	 * @brief 当前线程释放内存空间
	 * @param ptr 要释放内存空间的对象
	 * @param size 释放内存空间的大小
	*/
	void Deallocate(void* ptr, size_t size);

	/**
	 * @brief 中心缓存，当线程缓存里面没有空间了将会向中心缓存申请空间
	 * @param index 需要知道是那个桶申请的
	 * @param size 申请空间的大小
	 * @return 返回申请成功后的空间
	*/
	void* FetchFromCentralCache(size_t index, size_t size);

private:

	// 用哈希桶映射每个自由链表的位置
	FreeList _freeLists[NFREELIST];
};


// TLS thread local storage ,TLS是一个局部线程存储，用来保证每个线程都是不同的pTLShreadCache，这样就不不需要加锁，从而提高并发性能
static _declspec(thread) ThreadCache* pTLSThreadCache = nullptr;

#endif // !__THREAD_CACHE_H__