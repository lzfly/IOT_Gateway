#include <stdio.h>
#include <string.h>
#include "utils.h"

char* dupstr(char* s) {
	char* newStr;
	if(s == NULL) {
		return NULL;
	}
	newStr = (char*)malloc(strlen(s)+1);
	if(newStr != NULL) {
		strcpy(newStr, s);
	}
	return newStr;
}

char* strip(char* org) {
	size_t len;
	char* p;

	if(org == NULL) {
		return NULL;
	}
	len = strlen(org);
	p = org + len - 1;

	while(isspace(*p)) {
		*p = '\0';
		p--;
	}
	p = org;
	while(isspace(*p)) {
		p++;
	}

	return p;
}

int endsWith(char* s, char* s1) {
	if(s != NULL) {
		size_t len = strlen(s);
		size_t len1 = strlen(s1);
		if(len >= len1) {
			char* p = s + len - len1;
			if(strcmp(p, s1) == 0) {
				return BLST_TRUE;
			}
		}
	}
	return BLST_FALSE;
}

int startsWith(char* s, char* s1) {
	if(s != NULL) {
		size_t len = strlen(s);
		size_t len1 = strlen(s1);

		if(len >= len1 && strncmp(s, s1, strlen(s1)) == 0) {
			return BLST_TRUE;
		}
	}
	return BLST_FALSE;
}
