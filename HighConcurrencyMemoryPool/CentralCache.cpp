#include "CentralCache.h"
#include "PageCache.h"

CentralCache CentralCache::_sInst;

// ��ȡһ���ǿյ�span
Span* CentralCache::GetOneSpan(SpanList& list, size_t size)
{
	// �鿴��ǰ��spanlist���Ƿ��л���δ��������span
	Span* it = list.Begin();
	while (it != list.End())
	{
		if (it->_freeList != nullptr)
		{
			return it;
		}
		else
		{
			it = it->_next;
		}
	}

	// �Ȱ�central cache��Ͱ�������������������߳��ͷ��ڴ�����������������
	list._mtx.unlock();

	// �ߵ�����˵û�п���span�ˣ�ֻ����page cacheҪ
	PageCache::GetInstance()->_pageMtx.lock();
	Span* span = PageCache::GetInstance()->NewSpan(SizeClass::NumMovePage(size));
	PageCache::GetInstance()->_pageMtx.unlock();

	// �Ի�ȡspan�����з֣�����Ҫ��������Ϊ��������̷߳��ʲ������span

	// ����span�Ĵ���ڴ����ʼ��ַ�ʹ���ڴ�Ĵ�С(�ֽ���)
	char* start = (char*)(span->_pageId << PAGE_SHIFT);
	size_t bytes = span->_n << PAGE_SHIFT;
	char* end = start + bytes;

	// �Ѵ���ڴ��г�����������������
	// 1������һ������ȥ��ͷ������β��
	span->_freeList = start;
	start += size;
	void* tail = span->_freeList;
	int i = 1;
	while (start < end)
	{
		++i;
		NextObj(tail) = start;
		tail = NextObj(tail); // tail = start;
		start += size;
	}

	// �к�span�Ժ���Ҫ��span�ҵ�Ͱ����ȥ��ʱ���ټ���
	list._mtx.lock();
	list.PushFront(span);

	return span;
}

// �����Ļ����ȡһ�������Ķ����thread cache
size_t CentralCache::FetchRangeObj(void*& start, void*& end, size_t batchNum, size_t size)
{
	//������Ǹ� Thread Cache ��Ͱû�ռ���
	size_t index = SizeClass::Index(size);
	//�Ը�Ͱ����
	_spanLists[index]._mtx.lock();

	//�ҵ��յ�span�ռ�
	Span* span = GetOneSpan(_spanLists[index], size);
	assert(span);
	assert(span->_freeList);

	// ��span�л�ȡbatchNum������
	// �������batchNum�����ж����ö���
	start = span->_freeList;
	end = start;
	size_t i = 0;
	//ʵ�ʻ�ȡ��span�ĸ���
	size_t actualNum = 1;

	//���spanû�˻��ù� span�ͽ���
	while (i < batchNum && nullptr != NextObj(end))
	{
		end = NextObj(end);
		i++;
		//ÿ��һ�� span ��++
		++actualNum;
	}

	//�Ͽ����ӣ����½����µ�����
	span->_freeList = NextObj(end);
	NextObj(end) = nullptr;

	//�Ը�Ͱ���н���
	_spanLists[index]._mtx.unlock();

	return actualNum;
}