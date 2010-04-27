/*
 * oAuth string functions in POSIX-C.
 *
 * Copyright 2007, 2008, 2009 Robin Gareus <robin@gareus.org>
 * 
 * The base64 functions are by Jan-Henrik Haukeland, <hauk@tildeslash.com>
 * and un/escape_url() was inspired by libcurl's curl_escape under ISC-license
 * many thanks to Daniel Stenberg <daniel@haxx.se>.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 * 
 */
#if HAVE_CONFIG_H
# include <config.h>
#endif

#define WIPE_MEMORY ///< overwrite sensitve data before free()ing it.

#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <math.h>
#include <ctype.h> // isxdigit
#include <openssl/hmac.h>

#include "xmalloc.h"
#include "oauth.h"

#ifndef WIN32 // getpid() on POSIX systems
#include <sys/types.h>
#include <unistd.h>
#else
#define snprintf _snprintf
#define strncasecmp strnicmp
#endif

/**
 * Base64 encode one byte
 */
char oauth_b64_encode(unsigned char u) {
  if(u < 26)  return 'A'+u;
  if(u < 52)  return 'a'+(u-26);
  if(u < 62)  return '0'+(u-52);
  if(u == 62) return '+';
  return '/';
}

/**
 * Decode a single base64 character.
 */
unsigned char oauth_b64_decode(char c) {
  if(c >= 'A' && c <= 'Z') return(c - 'A');
  if(c >= 'a' && c <= 'z') return(c - 'a' + 26);
  if(c >= '0' && c <= '9') return(c - '0' + 52);
  if(c == '+')             return 62;
  return 63;
}

/**
 * Return TRUE if 'c' is a valid base64 character, otherwise FALSE
 */
