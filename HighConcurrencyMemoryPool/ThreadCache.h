#pragma once
#ifndef __THREAD_CACHE_H__
#define __THREAD_CACHE_H__

#include "Common.h"

// ÿ���̶߳���һ�� ThreadCache �������Բ���Ҫ��������ڸ߲����������ܻ�ǳ���
class ThreadCache
{
public:
	/**
	 * @brief ��ǰ�߳������ڴ�ռ�
	 * @param size �����ڴ��С
	 * @return ��������õ��ڴ�ռ�
	*/
	void* Allocate(size_t size);

	/**
	 * @brief ��ǰ�߳��ͷ��ڴ�ռ�
	 * @param ptr Ҫ�ͷ��ڴ�ռ�Ķ���
	 * @param size �ͷ��ڴ�ռ�Ĵ�С
	*/
	void Deallocate(void* ptr, size_t size);

	/**
	 * @brief ���Ļ��棬���̻߳�������û�пռ��˽��������Ļ�������ռ�
	 * @param index ��Ҫ֪�����Ǹ�Ͱ�����
	 * @param size ����ռ�Ĵ�С
	 * @return ��������ɹ���Ŀռ�
	*/
	void* FetchFromCentralCache(size_t index, size_t size);

private:

	// �ù�ϣͰӳ��ÿ�����������λ��
	FreeList _freeLists[NFREELIST];
};


// TLS thread local storage ,TLS��һ���ֲ��̴߳洢��������֤ÿ���̶߳��ǲ�ͬ��pTLShreadCache�������Ͳ�����Ҫ�������Ӷ���߲�������
static _declspec(thread) ThreadCache* pTLSThreadCache = nullptr;

#endif // !__THREAD_CACHE_H__