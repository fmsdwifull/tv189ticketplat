/**************************************************************
* Copyright (c) 2012,surfing
* All rights reserved.
* Filename：task_queue.h
* Description：声明任务队列类的变量和方法
***************************************************************/
/**************************************************************
* Version：1.0.0
* Original_Author：chengzhip
* Date：2013-05-14
* Modified_Author：
* Modified_Description：
* Modified_Date: 
***************************************************************/

#ifndef _TASK_QUEUE_H_
#define _TASK_QUEUE_H_

#include <iostream>

extern "C"{
	#include <stdlib.h>
}



//namespace baselib
//{
template <typename T>

class queue
{
public:
  queue(int init = 10);
  ~queue();
  bool is_empty() const;//判断队列是否为空
  void queue_trav() const;//遍历循环队列
  int get_queue_len() const;//返回队列中元素个数

  bool en_queue(T e);
  bool de_queue();

private:
  int size;//允许的最大存储空间，以元素为单位
  int front;//队列头指针
  int rear;//队列尾指针
  T *base;//存储空间基址
};

//init queue
template <typename T>
queue<T>::queue(int init)
{
  size = init;//初始大小
  base = new T[size];
  if(!base)
  	  exit(1);//存储分配失败
  front = rear = 0;//空栈中元素个数
}

//析构
template <typename T>
queue<T>::~queue()
{
 	delete base;
}

//获取队列长度
template <typename T>
int queue<T>::get_queue_len() const
{
  return ((rear-front+size)%size);
}



//入队
template <typename T>
bool queue<T>::en_queue(T e)
{
  if((rear+1)%size==front)
  {//队列满
   std::cout<<"queue is full!"<<std::endl;
     return false;
  }
  else
  {
    base[rear] = e;
    rear = (rear+1)%size;
    return true;
  }
}


//出队
template <typename T>
bool queue<T>::de_queue()
{//队列空
  if(front==rear)
  {
    return false;
  }
  std::cout<<base[front]<<std::endl;
  front = (front+1)%size;
  return true;
}


//队列遍历
template <typename T>
void queue<T>::queue_trav() const
{
  int length = (rear-front+size)%size;
  //std::cout<<"from end to start："<<std::endl;
  while(length--)
  { 
    std::cout<<base[front+length]<<" ";
  }
  std::cout<<std::endl;
}


//}
#endif
