/*
 * Wininet - URL tests
 *
 * Copyright 2002 Aric Stewart
 * Copyright 2004 Mike McCormack
 * Copyright 2005 Hans Leidekker
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>

#include "windef.h"
#include "winbase.h"
#include "wininet.h"

#include "wine/test.h"

#define TEST_URL "http://www.winehq.org/site/about"
#define TEST_URL_HOST "www.winehq.org"
#define TEST_URL_PATH "/site/about"
#define TEST_URL2 "http://www.myserver.com/myscript.php?arg=1"
#define TEST_URL2_SERVER "www.myserver.com"
#define TEST_URL2_PATH "/myscript.php"
#define TEST_URL2_PATHEXTRA "/myscript.php?arg=1"
#define TEST_URL2_EXTRA "?arg=1"
#define TEST_URL3 "file:///C:/Program%20Files/Atmel/AVR%20Tools/STK500/STK500.xml"

#define CREATE_URL1 "http://username:password@www.winehq.org/site/about"
#define CREATE_URL2 "http://username@www.winehq.org/site/about"
#define CREATE_URL3 "http://username:"
#define CREATE_URL4 "http://www.winehq.org/site/about"
#define CREATE_URL5 "http://"
#define CREATE_URL6 "nhttp://username:password@www.winehq.org:80/site/about"
#define CREATE_URL7 "http://username:password@www.winehq.org:42/site/about"
#define CREATE_URL8 "https://username:password@www.winehq.org/site/about"
#define CREATE_URL9 "about:blank"
#define CREATE_URL10 "about://host/blank"
#define CREATE_URL11 "about:"
#define CREATE_URL12 "http://www.winehq.org:65535"

static inline void copy_compsA(
    URL_COMPONENTSA *src, 
    URL_COMPONENTSA *dst, 
    DWORD scheLen,
    DWORD hostLen,
    DWORD userLen,
    DWORD passLen,
    DWORD pathLen,
    DWORD extrLen )
{
    *dst = *src;
    dst->dwSchemeLength    = scheLen;
    dst->dwHostNameLength  = hostLen;
    dst->dwUserNameLength  = userLen;
    dst->dwPasswordLength  = passLen;
    dst->dwUrlPathLength   = pathLen;
    dst->dwExtraInfoLength = extrLen;
    SetLastError(0xfaceabad);
}

static inline void zero_compsA(
    URL_COMPONENTSA *dst, 
    DWORD scheLen,
    DWORD hostLen,
    DWORD userLen,
    DWORD passLen,
    DWORD pathLen,
    DWORD extrLen )
{
    ZeroMemory(dst, sizeof(URL_COMPONENTSA));
    dst->dwStructSize = sizeof(URL_COMPONENTSA);
    dst->dwSchemeLength    = scheLen;
    dst->dwHostNameLength  = hostLen;
    dst->dwUserNameLength  = userLen;
    dst->dwPasswordLength  = passLen;
    dst->dwUrlPathLength   = pathLen;
    dst->dwExtraInfoLength = extrLen;
    SetLastError(0xfaceabad);
}

static void InternetCrackUrl_test(void)
{
  URL_COMPONENTSA urlSrc, urlComponents;
  char protocol[32], hostName[1024], userName[1024];
  char password[1024], extra[1024], path[1024];
  BOOL ret;
  DWORD GLE;

  ZeroMemory(&urlSrc, sizeof(urlSrc));
  urlSrc.dwStructSize = sizeof(urlSrc);
  urlSrc.lpszScheme = protocol;
  urlSrc.lpszHostName = hostName;
  urlSrc.lpszUserName = userName;
  urlSrc.lpszPassword = password;
  urlSrc.lpszUrlPath = path;
  urlSrc.lpszExtraInfo = extra;

  copy_compsA(&urlSrc, &urlComponents, 32, 1024, 1024, 1024, 2048, 1024);
  ret = InternetCrackUrl(TEST_URL, 0,0,&urlComponents);
  ok( ret, "InternetCrackUrl failed, error %lx\n",GetLastError());
  ok((strcmp(TEST_URL_PATH,path) == 0),"path cracked wrong\n");

  /* Bug 1805: Confirm the returned lengths are correct:                     */
  /* 1. When extra info split out explicitly */
  zero_compsA(&urlComponents, 0, 1, 0, 0, 1, 1);
  ok(InternetCrackUrlA(TEST_URL2, 0, 0, &urlComponents),"InternetCrackUrl failed, error 0x%lx\n", GetLastError());
  ok(urlComponents.dwUrlPathLength == strlen(TEST_URL2_PATH),".dwUrlPathLength should be %ld, but is %ld\n", (DWORD)strlen(TEST_URL2_PATH), urlComponents.dwUrlPathLength);
  ok(!strncmp(urlComponents.lpszUrlPath,TEST_URL2_PATH,strlen(TEST_URL2_PATH)),"lpszUrlPath should be %s but is %s\n", TEST_URL2_PATH, urlComponents.lpszUrlPath);
  ok(urlComponents.dwHostNameLength == strlen(TEST_URL2_SERVER),".dwHostNameLength should be %ld, but is %ld\n", (DWORD)strlen(TEST_URL2_SERVER), urlComponents.dwHostNameLength);
  ok(!strncmp(urlComponents.lpszHostName,TEST_URL2_SERVER,strlen(TEST_URL2_SERVER)),"lpszHostName should be %s but is %s\n", TEST_URL2_SERVER, urlComponents.lpszHostName);
  ok(urlComponents.dwExtraInfoLength == strlen(TEST_URL2_EXTRA),".dwExtraInfoLength should be %ld, but is %ld\n", (DWORD)strlen(TEST_URL2_EXTRA), urlComponents.dwExtraInfoLength);
  ok(!strncmp(urlComponents.lpszExtraInfo,TEST_URL2_EXTRA,strlen(TEST_URL2_EXTRA)),"lpszExtraInfo should be %s but is %s\n", TEST_URL2_EXTRA, urlComponents.lpszHostName);

  /* 2. When extra info is not split out explicitly and is in url path */
  zero_compsA(&urlComponents, 0, 1, 0, 0, 1, 0);
  ok(InternetCrackUrlA(TEST_URL2, 0, 0, &urlComponents),"InternetCrackUrl failed with GLE 0x%lx\n",GetLastError());
  ok(urlComponents.dwUrlPathLength == strlen(TEST_URL2_PATHEXTRA),".dwUrlPathLength should be %ld, but is %ld\n", (DWORD)strlen(TEST_URL2_PATHEXTRA), urlComponents.dwUrlPathLength);
  ok(!strncmp(urlComponents.lpszUrlPath,TEST_URL2_PATHEXTRA,strlen(TEST_URL2_PATHEXTRA)),"lpszUrlPath should be %s but is %s\n", TEST_URL2_PATHEXTRA, urlComponents.lpszUrlPath);
  ok(urlComponents.dwHostNameLength == strlen(TEST_URL2_SERVER),".dwHostNameLength should be %ld, but is %ld\n", (DWORD)strlen(TEST_URL2_SERVER), urlComponents.dwHostNameLength);
  ok(!strncmp(urlComponents.lpszHostName,TEST_URL2_SERVER,strlen(TEST_URL2_SERVER)),"lpszHostName should be %s but is %s\n", TEST_URL2_SERVER, urlComponents.lpszHostName);
  ok(urlComponents.nPort == INTERNET_DEFAULT_HTTP_PORT,"urlComponents->nPort should have been 80 instead of %d\n", urlComponents.nPort);
  ok(urlComponents.nScheme == INTERNET_SCHEME_HTTP,"urlComponents->nScheme should have been INTERNET_SCHEME_HTTP instead of %d\n", urlComponents.nScheme);

  zero_compsA(&urlComponents, 1, 1, 1, 1, 1, 1);
  ok(InternetCrackUrlA(TEST_URL, strlen(TEST_URL), 0, &urlComponents),"InternetCrackUrl failed with GLE 0x%lx\n",GetLastError());
  ok(urlComponents.dwUrlPathLength == strlen(TEST_URL_PATH),".dwUrlPathLength should be %d, but is %ld\n", strlen(TEST_URL_PATH), urlComponents.dwUrlPathLength);
  ok(!strncmp(urlComponents.lpszUrlPath,TEST_URL_PATH,strlen(TEST_URL_PATH)),"lpszUrlPath should be %s but is %s\n", TEST_URL_PATH, urlComponents.lpszUrlPath);
  ok(urlComponents.dwHostNameLength == strlen(TEST_URL_HOST),".dwHostNameLength should be %d, but is %ld\n", strlen(TEST_URL_HOST), urlComponents.dwHostNameLength);
  ok(!strncmp(urlComponents.lpszHostName,TEST_URL_HOST,strlen(TEST_URL_HOST)),"lpszHostName should be %s but is %s\n", TEST_URL_HOST, urlComponents.lpszHostName);
  ok(urlComponents.nPort == INTERNET_DEFAULT_HTTP_PORT,"urlComponents->nPort should have been 80 instead of %d\n", urlComponents.nPort);
  ok(urlComponents.nScheme == INTERNET_SCHEME_HTTP,"urlComponents->nScheme should have been INTERNET_SCHEME_HTTP instead of %d\n", urlComponents.nScheme);
  ok(!urlComponents.lpszUserName, ".lpszUserName should have been set to NULL\n");
  ok(!urlComponents.lpszPassword, ".lpszPassword should have been set to NULL\n");
  ok(!urlComponents.lpszExtraInfo, ".lpszExtraInfo should have been set to NULL\n");
  ok(!urlComponents.dwUserNameLength,".dwUserNameLength should be 0, but is %ld\n", urlComponents.dwUserNameLength);
  ok(!urlComponents.dwPasswordLength,".dwPasswordLength should be 0, but is %ld\n", urlComponents.dwPasswordLength);
  ok(!urlComponents.dwExtraInfoLength,".dwExtraInfoLength should be 0, but is %ld\n", urlComponents.dwExtraInfoLength);

  /*3. Check for %20 */
  copy_compsA(&urlSrc, &urlComponents, 32, 1024, 1024, 1024, 2048, 1024);
  ok(InternetCrackUrlA(TEST_URL3, 0, ICU_DECODE, &urlComponents),"InternetCrackUrl failed with GLE 0x%lx\n",GetLastError());


  /* Tests for lpsz* members pointing to real strings while 
   * some corresponding length members are set to zero */
  copy_compsA(&urlSrc, &urlComponents, 0, 1024, 1024, 1024, 2048, 1024);
  ret = InternetCrackUrlA(TEST_URL3, 0, ICU_DECODE, &urlComponents);
  ok(ret==1, "InternetCrackUrl returned %d with GLE=%ld (expected to return 1)\n",
    ret, GetLastError());

  copy_compsA(&urlSrc, &urlComponents, 32, 0, 1024, 1024, 2048, 1024);
  ret = InternetCrackUrlA(TEST_URL3, 0, ICU_DECODE, &urlComponents);
  ok(ret==1, "InternetCrackUrl returned %d with GLE=%ld (expected to return 1)\n",
    ret, GetLastError());

  copy_compsA(&urlSrc, &urlComponents, 32, 1024, 0, 1024, 2048, 1024);
  ret = InternetCrackUrlA(TEST_URL3, 0, ICU_DECODE, &urlComponents);
  ok(ret==1, "InternetCrackUrl returned %d with GLE=%ld (expected to return 1)\n",
    ret, GetLastError());

  copy_compsA(&urlSrc, &urlComponents, 32, 1024, 1024, 0, 2048, 1024);
  ret = InternetCrackUrlA(TEST_URL3, 0, ICU_DECODE, &urlComponents);
  ok(ret==1, "InternetCrackUrl returned %d with GLE=%ld (expected to return 1)\n",
    ret, GetLastError());

  copy_compsA(&urlSrc, &urlComponents, 32, 1024, 1024, 1024, 0, 1024);
  ret = InternetCrackUrlA(TEST_URL3, 0, ICU_DECODE, &urlComponents);
  GLE = GetLastError();
  todo_wine
  ok(ret==0 && (GLE==ERROR_INVALID_HANDLE || GLE==ERROR_INSUFFICIENT_BUFFER),
     "InternetCrackUrl returned %d with GLE=%ld (expected to return 0 and ERROR_INVALID_HANDLE or ERROR_INSUFFICIENT_BUFFER)\n",
    ret, GLE);

  copy_compsA(&urlSrc, &urlComponents, 32, 1024, 1024, 1024, 2048, 0);
  ret = InternetCrackUrlA(TEST_URL3, 0, ICU_DECODE, &urlComponents);
  GLE = GetLastError();
  todo_wine
  ok(ret==0 && (GLE==ERROR_INVALID_HANDLE || GLE==ERROR_INSUFFICIENT_BUFFER),
     "InternetCrackUrl returned %d with GLE=%ld (expected to return 0 and ERROR_INVALID_HANDLE or ERROR_INSUFFICIENT_BUFFER)\n",
    ret, GLE);

  copy_compsA(&urlSrc, &urlComponents, 0, 0, 0, 0, 0, 0);
  ret = InternetCrackUrlA(TEST_URL3, 0, ICU_DECODE, &urlComponents);
  GLE = GetLastError();
  todo_wine
  ok(ret==0 && GLE==ERROR_INVALID_PARAMETER,
     "InternetCrackUrl returned %d with GLE=%ld (expected to return 0 and ERROR_INVALID_PARAMETER)\n",
    ret, GLE);

  copy_compsA(&urlSrc, &urlComponents, 32, 1024, 1024, 1024, 2048, 1024);
  ret = InternetCrackUrl("about://host/blank", 0,0,&urlComponents);
  ok(ret, "InternetCrackUrl failed with %ld\n", GetLastError());
  ok(!strcmp(urlComponents.lpszScheme, "about"), "lpszScheme was \"%s\" instead of \"about\"\n", urlComponents.lpszScheme);
  ok(!strcmp(urlComponents.lpszHostName, "host"), "lpszHostName was \"%s\" instead of \"host\"\n", urlComponents.lpszHostName);
  ok(!strcmp(urlComponents.lpszUrlPath, "/blank"), "lpszUrlPath was \"%s\" instead of \"/blank\"\n", urlComponents.lpszUrlPath);
}

