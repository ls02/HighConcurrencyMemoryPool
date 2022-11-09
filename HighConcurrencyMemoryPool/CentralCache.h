#pragma once
#ifndef __CENTRAL_CACHE_H__
#define __CENTRAL_CACHE_H__

#include "Common.h"

//����ģʽ
class CentralCache
{
public:
	static CentralCache* GetInstance()
	{
		return &_sInst;
	}

	/**
	 * @brief ��ȡһ���ǿյ�span���ҵ�һ��û��ʹ�õ� spanList
	 * @param list ��ӦͰ��λ��
	 * @param byte_size 
	 * @return 
	*/
	Span* GetOneSpan(SpanList& list, size_t byte_size);

	/**
	 * @brief �����Ļ����ȡһ�������Ķ����thread cache
	 * @param start ����Ͳ��������ڷ���ֵֻ����һ�����Խ�������Ͳ�������ֵ
	 * @param end ����Ͳ���
	 * @param batchNum Ҫ���ٸ� span
	 * @param size ÿ�� span �Ƕ��
	 * @return ����ʵ�����뵽�� span ����
	*/
	size_t FetchRangeObj(void*& start, void*& end, size_t batchNum, size_t size);

	// ��һ�������Ķ����ͷŵ�span���
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