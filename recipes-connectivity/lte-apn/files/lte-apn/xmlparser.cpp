/******************************************************************************

 Copyright (c) 2020 VIA Technologies, Inc. All Rights Reserved.

 This PROPRIETARY SOFTWARE is the property of VIA Technologies, Inc.
 and may contain trade secrets and/or other confidential information of
 VIA Technologies, Inc. This file shall not be disclosed to any third
 party, in whole or in part, without prior written consent of VIA.

 THIS PROPRIETARY SOFTWARE AND ANY RELATED DOCUMENTATION ARE PROVIDED AS IS,
 WITH ALL FAULTS, AND WITHOUT WARRANTY OF ANY KIND EITHER EXPRESS OR IMPLIED,
 AND VIA TECHNOLOGIES, INC. DISCLAIMS ALL EXPRESS OR IMPLIED
 WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE, QUIET
 ENJOYMENT OR NON-INFRINGEMENT.

******************************************************************************/
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "xmlparser.h"

static apn_t* theApn = NULL;

//没有子节点，所以下方法舍弃
// static int parse_apn(xmlDocPtr doc, xmlNodePtr cur)
// {
//     // assert(doc || cur);
//     xmlChar *key;
// 	bool findMCC, findMNC = false;//如果该2个都为true代表找到对应的apn
//     cur = cur->xmlChildrenNode;
//     while (cur != NULL) {
// 		if ((!xmlStrcmp(cur->name, (const xmlChar *)"mcc"))) {
// 			key = xmlNodeListGetString(doc, cur->xmlChildrenNode, 1);
// 			printf("mcc: %s\t", key);
// 			if(strcmp(theApn->mcc, (const char*)key)==0){//判断mcc是否是要找的mcc
// 				findMCC = true;
// 			}
// 			xmlFree(key);
// 		}
// 		if ((!xmlStrcmp(cur->name, (const xmlChar *)"mnc"))) {
// 			key = xmlNodeListGetString(doc, cur->xmlChildrenNode, 1);
// 			printf("mnc: %s\t", key);
// 			if(strcmp(theApn->mnc, (const char*)key)==0){//判断mnc是否是要找的mnc
// 				findMNC = true;
// 			}
// 			xmlFree(key);
// 		}
// 		//获取user
// 		if ((!xmlStrcmp(cur->name, (const xmlChar *)"user"))) {
// 			key = xmlNodeListGetString(doc, cur->xmlChildrenNode, 1);
// 			printf("user: %s\t", key);
// 			if(findMCC && findMNC){
// 				strcpy(theApn->user, (const char*)key);
// 			}
// 			xmlFree(key);
// 		}
// 		//获取password
// 		if ((!xmlStrcmp(cur->name, (const xmlChar *)"password"))) {
// 			key = xmlNodeListGetString(doc, cur->xmlChildrenNode, 1);
// 			printf("password: %s\n", key);
// 			if(findMCC && findMNC){
// 				strcpy(theApn->password, (const char*)key);
// 			}
// 			xmlFree(key);
// 		}
// 		cur = cur->next;
//     }
// 	if(findMCC && findMNC){
// 		printf("FIND SUCCESS!\n");
// 		return 1;
// 	}
//     return 0;
// }

