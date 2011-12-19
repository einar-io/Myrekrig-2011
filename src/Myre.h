#ifndef MYRE_H
#define MYRE_H

/* assert() må man godt bruge */

#include <assert.h>

/* C++ må også godt være med */

#ifdef __cplusplus
extern "C" {
#endif // #ifdef __cplusplus

/* Definitioner læsbare af myrerne. */

#define NewBaseAnts 25
#define NewBaseFood 50
#define MaxSquareAnts 100
#define MaxSquareFood 200
#define BaseValue (NewBaseAnts+NewBaseFood)

/* Myre-registrerings-makro... */

#define DefineAnt(name,title,func,mem) \
extern int func(struct SquareData *, mem *); \
void name##_CopyIn(struct AntData *s, mem *a, int n) \
{while(n--){*a=*((mem *)(s->Mem));a++;s=s->MapNext;}} \
void name##_CopyOut(struct AntData *s, mem *a, int n) \
{while(n--){*((mem *)(s->Mem))=*a;a++;s=s->MapNext;}} \
char * const name##_Title = title; \
int (* const name##_Func)(struct SquareData *, mem *) = func; \
void (* const name##_CopyInFunc)(struct AntData *s, mem *a, int n) = name##_CopyIn; \
void (* const name##_CopyOutFunc)(struct AntData *s, mem *a, int n) = name##_CopyOut; \
const int name##_MemSize = sizeof(mem);

/* typedefs... */

#if !(defined(_SYS_BSD_TYPES_H) || defined(_LINUX_TYPES_H) || defined(_SYS_TYPES_H))
typedef unsigned char u_char;
typedef unsigned short u_short;
//typedef unsigned long u_long;
typedef unsigned long long u_long;	// linux has 64bit for long, we need to debug and windows requires long long for 64bit.
#endif
#ifndef __cplusplus
typedef int bool;
#define false 0
#define true (!false)
#endif

/* structs... */

struct SquareData {
   u_char NumAnts;
   u_char Base;
   u_char Team;
   u_char NumFood;
};

struct AntData {
   struct AntData *MapNext, *MapPrev;
//   u_short Index, Team, XPos, YPos;
   u_short Index, XPos, YPos;
   u_char Team;
   u_long Age, NextTurn;
   u_long Mem[1];
};

/* C++ må også godt være med */

#ifdef __cplusplus
}
#endif // #ifdef __cplusplus

#endif
