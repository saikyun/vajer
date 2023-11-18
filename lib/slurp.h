#pragma once

char *slurp(char *path)
{
    FILE *f = fopen(path, "r");

    assert(f != NULL);

    size_t n = 0;

    fseek(f, 0, SEEK_END);
    n = ftell(f);
    rewind(f);
    char *buffer = (char *)malloc((n + 1) * sizeof(char));
    assert(buffer != NULL);
    fread(buffer, sizeof(char), n, f);

    buffer[n] = '\0';

    fclose(f);

    return buffer;
}