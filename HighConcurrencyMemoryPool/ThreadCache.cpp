#include "Common.h"
#include "ThreadCache.h"
#include "CentralCache.h"

void* ThreadCache::Allocate(size_t size)
{
	//����������ֵ������������
	assert(size <= MAX_BYTES);

	//����ÿռ�Ķ�����
	size_t alignSize = SizeClass::RoundUp(size);
	//����ÿռ��Ӧ�Ĺ�ϣͰ�±�
	size_t index = SizeClass::Index(size);

	//�����Ϊ��˵�����滹�пռ䣬������ʹ��������������Ŀռ�
	if (!_freeLists[index].Empty())
	{
		return _freeLists[index].Pop();
	}
	else
	{
		//���������������û�ռ䣬��ô˵�� thread cache����Ҳû�ռ��ˣ���ô��Ҫ�� center cache ����
		return FetchFromCentralCache(index, size);
	}
}

void ThreadCache::Deallocate(void* ptr, size_t size)
{
	assert(ptr);
	assert(size <= MAX_BYTES);

	size_t index = SizeClass::Index(size);
	// �Ҷ�ӳ�����������Ͱ������������
	_freeLists[index].Push(ptr);
}


void* ThreadCache::FetchFromCentralCache(size_t index, size_t size)
{
	// ����ʼ���������㷨
// 1���ʼ����һ����central cacheһ������Ҫ̫�࣬��ΪҪ̫���˿����ò���
// 2������㲻Ҫ���size��С�ڴ�������ôbatchNum�ͻ᲻��������ֱ������
// 3��sizeԽ��һ����central cacheҪ��batchNum��ԽС
// 4��sizeԽС��һ����central cacheҪ��batchNum��Խ��
	size_t batchNum = min(_freeLists[index].MaxSize(), SizeClass::NumMoveSize(size));

	if (_freeLists[index].MaxSize() == batchNum)
	{
		_freeLists[index].MaxSize() += 1;
	}

	//����Ͳ��������������ȡ���ĵ�ַ
	void* start = nullptr;
	void* end = nullptr;

	size_t actualNum = CentralCache::GetInstance()->FetchRangeObj(start, end, batchNum, size);
	assert(actualNum > 0);

	if (actualNum == 1)
	{
		assert(start == end);

		return start;
	}
	else
	{
		_freeLists[index].PushRange(NextObj(start), end);

		return start;
	}

	return nullptr;
}