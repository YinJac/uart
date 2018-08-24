#include <stdio.h>
#include <sys/reboot.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdint.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <sys/ioctl.h>
#include <sys/reboot.h>
#include <fcntl.h>
#include <errno.h>
#include <dirent.h>
#include <stdarg.h>

#include <curl/curl.h>
#include <curl/easy.h> 
#include <wdk/cdb.h>
#include <time.h>
#include <signal.h>
#include <json/json.h>
#include <stdarg.h>

#include "report_key.h"


#define DIRECTIVES_PATH "http://shreportsit.suning.com/shreport-web/shvoice/keyReport.htm?"

static int get_duercfg(char *pName,char **device_id){
	char *json_str=NULL;
	int err=fp_read_file(&json_str,"/root/appresources/dueros_config.json","r");
	if(err<0){
		err=-1;
		goto Err1;
	}	
	struct json_object *obj=json_tokener_parse(json_str);
	if(is_error(obj)){
		err=-2;
		printf("err %d", err);
		goto Err2;
	}
	char *attr=NULL;
	struct json_object *attr_obj=NULL;
	attr_obj=json_object_object_get(obj,pName);
	if(is_error(attr_obj)){
		err=-3;
		printf("err %d", err);
		goto Err3;
	}
	attr=json_object_get_string(attr_obj);
	if(!attr){
		err=-4;
		printf("err %d", err);
		goto Err3;

	}
	*device_id = strdup(attr);
Err3:
	json_object_put(obj);
Err2:
	free(json_str);
Err1:	
	return err;	
}


struct MemoryStruct {
  char *memory;
  size_t size;
};
 
static size_t
WriteMemoryCallback(void *contents, size_t size, size_t nmemb, void *userp)
{
  size_t realsize = size * nmemb;
  struct MemoryStruct *mem = (struct MemoryStruct *)userp;
 
  mem->memory = realloc(mem->memory, mem->size + realsize + 1);
  if(mem->memory == NULL) {
    /* out of memory! */ 
    printf("not enough memory (realloc returned NULL)\n");
    return 0;
  }
 
  memcpy(&(mem->memory[mem->size]), contents, realsize);
  mem->size += realsize;
  mem->memory[mem->size] = 0;
 
  return realsize;
}


int curl_init_and_reportdata(char *url,const char *params)
{
	CURLcode code;
	CURL *curl;
	CURLMcode mcode;
//	struct data config;
	char *progress_data = "* ";  
	struct curl_slist *httpHeaderSlist;
	struct curl_slist *list = NULL;
	 

	CURL *curl_handle;
	CURLcode res;
	
	struct MemoryStruct chunk;
	
	chunk.memory = malloc(1);  /* will be grown as needed by the realloc above */ 
	chunk.size = 0;    /* no data at this point */ 

	curl = curl_easy_init();
	if (curl == NULL) 
	{
		DEBUG_ERROR("curl_easy_init() failed\n");
		return -1;
	}
//	DEBUG_ERROR("%s",params);

	curl_easy_setopt(curl, CURLOPT_URL, url);
	list = curl_slist_append(list, "Content-type: application/json");
	curl_easy_setopt(curl, CURLOPT_HTTPHEADER, list);
	
	/* pass in a pointer to the data - libcurl will not copy */
	curl_easy_setopt(curl, CURLOPT_POSTFIELDS, params);

	curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
	curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0);


	//curl_easy_setopt(curl, CURLOPT_HTTP_VERSION, CURL_HTTP_VERSION_2_0);
//	curl_easy_setopt(curl, CURLOPT_HEADERFUNCTION, Header_callback);

//	curl_easy_setopt(curl, CURLOPT_PROGRESSFUNCTION, progress_func);  
//	curl_easy_setopt(curl, CURLOPT_PROGRESSDATA, progress_data); 


	//curl_easy_setopt(curl, CURLOPT_NOSIGNAL, 1L);

	if (0) //DEBUG_ON
	{
		curl_easy_setopt(curl, CURLOPT_VERBOSE, 1);
	}

	curl_easy_setopt(curl, CURLOPT_TIMEOUT, 3);
	
	curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteMemoryCallback);
 
  /* we pass our 'chunk' struct to the callback function */ 
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)&chunk);

	res = curl_easy_perform(curl);
	/* check for errors */ 
	if(res != CURLE_OK) {
	  fprintf(stderr, "curl_easy_perform() failed: %s\n",
			  curl_easy_strerror(res));
	}
	else {	
	//  printf("%lu bytes retrieved\n", chunk.size);
	  DEBUG_ERROR("%s", chunk.memory);
	}

	curl_easy_cleanup(curl);

	free(chunk.memory);
//	printf(LIGHT_RED "handler:%p\n"NONE, curl);

