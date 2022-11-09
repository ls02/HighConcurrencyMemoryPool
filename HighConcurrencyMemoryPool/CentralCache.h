#pragma once
#ifndef __CENTRAL_CACHE_H__
#define __CENTRAL_CACHE_H__

#include "Common.h"

//懒汉模式
class CentralCache
{
public:
	static CentralCache* GetInstance()
	{
		return &_sInst;
	}

	/**
	 * @brief 获取一个非空的span，找到一个没人使用的 spanList
	 * @param list 对应桶的位置
	 * @param byte_size 
	 * @return 
	*/
	Span* GetOneSpan(SpanList& list, size_t byte_size);

	/**
	 * @brief 从中心缓存获取一定数量的对象给thread cache
	 * @param start 输出型参数，由于返回值只能有一个所以借助输出型参数返回值
	 * @param end 输出型参数
	 * @param batchNum 要多少个 span
	 * @param size 每个 span 是多大
	 * @return 返回实际申请到的 span 个数
	*/
	size_t FetchRangeObj(void*& start, void*& end, size_t batchNum, size_t size);

	// 将一定数量的对象释放到span跨度
	void ReleaseListToSpans(void* start, size_t byte_size);

private:
	SpanList _spanLists[NFREELIST];

private:
	CentralCache()
	{}

	CentralCache(const CentralCache&) = delete;

	static CentralCache _sInst;
};

#endif // !__CENTRAL_CACHE_H__