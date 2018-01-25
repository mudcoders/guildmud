#include <sys/types.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>

/* include main header file */
#include "mud.h"

LIST  *eventqueue[MAX_EVENT_HASH];
STACK *event_free = NULL;
LIST  *global_events = NULL;
int    current_bucket = 0;


/* function   :: enqueue_event()
 * arguments  :: the event to enqueue and the delay time.
 * ======================================================
 * This function takes an event which has _already_ been
 * linked locally to it's owner, and places it in the
 * event queue, thus making it execute in the given time.
 */
bool enqueue_event(EVENT_DATA *event, int game_pulses)
{
  int bucket, passes;

  /* check to see if the event has been attached to an owner */
  if (event->ownertype == EVENT_UNOWNED)
  {
    bug("enqueue_event: event type %d with no owner.", event->type);
    return FALSE;
  }

  /* An event must be enqueued into the future */
  if (game_pulses < 1)
    game_pulses = 1;

  /* calculate which bucket to put the event in,
   * and how many passes the event must stay in the queue.
   */
  bucket = (game_pulses + current_bucket) % MAX_EVENT_HASH;
  passes = game_pulses / MAX_EVENT_HASH;

  /* let the event store this information */
  event->passes = passes;
  event->bucket = bucket;

  /* attach the event in the queue */
  AttachToList(event, eventqueue[bucket]);

  /* success */
  return TRUE;
}

/* function   :: dequeue_event()
 * arguments  :: the event to dequeue.
 * ======================================================
 * This function takes an event which has _already_ been
 * enqueued, and removes it both from the event queue, and
 * from the owners local list. This function is usually
 * called when the owner is destroyed or after the event
 * is executed.
 */
void dequeue_event(EVENT_DATA *event)
{
  /* dequeue from the bucket */
  DetachFromList(event, eventqueue[event->bucket]);

  /* dequeue from owners local list */
  switch(event->ownertype)
  {
    default:
      bug("dequeue_event: event type %d has no owner.", event->type);
      break;
    case EVENT_OWNER_GAME:
      DetachFromList(event, global_events);
      break;
    case EVENT_OWNER_DMOB:
      DetachFromList(event, event->owner.dMob->events);
      break;
    case EVENT_OWNER_DSOCKET:
      DetachFromList(event, event->owner.dSock->events);
      break;
  }

  /* free argument */
  free(event->argument);

  /* attach to free stack */
  PushStack(event, event_free);
}

/* function   :: alloc_event()
 * arguments  :: none
 * ======================================================
 * This function allocates memory for an event, and
 * makes sure it's values are set to default.
 */
EVENT_DATA *alloc_event()
{
  EVENT_DATA *event;

  if (StackSize(event_free) <= 0)
    event = malloc(sizeof(*event));
  else
    event = (EVENT_DATA *) PopStack(event_free);

  /* clear the event */
  event->fun        = NULL;
  event->argument   = NULL;
  event->owner.dMob = NULL;  /* only need to NULL one of the union members */
  event->passes     = 0;
  event->bucket     = 0;
  event->ownertype  = EVENT_UNOWNED;
  event->type       = EVENT_NONE;

  /* return the allocated and cleared event */
  return event;
}

/* function   :: init_event_queue()
 * arguments  :: what section to initialize.
 * ======================================================
 * This function is used to initialize the event queue,
 * and the first section should be initialized at boot,
 * the second section should be called after all areas,
 * players, monsters, etc has been loaded into memory,
 * and it should contain all maintanence events.
 */
void init_event_queue(int section)
{
  EVENT_DATA *event;
  int i;

  if (section == 1)
  {
    for (i = 0; i < MAX_EVENT_HASH; i++)
    {
      eventqueue[i] = AllocList();
    }

    event_free = AllocStack();
    global_events = AllocList();
  }
  else if (section == 2)
  {
    event = alloc_event();
    event->fun = &event_game_tick;
    event->type = EVENT_GAME_TICK;
    add_event_game(event, 10 * 60 * PULSES_PER_SECOND);
  }
}

/* function   :: heartbeat()
 * arguments  :: none
 * ======================================================
 * This function is called once per game pulse, and it will
 * check the queue, and execute any pending events, which
 * has been enqueued to execute at this specific time.
 */
void heartbeat()
{
  EVENT_DATA *event;
  ITERATOR Iter;

  /* current_bucket should be global, it is also used in enqueue_event
   * to figure out what bucket to place the new event in.
   */
  current_bucket = (current_bucket + 1) % MAX_EVENT_HASH;

  AttachIterator(&Iter, eventqueue[current_bucket]);
  while ((event = (EVENT_DATA *) NextInList(&Iter)) != NULL)
  {
    /* Here we use the event->passes integer, to keep track of
     * how many times we have ignored this event.
     */
    if (event->passes-- > 0) continue;

    /* execute event and extract if needed. We assume that all
     * event functions are of the following prototype
     *
     * bool event_function ( EVENT_DATA *event );
     *
     * Any event returning TRUE is not dequeued, it is assumed
     * that the event has dequeued itself.
     */
    if (!((*event->fun)(event)))
      dequeue_event(event);
  }
  DetachIterator(&Iter);
}

/* function   :: add_event_mobile()
 * arguments  :: the event, the owner and the delay
 * ======================================================
 * This function attaches an event to a mobile, and sets
 * all the correct values, and makes sure it is enqueued
 * into the event queue.
 */
