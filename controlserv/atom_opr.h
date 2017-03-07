/**************************************************************
* Copyright (c) 2012,surfing
* All rights reserved.
* Filename：atom_opr.h
* Description：原子操作的定义
***************************************************************/
/**************************************************************
* Version：1.0.0
* Original_Author：chengzhip
* Date：2013-05-14
* Modified_Author：
* Modified_Description：
* Modified_Date: 
***************************************************************/

#ifndef _ATOMIC_OPR_H_
#define _ATOMIC_OPR_H_


#define CAS(_ptr, _old_value, _new_value)	\
	__sync_bool_compare_and_swap(_ptr, _old_value, _new_value)

#define ATOMIC_ADD(_ptr, value)	\
	__sync_add_and_fetch(_ptr, value)

#define ATOMIC_FADD(_ptr, value)\
	__sync_fetch_and_add(_ptr, value)

#define ATOMIC_SUB(_ptr, value)	\
	__sync_sub_and_fetch(_ptr, value)

#define ATOMIC_FSUB(_prt, value)\
	__sync_fetch_and_sub(_prt, value)



#endif /* _ATOMIC_OPR_H_ */


