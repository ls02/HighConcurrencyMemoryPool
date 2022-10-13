#pragma once
#ifndef __COMMON_H__
#define __COMMON_H__

//这里主要存放一些公共的库和类

#include <iostream>
#include <vector>
#include <thread>
#include <assert.h>
#include <ctime>

/**
 * @brief 这个值是表示能申请空间的最大值，如果超过这个值将申请失败
*/
static const size_t MAX_BYTES = 256 * 1024;

//该值是用来表示哈希桶的最大桶数，也就是说这个哈希桶最多有208个桶
static const size_t NFREELIST = 208;

/**
 * @brief 用于获取一个对象里面存放的地址
 * @param obj 需要获取地址的对象
 * @return 获取完成后返回该对象地址的引用
*/
static void*& NextObj(void* obj)
{
	return *(void**)obj;
}

//自由连表用来管理释放后的内存资源
//管理切分好的小对象的自由链表
class FreeList
{
public:
	/**
	 * @brief 头插，用来插入不需要使用的资源
	 * @param obj 待插入对象
	*/
	void Push(void* obj)
	{
		NextObj(obj) = _freeList;
		_freeList = obj;
	}

	/**
	 * @brief 头删，当有资源需要使用需要删除
	 * @return 返回被删除的对象地址
	*/
	void* Pop()
	{
		// 用于检查是否有存在对 nullptr 进行删除
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

// 计算对象大小的对齐映射规则
class SizeClass
{
public:
	// 整体控制在最多10%左右的内碎片浪费
	// [1,128]					8byte对齐	    freelist[0,16)
	// [128+1,1024]				16byte对齐	    freelist[16,72)
	// [1024+1,8*1024]			128byte对齐	    freelist[72,128)
	// [8*1024+1,64*1024]		1024byte对齐     freelist[128,184)
	// [64*1024+1,256*1024]		8*1024byte对齐   freelist[184,208)

	/**
	 * @brief 计算对齐数的方法，因为开辟空间是根据对齐数来开辟，只会多不会少，即使存在空间浪费也是控制在10%左右
	 * @param bytes 申请空间的大小
	 * @param alignNum 该空间大小的对齐数
	 * @return 返回计算后的对齐数
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
	 * @brief 计算该空间所在哈希桶的哪一个下标
	 * @param bytes 申请空间的大小
	 * @param align_shift 该空间对应的对齐数
	 * @return 返回对应的哈希桶下标
	*/
	static inline size_t _Index(size_t bytes, size_t align_shift)
	{
		//最后还要减一是因为哈希桶本质是一个数组，那么下标0就是第一号桶所以要减一对应起来
		return ((bytes + (1 << align_shift) - 1) >> align_shift) - 1;
	}

	// 计算映射的哪一个自由链表桶
	static inline size_t Index(size_t bytes)
	{
		assert(bytes <= MAX_BYTES);

		// 每个区间有多少个链
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