//	int re = analysis_json(chunk.memory);

	return 0;
}

#define MIN(a,b)    ((a)<(b) ? (a) : (b))

static int excute_cmd(char *rbuf, int rbuflen, const char *cmd, ...)
{
	va_list vlist;
	char *fmt_cmd;
	char buf[128];
	int rc = 0;
	FILE *pfile;
	int status = -2;
	char *p = rbuf;
	printf( "cmd: %s\n", cmd);

	rbuflen = (!rbuf) ? 0 : rbuflen;

	va_start(vlist, (char *)cmd);
	vsnprintf(buf, sizeof(buf), cmd, vlist);
	va_end(vlist);

	printf( "Executing command: %s\n", buf);


	if ((pfile = popen(buf, "r"))) {
		fcntl(fileno(pfile), F_SETFD, FD_CLOEXEC);
		while(!feof(pfile)) {
			if ((rbuflen > 0) && fgets(buf, MIN(rbuflen, sizeof(buf)), pfile)) {
				int len = snprintf(p, rbuflen, "%s", buf);
				rbuflen -= len;
				p += len;
			}
            else {
                break;
            }
		}
		if ((rbuf) && (p != rbuf) && (*(p-1) == '\n')) {
			*(p-1) = 0;
		}
		status = pclose(pfile);
	}


	return rc;
}

char cur_time[32] = {0};
int get_cur_time()

{

	char*wday[]={"Sun","Mon","Tue","Wed","Thu","Fri","Sat"};

	time_t t;

	struct tm *p;

	t=time(NULL);/*获取从1970年1月1日零时到现在的秒数，保存到变量t中*/

	p=localtime(&t); /*localtime current time ; gmtime UTC time*/ 

	sprintf(cur_time,"%d-%02d-%02d %02d:%02d:%02d",(1900+p->tm_year),(1+p->tm_mon),p->tm_mday, p->tm_hour,p->tm_min, p->tm_sec);

//	printf(" %s ", cur_time);

	return 0;

}


void report_key(char *key_name)
{
	char VOICE_VER[16] = {0};
	char date[128] = {0};
	char mac_addr[32] = {0};
	char OTA_PATH[128] = {0};
	char *hardVersion = "F-8198v1.2";

	cdb_get("$ota_path",OTA_PATH);
	cdb_get("$sw_build",VOICE_VER);
	cdb_get("$wanif_mac",mac_addr);
//	excute_cmd(date,sizeof(date)-1,"date  +'%Y-%m-%d %H:%M:%S'");
	char *device_id = NULL;
	int err=get_duercfg("device_id",&device_id);
		
	get_cur_time();

    struct json_object *infor_object = NULL;  
    infor_object = json_object_new_object();  
    if (NULL == infor_object)  
    {  
        printf("new json object failed.\n");  
        return;  
    }

    struct json_object *para_object = NULL;  
    para_object = json_object_new_object();  
    if (NULL == para_object)  
    {  
        json_object_put(infor_object);//free  
        printf("new json object failed.\n");  
        return;  
    }	
	struct json_object *array_object = NULL;  
	array_object = json_object_new_array();  
	if (NULL == array_object)	
	{	
		json_object_put(infor_object);//free  
		json_object_put(para_object);//free  
		printf("new json object failed.\n");  
		return;  
	}

    json_object_object_add(para_object, "sysVersion", json_object_new_string(VOICE_VER));  
    json_object_object_add(para_object, "hardVersion", json_object_new_string(hardVersion)); 
	json_object_object_add(para_object, "deviceId", json_object_new_string(device_id));  
    json_object_object_add(para_object, "macAddress", json_object_new_string(mac_addr));	
    json_object_object_add(para_object, "keyName", json_object_new_string(key_name));  
    json_object_object_add(para_object, "time", json_object_new_string(cur_time));  

	json_object_array_add(array_object, para_object); 

	json_object_object_add(infor_object, "events", array_object);  

/*	
	printf("-----------json infor ---------------------------\n");	
	printf("%s\n", json_object_to_json_string(infor_object));  
	printf("-----------json infor ---------------------------\n"); 

	printf("-----------array_object ---------------------------\n");	
	printf("%s\n", json_object_to_json_string(array_object));  
	printf("-----------array_object ---------------------------\n"); 

	
	struct json_object *result_object = NULL; 
	result_object = json_object_object_get(infor_object, "events");  
	printf("-----------result_object  array---------------------------\n");  
	printf("%s\n", json_object_to_json_string(result_object));	
	printf("-----------result_object array---------------------------\n"); 

*/
	if (NULL != device_id){
		free(device_id);
	}

	curl_init_and_reportdata(DIRECTIVES_PATH,json_object_to_json_string(infor_object));

//	DEBUG_ERROR("%s",host_header);
	

}



