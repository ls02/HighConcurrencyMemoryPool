#pragma once
#ifndef __PAGE_CACHE_H__
#define __PAGE_CACHE_H__

#include "Common.h"

//����ģʽ
class PageCache
{
public:
	static PageCache* GetInstance()
	{
		return &_sInst;
	}

	// ��ȡһ��Kҳ��span
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
