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
 * @brief ���ֵ�Ǳ�ʾ������ռ�����ֵ������������ֵ������ʧ��
*/
static const size_t MAX_BYTES = 256 * 1024;

//��ֵ��������ʾ��ϣͰ�����Ͱ����Ҳ����˵�����ϣͰ�����208��Ͱ
static const size_t NFREELIST = 208;

/**
 * @brief ���ڻ�ȡһ�����������ŵĵ�ַ
 * @param obj ��Ҫ��ȡ��ַ�Ķ���
 * @return ��ȡ��ɺ󷵻ظö����ַ������
*/
static void*& NextObj(void* obj)
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

	bool Empty()
	{
		return _freeList == nullptr;
	}

private:
	void* _freeList = nullptr;
};

// ��������С�Ķ���ӳ�����
class SizeClass
{
public:
	// ������������10%���ҵ�����Ƭ�˷�
	// [1,128]					8byte����	    freelist[0,16)
	// [128+1,1024]				16byte����	    freelist[16,72)
	// [1024+1,8*1024]			128byte����	    freelist[72,128)
	// [8*1024+1,64*1024]		1024byte����     freelist[128,184)
	// [64*1024+1,256*1024]		8*1024byte����   freelist[184,208)

	/**
	 * @brief ����������ķ�������Ϊ���ٿռ��Ǹ��ݶ����������٣�ֻ��಻���٣���ʹ���ڿռ��˷�Ҳ�ǿ�����10%����
	 * @param bytes ����ռ�Ĵ�С
	 * @param alignNum �ÿռ��С�Ķ�����
	 * @return ���ؼ����Ķ�����
	*/
	static inline size_t _RoundUp(size_t bytes, size_t alignNum)
	{
		return ((bytes + alignNum - 1) & ~(alignNum - 1));
	}

	static inline size_t RoundUp(size_t size)
	{
		if (size <= 128)
		{
			return _RoundUp(size, 8);
		}
		else if (size <= 1024)
		{
			return _RoundUp(size, 16);
		}
		else if (size <= 8 * 1024)
		{
			return _RoundUp(size, 128);
		}
		else if (size <= 64 * 1024)
		{
			return _RoundUp(size, 1024);
		}
		else if (size <= 256 * 1024)
		{
			return _RoundUp(size, 8 * 1024);
		}
		else
		{
			assert(false);
			
			return -1;
		}
	}

	/**
	 * @brief ����ÿռ����ڹ�ϣͰ����һ���±�
	 * @param bytes ����ռ�Ĵ�С
	 * @param align_shift �ÿռ��Ӧ�Ķ�����
	 * @return ���ض�Ӧ�Ĺ�ϣͰ�±�
	*/
	static inline size_t _Index(size_t bytes, size_t align_shift)
	{
		//���Ҫ��һ����Ϊ��ϣͰ������һ�����飬��ô�±�0���ǵ�һ��Ͱ����Ҫ��һ��Ӧ����
		return ((bytes + (1 << align_shift) - 1) >> align_shift) - 1;
	}

	// ����ӳ�����һ����������Ͱ
	static inline size_t Index(size_t bytes)
	{
		assert(bytes <= MAX_BYTES);

		// ÿ�������ж��ٸ���
		static int group_array[4] = { 16, 56, 56, 56 };
		if (bytes <= 128) {
			return _Index(bytes, 3);
		}
		else if (bytes <= 1024) {
			return _Index(bytes - 128, 4) + group_array[0];
		}
		else if (bytes <= 8 * 1024) {
			return _Index(bytes - 1024, 7) + group_array[1] + group_array[0];
		}
		else if (bytes <= 64 * 1024) {
			return _Index(bytes - 8 * 1024, 10) + group_array[2] + group_array[1] + group_array[0];
		}
		else if (bytes <= 256 * 1024) {
			return _Index(bytes - 64 * 1024, 13) + group_array[3] + group_array[2] + group_array[1] + group_array[0];
		}
		else {
			assert(false);
		}

		return -1;
	}
};


#endif // !__COMMON_H__