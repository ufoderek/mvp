#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <debug_utils.h>
#include <debug_utils_settings.h>

// BEGIN HACK

#ifdef __cplusplus
#include <systemc.h>
#endif

#ifdef X
#undef X
#endif
#define X(x) #x // magic
const char* debug_unit_names[] = {DEBUG_UNIT_LIST};
#undef X

char path_prefix[128];
debug_unit debug_units[DEBUG_UNIT_COUNT];

void debug_utils_init(char* _path_prefix)
{
    int i;

    strcpy(path_prefix, _path_prefix);

    atexit(debug_utils_close);

    for(i = 0; i < DEBUG_UNIT_COUNT; i++)
    {
        debug_units[i].print = 0;
        debug_units[i].log = 0;
        debug_units[i].fd = 0;
    }
}

void debug_utils_close()
{
#ifndef RELEASE
    int i;
    for(i = 0; i < DEBUG_UNIT_COUNT; i++)
    {
        if(debug_units[i].fd != 0)
        {
            fclose(debug_units[i].fd);
        }
    }
#endif
}

void debug_utils_setp(debug_unit_enum ue)
{
#ifndef RELEASE
    debug_units[ue].print = 1;
#endif
}

void debug_utils_unsetp(debug_unit_enum ue)
{
#ifndef RELEASE
    debug_units[ue].print = 0;
#endif
}

void debug_utils_setl(debug_unit_enum ue)
{
#ifndef RELEASE
    if(debug_units[ue].fd == 0)
    {

        char file_name[128] = "";

        strcpy(file_name, path_prefix);
        strcat(file_name, debug_unit_names[ue]);
        strcat(file_name, ".log");

        debug_units[ue].fd = fopen(file_name, "w");
        if(debug_units[ue].fd == 0)
        {
            printb(ue, "error opening log file: %s", file_name);
        }
    }

    debug_units[ue].log = 1;
#endif
}

void debug_utils_unsetl(debug_unit_enum ue)
{
#ifndef RELEASE
    debug_units[ue].log = 0;
#endif
}

void printd_base(debug_unit_enum ue, char* msg, ...)
{
#ifndef RELEASE
    const unsigned int buf_size = 256;
    char head[buf_size / 2];
    char _main[buf_size];

    if(debug_units[ue].print || debug_units[ue].log)
    {
        va_list args;
#ifdef __cplusplus
        snprintf(head, buf_size / 2, "[%s][%s][", sc_time_stamp().to_string().c_str(), debug_unit_names[ue]);
#else
        snprintf(head, buf_size / 2, "[%s][", debug_unit_names[ue]);
#endif
        va_start(args, msg);
        vsnprintf(_main, buf_size, msg, args);
        va_end(args);

        if(debug_units[ue].print)
        {
#ifdef __cplusplus
            fprintf(stderr, "%s%s]\n", head, _main);
#else
            fprintf(stderr, "   %s%s]\n", head, _main);
#endif
        }

        if(debug_units[ue].log)
        {
            fprintf(debug_units[ue].fd, "%s%s]\n", head, _main);
        }
    }
#endif
}

#ifdef __cplusplus
void printd_base(unsigned int id,  debug_unit_enum ue, char* msg, ...)
#else
void printdi_base(unsigned int id,  debug_unit_enum ue,  char* msg, ...)
#endif
{
#ifndef RELEASE
    const unsigned int buf_size = 256;
    char head[buf_size / 2];
    char _main[buf_size];

    if(debug_units[ue].print || debug_units[ue].log)
    {
        va_list args;

#ifdef __cplusplus
        snprintf(head, buf_size / 2, "[%s][%s_%d][", sc_time_stamp().to_string().c_str(), debug_unit_names[ue], id);
#else
        snprintf(head, buf_size / 2, "[%s_%d][", debug_unit_names[ue], id);
#endif

        va_start(args, msg);
        vsnprintf(_main, buf_size, msg, args);
        va_end(args);

        if(debug_units[ue].print)
        {
#ifdef __cplusplus
            fprintf(stderr, "%s%s]\n", head, _main);
#else
            fprintf(stderr, "   %s%s]\n", head, _main);
#endif
        }

        if(debug_units[ue].log)
        {
            fprintf(debug_units[ue].fd, "%s%s]\n", head, _main);
        }
    }
#endif
}

void printm_base(int leave, debug_unit_enum ue, char* msg, ...)
{
    const unsigned int buf_size = 256;
    char head[buf_size / 2];
    char _main[buf_size];

#ifdef __cplusplus
    snprintf(head, buf_size / 2, "[%s][%s][", sc_time_stamp().to_string().c_str(), debug_unit_names[ue]);
#else
    snprintf(head, buf_size / 2, "[%s][", debug_unit_names[ue]);
#endif

    va_list args;
    va_start(args, msg);
    vsnprintf(_main, buf_size, msg, args);
    va_end(args);

#ifdef __cplusplus
    fprintf(stderr, "%s%s]\n", head, _main);
#else
    fprintf(stderr, "   %s%s]\n", head, _main);
#endif

#ifndef RELEASE
    if(debug_units[ue].log)
    {
        fprintf(debug_units[ue].fd, "%s%s]\n", head, _main);
    }
#endif

    if(leave)
    {
        exit(EXIT_SUCCESS);
    }
}

#ifdef __cplusplus
void printm_base(int leave, unsigned int id, debug_unit_enum ue, char* msg, ...)
#else
void printmi_base(int leave, unsigned int id, debug_unit_enum ue, char* msg, ...)
#endif
{
    const unsigned int buf_size = 256;
    char head[buf_size / 2];
    char _main[buf_size];

#ifdef __cplusplus
    snprintf(head, buf_size / 2, "[%s][%s_%d][", sc_time_stamp().to_string().c_str(), debug_unit_names[ue], id);
#else
    snprintf(head, buf_size / 2, "[%s_%d][", debug_unit_names[ue], id);
#endif

    va_list args;
    va_start(args, msg);
    vsnprintf(_main, buf_size, msg, args);
    va_end(args);

#ifdef __cplusplus
    fprintf(stderr, "%s%s]\n", head, _main);
#else
    fprintf(stderr, "   %s%s]\n", head, _main);
#endif

#ifndef RELEASE
    if(debug_units[ue].log)
    {
        fprintf(debug_units[ue].fd, "%s%s]\n", head, _main);
    }
#endif

    if(leave)
    {
        exit(EXIT_SUCCESS);
    }
}

void printe_base(char* msg, ...)
{
    const unsigned int buf_size = 256;
    char _main[buf_size];

    va_list args;
    va_start(args, msg);
    vsnprintf(_main, buf_size, msg, args);
    va_end(args);

#ifdef __cplusplus
    fprintf(stderr, "[%s]\n", _main);
#else
    fprintf(stderr, "   [%s]\n", _main);
#endif

}

// END HACK: I feel dirty.