static void InternetCrackUrlW_test(void)
{
    WCHAR url[] = {
        'h','t','t','p',':','/','/','1','9','2','.','1','6','8','.','0','.','2','2','/',
        'C','F','I','D','E','/','m','a','i','n','.','c','f','m','?','C','F','S','V','R',
        '=','I','D','E','&','A','C','T','I','O','N','=','I','D','E','_','D','E','F','A',
        'U','L','T', 0 };
    static const WCHAR url2[] = { '.','.','/','R','i','t','z','.','x','m','l',0 };
    URL_COMPONENTSW comp;
    WCHAR scheme[20], host[20], user[20], pwd[20], urlpart[50], extra[50];
    BOOL r;

    urlpart[0]=0;
    scheme[0]=0;
    extra[0]=0;
    host[0]=0;
    user[0]=0;
    pwd[0]=0;
    memset(&comp, 0, sizeof comp);
    comp.dwStructSize = sizeof comp;
    comp.lpszScheme = scheme;
    comp.dwSchemeLength = sizeof scheme;
    comp.lpszHostName = host;
    comp.dwHostNameLength = sizeof host;
    comp.lpszUserName = user;
    comp.dwUserNameLength = sizeof user;
    comp.lpszPassword = pwd;
    comp.dwPasswordLength = sizeof pwd;
    comp.lpszUrlPath = urlpart;
    comp.dwUrlPathLength = sizeof urlpart;
    comp.lpszExtraInfo = extra;
    comp.dwExtraInfoLength = sizeof extra;

    r = InternetCrackUrlW(url, 0, 0, &comp );
    ok( r, "failed to crack url\n");
    ok( comp.dwSchemeLength == 4, "scheme length wrong\n");
    ok( comp.dwHostNameLength == 12, "host length wrong\n");
    ok( comp.dwUserNameLength == 0, "user length wrong\n");
    ok( comp.dwPasswordLength == 0, "password length wrong\n");
    ok( comp.dwUrlPathLength == 15, "url length wrong\n");
    ok( comp.dwExtraInfoLength == 29, "extra length wrong\n");
 
    urlpart[0]=0;
    scheme[0]=0;
    extra[0]=0;
    host[0]=0;
    user[0]=0;
    pwd[0]=0;
    memset(&comp, 0, sizeof comp);
    comp.dwStructSize = sizeof comp;
    comp.lpszHostName = host;
    comp.dwHostNameLength = sizeof host;
    comp.lpszUrlPath = urlpart;
    comp.dwUrlPathLength = sizeof urlpart;

    r = InternetCrackUrlW(url, 0, 0, &comp );
    ok( r, "failed to crack url\n");
    ok( comp.dwSchemeLength == 0, "scheme length wrong\n");
    ok( comp.dwHostNameLength == 12, "host length wrong\n");
    ok( comp.dwUserNameLength == 0, "user length wrong\n");
    ok( comp.dwPasswordLength == 0, "password length wrong\n");
    ok( comp.dwUrlPathLength == 44, "url length wrong\n");
    ok( comp.dwExtraInfoLength == 0, "extra length wrong\n");

    urlpart[0]=0;
    scheme[0]=0;
    extra[0]=0;
    host[0]=0;
    user[0]=0;
    pwd[0]=0;
    memset(&comp, 0, sizeof comp);
    comp.dwStructSize = sizeof comp;
    comp.lpszHostName = host;
    comp.dwHostNameLength = sizeof host;
    comp.lpszUrlPath = urlpart;
    comp.dwUrlPathLength = sizeof urlpart;
    comp.lpszExtraInfo = NULL;
    comp.dwExtraInfoLength = sizeof extra;

    r = InternetCrackUrlW(url, 0, 0, &comp );
    ok( r, "failed to crack url\n");
    ok( comp.dwSchemeLength == 0, "scheme length wrong\n");
    ok( comp.dwHostNameLength == 12, "host length wrong\n");
    ok( comp.dwUserNameLength == 0, "user length wrong\n");
    ok( comp.dwPasswordLength == 0, "password length wrong\n");
    ok( comp.dwUrlPathLength == 15, "url length wrong\n");
    ok( comp.dwExtraInfoLength == 29, "extra length wrong\n");

    urlpart[0]=0;
    scheme[0]=0;
    extra[0]=0;
    host[0]=0;
    user[0]=0;
    pwd[0]=0;
    memset(&comp, 0, sizeof(comp));
    comp.dwStructSize = sizeof(comp);
    comp.lpszScheme = scheme;
    comp.dwSchemeLength = sizeof(scheme)/sizeof(scheme[0]);
    comp.lpszHostName = host;
    comp.dwHostNameLength = sizeof(host)/sizeof(host[0]);
    comp.lpszUserName = user;
    comp.dwUserNameLength = sizeof(user)/sizeof(user[0]);
    comp.lpszPassword = pwd;
    comp.dwPasswordLength = sizeof(pwd)/sizeof(pwd[0]);
    comp.lpszUrlPath = urlpart;
    comp.dwUrlPathLength = sizeof(urlpart)/sizeof(urlpart[0]);
    comp.lpszExtraInfo = extra;
    comp.dwExtraInfoLength = sizeof(extra)/sizeof(extra[0]);

    r = InternetCrackUrlW(url2, 0, 0, &comp);
    todo_wine {
    ok(!r, "InternetCrackUrl should have failed\n");
    ok(GetLastError() == ERROR_INTERNET_UNRECOGNIZED_SCHEME,
        "InternetCrackUrl should have failed with error ERROR_INTERNET_UNRECOGNIZED_SCHEME instead of error %ld\n",
        GetLastError());
    }
}

