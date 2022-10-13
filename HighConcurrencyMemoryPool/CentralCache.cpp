#include "CentralCache.h"

CentralCache CentralCache::_sInst;

// ��ȡһ���ǿյ�span
Span* CentralCache::GetOneSpan(SpanList& list, size_t size)
{
	// ...
	return nullptr;
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