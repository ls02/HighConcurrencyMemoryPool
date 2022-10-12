#pragma once
#ifndef __THREAD_CACHE_H__
#define __THREAD_CACHE_H__

#include "Common.h"

// ÿ���̶߳���һ�� ThreadCache �������Բ���Ҫ��������ڸ߲����������ܻ�ǳ���
class ThreadCache
{
public:
	/**
	 * @brief �����ڴ�ռ�
	 * @param size �����ڴ��С
	 * @return ��������õ��ڴ�ռ�
	*/
	void* Allocate(size_t size);

	/**
	 * @brief �ͷ��ڴ�ռ�
	 * @param ptr Ҫ�ͷ��ڴ�ռ�Ķ���
	 * @param size �ͷ��ڴ�ռ�Ĵ�С
	*/
	void Deallocate(void* ptr, size_t size);
private:

	// �ù�ϣͰӳ��ÿ�����������λ��
	FreeList _freeList[];
};

#endif // !__THREAD_CACHE_H__