int oauth_b64_is_base64(char c) {
  if((c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z') ||
     (c >= '0' && c <= '9') || (c == '+')             ||
     (c == '/')             || (c == '=')) {
    return 1;
  }
  return 0;
}

/**
 * Base64 encode and return size data in 'src'. The caller must free the
 * returned string.
 *
 * @param size The size of the data in src
 * @param src The data to be base64 encode
 * @return encoded string otherwise NULL
 */
char *oauth_encode_base64(int size, const unsigned char *src) {
  int i;
  char *out, *p;

  if(!src) return NULL;
  if(!size) size= strlen((char *)src);
  out= (char*) xcalloc(sizeof(char), size*4/3+4);
  p= out;

  for(i=0; i<size; i+=3) {
    unsigned char b1=0, b2=0, b3=0, b4=0, b5=0, b6=0, b7=0;
    b1= src[i];
    if(i+1<size) b2= src[i+1];
    if(i+2<size) b3= src[i+2];
      
    b4= b1>>2;
    b5= ((b1&0x3)<<4)|(b2>>4);
    b6= ((b2&0xf)<<2)|(b3>>6);
    b7= b3&0x3f;
      
    *p++= oauth_b64_encode(b4);
    *p++= oauth_b64_encode(b5);
      
    if(i+1<size) *p++= oauth_b64_encode(b6);
    else *p++= '=';
      
    if(i+2<size) *p++= oauth_b64_encode(b7);
    else *p++= '=';
  }
  return out;
}

/**
 * Decode the base64 encoded string 'src' into the memory pointed to by
 * 'dest'. 
 *
 * @param dest Pointer to memory for holding the decoded string.
 * Must be large enough to recieve the decoded string.
 * @param src A base64 encoded string.
 * @return the length of the decoded string if decode
 * succeeded otherwise 0.
 */
int oauth_decode_base64(unsigned char *dest, const char *src) {
  if(src && *src) {
    unsigned char *p= dest;
    int k, l= strlen(src)+1;
    unsigned char *buf= (unsigned char*) xcalloc(sizeof(unsigned char), l);

    /* Ignore non base64 chars as per the POSIX standard */
    for(k=0, l=0; src[k]; k++) {
      if(oauth_b64_is_base64(src[k])) {
        buf[l++]= src[k];
      }
    } 
    
    for(k=0; k<l; k+=4) {
      char c1='A', c2='A', c3='A', c4='A';
      unsigned char b1=0, b2=0, b3=0, b4=0;
      c1= buf[k];

      if(k+1<l) c2= buf[k+1];
      if(k+2<l) c3= buf[k+2];
      if(k+3<l) c4= buf[k+3];
      
      b1= oauth_b64_decode(c1);
      b2= oauth_b64_decode(c2);
      b3= oauth_b64_decode(c3);
      b4= oauth_b64_decode(c4);
      
      *p++=((b1<<2)|(b2>>4) );
      
      if(c3 != '=') *p++=(((b2&0xf)<<4)|(b3>>2) );
      if(c4 != '=') *p++=(((b3&0x3)<<6)|b4 );
    }
    free(buf);
    dest[p-dest]='\0';
    return(p-dest);
  }
  return 0;
}

/**
 * Escape 'string' according to RFC3986 and
 * http://oauth.net/core/1.0/#encoding_parameters.
 *
 * @param string The data to be encoded
 * @return encoded string otherwise NULL
 * The caller must free the returned string.
 */
char *oauth_url_escape(const char *string) {
  size_t alloc, newlen;
  char *ns = NULL, *testing_ptr = NULL;
  unsigned char in; 
  size_t strindex=0;
  size_t length;

  if (!string) return xstrdup("");

  alloc = strlen(string)+1;
  newlen = alloc;

  ns = (char*) xmalloc(alloc);

  length = alloc-1;
  while(length--) {
    in = *string;

    switch(in){
    case '0': case '1': case '2': case '3': case '4':
    case '5': case '6': case '7': case '8': case '9':
    case 'a': case 'b': case 'c': case 'd': case 'e':
    case 'f': case 'g': case 'h': case 'i': case 'j':
    case 'k': case 'l': case 'm': case 'n': case 'o':
    case 'p': case 'q': case 'r': case 's': case 't':
    case 'u': case 'v': case 'w': case 'x': case 'y': case 'z':
    case 'A': case 'B': case 'C': case 'D': case 'E':
    case 'F': case 'G': case 'H': case 'I': case 'J':
    case 'K': case 'L': case 'M': case 'N': case 'O':
    case 'P': case 'Q': case 'R': case 'S': case 'T':
    case 'U': case 'V': case 'W': case 'X': case 'Y': case 'Z':
    case '_': case '~': case '.': case '-':
      ns[strindex++]=in;
      break;
    default:
      newlen += 2; /* this'll become a %XX */
      if(newlen > alloc) {
        alloc *= 2;
        testing_ptr = (char*) xrealloc(ns, alloc);
        ns = testing_ptr;
      }
      snprintf(&ns[strindex], 4, "%%%02X", in);
      strindex+=3;
      break;
    }
    string++;
  }
  ns[strindex]=0;
  return ns;
}

#ifndef ISXDIGIT
# define ISXDIGIT(x) (isxdigit((int) ((unsigned char)x)))
#endif

/**
 * Parse RFC3986 encoded 'string' back to  unescaped version.
 *
 * @param string The data to be unescaped
 * @param olen unless NULL the length of the returned string is stored there.
 * @return decoded string or NULL
 * The caller must free the returned string.
 */
char *oauth_url_unescape(const char *string, size_t *olen) {
  size_t alloc, strindex=0;
  char *ns = NULL;
  unsigned char in;
  long hex;

  if (!string) return NULL;
  alloc = strlen(string)+1;
  ns = (char*) xmalloc(alloc);

  while(--alloc > 0) {
    in = *string;
    if(('%' == in) && ISXDIGIT(string[1]) && ISXDIGIT(string[2])) {
      char hexstr[3]; // '%XX'
      hexstr[0] = string[1];
      hexstr[1] = string[2];
      hexstr[2] = 0;
      hex = strtol(hexstr, NULL, 16);
      in = (unsigned char)hex; /* hex is always < 256 */
      string+=2;
      alloc-=2;
    }
    ns[strindex++] = in;
    string++;
  }
  ns[strindex]=0;
  if(olen) *olen = strindex;
  return ns;
}

/**
 * returns base64 encoded HMAC-SHA1 signature for
 * given message and key.
 * both data and key need to be urlencoded.
 *
 * the returned string needs to be freed by the caller
 *
 * @param m message to be signed
 * @param k key used for signing
 * @return signature string.
 */
char *oauth_sign_hmac_sha1 (const char *m, const char *k) {
  return(oauth_sign_hmac_sha1_raw (m, strlen(m), k, strlen(k)));
}

char *oauth_sign_hmac_sha1_raw (const char *m, const size_t ml, const char *k, const size_t kl) {
  unsigned char result[EVP_MAX_MD_SIZE];
  unsigned int resultlen = 0;
  
  HMAC(EVP_sha1(), k, kl, 
      (unsigned char*) m, ml,
      result, &resultlen);

  return(oauth_encode_base64(resultlen, result));
}

/**
 * returns plaintext signature for the given key.
 *
 * the returned string needs to be freed by the caller
 *
 * @param m message to be signed
 * @param k key used for signing
 * @return signature string
 */
char *oauth_sign_plaintext (const char *m, const char *k) {
  return(oauth_url_escape(k));
}

#include <openssl/evp.h>
#include <openssl/x509.h>
#include <openssl/x509v3.h>
#include <openssl/ssl.h>

/**
 * returns RSA-SHA1 signature for given data.
 * the returned signature needs to be freed by the caller.
 *
 * @param m message to be signed
 * @param k private-key PKCS and Base64-encoded 
 * @return base64 encoded signature string.
 */
char *oauth_sign_rsa_sha1 (const char *m, const char *k) {
  unsigned char *sig = NULL;
  unsigned char *passphrase = NULL;
  unsigned int len=0;
  EVP_MD_CTX md_ctx;

  EVP_PKEY *pkey;
  BIO *in;
  in = BIO_new_mem_buf((unsigned char*) k, strlen(k));
  pkey = PEM_read_bio_PrivateKey(in, NULL, 0, passphrase); // generate sign
  BIO_free(in);

  if (pkey == NULL) {
  //fprintf(stderr, "liboauth/ssl: can not read private key\n");
    return xstrdup("liboauth/ssl: can not read private key");
  }

  len = EVP_PKEY_size(pkey);
  sig = (unsigned char*)xmalloc((len+1)*sizeof(char));

  EVP_SignInit(&md_ctx, EVP_sha1());
  EVP_SignUpdate(&md_ctx, m, strlen(m));
  if (EVP_SignFinal (&md_ctx, sig, &len, pkey)) {
    char *tmp;
    sig[len] = '\0';
    tmp = oauth_encode_base64(len,sig);
    OPENSSL_free(sig);
    EVP_PKEY_free(pkey);
    return tmp;
  }
  return xstrdup("liboauth/ssl: rsa-sha1 signing failed");
}

/**
 * verify RSA-SHA1 signature.
 *
 * returns the output of EVP_VerifyFinal() for a given message,
 * cert/pubkey and signature
 *
 * @param m message to be verified
 * @param c public-key or x509 certificate
 * @param s base64 encoded signature
 * @return 1 for a correct signature, 0 for failure and -1 if some other error occurred
 */
int oauth_verify_rsa_sha1 (const char *m, const char *c, const char *s) {
  EVP_MD_CTX md_ctx;
  EVP_PKEY *pkey;
  BIO *in;
  X509 *cert = NULL;
  unsigned char *b64d;
  int slen, err;

  in = BIO_new_mem_buf((unsigned char*)c, strlen(c));
  cert = PEM_read_bio_X509(in, NULL, 0, NULL);
  if (cert)  {
    pkey = (EVP_PKEY *) X509_get_pubkey(cert); 
    X509_free(cert);
  } else {
    pkey = PEM_read_bio_PUBKEY(in, NULL, 0, NULL);
  }
  BIO_free(in);
  if (pkey == NULL) {
  //fprintf(stderr, "could not read cert/pubkey.\n");
    return -2;
  }

  b64d= (unsigned char*) xmalloc(sizeof(char)*strlen(s));
  slen = oauth_decode_base64(b64d, s);

  EVP_VerifyInit(&md_ctx, EVP_sha1());
  EVP_VerifyUpdate(&md_ctx, m, strlen(m));
  err = EVP_VerifyFinal(&md_ctx, b64d, slen, pkey);
  EVP_MD_CTX_cleanup(&md_ctx);
  EVP_PKEY_free(pkey);
  free(b64d);
  return (err);
}

/**
 * encode strings and concatenate with '&' separator.
 * The number of strings to be concatenated must be
 * given as first argument.
 * all arguments thereafter must be of type (char *) 
 *
 * @param len the number of arguments to follow this parameter
 * @param ... string to escape and added (may be NULL)
 *
 * @return pointer to memory holding the concatenated 
 * strings - needs to be free(d) by the caller. or NULL
 * in case we ran out of memory.
 */
char *oauth_catenc(int len, ...) {
  va_list va;
  int i;
  char *rv = (char*) xmalloc(sizeof(char));
  *rv='\0';
  va_start(va, len);
  for(i=0;i<len;i++) {
    char *arg = va_arg(va, char *);
    char *enc;
    int len;
    enc = oauth_url_escape(arg);
    if(!enc) break;
    len = strlen(enc) + 1 + ((i>0)?1:0);
    if(rv) len+=strlen(rv);
    rv=(char*) xrealloc(rv,len*sizeof(char));

    if(i>0) strcat(rv, "&");
    strcat(rv, enc);
    free(enc);
  }
  va_end(va);
  return(rv);
}

/**
 * splits the given url into a parameter array. 
 * (see \ref oauth_serialize_url and \ref oauth_serialize_url_parameters for the reverse)
 *
 * NOTE: Request-parameters-values may include an ampersand character.
 * However if unescaped this function will use them as parameter delimiter. 
 * If you need to make such a request, this function since version 0.3.5 allows
 * to use the ASCII SOH (0x01) character as alias for '&' (0x26).
 * (the motivation is convenience: SOH is /untypeable/ and much more 
 * unlikely to appear than '&' - If you plan to sign fancy URLs you 
 * should not split a query-string, but rather provide the parameter array
 * directly to \ref oauth_serialize_url)
 *
 * @param url the url or query-string to parse. 
 * @param argv pointer to a (char *) array where the results are stored.
 *  The array is re-allocated to match the number of parameters and each 
 *  parameter-string is allocated with strdup. - The memory needs to be freed
 *  by the caller.
 * @param qesc use query parameter escape (vs post-param-escape) - if set
 *        to 1 all '+' are treated as spaces ' '
 * 
 * @return number of parameter(s) in array.
 */
int oauth_split_post_paramters(const char *url, char ***argv, short qesc) {
  int argc=0;
  char *token, *tmp, *t1;
  if (!argv) return 0;
  if (!url) return 0;
  t1=xstrdup(url);

  // '+' represents a space, in a URL query string
  while ((qesc&1) && (tmp=strchr(t1,'+'))) *tmp=' ';

  tmp=t1;
  while((token=strtok(tmp,"&?"))) {
    if(!strncasecmp("oauth_signature=",token,16)) continue;
    (*argv)=(char**) xrealloc(*argv,sizeof(char*)*(argc+1));
    while (!(qesc&2) && (tmp=strchr(token,'\001'))) *tmp='&';
    (*argv)[argc]=oauth_url_unescape(token, NULL);
    if (argc==0 && strstr(token, ":/")) {
      // HTTP does not allow empty absolute paths, so the URL 
      // 'http://example.com' is equivalent to 'http://example.com/' and should
      // be treated as such for the purposes of OAuth signing (rfc2616, section 3.2.1)
      // see http://groups.google.com/group/oauth/browse_thread/thread/c44b6f061bfd98c?hl=en
      char *slash=strstr(token, ":/");
      while (slash && *(++slash) == '/')  ; // skip slashes eg /xxx:[\/]*/
#if 0
      // skip possibly unescaped slashes in the userinfo - they're not allowed by RFC2396 but have been seen.
      // the hostname/IP may only contain alphanumeric characters - so we're safe there.
      if (slash && strchr(slash,'@')) slash=strchr(slash,'@'); 
#endif
      if (slash && !strchr(slash,'/')) {
#ifdef DEBUG_OAUTH
        fprintf(stderr, "\nliboauth: added trailing slash to URL: '%s'\n\n", token);
#endif
        free((*argv)[argc]);
        (*argv)[argc]= (char*) xmalloc(sizeof(char)*(2+strlen(token))); 
        strcpy((*argv)[argc],token);
        strcat((*argv)[argc],"/");
      }
    }
    if (argc==0 && (tmp=strstr((*argv)[argc],":80/"))) {
        memmove(tmp, tmp+3, strlen(tmp+2));
    }
    tmp=NULL;
    argc++;
  }

  free(t1);
  return argc;
}

int oauth_split_url_parameters(const char *url, char ***argv) {
  return oauth_split_post_paramters(url, argv, 1);
}

/**
 * build a url query string from an array.
 *
 * @param argc the total number of elements in the array
 * @param start element in the array at which to start concatenating.
 * @param argv parameter-array to concatenate.
 * @return url string needs to be freed by the caller.
 *
 */
char *oauth_serialize_url (int argc, int start, char **argv) {
  return oauth_serialize_url_sep( argc, start, argv, "&");
}

/**
 * encode query parameters from an array.
 *
 * @param argc the total number of elements in the array
 * @param start element in the array at which to start concatenating.
 * @param argv parameter-array to concatenate.
 * @param sep separator for parameters (usually "&") 
 * @return url string needs to be freed by the caller.
 */
char *oauth_serialize_url_sep (int argc, int start, char **argv, char *sep) {
  char  *tmp, *t1;
  int i;
  int  first=0;
  int seplen=strlen(sep);
  char *query = (char*) xmalloc(sizeof(char)); 
  *query='\0';
  for(i=start; i< argc; i++) {
    int len = 0;
    if (query) len+=strlen(query);

    if (i==start && i==0 && strstr(argv[i], ":/")) {
      tmp=xstrdup(argv[i]);
      len+=strlen(tmp);
    } else if(!(t1=strchr(argv[i], '='))) {
    // see http://oauth.net/core/1.0/#anchor14
    // escape parameter names and arguments but not the '='
      tmp=xstrdup(argv[i]);
      tmp=(char*) xrealloc(tmp,(strlen(tmp)+2)*sizeof(char));
      strcat(tmp,"=");
      len+=strlen(tmp);
    } else {
      *t1=0;
      tmp = oauth_url_escape(argv[i]);
      *t1='=';
      t1 = oauth_url_escape((t1+1));
      tmp=(char*) xrealloc(tmp,(strlen(tmp)+strlen(t1)+2)*sizeof(char));
      strcat(tmp,"=");
      strcat(tmp,t1);
      free(t1);
      len+=strlen(tmp);
    }
    len+=seplen+1;
    query=(char*) xrealloc(query,len*sizeof(char));
    strcat(query, ((i==start||first)?"":sep));
    first=0;
    strcat(query, tmp);
    if (i==start && i==0 && strstr(tmp, ":/")) {
      strcat(query, "?");
      first=1;
    }
    free(tmp);
  }
  return (query);
}

/**
 * build a query parameter string from an array.
 *
 * This function is a shortcut for \ref oauth_serialize_url (argc, 1, argv). 
 * It strips the leading host/path, which is usually the first 
 * element when using oauth_split_url_parameters on an URL.
 *
 * @param argc the total number of elements in the array
 * @param argv parameter-array to concatenate.
 * @return url string needs to be freed by the caller.
 */
char *oauth_serialize_url_parameters (int argc, char **argv) {
  return oauth_serialize_url(argc, 1, argv);
}

/**
 * generate a random string between 15 and 32 chars length
 * and return a pointer to it. The value needs to be freed by the
 * caller
 *
 * @return zero terminated random string.
 */
char *oauth_gen_nonce() {
  char *nc;
  static int rndinit = 1;
  const char *chars = "abcdefghijklmnopqrstuvwxyz"
    "ABCDEFGHIJKLMNOPQRSTUVWXYZ" "0123456789_";
  unsigned int max = strlen( chars );
  int i, len;

  if(rndinit) {srand(time(NULL) 
#ifndef WIN32 // quick windows check.
    * getpid()
#endif
  ); rndinit=0;} // seed random number generator - FIXME: we can do better ;)

  len=15+floor(rand()*16.0/(double)RAND_MAX);
  nc = (char*) xmalloc((len+1)*sizeof(char));
  for(i=0;i<len; i++) {
    nc[i] = chars[ rand() % max ];
  }
  nc[i]='\0';
  return (nc);
}

/**
 * string compare function for oauth parameters.
 *
 * used with qsort. needed to normalize request parameters.
 * see http://oauth.net/core/1.0/#anchor14
 */
int oauth_cmpstringp(const void *p1, const void *p2) {
  char *v1,*v2;
  char *t1,*t2;
  int rv;
  // TODO: this is not fast - we should escape the 
  // array elements (once) before sorting.
  v1=oauth_url_escape(* (char * const *)p1);
  v2=oauth_url_escape(* (char * const *)p2);

  // '=' signs are not "%3D" !
  if ((t1=strstr(v1,"%3D"))) {
    t1[0]='\0'; t1[1]='='; t1[2]='=';
  }
  if ((t2=strstr(v2,"%3D"))) {
    t2[0]='\0'; t2[1]='='; t2[2]='=';
  }

  // compare parameter names
  rv=strcmp(v1,v2);
  if (rv!=0) {
    if (v1) free(v1);
    if (v2) free(v2);
    return rv;
  }

  // if parameter names are equal, sort by value.
  if (t1) t1[0]='='; 
  if (t2) t2[0]='='; 
  rv=strcmp(t1,t2);
  if (v1) free(v1);
  if (v2) free(v2);
  return rv;
}

/**
 * search array for parameter key.
 * @param argv length of array to search
 * @param argc parameter array to search
 * @param key key of parameter to check.
 *
 * @return FALSE (0) if array does not contain a paramater with given key, TRUE (1) otherwise.
 */
int oauth_param_exists(char **argv, int argc, char *key) {
  int i;
  size_t l= strlen(key);
  for (i=0;i<argc;i++)
    if (strlen(argv[i])>l && !strncmp(argv[i],key,l) && argv[i][l] == '=') return 1;
  return 0;
}

/**
 * add query parameter to array
 *
 * @param argcp pointer to array length int
 * @param argvp pointer to array values 
 * @param addparam parameter to add (eg. "foo=bar")
 */
void oauth_add_param_to_array(int *argcp, char ***argvp, const char *addparam) {
  (*argvp)=(char**) xrealloc(*argvp,sizeof(char*)*((*argcp)+1));
  (*argvp)[(*argcp)++]= (char*) xstrdup(addparam);
}

/**
 *
 */
void oauth_add_protocol(int *argcp, char ***argvp, 
  OAuthMethod method, 
  const char *c_key, //< consumer key - posted plain text
  const char *t_key //< token key - posted plain text in URL
 ){
  char oarg[1024];

  // add oAuth specific arguments
  if (!oauth_param_exists(*argvp,*argcp,"oauth_nonce")) {
    char *tmp;
    snprintf(oarg, 1024, "oauth_nonce=%s", (tmp=oauth_gen_nonce()));
    oauth_add_param_to_array(argcp, argvp, oarg);
    free(tmp);
  }

  if (!oauth_param_exists(*argvp,*argcp,"oauth_timestamp")) {
    snprintf(oarg, 1024, "oauth_timestamp=%li", (long int) time(NULL));
    oauth_add_param_to_array(argcp, argvp, oarg);
  }

  if (t_key) {
    snprintf(oarg, 1024, "oauth_token=%s", t_key);
    oauth_add_param_to_array(argcp, argvp, oarg);
  }

  snprintf(oarg, 1024, "oauth_consumer_key=%s", c_key);
  oauth_add_param_to_array(argcp, argvp, oarg);

  snprintf(oarg, 1024, "oauth_signature_method=%s",
      method==0?"HMAC-SHA1":method==1?"RSA-SHA1":"PLAINTEXT");
  oauth_add_param_to_array(argcp, argvp, oarg);

  if (!oauth_param_exists(*argvp,*argcp,"oauth_version")) {
    snprintf(oarg, 1024, "oauth_version=1.0");
    oauth_add_param_to_array(argcp, argvp, oarg);
  }

#if 0 // oauth_version 1.0 Rev A
  if (!oauth_param_exists(argv,argc,"oauth_callback")) {
    snprintf(oarg, 1024, "oauth_callback=oob");
    oauth_add_param_to_array(argcp, argvp, oarg);
  }
#endif

}

char *oauth_sign_url (const char *url, char **postargs, 
  OAuthMethod method, 
  const char *c_key, //< consumer key - posted plain text
  const char *c_secret, //< consumer secret - used as 1st part of secret-key 
  const char *t_key, //< token key - posted plain text in URL
  const char *t_secret //< token secret - used as 2st part of secret-key
  ) {
  return oauth_sign_url2(url, postargs, 
    method, NULL,
    c_key, c_secret,
    t_key, t_secret);
} 

char *oauth_sign_url2 (const char *url, char **postargs, 
  OAuthMethod method, 
  const char *http_method, //< HTTP request method
  const char *c_key, //< consumer key - posted plain text
  const char *c_secret, //< consumer secret - used as 1st part of secret-key 
  const char *t_key, //< token key - posted plain text in URL
  const char *t_secret //< token secret - used as 2st part of secret-key
  ) {
  int  argc;
  char **argv = NULL;
  char *rv;

  if (postargs)
    argc = oauth_split_post_paramters(url, &argv, 0);
  else
    argc = oauth_split_url_parameters(url, &argv);

  rv=oauth_sign_array2(&argc, &argv, postargs, 
    method, http_method,
    c_key, c_secret, t_key, t_secret);

  oauth_free_array(&argc, &argv);
  return(rv);
}

char *oauth_sign_array (int *argcp, char***argvp,
  char **postargs,
  OAuthMethod method, 
  const char *c_key, //< consumer key - posted plain text
  const char *c_secret, //< consumer secret - used as 1st part of secret-key 
  const char *t_key, //< token key - posted plain text in URL
  const char *t_secret //< token secret - used as 2st part of secret-key
  ) {
  return oauth_sign_array2 (argcp, argvp, 
                            postargs, method,
                            NULL,
                            c_key, c_secret,
                            t_key, t_secret);
}

char *oauth_sign_array2 (int *argcp, char***argvp,
  char **postargs,
  OAuthMethod method, 
  const char *http_method, //< HTTP request method
  const char *c_key, //< consumer key - posted plain text
  const char *c_secret, //< consumer secret - used as 1st part of secret-key 
  const char *t_key, //< token key - posted plain text in URL
  const char *t_secret //< token secret - used as 2st part of secret-key
  ) {
  char oarg[1024];
  char *query;
  char *okey, *odat, *sign;
  char *result;
  char *http_request_method;

  if (!http_method) {
    http_request_method = xstrdup(postargs?"POST":"GET");
  } else {
    int i;
    http_request_method = xstrdup(http_method);
    for (i=0;i<strlen(http_request_method);i++) 
      http_request_method[i]=toupper(http_request_method[i]);
  }

  // add OAuth protocol parameters
  oauth_add_protocol(argcp, argvp, method, c_key, t_key);

  // sort parameters
  qsort(&(*argvp)[1], (*argcp)-1, sizeof(char *), oauth_cmpstringp);

  // serialize URL
  query= oauth_serialize_url_parameters(*argcp, *argvp);

  // generate signature
  okey = oauth_catenc(2, c_secret, t_secret);
  odat = oauth_catenc(3, http_request_method, (*argvp)[0], query);
  free(http_request_method);
#ifdef DEBUG_OAUTH
  fprintf (stderr, "\nliboauth: data to sign='%s'\n\n", odat);
  fprintf (stderr, "\nliboauth: key='%s'\n\n", okey);
#endif
  switch(method) {
    case OA_RSA:
      sign = oauth_sign_rsa_sha1(odat,okey); // XXX okey needs to be RSA key!
      break;
    case OA_PLAINTEXT:
      sign = oauth_sign_plaintext(odat,okey);
      break;
    default:
      sign = oauth_sign_hmac_sha1(odat,okey);
  }
#ifdef WIPE_MEMORY
  memset(okey,0, strlen(okey));
  memset(odat,0, strlen(odat));
#endif
  free(odat); 
  free(okey);

  // append signature to query args.
  snprintf(oarg, 1024, "oauth_signature=%s",sign);
  oauth_add_param_to_array(argcp, argvp, oarg);
  free(sign);

  // build URL params
  result = oauth_serialize_url(*argcp, (postargs?1:0), *argvp);

  if(postargs) { 
    *postargs = result;
    result = xstrdup((*argvp)[0]);
  }

  if(query) free(query);

  return result;
}

/**
 * free array args
 *
 * @param argcp pointer to array length int
 * @param argvp pointer to array values to be free()d
 */
void oauth_free_array(int *argcp, char ***argvp) {
  int i;
  for (i=0;i<(*argcp);i++) {
    free((*argvp)[i]);
  }
  if(*argvp) free(*argvp);
}

/** 
 * http://oauth.googlecode.com/svn/spec/ext/body_hash/1.0/drafts/4/spec.html
 */
char *oauth_body_hash_file(char *filename) {
  unsigned char fb[BUFSIZ];
  EVP_MD_CTX ctx;
  size_t len=0;
  unsigned char *md;
  FILE *F= fopen(filename, "r");
  if (!F) return NULL;

  EVP_MD_CTX_init(&ctx);
  EVP_DigestInit(&ctx,EVP_sha1());
  while (!feof(F) && (len=fread(fb,sizeof(char),BUFSIZ, F))>0) {
    EVP_DigestUpdate(&ctx, fb, len);
  }
  fclose(F);
  len=0;
  md=(unsigned char*) calloc(EVP_MD_size(EVP_sha1()),sizeof(unsigned char));
  EVP_DigestFinal(&ctx, md,(unsigned int*) &len);
  EVP_MD_CTX_cleanup(&ctx);
  return oauth_body_hash_encode(len, md);
}

char *oauth_body_hash_data(size_t length, const char *data) {
  EVP_MD_CTX ctx;
  size_t len=0;
  unsigned char *md;
  md=(unsigned char*) calloc(EVP_MD_size(EVP_sha1()),sizeof(unsigned char));
  EVP_MD_CTX_init(&ctx);
  EVP_DigestInit(&ctx,EVP_sha1());
  EVP_DigestUpdate(&ctx, data, length);
  EVP_DigestFinal(&ctx, md,(unsigned int*) &len);
  EVP_MD_CTX_cleanup(&ctx);
  return oauth_body_hash_encode(len, md);
}

/**
 * base64 encode digest, free it and return a URL parameter
 * with the oauth_body_hash
 */
char *oauth_body_hash_encode(size_t len, unsigned char *digest) {
  char *sign=oauth_encode_base64(len,digest);
  char *sig_url = (char*)malloc(17+strlen(sign));
  sprintf(sig_url,"oauth_body_hash=%s", sign);
  free(sign);
  free(digest);
  return sig_url;
}


/**
 * xep-0235 - TODO
 */
char *oauth_sign_xmpp (const char *xml,
  OAuthMethod method, 
  const char *c_secret, //< consumer secret - used as 1st part of secret-key 
  const char *t_secret //< token secret - used as 2st part of secret-key
  ) {

  return NULL;
}

// vi: sts=2 sw=2 ts=2
