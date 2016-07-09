#ifndef __JSIUTF8_H__
#define __JSIUTF8_H__

int jsi_utf8_charlen(int c);
int jsi_utf8_index(const char *str, int charindex);
int jsi_utf8_lower(int uc);
int jsi_utf8_prev_len(const char *str, int len);
int jsi_utf8_strlen(const char *str);
int jsi_utf8_title(int uc);
int jsi_utf8_tounicode(const char *str, int *uc);
int jsi_utf8_upper(int uc);
int jsi_utf8_tounicode_case(const char *s, int *uc, int upper);
int jsi_utf8_fromunicode(char *p, unsigned uc);

#endif /* __JSIUTF8_H__ */
