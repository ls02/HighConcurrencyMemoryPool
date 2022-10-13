#pragma once
#ifndef __CONCURRENT_ALLOC_H__
#define __CONCURRENT_ALLOC_H__

//���ļ���Ҫʹ������װ������ռ���ͷſռ�ĺ�����

#include "Common.h"
#include "ThreadCache.h"

static void* ConcurrentAlloc(size_t size)
{
	// ͨ��TLS ÿ���߳������Ļ�ȡ�Լ���ר����ThreadCache����

	//�����һ��ʹ��һ��Ϊ�գ���ô������Ҫnewһ������
	if (pTLSThreadCache == nullptr)
	{
		pTLSThreadCache = new ThreadCache;
	}

	std::cout << std::this_thread::get_id() << ":" << pTLSThreadCache << std::endl;


	//���øö��������������Դ����������ռ�
	return pTLSThreadCache->Allocate(size);
}

static void ConcurrentFree(void* ptr, size_t size)
{
	//�����Ҫ�ͷ���Դ����ô��һ���������Դ��Ϊ�˷�ֹû������Դ�����ͷ����Լ��˸������ж�һ��
	assert(pTLSThreadCache);

	//���øö�����ͷſռ亯����������ͷ�
	pTLSThreadCache->Deallocate(ptr, size);
}

#endif // !__CONCURRENT_ALLOC_H__
