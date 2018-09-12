#ifndef _WORKTHREAD_BASIC_H
#define _WORKTHREAD_BASIC_H

#include "ThreadInterface.h"

class UTIL_CLASS  CWorkThread: public CThreadInferface
{
public:
	CWorkThread (){}
	virtual ~CWorkThread(){}

protected:
	virtual void OnStart() {}//什么都没做，提醒派生类，添加类似控制函数
	virtual void OnStop() {}//什么都没做，提醒派生类，添加类似控制函数

}; 
#endif