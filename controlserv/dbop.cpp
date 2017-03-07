
#include "dbop.h"


Dboperatong::Dboperatong()
{

}
Dboperatong::~Dboperatong()
{

}

char* Dboperatong::getdbdir()
{

}
DIR *Dboperatong::open_db_dir(char *dir)
{  
	DIR *ret_dir;


    if((ret_dir=opendir(dir))==NULL)
    {
        fprintf(stderr,"cannot open directory:%s\n",dir);
        return NULL;
    }
	return ret_dir;
}
char* Dboperatong::modifylogfile(char *dirname,char *filename)
{
	static char new_filename[128];
	char dst_cmd[128];	
	char tmp_buf[256];
	
	DIR* dir;
	
	strcpy(new_filename,"_");
	strcat(new_filename,filename);
    
	struct dirent *entry;
    struct stat statbuf;

		//open the dir
	if((dir=open_db_dir(dirname))==NULL)
	{
		perror("open dir error");
		exit(-1);
	}
	
	chdir(dirname);
    while((entry=readdir(dir))!=NULL)
    {
        lstat(entry->d_name,&statbuf);
        if(!S_ISDIR(statbuf.st_mode))//notice
        {
            if((strcmp(entry->d_name,filename)==0))
            {
				sprintf(tmp_buf,"cd %s;mv %s %s",dirname,filename,new_filename);
				system(tmp_buf);
            }
        }
    }
	return new_filename;

}


void *Dboperatong::modify_name(char* file)
{

}



/***********************dir and file name parase******************************/

int  Dboperatong::Fileparase(char *src,char *filename,char *dirname)
{
	int total_len=0;
	int sub_len=0;

	char *p;


	total_len=strlen(src);
    //printf("total_len:%d\n",total_len);
	p=strrchr(src,'/')+1;
	strcpy(filename,p);
	printf("file name is :%s\n",filename);

	sub_len=strlen(p);
	//printf("sub_len:%d\n",sub_len);

	src[total_len-sub_len]='\0';
	strcpy(dirname,src);
	//printf("dir is :%s\n",src);
	
	return 0;
}

