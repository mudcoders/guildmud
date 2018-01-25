/* file: stack.h
 *
 * The implementation of a stack
 */

#include <stdlib.h>

#include "stack.h"

typedef struct BBGStackCell SCELL;

struct BBGStack
{
  SCELL *_pCells;
  int    _iSize;
};

struct BBGStackCell
{
  SCELL  *_pNext;
  void   *_pContent;
};


STACK *AllocStack()
{
  STACK *pStack;

  pStack = malloc(sizeof(*pStack));
  pStack->_pCells = NULL;
  pStack->_iSize = 0;

  return pStack;
}

void FreeStack(STACK *pStack)
{
  SCELL *pCell;

  while ((pCell = pStack->_pCells) != NULL)
  {
    pStack->_pCells = pCell->_pNext;
    free(pCell);
  }

  free(pStack);
}

void *PopStack(STACK *pStack)
{
  SCELL *pCell;

  if ((pCell = pStack->_pCells) != NULL)
  {
    void *pContent = pCell->_pContent;

    pStack->_pCells = pCell->_pNext;
    pStack->_iSize--;
    free(pCell);

    return pContent;
  }

  return NULL;
}

void PushStack(void *pContent, STACK *pStack)
{
  SCELL *pCell;

  pCell = malloc(sizeof(*pCell));
  pCell->_pNext = pStack->_pCells;
  pCell->_pContent = pContent;
  pStack->_pCells = pCell;
  pStack->_iSize++;
}

int StackSize(STACK *pStack)
{
  return pStack->_iSize;
}
