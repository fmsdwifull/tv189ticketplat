/**************************************************************
* Copyright (c) 2012,surfing
* All rights reserved.
* Filename：
* Description：
***************************************************************/
/**************************************************************
* Version：1.0.0
* Original_Author：pxh
* Date：2012-12-18-14.34
***************************************************************/


#include "pure_log.h"
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <error.h>
#include <errno.h>
#include <string.h>

namespace p_base
{


    const   char* lvl[7] = {"TERM","CRIT","ERROR","WARN","INFO","DEBUG","default"};

    CLog::CLog()
    {
		memset(m_filename,0,PURE_LOG_MAX_FILE_NAME_LEN);	//pathname+filename
		//当前文件创建时间
		memset(m_cur_filetime,0,sizeof(tm)*2);
		//当前文件大小
		m_cur_file_size[0] = 0;
		m_cur_file_size[1] = 0;
	
	
		m_pos = -1;						    	//文件生成位置
		m_busy = 0;								//正在创建文件标志
		m_fd[0] = -1;
		m_fd[1] = -1;
		
        m_cur_file_size[0] = 0;
	    m_cur_file_size[1] = 0;
	
	    m_max_file_size = PURE_LOG_MAX_FILE_SIZE;						   //单个文件最大长度
        m_amount_size = 10;
		
        m_stratege_type = PURE_LOG_TIME;
	    m_time_stratege_type = HOUR;
        m_log_level = LOG_DEBUG;
    }

    CLog::~CLog()
    {
    		if(m_fd[0] > 0)
			   close(m_fd[0]);
		    if(m_fd[1] > 0)
			   close(m_fd[1]);
    }


    //设置日志级别 s32 CLog::set_log_level(s32 i_lvl)
    /*
    *function:set log module level
    *
    */
    s32 CLog::set_log_level(PURE_LOG_TYPE_E i_lvl)
    {
        m_log_level = i_lvl;
        return 0;
    }


    //日志模块初始化:s32 log_init(const char * i_path,const char * i_filename)
    /*
    *function:init log module,set path and filename-prefix
    *@i_path: path
    *@i_filename: filename-prefix
    *return value:negative indicates failure,0 indicates success.
    */
    s32   CLog::init(const char * i_path,const char * i_filename)
    {
		if(strlen(i_path) + strlen(i_filename) > PURE_LOG_MAX_FILE_NAME_LEN - 1)
		{
		    printf("Too long path and filename\n");
		    return -1;
		}

		snprintf(m_filename,PURE_LOG_MAX_FILE_NAME_LEN,"%s%s",i_path,i_filename);
		new_file();
	    
    }

#if 0
    s32   CLog::trace_in(const char * i_trace_str)
    {
        s32 trace_node_len = strlen(i_trace_str);
        if(m_trace_lvl >= 256)
        {
            printf("Too many level in trace\n");
            return -1;
        }

        if(trace_node_len + m_trace_pos + 1 > PURE_LOG_MAX_TRACE)
        {
            printf("There is not enough space in trace.\n");
            return -1;
        }

        m_trace_queue[m_trace_lvl] = (s8)snprintf(m_trace+m_trace_pos,255,">%s",i_trace_str);
        m_trace_pos += m_trace_queue[m_trace_lvl];
        m_trace_lvl++;

        return 0;
    }

    s32 CLog::trace_out()
    {
        if(m_trace > 0)
        {
            m_trace_lvl--;
            m_trace_pos -= m_trace_queue[m_trace_lvl];
            m_trace[m_trace_pos] = '\0';

        }
        return 0;
    }
#endif

