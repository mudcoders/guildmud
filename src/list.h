/* file: list.h
 *
 * Headerfile for a basic double-linked list
 */

#ifndef _LIST_HEADER
#define _LIST_HEADER

typedef struct Cell
{
  struct Cell  *_pNextCell;
  struct Cell  *_pPrevCell;
  void         *_pContent;
  int           _valid;
} CELL;

typedef struct List
{
  CELL  *_pFirstCell;
  CELL  *_pLastCell;
  int    _iterators;
  int    _size;
  int    _valid;
} LIST;

typedef struct Iterator
{
  LIST  *_pList;
  CELL  *_pCell;
} ITERATOR;

LIST *AllocList          ( void );
void  AttachIterator     ( ITERATOR *pIter, LIST *pList);
void *NextInList         ( ITERATOR *pIter );
void  AttachToList       ( void *pContent, LIST *pList );
void  DetachFromList     ( void *pContent, LIST *pList );
void  DetachIterator     ( ITERATOR *pIter );
void  FreeList           ( LIST *pList );
int   SizeOfList         ( LIST *pList );

#endif
