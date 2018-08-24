#include "uart_start.h"
#include "uart_protocol.h"
#include "dueros_handle.h"
#include <json/json.h>
#include <mon_config.h>

#if 0
//C json数据处理
int get_duercfg_volume(char *pName,int *pVol){
	char *json_str=NULL;
	int err=fp_read_file(&json_str,"/root/appresources/dueros_config.json","r");
	if(err<0){
		err=-1;
		goto Err1;
	}	
	struct json_object *obj=json_tokener_parse(json_str);
	if(is_error(obj)){
		err=-2;
		goto Err2;
	}
	char *attr=NULL;
	struct json_object *attr_obj=NULL;
	attr_obj=json_object_object_get(obj,pName);
	if(is_error(attr_obj)){
		err=-3;
		goto Err2;
	}
	attr=json_object_get_string(attr_obj);
	if(!attr){
		err=-4;
		goto Err2;

	}
	*pVol=atoi(attr);	
Err2:
	free(json_str);
Err1:	
	return err;	
}
#endif

int delete_file(char *filename)
{
    char cmd[64];
    
    memset(cmd,0,64);
    if(0 == access(filename,F_OK))
    {
        sprintf(cmd,"rm %s",filename);
        system(cmd);
        usleep(100*1000);
    }
}

int file_exists(const char *path)    // note: anything but a directory
{
    struct stat st;
    
    memset(&st, 0 , sizeof(st));
    return (stat(path, &st) == 0) && (!S_ISDIR(st.st_mode)) && (st.st_size > 0);
}


int hangshu(char file[])
{

 int ch=0;
 int n=0;
 FILE *fp;
 fp=fopen(file,"r");
 if(fp==NULL)
      return -1;
 while((ch = fgetc(fp)) != EOF) 
         {  
             if(ch == '\n')  
             {  
                 n++;  
             } 
         }  

 fclose(fp); 
 return n; 

}


int compare_FileAndUrl(char *filepatch,char *Url)
{

   if((NULL!=Url)&&(NULL!=filepatch)){

      FILE *dp =NULL;
	  char *deline;
      char buf[512];
	  int ret=0;
	  int i;
	  int hangnum=hangshu(filepatch);
	  printf("hangnum===[%d]\n",hangnum);
   	   dp= fopen(filepatch, "r+");
	    if(NULL==dp){
		   DEBUG_INFO("file no exist\n");
           return -1;
		}
            
	         for(i=0;i<hangnum;i++)
	         {
			 deline=fgets(buf,512, dp);
			 if(deline==NULL)
			 	{
			 	    fclose(dp);
			 	 	return 0;
			 	}
			 ret=strncmp(deline,Url,strlen(Url)-1);
			 if(0==ret){
			 	i++;
				fclose(dp);
			 	return i;
			 	}	      	 
			 }
			 
			 
			 fclose(dp);
			 dp= NULL;
		  
		return 0;
 }
}

void Process_AT_TIME_CMD(void *pcmd)
{
	struct   tm     *ptm;		
	long       ts;		
	int         year,hour,day,month,minute,second,week;		

	char run[64]={0};		
	ts   =   time(NULL);		
	ptm   =   localtime(&ts); 				
	year   =   ptm-> tm_year+1900;     //?ê		
	month   =   ptm-> tm_mon+1;             //??		
	day   =   ptm-> tm_mday;               //è?		
	hour   =   ptm-> tm_hour;               //ê±		
	minute   =   ptm-> tm_min;                 //·?		
	second   =   ptm-> tm_sec;                 //?? 		
	week = ptm->tm_wday;				

	snprintf(run, 64, "AXX+TIM+%02d%02d%02d%02d%d%02d%4d&", second, minute, hour, day, week, month, year);

	ready_send_data(UART, run, strlen(run));
	
	return ;
}


void pipe_usboul_handler(void *argv)
{
	const char *send_data = "AXX+USB+OUL&";
	ready_send_data(UART, send_data, strlen(send_data));
}

unsigned int pipe_handle_analy[128] = {0};

protocol_handle pipe_handle[] = {
	{"usb", 					pipe_usboul_handler}
};

unsigned int uart_handle_analy[64] = {0};

protocol_handle uart_handle[] = {
	{"time",		Process_AT_TIME_CMD }
};

	
int get_sizeof_uart_handle()
{
	return sizeof(uart_handle)/sizeof(protocol_handle);
}

int get_sizeof_pipe_handle()
{
	return sizeof(pipe_handle)/sizeof(protocol_handle);
}


