#include "Common.h"
#include "ThreadCache.h"

void* ThreadCache::Allocate(size_t size)
{
	//����������ֵ������������
	assert(size <= MAX_BYTES);

	//����ÿռ�Ķ�����
	size_t alignSize = SizeClass::RoundUp(size);
	//����ÿռ��Ӧ�Ĺ�ϣͰ�±�
	size_t index = SizeClass::Index(size);

	//�����Ϊ��˵�����滹�пռ䣬������ʹ��������������Ŀռ�
	if (!_freeList[index].Empty())
	{
		return _freeList[index].Pop();
	}
	else
	{
		//���������������û�ռ䣬��ô˵�� thread cache����Ҳû�ռ��ˣ���ô��Ҫ�� center cache ����
		return FetchFromCentralCache(index, size);
	}
}

/**
 * @brief �ͷ��̻߳�����ڴ���Դ���Ѹ���Դ�ŵ���������������������
 * @param ptr ��Ҫ�ͷ���Դ�ĵ�ַ
 * @param size �ͷ���Դ�Ŀռ��С
*/
void ThreadCache::Deallocate(void* ptr, size_t size)
{
	assert(ptr);
	assert(size <= MAX_BYTES);

	size_t index = SizeClass::Index(size);
	// �Ҷ�ӳ�����������Ͱ������������
	_freeList[index].Push(ptr);
}


/**
 * @brief ���Ļ������̻߳������һ�㣬���̻߳���û�ڴ�ռ��ʱ����Ҫ����һ������
 * @param index Ҫ���뻺������ڹ�ϣͰλ��
 * @param size ������ռ�
 * @return ��������õĿռ�
*/
void* ThreadCache::FetchFromCentralCache(size_t index, size_t size)
{
	return nullptr;
}