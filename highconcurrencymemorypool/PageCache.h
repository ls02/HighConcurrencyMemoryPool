#pragma once
#ifndef __PAGE_CACHE_H__
#define __PAGE_CACHE_H__

#include "Common.h"

//单列模式
class PageCache
{
public:
	static PageCache* GetInstance()
	{
		return &_sInst;
	}

	// 获取一个K页的span
	Span* NewSpan(size_t k);

	std::mutex _pageMtx;
private:
	SpanList _spanLists[NPAGES];

	PageCache()
	{}
	PageCache(const PageCache&) = delete;


	static PageCache _sInst;
};

#endif // !__PAGE_CACHE_H__
