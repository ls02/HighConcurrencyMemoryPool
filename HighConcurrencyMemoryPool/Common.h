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
 * @brief 用于获取一个对象里面存放的地址
 * @param obj 需要获取地址的对象
 * @return 获取完成后返回该对象地址的引用
*/
void*& NextObj(void* obj)
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

private:
	void* _freeList = nullptr;
};


#endif // !__COMMON_H__
