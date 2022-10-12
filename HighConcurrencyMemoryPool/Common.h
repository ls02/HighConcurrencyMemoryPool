#pragma once
#ifndef __COMMON_H__
#define __COMMON_H__

//������Ҫ���һЩ�����Ŀ����

#include <iostream>
#include <vector>
#include <thread>
#include <assert.h>
#include <ctime>

/**
 * @brief ���ڻ�ȡһ�����������ŵĵ�ַ
 * @param obj ��Ҫ��ȡ��ַ�Ķ���
 * @return ��ȡ��ɺ󷵻ظö����ַ������
*/
void*& NextObj(void* obj)
{
	return *(void**)obj;
}

//�����������������ͷź���ڴ���Դ
//�����зֺõ�С�������������
class FreeList
{
public:
	/**
	 * @brief ͷ�壬�������벻��Ҫʹ�õ���Դ
	 * @param obj ���������
	*/
	void Push(void* obj)
	{
		NextObj(obj) = _freeList;
		_freeList = obj;
	}

	/**
	 * @brief ͷɾ��������Դ��Ҫʹ����Ҫɾ��
	 * @return ���ر�ɾ���Ķ����ַ
	*/
	void* Pop()
	{
		// ���ڼ���Ƿ��д��ڶ� nullptr ����ɾ��
		assert(_freeList);
		
		void* obj = _freeList;
		_freeList = NextObj(obj);

		return obj;
	}

private:
	void* _freeList = nullptr;
};


#endif // !__COMMON_H__
