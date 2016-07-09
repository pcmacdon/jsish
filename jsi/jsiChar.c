#ifndef JSI_AMALGAMATION
#include "jsiInt.h"
#endif

#ifdef __WIN32
char *strcasestr(const char *haystack, const char *needle)
{
        int nlen = strlen(needle);
        int hlen = strlen(haystack) - nlen + 1;
        int i;

        for (i = 0; i < hlen; i++) {
                int j;
                for (j = 0; j < nlen; j++) {
                        unsigned char c1 = haystack[i+j];
                        unsigned char c2 = needle[j];
                        if (toupper(c1) != toupper(c2))
                                goto next;
                }
                return (char *) haystack + i;
        next:
                ;
        }
        return NULL;
}
#endif

uint Jsi_Strlen(const char *str) {
    return strlen(str);
}

int Jsi_Strncasecmp(const char *str1, const char *str2, int n)
{
    if (n<0)
        return strcasecmp(str1,str2);
    return strncasecmp(str1,str2,n);
}
int Jsi_Strncmp(const char *str1, const char *str2, int n)
{
   return strncmp(str1,str2,n);
}

uint Jsi_StrlenSet(const char *str, uint len) {
    char *s = (char*)str;
    uint olen = strlen(str);
    assert(len>=0);
    if (olen<len)
        return olen;
    s[len] = 0;
    return len;
}

char *Jsi_Strdup(const char *str) {
    return strdup(str);
}

char *jsi_SubstrDup(const char *a, int start, int len)
{
    if (len == 0) return Jsi_Strdup("");
    
    int lenofa = Jsi_Strlen(a);
    while (start < 0) start += lenofa;
    if (start >= lenofa) return Jsi_Strdup("");
    
    int maxcpy = lenofa - start;
    
    if (len > 0) {
        maxcpy = maxcpy < len ? maxcpy : len;
    }
    
    char *r = (char*)Jsi_Malloc(maxcpy + 1);

    Jsi_Strncpy(r, a + start, maxcpy + 1);
    return r;
}


char* Jsi_Strchr(const char *str, int c)
{
    return strchr((char*)str, c);
}

char* Jsi_Strcpy(char *dst, const char *src)
{
    return strcpy(dst, src);
}

char* Jsi_Strncpy(char *str1, const char *str2, int len)
{
    char* cp = strncpy(str1, str2, len-1);
    str1[len-1] = 0;
    return cp;
}

int Jsi_Strcmp(const char *str1, const char *str2)
{
    return strcmp(str1, str2);
}

int Jsi_StrcmpDict(const char *str1, const char *str2, int nocase, int dict)
{
    if (dict==0)
        return (nocase ? Jsi_Strncasecmp(str1,str2, -1) : strcmp(str1, str2));
    return Jsi_DictionaryCompare(str1, str2);
}

char *Jsi_Strcatdup(const char *str1, const char *str2)
{
    int l1 = strlen(str1), l2 = strlen(str2);
    char *cp = (char*)Jsi_Malloc(l1+l2+1);
    strcpy(cp, str1);
    return strcat(cp, str2);
}


int Jsi_Strpos(const char *str, int start, const char *s2, int nocase)
{
    const char *sstr = str;
    int len = strlen(str);
    if (len<start)
        return -1;
    str += start;
    const char *s = (nocase?strcasestr(str,s2):strstr(str, s2));
    if (!s)
        return -1;
    return (s-sstr);
}

int Jsi_Strrpos(const char *str, int start, const char *s2, int nocase)
{
    const char *sstr = str, *s, *os = NULL;
    int len = strlen(str);
    if (len<start)
        return -1;
    str += start;
    while (*str && (s = (nocase?strcasestr(str,s2):strstr(str, s2)))) {
        os = s;
        str += 1;
    }
    if (!os)
        return -1;
    return (os-sstr);
}

int
Jsi_DictionaryCompare( const char *left, const char *right)
{
  int diff, zeros;
  int secondaryDiff = 0;

  while (1) {
    if (isdigit(UCHAR(*right)) && isdigit(UCHAR(*left))) {
      /*
       * There are decimal numbers embedded in the two
       * strings.  Compare them as numbers, rather than
       * strings.  If one number has more leading zeros than
       * the other, the number with more leading zeros sorts
       * later, but only as a secondary choice.
       */

      zeros = 0;
      while ((*right == '0') && (isdigit(UCHAR(right[1])))) {
        right++;
        zeros--;
      }
      while ((*left == '0') && (isdigit(UCHAR(left[1])))) {
        left++;
        zeros++;
      }
      if (secondaryDiff == 0) {
        secondaryDiff = zeros;
      }

      /*
       * The code below compares the numbers in the two
       * strings without ever converting them to integers.  It
       * does this by first comparing the lengths of the
       * numbers and then comparing the digit values.
       */

      diff = 0;
      while (1) {
        if (diff == 0) {
          diff = UCHAR(*left) - UCHAR(*right);
        }
        right++;
        left++;
        /* Ignore commas in numbers. */
        if (*left == ',') {
          left++;
        }
        if (*right == ',') {
          right++;
        }
        if (!isdigit(UCHAR(*right))) {
          if (isdigit(UCHAR(*left))) {
            return 1;
          } else {
            /*
             * The two numbers have the same length. See
             * if their values are different.
             */

            if (diff != 0) {
              return diff;
            }
            break;
          }
        } else if (!isdigit(UCHAR(*left))) {
          return -1;
        }
      }
      continue;
    }
    diff = UCHAR(*left) - UCHAR(*right);
    if (diff) {
      if (isupper(UCHAR(*left)) && islower(UCHAR(*right))) {
        diff = UCHAR(tolower(*left)) - UCHAR(*right);
        if (diff) {
          return diff;
        } else if (secondaryDiff == 0) {
          secondaryDiff = -1;
        }
      } else if (isupper(UCHAR(*right)) && islower(UCHAR(*left))) {
        diff = UCHAR(*left) - UCHAR(tolower(UCHAR(*right)));
        if (diff) {
          return diff;
        } else if (secondaryDiff == 0) {
          secondaryDiff = 1;
        }
      } else {
        return diff;
      }
    }
    if (*left == 0) {
      break;
    }
    left++;
    right++;
  }
  if (diff == 0) {
    diff = secondaryDiff;
  }
  return diff;
}

