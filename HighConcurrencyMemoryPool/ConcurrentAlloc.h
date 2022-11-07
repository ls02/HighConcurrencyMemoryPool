#pragma once
#ifndef __CONCURRENT_ALLOC_H__
#define __CONCURRENT_ALLOC_H__

//本文件主要使用来封装“申请空间和释放空间的函数”

#include "Common.h"
#include "ThreadCache.h"

/**
 * @brief 申请空间
 * @param size 申请空间大小 
 * @return 返回申请好的空间
*/
static void* ConcurrentAlloc(size_t size)
{
	// 通过TLS 每个线程无锁的获取自己的专属的ThreadCache对象

	//如果第一次使用一定为空，那么我们需要new一个对象
	if (pTLSThreadCache == nullptr)
	{
		pTLSThreadCache = new ThreadCache;
	}

	std::cout << std::this_thread::get_id() << ":" << pTLSThreadCache << std::endl;


	//调用该对象里面的申请资源函数来申请空间
	return pTLSThreadCache->Allocate(size);
}

/**
 * @brief 释放空间
 * @param ptr 释放空间的地址
 * @param size 释放多大
*/
static void ConcurrentFree(void* ptr, size_t size)
{
	//如果需要释放资源，那么它一定申请过资源，为了防止没申请资源就来释放所以加了个断言判断一下
	assert(pTLSThreadCache);

	//调用该对象的释放空间函数，来完成释放
	pTLSThreadCache->Deallocate(ptr, size);
}

#endif // !__CONCURRENT_ALLOC_H__
