#ifndef  _DBOP_H_
#define  _DBOP_H_

extern "C"{
      #include <string.h>
      #include <assert.h>
      #include <time.h>
      #include <string.h>
      #include <stdio.h>
      #include <stdlib.h>
      #include <errno.h>
      #include <sys/types.h>
      #include <sys/inotify.h>
      #include <error.h>
      
      #include <sys/types.h>
      #include <sys/stat.h>
      #include <dirent.h>
      #include <sys/select.h>
      
      #include <sys/ipc.h>
      #include <sys/shm.h>

}

#include <iostream>
#include <string>

class Dboperatong
{
	public:
			//get nfs path
			Dboperatong();
			~Dboperatong();
			char* getdbdir();
			DIR *open_db_dir(char *dir);
			char* modifylogfile(char *dirname,char *filename);
			void *modify_name(char* file);			
			int  Fileparase(char *src,char *filename,char *dirname);
	private:
		    char logdir[100];
			
};
#endif


