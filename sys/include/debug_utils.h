#ifndef _DEBUG_UTILS_H_
#define _DEBUG_UTILS_H_

#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <debug_utils_settings.h>

/* C and C++ section */
#ifdef X
#undef X
#endif
#define X(x) x // magic. Do not touch.
typedef enum debug_unit_enum
{
    DEBUG_UNIT_LIST
} debug_unit_enum;
#undef X

typedef struct debug_unit
{
    int print;
    int log;
    FILE* fd;

} debug_unit;

void debug_utils_init(char* _path_prefix);
void debug_utils_close();

void debug_utils_setp(debug_unit_enum ue);
void debug_utils_unsetp(debug_unit_enum ue);

void debug_utils_setl(debug_unit_enum ue);
void debug_utils_unsetl(debug_unit_enum ue);

#define printd(arg1,arg2,...) {\
        printd_base(arg1,arg2,##__VA_ARGS__);\
    }

#define printm(arg1,arg2,...) {\
        printm_base(0,arg1,arg2,##__VA_ARGS__);\
    }

#define printb(arg1,arg2,...) {\
        printm_base(1,arg1,arg2,##__VA_ARGS__);\
    }

#define printm_once(arg1,arg2,...) {\
        {\
            static int __first__=1;\
            if(__first__){\
                __first__=0;\
                printm_base(0,arg1,arg2,##__VA_ARGS__);\
            }\
        }\
    }

#define printe(arg1,...) {\
        {\
            printe_base(arg1,##__VA_ARGS__);\
        }\
    }

void printd_base(debug_unit_enum ue, char* msg, ...);
void printm_base(int leave, debug_unit_enum ue, char* msg, ...);
void printe_base(char* msg, ...);
/* end of C and C++ section */

/* C section */
#ifndef __cplusplus

#define printdi(arg1,arg2,...) {\
        printdi_base(arg1,arg2,##__VA_ARGS__);\
    }

#define printmi(arg1,arg2,...) {\
        printmi_base(0,arg1,arg2,##__VA_ARGS__);\
    }

#define printbi(arg1,arg2,...) {\
        printmi_base(1,arg1,arg2,##__VA_ARGS__);\
    }

#define printmi_once(arg1,arg2,...) {\
        {\
            static int __first__=1;\
            if(__first__){\
                __first__=0;\
                printmi_base(0,arg1,arg2,##__VA_ARGS__);\
            }\
        }\
    }

void printdi_base(unsigned int id, debug_unit_enum ue, char* msg, ...);
void printmi_base(int leave, unsigned int id, debug_unit_enum ue, char* msg, ...);

#endif
/* end of C section */


/* C++ section */
#ifdef __cplusplus

#define BEGIN_DEBUG_UTILS_HELPER\
    class debug_utils_helper: public sc_module\
    {\
        public:\
            SC_HAS_PROCESS(debug_utils_helper);\
            debug_utils_helper(sc_module_name name): sc_module(name)\
            {\
                debug_utils_init("");\
                SC_THREAD(helper);\
                \
            }\
            void helper()\
            {

#define END_DEBUG_UTILS_HELPER\
    }\
    };\
    static debug_utils_helper debug_utils_helper_0("debug_utils_helper_0");

void printd_base(unsigned int id, debug_unit_enum ue, char* msg, ...);
void printm_base(int leave, unsigned int id, debug_unit_enum ue, char* msg, ...);

#endif
/* end of C++ section */

#endif

