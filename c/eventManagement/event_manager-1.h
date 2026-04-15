#ifndef EVENT_MANAGER_H
#define EVENT_MANAGER_H

/* ============================================================
 * Event Management System
 * A production-grade C application demonstrating:
 *   - Modular architecture & separation of concerns
 *   - Dynamic memory management with safe allocations
 *   - Persistent storage via CSV serialization
 *   - Priority queue (min-heap) for deadline scheduling
 *   - Hash map for O(1) event lookup by ID
 *   - Doubly-linked list for ordered traversal
 *   - ANSI terminal UI with interactive menus
 * ============================================================ */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <stdbool.h>
#include <ctype.h>
#include <errno.h>

/* ── Compile-time constants ─────────────────────────────────── */
#define APP_VERSION        "1.0.0"
#define MAX_TITLE_LEN      128
#define MAX_DESC_LEN       512
#define MAX_LOCATION_LEN   128
#define MAX_ORGANIZER_LEN  64
#define MAX_TAG_LEN        32
#define MAX_TAGS           8
#define MAX_ATTENDEES      64
#define MAX_ATTENDEE_LEN   64
#define HASH_TABLE_SIZE    256
#define DEFAULT_DB_FILE    "events.csv"
#define DATE_FMT           "%Y-%m-%d %H:%M"

/* ── Return / error codes ───────────────────────────────────── */
typedef enum {
    EM_OK            =  0,
    EM_ERR_ALLOC     = -1,
    EM_ERR_NOT_FOUND = -2,
    EM_ERR_DUPLICATE = -3,
    EM_ERR_INVALID   = -4,
    EM_ERR_IO        = -5,
    EM_ERR_FULL      = -6
} EMStatus;

/* ── Event priority / category ──────────────────────────────── */
typedef enum {
    PRIORITY_LOW    = 1,
    PRIORITY_MEDIUM = 2,
    PRIORITY_HIGH   = 3,
    PRIORITY_URGENT = 4
} Priority;

typedef enum {
    CAT_CONFERENCE  = 0,
    CAT_MEETING     = 1,
    CAT_WORKSHOP    = 2,
    CAT_SOCIAL      = 3,
    CAT_WEBINAR     = 4,
    CAT_OTHER       = 5,
    CAT_COUNT       = 6
} Category;

/* ── Core event structure ───────────────────────────────────── */
typedef struct Event {
    unsigned int  id;
    char          title[MAX_TITLE_LEN];
    char          description[MAX_DESC_LEN];
    char          location[MAX_LOCATION_LEN];
    char          organizer[MAX_ORGANIZER_LEN];
    time_t        start_time;
    time_t        end_time;
    Priority      priority;
    Category      category;
    bool          is_recurring;
    int           recur_days;          /* 0 = no recurrence          */
    char          tags[MAX_TAGS][MAX_TAG_LEN];
    int           tag_count;
    char          attendees[MAX_ATTENDEES][MAX_ATTENDEE_LEN];
    int           attendee_count;
    bool          is_cancelled;
    time_t        created_at;
    time_t        updated_at;

    /* Intrusive doubly-linked list pointers */
    struct Event *prev;
    struct Event *next;
} Event;

/* ── Doubly-linked list ─────────────────────────────────────── */
typedef struct {
    Event       *head;
    Event       *tail;
    size_t       count;
} EventList;

/* ── Min-heap (priority queue) sorted by start_time ────────── */
typedef struct {
    Event      **data;
    size_t       size;
    size_t       capacity;
} EventHeap;

/* ── Hash map bucket (chaining) ─────────────────────────────── */
typedef struct HashNode {
    unsigned int   key;          /* event id */
    Event         *event;
    struct HashNode *next;
} HashNode;

typedef struct {
    HashNode     *buckets[HASH_TABLE_SIZE];
    size_t        count;
} HashMap;

/* ── Top-level manager ──────────────────────────────────────── */
typedef struct {
    EventList     list;
    EventHeap     heap;
    HashMap       map;
    unsigned int  next_id;
    char          db_path[256];
} EventManager;

/* ── Function declarations ──────────────────────────────────── */

/* Lifecycle */
EventManager *em_create(const char *db_path);
void          em_destroy(EventManager *em);

/* CRUD */
EMStatus      em_add_event(EventManager *em, Event *ev);
EMStatus      em_remove_event(EventManager *em, unsigned int id);
Event        *em_find_by_id(const EventManager *em, unsigned int id);
EMStatus      em_update_event(EventManager *em, unsigned int id, const Event *updated);

/* Queries */
EventList    *em_filter_by_category(const EventManager *em, Category cat);
EventList    *em_filter_by_priority(const EventManager *em, Priority p);
EventList    *em_search_by_title(const EventManager *em, const char *keyword);
EventList    *em_upcoming_events(const EventManager *em, int days);
Event        *em_next_event(const EventManager *em);
void          em_free_result_list(EventList *list);

/* Persistence */
EMStatus      em_save(const EventManager *em);
EMStatus      em_load(EventManager *em);

/* Statistics */
void          em_print_stats(const EventManager *em);

/* Utilities */
Event        *em_event_alloc(void);
void          em_event_free(Event *ev);
Event        *em_event_clone(const Event *src);
const char   *em_priority_str(Priority p);
const char   *em_category_str(Category c);
const char   *em_status_str(EMStatus s);
bool          em_parse_datetime(const char *s, time_t *out);
void          em_format_datetime(time_t t, char *buf, size_t n);
void          em_print_event(const Event *ev, bool detailed);

/* Interactive UI */
void          em_run_ui(EventManager *em);

#endif /* EVENT_MANAGER_H */