static void fill_url_components(LPURL_COMPONENTS lpUrlComponents)
{
	lpUrlComponents->dwStructSize = sizeof(URL_COMPONENTS);
	lpUrlComponents->lpszScheme = "http";
	lpUrlComponents->dwSchemeLength = strlen(lpUrlComponents->lpszScheme);
	lpUrlComponents->nScheme = INTERNET_SCHEME_HTTP;
	lpUrlComponents->lpszHostName = "www.winehq.org";
	lpUrlComponents->dwHostNameLength = strlen(lpUrlComponents->lpszHostName);
	lpUrlComponents->nPort = 80;
	lpUrlComponents->lpszUserName = "username";
	lpUrlComponents->dwUserNameLength = strlen(lpUrlComponents->lpszUserName);
	lpUrlComponents->lpszPassword = "password";
	lpUrlComponents->dwPasswordLength = strlen(lpUrlComponents->lpszPassword);
	lpUrlComponents->lpszUrlPath = "/site/about";
	lpUrlComponents->dwUrlPathLength = strlen(lpUrlComponents->lpszUrlPath);
	lpUrlComponents->lpszExtraInfo = "";
	lpUrlComponents->dwExtraInfoLength = strlen(lpUrlComponents->lpszExtraInfo);
}

static void InternetCreateUrlA_test(void)
{
	URL_COMPONENTS urlComp;
	LPSTR szUrl;
	DWORD len = -1;
	BOOL ret;

	/* test NULL lpUrlComponents */
	ret = InternetCreateUrlA(NULL, 0, NULL, &len);
	SetLastError(0xdeadbeef);
	ok(!ret, "Expected failure\n");
	ok(GetLastError() == 0xdeadbeef,
		"Expected 0xdeadbeef, got %ld\n", GetLastError());
	ok(len == -1, "Expected len -1, got %ld\n", len);

	/* test zero'ed lpUrlComponents */
	ZeroMemory(&urlComp, sizeof(URL_COMPONENTS));
	SetLastError(0xdeadbeef);
	ret = InternetCreateUrlA(&urlComp, 0, NULL, &len);
	ok(!ret, "Expected failure\n");
	ok(GetLastError() == ERROR_INVALID_PARAMETER,
		"Expected ERROR_INVALID_PARAMETER, got %ld\n", GetLastError());
	ok(len == -1, "Expected len -1, got %ld\n", len);

	/* test valid lpUrlComponets, NULL lpdwUrlLength */
	fill_url_components(&urlComp);
	SetLastError(0xdeadbeef);
	ret = InternetCreateUrlA(&urlComp, 0, NULL, NULL);
	ok(!ret, "Expected failure\n");
	ok(GetLastError() == ERROR_INVALID_PARAMETER,
		"Expected ERROR_INVALID_PARAMETER, got %ld\n", GetLastError());
	ok(len == -1, "Expected len -1, got %ld\n", len);

	/* test valid lpUrlComponets, emptry szUrl
	 * lpdwUrlLength is size of buffer required on exit, including
	 * the terminating null when GLE == ERROR_INSUFFICIENT_BUFFER
	 */
	SetLastError(0xdeadbeef);
	ret = InternetCreateUrlA(&urlComp, 0, NULL, &len);
	ok(!ret, "Expected failure\n");
	ok(GetLastError() == ERROR_INSUFFICIENT_BUFFER,
		"Expected ERROR_INSUFFICIENT_BUFFER, got %ld\n", GetLastError());
	ok(len == 51, "Expected len 51, got %ld\n", len);

	/* test correct size, NULL szUrl */
	fill_url_components(&urlComp);
	SetLastError(0xdeadbeef);
	ret = InternetCreateUrlA(&urlComp, 0, NULL, &len);
	ok(!ret, "Expected failure\n");
	ok(GetLastError() == ERROR_INSUFFICIENT_BUFFER,
		"Expected ERROR_INSUFFICIENT_BUFFER, got %ld\n", GetLastError());
	ok(len == 51, "Expected len 51, got %ld\n", len);

	/* test valid lpUrlComponets, alloced szUrl, small size */
	SetLastError(0xdeadbeef);
	szUrl = HeapAlloc(GetProcessHeap(), 0, len);
	len -= 2;
	ret = InternetCreateUrlA(&urlComp, 0, szUrl, &len);
	ok(!ret, "Expected failure\n");
	ok(GetLastError() == ERROR_INSUFFICIENT_BUFFER,
		"Expected ERROR_INSUFFICIENT_BUFFER, got %ld\n", GetLastError());
	ok(len == 51, "Expected len 51, got %ld\n", len);

	/* alloced szUrl, NULL lpszScheme
	 * shows that it uses nScheme instead
	 */
	SetLastError(0xdeadbeef);
	urlComp.lpszScheme = NULL;
	ret = InternetCreateUrlA(&urlComp, 0, szUrl, &len);
	ok(ret, "Expected success\n");
	ok(GetLastError() == 0xdeadbeef,
		"Expected 0xdeadbeef, got %ld\n", GetLastError());
	ok(len == 50, "Expected len 50, got %ld\n", len);
	ok(!strcmp(szUrl, CREATE_URL1), "Expected %s, got %s\n", CREATE_URL1, szUrl);

	/* alloced szUrl, invalid nScheme
	 * any nScheme out of range seems ignored
	 */
	fill_url_components(&urlComp);
	SetLastError(0xdeadbeef);
	urlComp.nScheme = -3;
	len++;
	ret = InternetCreateUrlA(&urlComp, 0, szUrl, &len);
	ok(ret, "Expected success\n");
	ok(GetLastError() == 0xdeadbeef,
		"Expected 0xdeadbeef, got %ld\n", GetLastError());
	ok(len == 50, "Expected len 50, got %ld\n", len);

	/* test valid lpUrlComponets, alloced szUrl */
	fill_url_components(&urlComp);
	SetLastError(0xdeadbeef);
	len = 51;
	ret = InternetCreateUrlA(&urlComp, 0, szUrl, &len);
	ok(ret, "Expected success\n");
	ok(GetLastError() == 0xdeadbeef,
		"Expected 0xdeadbeef, got %ld\n", GetLastError());
	ok(len == 50, "Expected len 50, got %ld\n", len);
	ok(strstr(szUrl, "80") == NULL, "Didn't expect to find 80 in szUrl\n");
	ok(!strcmp(szUrl, CREATE_URL1), "Expected %s, got %s\n", CREATE_URL1, szUrl);

	/* valid username, NULL password */
	fill_url_components(&urlComp);
	SetLastError(0xdeadbeef);
	urlComp.lpszPassword = NULL;
	len = 42;
	ret = InternetCreateUrlA(&urlComp, 0, szUrl, &len);
	ok(ret, "Expected success\n");
	ok(GetLastError() == 0xdeadbeef,
		"Expected 0xdeadbeef, got %ld\n", GetLastError());
	ok(len == 41, "Expected len 41, got %ld\n", len);
	ok(!strcmp(szUrl, CREATE_URL2), "Expected %s, got %s\n", CREATE_URL2, szUrl);

	/* valid username, empty password */
	fill_url_components(&urlComp);
	SetLastError(0xdeadbeef);
	urlComp.lpszPassword = "";
	len = 51;
	ret = InternetCreateUrlA(&urlComp, 0, szUrl, &len);
	ok(ret, "Expected success\n");
	ok(GetLastError() == 0xdeadbeef,
		"Expected 0xdeadbeef, got %ld\n", GetLastError());
	ok(len == 50, "Expected len 50, got %ld\n", len);
	ok(!strcmp(szUrl, CREATE_URL3), "Expected %s, got %s\n", CREATE_URL3, szUrl);

	/* valid password, NULL username
	 * if password is provided, username has to exist
	 */
	fill_url_components(&urlComp);
	SetLastError(0xdeadbeef);
	urlComp.lpszUserName = NULL;
	len = 42;
	ret = InternetCreateUrlA(&urlComp, 0, szUrl, &len);
	ok(!ret, "Expected failure\n");
	ok(GetLastError() == ERROR_INVALID_PARAMETER,
		"Expected ERROR_INVALID_PARAMETER, got %ld\n", GetLastError());
	ok(len == 42, "Expected len 42, got %ld\n", len);
	ok(!strcmp(szUrl, CREATE_URL3), "Expected %s, got %s\n", CREATE_URL3, szUrl);

	/* valid password, empty username
	 * if password is provided, username has to exist
	 */
	fill_url_components(&urlComp);
	SetLastError(0xdeadbeef);
	urlComp.lpszUserName = "";
	len = 51;
	ret = InternetCreateUrlA(&urlComp, 0, szUrl, &len);
	ok(ret, "Expected success\n");
	ok(GetLastError() == 0xdeadbeef,
		"Expected 0xdeadbeef, got %ld\n", GetLastError());
	ok(len == 50, "Expected len 50, got %ld\n", len);
	ok(!strcmp(szUrl, CREATE_URL5), "Expected %s, got %s\n", CREATE_URL5, szUrl);

	/* NULL username, NULL password */
	fill_url_components(&urlComp);
	SetLastError(0xdeadbeef);
	urlComp.lpszUserName = NULL;
	urlComp.lpszPassword = NULL;
	len = 42;
	ret = InternetCreateUrlA(&urlComp, 0, szUrl, &len);
	ok(ret, "Expected success\n");
	ok(GetLastError() == 0xdeadbeef,
		"Expected 0xdeadbeef, got %ld\n", GetLastError());
	ok(len == 32, "Expected len 32, got %ld\n", len);
	ok(!strcmp(szUrl, CREATE_URL4), "Expected %s, got %s\n", CREATE_URL4, szUrl);

	/* empty username, empty password */
	fill_url_components(&urlComp);
	SetLastError(0xdeadbeef);
	urlComp.lpszUserName = "";
	urlComp.lpszPassword = "";
	len = 51;
	ret = InternetCreateUrlA(&urlComp, 0, szUrl, &len);
	ok(ret, "Expected success\n");
	ok(GetLastError() == 0xdeadbeef,
		"Expected 0xdeadbeef, got %ld\n", GetLastError());
	ok(len == 50, "Expected len 50, got %ld\n", len);
	ok(!strcmp(szUrl, CREATE_URL5), "Expected %s, got %s\n", CREATE_URL5, szUrl);

	/* shows that nScheme is ignored, as the appearance of the port number
	 * depends on lpszScheme and the string copied depends on lpszScheme.
	 */
	fill_url_components(&urlComp);
	HeapFree(GetProcessHeap(), 0, szUrl);
	urlComp.lpszScheme = "nhttp";
	urlComp.dwSchemeLength = strlen(urlComp.lpszScheme);
	len = strlen(CREATE_URL6) + 1;
	szUrl = (char *)HeapAlloc(GetProcessHeap(), 0, len);
	ret = InternetCreateUrlA(&urlComp, ICU_ESCAPE, szUrl, &len);
	ok(ret, "Expected success\n");
	ok(len == strlen(CREATE_URL6), "Expected len %d, got %ld\n", strlen(CREATE_URL6) + 1, len);
	ok(!strcmp(szUrl, CREATE_URL6), "Expected %s, got %s\n", CREATE_URL6, szUrl);

	/* if lpszScheme != "http" or nPort != 80, display nPort */
	HeapFree(GetProcessHeap(), 0, szUrl);
	urlComp.lpszScheme = "http";
	urlComp.dwSchemeLength = strlen(urlComp.lpszScheme);
	urlComp.nPort = 42;
	szUrl = HeapAlloc(GetProcessHeap(), 0, ++len);
	ret = InternetCreateUrlA(&urlComp, ICU_ESCAPE, szUrl, &len);
	ok(ret, "Expected success\n");
	ok(len == 53, "Expected len 53, got %ld\n", len);
	ok(strstr(szUrl, "42") != NULL, "Expected to find 42 in szUrl\n");
	ok(!strcmp(szUrl, CREATE_URL7), "Expected %s, got %s\n", CREATE_URL7, szUrl);

	HeapFree(GetProcessHeap(), 0, szUrl);

	memset(&urlComp, 0, sizeof(urlComp));
	urlComp.dwStructSize = sizeof(URL_COMPONENTS);
	urlComp.lpszScheme = "http";
	urlComp.dwSchemeLength = 0;
	urlComp.nScheme = INTERNET_SCHEME_HTTP;
	urlComp.lpszHostName = "www.winehq.org";
	urlComp.dwHostNameLength = 0;
	urlComp.nPort = 80;
	urlComp.lpszUserName = "username";
	urlComp.dwUserNameLength = 0;
	urlComp.lpszPassword = "password";
	urlComp.dwPasswordLength = 0;
	urlComp.lpszUrlPath = "/site/about";
	urlComp.dwUrlPathLength = 0;
	urlComp.lpszExtraInfo = "";
	urlComp.dwExtraInfoLength = 0;
	len = strlen(CREATE_URL1);
	szUrl = (char *)HeapAlloc(GetProcessHeap(), 0, ++len);
	ret = InternetCreateUrlA(&urlComp, ICU_ESCAPE, szUrl, &len);
	ok(ret, "Expected success\n");
	ok(len == strlen(CREATE_URL1), "Expected len %d, got %ld\n", strlen(CREATE_URL1), len);
	ok(!strcmp(szUrl, CREATE_URL1), "Expected %s, got %s\n", CREATE_URL1, szUrl);

	memset(&urlComp, 0, sizeof(urlComp));
	urlComp.dwStructSize = sizeof(URL_COMPONENTS);
	urlComp.lpszScheme = "https";
	urlComp.dwSchemeLength = 0;
	urlComp.nScheme = INTERNET_SCHEME_HTTP;
	urlComp.lpszHostName = "www.winehq.org";
	urlComp.dwHostNameLength = 0;
	urlComp.nPort = 443;
	urlComp.lpszUserName = "username";
	urlComp.dwUserNameLength = 0;
	urlComp.lpszPassword = "password";
	urlComp.dwPasswordLength = 0;
	urlComp.lpszUrlPath = "/site/about";
	urlComp.dwUrlPathLength = 0;
	urlComp.lpszExtraInfo = "";
	urlComp.dwExtraInfoLength = 0;
	len = strlen(CREATE_URL8);
	szUrl = (char *)HeapAlloc(GetProcessHeap(), 0, ++len);
	ret = InternetCreateUrlA(&urlComp, ICU_ESCAPE, szUrl, &len);
	ok(ret, "Expected success\n");
	ok(len == strlen(CREATE_URL8), "Expected len %d, got %ld\n", strlen(CREATE_URL8), len);
	ok(!strcmp(szUrl, CREATE_URL8), "Expected %s, got %s\n", CREATE_URL8, szUrl);

	HeapFree(GetProcessHeap(), 0, szUrl);

	memset(&urlComp, 0, sizeof(urlComp));
	urlComp.dwStructSize = sizeof(URL_COMPONENTS);
	urlComp.lpszScheme = "about";
	urlComp.dwSchemeLength = 5;
	urlComp.lpszUrlPath = "blank";
	urlComp.dwUrlPathLength = 5;
	len = strlen(CREATE_URL9);
	len++; /* work around bug in native wininet */
	szUrl = (char *)HeapAlloc(GetProcessHeap(), 0, ++len);
	ret = InternetCreateUrlA(&urlComp, ICU_ESCAPE, szUrl, &len);
	ok(ret, "Expected success\n");
	ok(len == strlen(CREATE_URL9), "Expected len %d, got %ld\n", strlen(CREATE_URL9), len);
	ok(!strcmp(szUrl, CREATE_URL9), "Expected %s, got %s\n", CREATE_URL9, szUrl);

	HeapFree(GetProcessHeap(), 0, szUrl);

	memset(&urlComp, 0, sizeof(urlComp));
	urlComp.dwStructSize = sizeof(URL_COMPONENTS);
	urlComp.lpszScheme = "about";
	urlComp.lpszHostName = "host";
	urlComp.lpszUrlPath = "blank";
	len = strlen(CREATE_URL10);
	len++; /* work around bug in native wininet */
	szUrl = (char *)HeapAlloc(GetProcessHeap(), 0, ++len);
	ret = InternetCreateUrlA(&urlComp, ICU_ESCAPE, szUrl, &len);
	ok(ret, "Expected success\n");
	ok(len == strlen(CREATE_URL10), "Expected len %d, got %ld\n", strlen(CREATE_URL10), len);
	ok(!strcmp(szUrl, CREATE_URL10), "Expected %s, got %s\n", CREATE_URL10, szUrl);

	HeapFree(GetProcessHeap(), 0, szUrl);

	memset(&urlComp, 0, sizeof(urlComp));
	urlComp.dwStructSize = sizeof(URL_COMPONENTS);
	urlComp.nPort = 8080;
	urlComp.lpszScheme = "about";
	len = strlen(CREATE_URL11);
	szUrl = (char *)HeapAlloc(GetProcessHeap(), 0, ++len);
	ret = InternetCreateUrlA(&urlComp, ICU_ESCAPE, szUrl, &len);
	ok(ret, "Expected success\n");
	ok(len == strlen(CREATE_URL11), "Expected len %d, got %ld\n", strlen(CREATE_URL11), len);
	ok(!strcmp(szUrl, CREATE_URL11), "Expected %s, got %s\n", CREATE_URL11, szUrl);

	HeapFree(GetProcessHeap(), 0, szUrl);

	memset(&urlComp, 0, sizeof(urlComp));
	urlComp.dwStructSize = sizeof(URL_COMPONENTS);
	urlComp.lpszScheme = "http";
	urlComp.dwSchemeLength = 0;
	urlComp.nScheme = INTERNET_SCHEME_HTTP;
	urlComp.lpszHostName = "www.winehq.org";
	urlComp.dwHostNameLength = 0;
	urlComp.nPort = 65535;
	len = strlen(CREATE_URL12);
	szUrl = (char *)HeapAlloc(GetProcessHeap(), 0, ++len);
	ret = InternetCreateUrlA(&urlComp, ICU_ESCAPE, szUrl, &len);
	ok(ret, "Expected success\n");
	ok(len == strlen(CREATE_URL12), "Expected len %d, got %ld\n", strlen(CREATE_URL12), len);
	ok(!strcmp(szUrl, CREATE_URL12), "Expected %s, got %s\n", CREATE_URL12, szUrl);

	HeapFree(GetProcessHeap(), 0, szUrl);
}

START_TEST(url)
{
    InternetCrackUrl_test();
    InternetCrackUrlW_test();
    InternetCreateUrlA_test();
}
