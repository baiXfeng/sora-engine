//
// Created by baifeng on 2021/6/24.
// fork from: https://github.com/tonyhack/utf8_unicode.git
//

#ifndef SE_UTF8_TO_UNICODE_H
#define SE_UTF8_TO_UNICODE_H

#include <vector>
#include "common/macro.h"

mge_begin

bool utf8_check_bytes(const char *first, size_t count);
size_t utf8_get_size(const char *str);
int utf8_convert_unicode(const char *str, size_t size);

mge_end

#endif //SE_UTF8_TO_UNICODE_H
