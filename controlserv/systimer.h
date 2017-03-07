#ifndef __SYSTEM_TIMER_H
#define __SYSTEM_TIMER_H




#include <time.h>
#include <iostream>



#include<sys/wait.h>






#define PER_DAY_SEC 18600


//#define PER_HOUR_SEC 60*5    //一秒为单位，表示5分钟一次

//#define PER_HOUR_SEC 60*5   //一秒为单位，表示5分钟一次
//#define PER_ONCE  24*(60/5)  //一天游多少个5分钟 





using namespace std; 

typedef  struct{
	time_t secs;
	bool   flag;
}time_elems; 

#endif

