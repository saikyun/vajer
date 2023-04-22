#include <assert.h>
#include <stdlib.h>
#include <string.h>

typedef struct String
{
    char *str;
    int length;
    int capacity;
} String;

void string(String *str, char *src)
{
    int length = strlen(src);

    while (str->capacity <= str->length + length + 1)
    {
        int new_cap = (str->capacity + length + 1) * 2;
        void *res = realloc(str->str, new_cap * sizeof(char));
        assert(res != 0);
        str->str = res;
        str->capacity = new_cap;
    }

    memcpy(str->str + str->length, src, length);
    str->length += length;

    assert(str->length + 1 < str->capacity);

    memset(str->str + str->length, '\0', 1);
}

void _strstr(String *str, int count, char **arr)
{
    for (int i = 0; i < count; i++)
    {
        string(str, arr[i]);
    }
}

#define strstr(str, ...)                                   \
    (_strstr(                                              \
        str,                                               \
        sizeof(char *[]){ __VA_ARGS__ } / sizeof(char *), \
        (char *[]){ __VA_ARGS__ }))
