#pragma once
#ifndef __COMMON_H__
#define __COMMON_H__

//这里主要存放一些公共的库和类

#include <iostream>
#include <vector>
#include <thread>
#include <assert.h>
#include <ctime>
#include <mutex>
#include <algorithm>

#ifdef _WIN32
#include <windows.h>
#else
	// ...
#endif

// 用于适配不同环境下的页数大小
#ifdef _WIN64
	typedef unsigned long long PAGE_ID;
#elif _WIN32
	typedef size_t PAGE_ID;
#else
	// linux
#endif

/**
 * @brief 这个值是表示能申请空间的最大值，如果超过这个值将申请失败
*/
static const size_t MAX_BYTES = 256 * 1024;

//该值是用来表示哈希桶的最大桶数，也就是说这个哈希桶最多有208个桶
static const size_t NFREELIST = 208;

//PageCache的桶的个数
static const size_t NPAGES = 129;
static const size_t PAGE_SHIFT = 13;

/**
 * @brief 用于获取一个对象里面存放的地址
 * @param obj 需要获取地址的对象
 * @return 获取完成后返回该对象地址的引用
*/
static void*& NextObj(void* obj)
{
	return *(void**)obj;
}

// 直接去堆上按页申请空间
inline static void* SystemAlloc(size_t kpage)
{
#ifdef _WIN32
	void* ptr = VirtualAlloc(0, kpage << 13, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
#else
	// linux下brk mmap等
#endif

	if (ptr == nullptr)
		throw std::bad_alloc();

	return ptr;
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

	/**
	 * @brief 头插，一块自由链表插入该链表头部
	 * @param start 自由链表的头节点
	 * @param end 自由链表的尾节点
	*/
	void PushRange(void* start, void* end)
	{
		//建立新的链接
		NextObj(end) = _freeList;
		_freeList = start;
	}

	bool Empty()
	{
		return _freeList == nullptr;
	}

	size_t& MaxSize()
	{
		return _maxSize;
	}

private:
	void* _freeList = nullptr;
	size_t _maxSize = 1;
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

	/**
	 * @brief 一次thread cache从中心缓存获取多少个，限制最大值和最小值
	 * @param size 申请空间的大小
	 * @return 返回所需要的span个数
	*/
	static size_t NumMoveSize(size_t size)
	{
		assert(size > 0);

		// [2, 512]，一次批量移动多少个对象的(慢启动)上限值
		// 小对象一次批量上限高
		// 小对象一次批量上限低
		int num = MAX_BYTES / size;
		if (num < 2)
			num = 2;

		if (num > 512)
			num = 512;

		return num;
	}

	// 计算一次向系统获取几个页
	// 单个对象 8byte
	// 单个对象 256KB
	static size_t NumMovePage(size_t size)
	{
		size_t num = NumMoveSize(size);
		size_t npage = num * size;

		npage >>= PAGE_SHIFT;
		if (npage == 0)
			npage = 1;

		return npage;
	}
};

// 管理多个连续页大块内存跨度结构
struct Span
{
	PAGE_ID _pageId = 0;//大块内存起始页的页号
	size_t _n = 0;// 页的数量

	Span* _next = nullptr;	// 双向链表结构
	Span* _prev = nullptr;

	size_t _useCount = 0;	//	切好的小块内存被分配给 Thread Cache的计数
	void* _freeList = nullptr;	//	切好的小块内存自由链表
};

//带头双向循环链表
class SpanList
{
public:
	SpanList()
	{
		_head = new Span;
		_head->_next = _head;
		_head->_prev = _head;
	}

	void Insert(Span* pos, Span* newSpan)
	{
		assert(pos);
		assert(newSpan);

		Span* prev = pos->_prev;
		prev->_next = newSpan;
		newSpan->_next = pos;
		newSpan->_prev = prev;
		pos->_prev = newSpan;
	}
	
	void Erase(Span* pos)
	{
		assert(pos);
		assert(nullptr != pos);

		Span* next = pos->_next;
		Span* prev = pos->_prev;

		prev->_next = next;
		next->_prev = prev;
	}

	void PushFront(Span* span)
	{
		Insert(Begin(), span);
	}

	Span* PopFront()
	{
		Span* front = _head->_next;
		Erase(front);
		return front;
	}

	bool Empty()
	{
		return _head->_next == _head;
	}

	Span* Begin()
	{
		return _head->_next;
	}

	Span* End()
	{
		return _head;
	}
private:
	Span* _head;

public:
	std::mutex _mtx;//桶锁
};

#endif // !__COMMON_H__