    s32 CLog::prt(PURE_LOG_TYPE_E i_log_lvl,const char * fmt,...)
    {
		if(i_log_lvl > m_log_level )
		{
		    return 0;
		}

		/*更新时间*/
		time_t t = time(NULL);
		struct tm  ptime;
		memset(&ptime,0,sizeof(ptime));
		localtime_r(&t,&ptime);

		/*更新时间字符串*/
		char date_str[PURE_LOG_MAX_TIME_STR_LEN] = {0};	
		snprintf(date_str,PURE_LOG_MAX_TIME_STR_LEN,"[%04d-%02d-%02d   %02d-%02d-%02d]",ptime.tm_year+1900, ptime.tm_mon+1, ptime.tm_mday, ptime.tm_hour, ptime.tm_min, ptime.tm_sec);

		/*拼接打印字符串*/
		char parameters[PURE_LOG_MAX_PARA_STR_LEN];
		va_list argptr;
		int cnt;
		va_start(argptr, fmt);
		cnt = vsnprintf(parameters,PURE_LOG_MAX_PARA_STR_LEN, fmt, argptr);
		va_end(argptr);


		/*是否在终端打印*/
        if(i_log_lvl == LOG_TERM)
        {
            printf("%s,%s\n",date_str,parameters);
            return 0;
        }
		
			
		char buf[PURE_LOG_MAX_PARA_STR_LEN*2] = {0};					//文件内容缓冲区
        	snprintf(buf,PURE_LOG_MAX_PARA_STR_LEN*2,"%s\t%s\t%s\n",lvl[i_log_lvl],date_str,parameters);
		u32 buf_len = strlen(buf);

		bool cut = false; 			//是否生成文件标志
		s32 ret = 0;				//write 函数返回值

		switch(m_stratege_type)
		{
		case PURE_LOG_TIME: 
			if(m_fd[m_pos & 0x1] == -1)
			{
				if(new_file() == FAILURE)
				{
					return -1; 
				}
			}
        		if(m_time_stratege_type == HOUR)
			{
				if(m_cur_filetime[m_pos & 0x1].tm_hour != ptime.tm_hour)
				{
					cut = true;
				}
			}
			else if(m_time_stratege_type == DAY)
			{ 
				if(m_cur_filetime[m_pos & 0x1].tm_mday != ptime.tm_mday)
				{
					cut = true;
				}
			}
			if(cut)
			{
				if(new_file() == FAILURE)
				{
					return -1; 
				}
				
			}
			
			if((ret = write(m_fd[m_pos & 0x1],buf,buf_len))> 0 )
			{				
				ATOMIC_FADD(&m_cur_file_size[m_pos & 0x1],ret);
			}
			return ret;
			break;
		case PURE_LOG_SPACE: 
			if(m_fd[m_pos & 0x1] == -1)
			{			
				if(new_file() == FAILURE)
				{
					return FAILURE; 
				}
			}
			if(m_cur_file_size[m_pos & 0x1] < m_max_file_size - PURE_LOG_MAX_PARA_STR_LEN)
			{

				if((ret = write(m_fd[m_pos & 0x1],buf,buf_len))> 0 )
				{
					
					ATOMIC_FADD(&m_cur_file_size[m_pos & 0x1],ret);
				}
				return ret;
			}
			else
			{
				if(new_file() == FAILURE)
				{
					return FAILURE; 
				}
				
				if((ret = write(m_fd[m_pos & 0x1],buf,buf_len))> 0 )
				{
					
					ATOMIC_FADD(&m_cur_file_size[m_pos & 0x1],ret);
				}
				return ret;
			} 
			break;
		}
		return 0;

    }


