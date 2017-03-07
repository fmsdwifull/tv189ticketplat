/******************
 *Copyright  (c)  2012-11-10  surfing
 *All  rights reserved
 *
 *
 *****************/


/*****************
 *author:pxh
 *date:  2012/10/11
 ***************/

/*
 *	v1.1
 *	suport:	
 *		fix a bug that in the multithread case.
 */

#ifndef _PURE_LOG_H_
#define _PURE_LOG_H_


#include "sys_headers.h"
#include "basic_type.h"
#include "atom_opr.h"

//#define  PURE_LOG_MAX_TRACE   (1024*4)     	   //TRACE内容最大4K
#define  PURE_LOG_MAX_FILE_NAME_LEN  256      //文件名最大长度为255
#define  PURE_LOG_MAX_TIME_STR_LEN   128      //MAX时间字符串长度
#define  PURE_LOG_MAX_PARA_STR_LEN   512	  //自定义日志内容最大长度512

//LOG_STRATEGE_TYPE
#define  PURE_LOG_BUFSIZE  (1024*1024)
#define  PURE_LOG_MAX_FILE_SIZE  (10*1024*1024)

#ifdef TRACE

//#define TRACE_IN(a,b) (a)->trace_in((b));
//#define TRACE_OUT(a)	(a)->trace_out();
//#define DEBUG(a,b,...)	(a)->prt(PURE_LOG_TYPE_TERM,"File:%s, Line:%d, Function:%s\n\t"b,__FILE__,__LINE__,__FUNCTION__,##__VA_ARGS__);
#define NDEBUG(b,...)	printf("File:%s, Line:%d, Function:%s\t"b"\n",__FILE__,__LINE__,__FUNCTION__,##__VA_ARGS__);
#else

//#define TRACE_IN(a,b)
//#define TRACE_OUT(a)
//#define DEBUG(a,b,...)
#define NDEBUG(b,...)

#endif



namespace p_base
{

//LOG_TYPE类型
typedef enum PURE_LOG_TYPE_E
{
	LOG_TERM =0,	//直接输出到终端
	LOG_CRIT,       //致命错误
	LOG_ERROR,
	LOG_WARN,
	LOG_INFO,
	LOG_DEBUG
}PLT_E;

//LOG策略类型
enum PURE_LOG_STRATEGE_TYPE_E
{
	PURE_LOG_TIME = 0,				//按指定时间或日期生成文件。
	PURE_LOG_SPACE				    //按日志文件大小生成文件，并限制为一定数量
};

enum PURE_LOG_TIME_STRATEGE_E
{
	HOUR,
	DAY,
	WEEK
};

class CLog
{
public:
	CLog();
	virtual ~CLog();

	s32 init(const char * i_path,const char * i_filename);
	s32 prt(PURE_LOG_TYPE_E i_log_lvl,const char * i_fmt,...);
	s32 set_log_out_by_time(int i);
	s32 set_log_out_by_amount(s32 i_len,s32 i_size);
	s32 set_log_level(PURE_LOG_TYPE_E i_lvl);


protected:

	char m_filename[PURE_LOG_MAX_FILE_NAME_LEN];	//pathname+filename

	PURE_LOG_STRATEGE_TYPE_E m_stratege_type;			//储存策略
	PURE_LOG_TIME_STRATEGE_E m_time_stratege_type;	//时间策略
	s32 m_amount_size;								//文件数量
	PURE_LOG_TYPE_E m_log_level;					//日志级别
	struct tm  m_cur_filetime[2];					//当前文件创建时间
	u32 m_cur_file_size[2];						//当前文件大小
		
	u32 m_max_file_size;							//单个文件最大长度

	s32 m_fd[2];									//2个FD组
	volatile u32 m_pos;							//文件生成位置
	volatile s32 m_busy;							//正在创建文件标志
	s32 new_file();

};

} /* namespace pure_baselib */
#endif /* PURELOG_H_ */
