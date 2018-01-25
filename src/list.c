/* file: list.c
 *
 * The implementation of a basic double-linked list
 */

#include <stdlib.h>

#include "list.h"

/* local procedures */
void  FreeCell        ( CELL *pCell, LIST *pList );
CELL *AllocCell       ( void );
void  InvalidateCell  ( CELL *pCell );

LIST *AllocList()
{
  LIST *pList;

  pList = malloc(sizeof(*pList));
  pList->_pFirstCell = NULL;
  pList->_pLastCell = NULL;
  pList->_iterators = 0;
  pList->_valid = 1;
  pList->_size = 0;

  return pList;
}

void AttachIterator(ITERATOR *pIter, LIST *pList)
{
  pIter->_pList = pList;

  if (pList != NULL)
  {
    pList->_iterators++;
    pIter->_pCell = pList->_pFirstCell;
  }
  else
    pIter->_pCell = NULL;
}

CELL *AllocCell()
{
  CELL *pCell;

  pCell = malloc(sizeof(*pCell));
  pCell->_pNextCell = NULL;
  pCell->_pPrevCell = NULL;
  pCell->_pContent = NULL;
  pCell->_valid = 1;

  return pCell;
}

void AttachToList(void *pContent, LIST *pList)
{
  CELL *pCell;
  int found = 0;

  for (pCell = pList->_pFirstCell; pCell != NULL; pCell = pCell->_pNextCell)
  {
    if (!pCell->_valid)
      continue;

    if (pCell->_pContent == pContent)
    {
      found = 1;
      break;
    }
  }

  /* do not attach to list if already here */
  if (found)
    return;

  pCell = AllocCell();
  pCell->_pContent = pContent;
  pCell->_pNextCell = pList->_pFirstCell;

  if (pList->_pFirstCell != NULL)
    pList->_pFirstCell->_pPrevCell = pCell;
  if (pList->_pLastCell == NULL)
    pList->_pLastCell = pCell;

  pList->_pFirstCell = pCell;

  pList->_size++;
}

void DetachFromList(void *pContent, LIST *pList)
{
  CELL *pCell;

  for (pCell = pList->_pFirstCell; pCell != NULL; pCell = pCell->_pNextCell)
  {
    if (pCell->_pContent == pContent)
    {
      if (pList->_iterators > 0)
        InvalidateCell(pCell);
      else
        FreeCell(pCell, pList);
      pList->_size--;
      return;
    }
  }
}

void DetachIterator(ITERATOR *pIter)
{
  LIST *pList = pIter->_pList;

  if (pList != NULL)
  {
    pList->_iterators--;

    /* if this is the only iterator, free all invalid cells */
    if (pList->_iterators <= 0)
    {
      CELL *pCell, *pNextCell;

      for (pCell = pList->_pFirstCell; pCell != NULL; pCell = pNextCell)
      {
        pNextCell = pCell->_pNextCell;

        if (!pCell->_valid)
          FreeCell(pCell, pList);
      }

      if (!pList->_valid)
        FreeList(pList);
    }
  }
}

void FreeList(LIST *pList)
{
  CELL *pCell, *pNextCell;

  /* if we have any unfinished iterators, wait for later */
  if (pList->_iterators > 0)
  {
    pList->_valid = 0;
    return;
  }

  for (pCell = pList->_pFirstCell; pCell != NULL; pCell = pNextCell)
  {
    pNextCell = pCell->_pNextCell;

    FreeCell(pCell, pList);
  }

  free(pList);
}

void FreeCell(CELL *pCell, LIST *pList)
{
  if (pList->_pFirstCell == pCell)
    pList->_pFirstCell = pCell->_pNextCell;

  if (pList->_pLastCell == pCell)
    pList->_pLastCell = pCell->_pPrevCell;

  if (pCell->_pPrevCell != NULL)
    pCell->_pPrevCell->_pNextCell = pCell->_pNextCell;

  if (pCell->_pNextCell != NULL) 
    pCell->_pNextCell->_pPrevCell = pCell->_pPrevCell;

  free(pCell);
}

void InvalidateCell(CELL *pCell)
{
  pCell->_valid = 0;
}

void *NextInList(ITERATOR *pIter)
{
  void *pContent = NULL;

  /* skip invalid cells */
  while (pIter->_pCell != NULL && !pIter->_pCell->_valid)
  {
    pIter->_pCell = pIter->_pCell->_pNextCell;
  }

  if (pIter->_pCell != NULL)
  {
    pContent = pIter->_pCell->_pContent;
    pIter->_pCell = pIter->_pCell->_pNextCell;
  }

  return pContent;
}

int SizeOfList(LIST *pList)
{
  return pList->_size;
}
