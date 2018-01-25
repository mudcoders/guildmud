/* file: stack.h
 *
 * A stack, for pushing and popping
 */

#ifndef _STACK_HEADER
#define _STACK_HEADER

typedef struct BBGStack     STACK;

STACK *AllocStack   ( void );
void   FreeStack    ( STACK *pStack );
void  *PopStack     ( STACK *pStack );
void   PushStack    ( void *pContent, STACK *pStack );
int    StackSize    ( STACK *pStack );

#endif
