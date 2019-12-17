#ifndef _CJSON_H_
#define _CJSON_H_

#define GET_JSON(fmt, ...) _get_json(fmt, ##__VA_ARGS__)
#define URL "http://paas.bk.com:80/api/c/compapi/v2/cc/search_host/"

typedef struct _host_info{
	char ip[128];
	char set_name[ELEMENT_SIZE];
}host_info_s;


#endif