void add_event_mobile(EVENT_DATA *event, D_MOBILE *dMob, int delay)
{
  /* check to see if the event has a type */
  if (event->type == EVENT_NONE)
  {
    bug("add_event_mobile: no type.");
    return;
  }

  /* check to see of the event has a callback function */
  if (event->fun == NULL)
  {
    bug("add_event_mobile: event type %d has no callback function.", event->type);
    return;
  }

  /* set the correct variables for this event */
  event->ownertype  = EVENT_OWNER_DMOB;
  event->owner.dMob = dMob;

  /* attach the event to the mobiles local list */
  AttachToList(event, dMob->events);

  /* attempt to enqueue the event */
  if (enqueue_event(event, delay) == FALSE)
    bug("add_event_mobile: event type %d failed to be enqueued.", event->type);
}

/* function   :: add_event_socket()
 * arguments  :: the event, the owner and the delay
 * ======================================================
 * This function attaches an event to a socket, and sets
 * all the correct values, and makes sure it is enqueued
 * into the event queue.
 */
void add_event_socket(EVENT_DATA *event, D_SOCKET *dSock, int delay)
{
  /* check to see if the event has a type */
  if (event->type == EVENT_NONE)
  {
    bug("add_event_socket: no type.");
    return;
  }

  /* check to see of the event has a callback function */
  if (event->fun == NULL)
  {
    bug("add_event_socket: event type %d has no callback function.", event->type);
    return;
  }

  /* set the correct variables for this event */
  event->ownertype   = EVENT_OWNER_DSOCKET;
  event->owner.dSock = dSock;

  /* attach the event to the sockets local list */
  AttachToList(event, dSock->events);

  /* attempt to enqueue the event */
  if (enqueue_event(event, delay) == FALSE)
    bug("add_event_socket: event type %d failed to be enqueued.", event->type);
}

/* function   :: add_event_game()
 * arguments  :: the event and the delay
 * ======================================================
 * This funtion attaches an event to the list og game
 * events, and makes sure it's enqueued with the correct
 * delay time.
 */
void add_event_game(EVENT_DATA *event, int delay)
{
  /* check to see if the event has a type */
  if (event->type == EVENT_NONE)
  {
    bug("add_event_game: no type.");
    return;
  }

  /* check to see of the event has a callback function */
  if (event->fun == NULL)
  {
    bug("add_event_game: event type %d has no callback function.", event->type);
    return;
  }

  /* set the correct variables for this event */
  event->ownertype = EVENT_OWNER_GAME;

  /* attach the event to the gamelist */
  AttachToList(event, global_events);

  /* attempt to enqueue the event */
  if (enqueue_event(event, delay) == FALSE)
    bug("add_event_game: event type %d failed to be enqueued.", event->type);
}

/* function   :: event_isset_socket()
 * arguments  :: the socket and the type of event
 * ======================================================
 * This function checks to see if a given type of event
 * is enqueued/attached to a given socket, and if it is,
 * it will return a pointer to this event.
 */
EVENT_DATA *event_isset_socket(D_SOCKET *dSock, int type)
{
  EVENT_DATA *event;
  ITERATOR Iter;

  AttachIterator(&Iter, dSock->events);
  while ((event = (EVENT_DATA *) NextInList(&Iter)) != NULL)
  {
    if (event->type == type)
      break;
  }
  DetachIterator(&Iter);

  return event;
}

/* function   :: event_isset_mobile()
 * arguments  :: the mobile and the type of event
 * ======================================================
 * This function checks to see if a given type of event
 * is enqueued/attached to a given mobile, and if it is,
 * it will return a pointer to this event.
 */
EVENT_DATA *event_isset_mobile(D_MOBILE *dMob, int type)
{
  EVENT_DATA *event;
  ITERATOR Iter;

  AttachIterator(&Iter, dMob->events);
  while ((event = (EVENT_DATA *) NextInList(&Iter)) != NULL)
  {
    if (event->type == type)
      break;
  }
  DetachIterator(&Iter);

  return event;
}

/* function   :: strip_event_socket()
 * arguments  :: the socket and the type of event
 * ======================================================
 * This function will dequeue all events of a given type
 * from the given socket.
 */
void strip_event_socket(D_SOCKET *dSock, int type)
{
  EVENT_DATA *event;
  ITERATOR Iter;

  AttachIterator(&Iter, dSock->events);
  while ((event = (EVENT_DATA *) NextInList(&Iter)) != NULL)
  {
    if (event->type == type)
      dequeue_event(event);
  }
  DetachIterator(&Iter);
}

/* function   :: strip_event_mobile()
 * arguments  :: the mobile and the type of event
 * ======================================================
 * This function will dequeue all events of a given type
 * from the given mobile.
 */
void strip_event_mobile(D_MOBILE *dMob, int type)
{
  EVENT_DATA *event;
  ITERATOR Iter;

  AttachIterator(&Iter, dMob->events);
  while ((event = (EVENT_DATA *) NextInList(&Iter)) != NULL)
  {
    if (event->type == type)
      dequeue_event(event);
  }
  DetachIterator(&Iter);
}

/* function   :: init_events_mobile()
 * arguments  :: the mobile
 * ======================================================
 * this function should be called when a player is loaded,
 * it will initialize all updating events for that player.
 */
void init_events_player(D_MOBILE *dMob)
{
  EVENT_DATA *event;

  /* save the player every 2 minutes */
  event = alloc_event();
  event->fun = &event_mobile_save;
  event->type = EVENT_MOBILE_SAVE;
  add_event_mobile(event, dMob, 2 * 60 * PULSES_PER_SECOND);
}

/* function   :: init_events_socket()
 * arguments  :: the mobile
 * ======================================================
 * this function should be called when a socket connects,
 * it will initialize all updating events for that socket.
 */
void init_events_socket(D_SOCKET *dSock)
{
  EVENT_DATA *event;

  /* disconnect/idle */
  event = alloc_event();
  event->fun = &event_socket_idle;
  event->type = EVENT_SOCKET_IDLE;
  add_event_socket(event, dSock, 5 * 60 * PULSES_PER_SECOND);
}