static int parse_apn_file(const char *file_name)
{
    // assert(file_name);

    xmlDocPtr doc;   //xml整个文档的树形结构
    xmlNodePtr cur;  //xml节点
    //xmlChar *carrier = NULL;     //carrier
    xmlChar *mcc = NULL;     //mcc
    xmlChar *mnc = NULL;     //mnc
    xmlChar *apn = NULL;     //apn
    xmlChar *user = NULL;     //user
    xmlChar *password = NULL;     //password
    xmlChar *type = NULL;     //type
	//int ret = 0;
	int findMCC, findMNC;//如果该2个都为true代表找到对应的apn
	int found_apn = 0;

	printf("%s: parse %s\n", __func__, file_name);

    //获取树形结构
    doc = xmlParseFile(file_name);
    if(doc == NULL)
	{
		fprintf(stderr, "Failed to parse xml file:%s\n", file_name);
		goto FAILED;
    }
    //获取根节点
    cur = xmlDocGetRootElement(doc);
    if(cur == NULL)
	{
		fprintf(stderr, "Root is empty.\n");
		goto FAILED;
    }

    if(xmlStrcmp(cur->name, (const xmlChar *)"apns"))
	{
		fprintf(stderr, "The root is not apns.\n");
		goto FAILED;
    }

    //遍历处理根节点的每一个子节点
    cur = cur->xmlChildrenNode;
    while (cur != NULL)
	{
		if(!xmlStrcmp(cur->name, (const xmlChar *)"apn"))
		{
			findMCC = 0;
			findMNC = 0;

			//carrier = xmlGetProp(cur, (const xmlChar*)"carrier");
			mcc = xmlGetProp(cur, (const xmlChar*)"mcc");
			mnc = xmlGetProp(cur, (const xmlChar*)"mnc");
			apn = xmlGetProp(cur, (const xmlChar*)"apn");
			user = xmlGetProp(cur, (const xmlChar*)"user");
			password = xmlGetProp(cur, (const xmlChar*)"password");
			type = xmlGetProp(cur, (const xmlChar*)"type");

			//printf("carrier: %s, mcc: %s, mnc: %s, apn: %s, user: %s, password: %s, type: %s\n", carrier, mcc, mnc, apn, user, password, type);
			if (mcc == NULL || mnc == NULL)
			{
				if(found_apn == 1)
				{
					// printf("--------> find_apn = 1\n");
					break;
				}
				cur = cur->next;
				continue;
			}
			
			if(strcmp(theApn->mcc, (char*)mcc) == 0) //判断mcc是否是要找的mcc
			{
				findMCC = 1;
			}

			if(strcmp(theApn->mnc, (char*)mnc) == 0) //判断mnc是否是要找的mnc
			{
				findMNC = 1;
			}

			if(findMCC && findMNC)
			{
				if(apn)
				{
					printf("%s: Found APN: %s\n", __func__, (char*)apn);

					found_apn = 1;

					if(strlen(theApn->apn) == 0)
					{
						strcpy(theApn->apn, (char*)apn);

						if(user)
							strcpy(theApn->user, (char*)user);

						if(password)
							strcpy(theApn->password, (char*)password);

						if(type)
						{
							strcpy(theApn->type, (char*)type);

							//We prefer to select the apn with type containing the "default" field. For examle: type="default,supl".
							if(strstr((char *)type, "default"))
							{
								//printf("--------> find default\n");
								break;
							}
						}
					}
					else
					{
						if(type)
						{
							//We prefer to select the apn with type containing the "default" field. For examle: type="default,supl".
							if(strstr((char *)type, "default"))
							{
								memset(theApn->apn, 0 , sizeof(theApn->apn));
								memset(theApn->user, 0 , sizeof(theApn->user));
								memset(theApn->password, 0 , sizeof(theApn->password));
								memset(theApn->type, 0 , sizeof(theApn->type));

								if(apn)
									strcpy(theApn->apn, (char*)apn);

								if(user)
									strcpy(theApn->user, (char*)user);

								if(password)
									strcpy(theApn->password, (char*)password);

								if(type)
									strcpy(theApn->type, (char*)type);

								break;
							}
						}
					}
				}
			}
			else
			{
				if(found_apn == 1)
				{
					// printf("--------> find_apn = 1\n");
					break;
				}
			}

			//if(carrier)
			//{
			// xmlFree(carrier);
			// carrier = NULL;
			//}

			if(mnc)
			{
				xmlFree(mnc);
				mnc = NULL;
			}

			if(mcc)
			{
				xmlFree(mcc);
				mcc = NULL;
			}

			if(apn)
			{
				xmlFree(apn);
				apn = NULL;
			}

			if(user)
			{
				xmlFree(user);
				user = NULL;
			}

			if(password)
			{
				xmlFree(password);
				password = NULL;
			}

			if(type)
			{
				xmlFree(type);
				type = NULL;
			}
		}
		cur = cur->next;
    }

	//if(carrier)
	//{
	// xmlFree(carrier);
	//}

	if(mnc)
	{
		xmlFree(mnc);
	}

	if(mcc)
	{
		xmlFree(mcc);
	}

	if(apn)
	{
		xmlFree(apn);
	}

	if(user)
	{
		xmlFree(user);
	}

	if(password)
	{
		xmlFree(password);
	}

	if(type)
	{
		xmlFree(type);
	}

    xmlFreeDoc(doc);

	if(strlen(theApn->apn) > 0)
	{
		if(strlen(theApn->type) > 0)
			printf("%s: The Best APN is \"%s\". Type = \"%s\"\n", __func__, theApn->apn, theApn->type);
		else
			printf("%s: The Best APN is \"%s\"\n", __func__, theApn->apn);
	}

    return 0;

FAILED:
    if(doc)
	{
  	  xmlFreeDoc(doc);
    }
    return -1;
}

/***************************************************************************************************
Function   : findApn
Description: Find APN from /etc/apns-conf.xml according to MCC and MNC.
             The MCC and MNC are read from SIM card using AT command "AT+CIMI".
Parameter  : mcc: the MCC of SIM card
             mnc: the MNC of SIM card
Return     : apn_t: the apn_t structure which includes apn, user, password for dial-up
***************************************************************************************************/
apn_t* findApn(const char* mcc,const char* mnc){
	if(theApn == NULL){
		theApn = (apn_t*)malloc(sizeof(apn_t));
	}
	memset(theApn, 0 , sizeof(apn_t));
	strcpy(theApn->mcc, mcc);
	strcpy(theApn->mnc, mnc);
	const char *xml_file = DEFAULT_XML_FILE;

	int fd = open(xml_file, O_RDONLY|O_SYNC|O_NONBLOCK);
	if (fd < 0)
		xml_file = TEST_XML_FILE;
	else
		close(fd);

	if (parse_apn_file(xml_file) != 0) {
		fprintf(stderr, "Failed to parse apn from file %s.\n", xml_file);
    }
	return theApn;
}

/***************************************************************************************************
Function   : freeApn
Description: Free the memory allocated by findApn()
Return     : None
***************************************************************************************************/
void freeApn(void)
{
	if(theApn)
	{
		free(theApn);
		theApn = NULL;
	}
}

/*
int main(int argc, char * argv[])
{
	printf("[%s] start!\n",argv[0]);

	if(theApn == NULL){
		theApn = (apn_t*)malloc(sizeof(apn_t));
	}
	memset(theApn, 0 , sizeof(apn_t));
	strcpy(theApn->mcc, "466");
	strcpy(theApn->mnc, "92");

    char *xml_file = DEFAULT_XML_FILE;

    if (argc == 2) {
    	xml_file = argv[1];
    }

	printf("[%s] search mcc = %s, mnc = %s\n", argv[0], theApn->mcc, theApn->mnc);

    if (parse_apn_file(xml_file) != 0) {
		fprintf(stderr, "Failed to parse apn.\n");
		return -1;
    }

    return 0;
}
*/