    s32 CLog::new_file()
    {
    		if(CAS(&m_busy,0,1) != true)
    		{
    			return RETRY;
    		}

		u32 m_next = (m_pos +1)&0x1;
		u32 m_index = 0;
		char filename_suffix[PURE_LOG_MAX_TIME_STR_LEN] = {0};
		char m_cur_filename[PURE_LOG_MAX_FILE_NAME_LEN] = {0};

		time_t t = time(NULL);
		struct tm  ptime;
		memset(&ptime,0,sizeof(ptime));
		localtime_r(&t,&ptime);
		
		switch(m_stratege_type)
		{
		case PURE_LOG_TIME:
			if(m_fd[m_next] == -1)
			{
				snprintf(filename_suffix,PURE_LOG_MAX_TIME_STR_LEN,"%04d%02d%02d_%02dh",ptime.tm_year+1900,ptime.tm_mon+1,ptime.tm_mday,ptime.tm_hour,ptime.tm_min);
				snprintf(m_cur_filename,PURE_LOG_MAX_FILE_NAME_LEN,"%s_%s.log",m_filename,filename_suffix);
				if((m_fd[m_next]=open(m_cur_filename,O_WRONLY|O_CREAT|O_APPEND,0644))==-1) 
				{
					printf("log mod : Open %s Error:%s\n",m_cur_filename,strerror(errno));
					m_busy = 0;
					return FAILURE;
				}

				m_cur_filetime[m_next] = ptime;
				m_cur_file_size[m_next] = 0;
				m_pos++;			//更新位置
				m_busy = 0;		//解锁

				return SUCCESS;
			}
			else
			{

				close(m_fd[m_next]);
				snprintf(filename_suffix,PURE_LOG_MAX_TIME_STR_LEN,"%04d%02d%02d_%02dh",ptime.tm_year+1900,ptime.tm_mon+1,ptime.tm_mday,ptime.tm_hour,ptime.tm_min);
				snprintf(m_cur_filename,PURE_LOG_MAX_FILE_NAME_LEN,"%s_%s.log",m_filename,filename_suffix);
				if((m_fd[m_next]=open(m_cur_filename,O_WRONLY|O_CREAT|O_APPEND,0644))==-1) 
				{
					printf("log mod : Open %s Error:%s\n",m_cur_filename,strerror(errno));
					m_busy = 0;
					return FAILURE;
				}

				
				m_cur_filetime[m_next] = ptime;
				m_cur_file_size[m_next] = 0;
				m_pos++;			//更新位置
				m_busy = 0; 	//解锁
				return SUCCESS;
				
			}
			
		case PURE_LOG_SPACE:

			if(m_fd[m_next] == -1)
			{
				m_index = (m_pos+1)%m_amount_size + 1; 
				snprintf(m_cur_filename,PURE_LOG_MAX_FILE_NAME_LEN,"%s_%d.log",m_filename,m_index);

				
				if((m_fd[m_next]=open(m_cur_filename,O_WRONLY|O_CREAT|O_APPEND,0644))==-1) 
				{
					printf("log mod : Open %s Error:%s\n",m_cur_filename,strerror(errno));
					m_busy = 0;
					return FAILURE;
				}

		
				m_cur_filetime[m_next] = ptime;
				m_cur_file_size[m_next] = 0;
				m_pos++;			//更新位置
				m_busy = 0; 	//解锁
				return SUCCESS;

			}
			else
			{
				close(m_fd[m_next]);
				m_index = (m_pos+1)%m_amount_size + 1; 
				snprintf(m_cur_filename,PURE_LOG_MAX_FILE_NAME_LEN,"%s_%d.log",m_filename,m_index);

				
				if((m_fd[m_next]=open(m_cur_filename,O_WRONLY|O_CREAT|O_APPEND,0644))==-1) 
				{
					printf("log mod : Open %s Error:%s\n",m_cur_filename,strerror(errno));
					m_busy = 0;
					return FAILURE;
				}

		
				m_cur_filetime[m_next] = ptime;
				m_cur_file_size[m_next] = 0;
				m_pos++;			//更新位置
				m_busy = 0; 	//解锁
				return SUCCESS;
			}
		}
		
    }


    s32 CLog::set_log_out_by_time(int h_or_d)
    {
		m_stratege_type = PURE_LOG_TIME;
		if(h_or_d >0)
		{
			m_time_stratege_type = HOUR;
		}
		else
		{
			m_time_stratege_type = DAY;
		}
		
		return SUCCESS;
    }



    s32 CLog::set_log_out_by_amount(s32 i_len,s32 i_size)
    {
        m_stratege_type = PURE_LOG_SPACE;
        m_amount_size = i_size;
        m_max_file_size = i_len;
        return SUCCESS;
    }

	
}
