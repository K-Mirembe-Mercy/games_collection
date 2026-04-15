/* ============================================================
 * event_ds.c  –  Data-structure implementations
 *   • Doubly-linked list  (EventList)
 *   • Min-heap            (EventHeap)  – sorted by start_time
 *   • Hash map            (HashMap)    – keyed by event id
 * ============================================================ */

#include "event_manager.h"

/* ── Internal helpers ───────────────────────────────────────── */

static unsigned int hash_id(unsigned int id) {
    /* Knuth multiplicative hashing */
    return (id * 2654435761u) & (HASH_TABLE_SIZE - 1);
}

/* ════════════════════════════════════════════════════════════
 * EVENT  ALLOCATION
 * ════════════════════════════════════════════════════════════ */

Event *em_event_alloc(void) {
    Event *ev = calloc(1, sizeof(Event));
    if (!ev) return NULL;
    ev->created_at = time(NULL);
    ev->updated_at = ev->created_at;
    return ev;
}

void em_event_free(Event *ev) {
    free(ev);
}

Event *em_event_clone(const Event *src) {
    if (!src) return NULL;
    Event *dst = malloc(sizeof(Event));
    if (!dst) return NULL;
    memcpy(dst, src, sizeof(Event));
    dst->prev = dst->next = NULL;
    return dst;
}

/* ════════════════════════════════════════════════════════════
 * DOUBLY-LINKED LIST
 * ════════════════════════════════════════════════════════════ */

static void list_append(EventList *list, Event *ev) {
    ev->prev = list->tail;
    ev->next = NULL;
    if (list->tail) list->tail->next = ev;
    else            list->head       = ev;
    list->tail = ev;
    list->count++;
}

static void list_remove(EventList *list, Event *ev) {
    if (ev->prev) ev->prev->next = ev->next;
    else          list->head     = ev->next;
    if (ev->next) ev->next->prev = ev->prev;
    else          list->tail     = ev->prev;
    ev->prev = ev->next = NULL;
    list->count--;
}

/* ════════════════════════════════════════════════════════════
 * MIN-HEAP  (keyed on start_time)
 * ════════════════════════════════════════════════════════════ */

#define HEAP_INITIAL_CAPACITY 16

static EMStatus heap_init(EventHeap *h) {
    h->data     = malloc(HEAP_INITIAL_CAPACITY * sizeof(Event *));
    if (!h->data) return EM_ERR_ALLOC;
    h->size     = 0;
    h->capacity = HEAP_INITIAL_CAPACITY;
    return EM_OK;
}

static void heap_destroy(EventHeap *h) {
    free(h->data);
    h->data = NULL;
    h->size = h->capacity = 0;
}

static void heap_swap(EventHeap *h, size_t a, size_t b) {
    Event *tmp  = h->data[a];
    h->data[a]  = h->data[b];
    h->data[b]  = tmp;
}

static void heap_sift_up(EventHeap *h, size_t i) {
    while (i > 0) {
        size_t parent = (i - 1) / 2;
        if (h->data[parent]->start_time <= h->data[i]->start_time) break;
        heap_swap(h, parent, i);
        i = parent;
    }
}

static void heap_sift_down(EventHeap *h, size_t i) {
    size_t n = h->size;
    while (1) {
        size_t smallest = i;
        size_t left  = 2 * i + 1;
        size_t right = 2 * i + 2;
        if (left  < n && h->data[left]->start_time  < h->data[smallest]->start_time) smallest = left;
        if (right < n && h->data[right]->start_time < h->data[smallest]->start_time) smallest = right;
        if (smallest == i) break;
        heap_swap(h, smallest, i);
        i = smallest;
    }
}

static EMStatus heap_push(EventHeap *h, Event *ev) {
    if (h->size == h->capacity) {
        size_t new_cap = h->capacity * 2;
        Event **tmp = realloc(h->data, new_cap * sizeof(Event *));
        if (!tmp) return EM_ERR_ALLOC;
        h->data     = tmp;
        h->capacity = new_cap;
    }
    h->data[h->size++] = ev;
    heap_sift_up(h, h->size - 1);
    return EM_OK;
}

static void heap_remove_by_id(EventHeap *h, unsigned int id) {
    for (size_t i = 0; i < h->size; i++) {
        if (h->data[i]->id == id) {
            h->data[i] = h->data[--h->size];
            if (i < h->size) {
                heap_sift_up(h, i);
                heap_sift_down(h, i);
            }
            return;
        }
    }
}

/* ════════════════════════════════════════════════════════════
 * HASH MAP  (open chaining)
 * ════════════════════════════════════════════════════════════ */

static EMStatus map_insert(HashMap *m, Event *ev) {
    unsigned int bucket = hash_id(ev->id);
    /* Check for duplicate */
    for (HashNode *n = m->buckets[bucket]; n; n = n->next)
        if (n->key == ev->id) return EM_ERR_DUPLICATE;

    HashNode *node = malloc(sizeof(HashNode));
    if (!node) return EM_ERR_ALLOC;
    node->key   = ev->id;
    node->event = ev;
    node->next  = m->buckets[bucket];
    m->buckets[bucket] = node;
    m->count++;
    return EM_OK;
}

static Event *map_find(const HashMap *m, unsigned int id) {
    unsigned int bucket = hash_id(id);
    for (HashNode *n = m->buckets[bucket]; n; n = n->next)
        if (n->key == id) return n->event;
    return NULL;
}

static bool map_remove(HashMap *m, unsigned int id) {
    unsigned int bucket = hash_id(id);
    HashNode **pp = &m->buckets[bucket];
    while (*pp) {
        if ((*pp)->key == id) {
            HashNode *dead = *pp;
            *pp = dead->next;
            free(dead);
            m->count--;
            return true;
        }
        pp = &(*pp)->next;
    }
    return false;
}

static void map_destroy(HashMap *m) {
    for (int i = 0; i < HASH_TABLE_SIZE; i++) {
        HashNode *n = m->buckets[i];
        while (n) {
            HashNode *next = n->next;
            free(n);
            n = next;
        }
        m->buckets[i] = NULL;
    }
    m->count = 0;
}

/* ════════════════════════════════════════════════════════════
 * EVENT MANAGER  –  lifecycle
 * ════════════════════════════════════════════════════════════ */

EventManager *em_create(const char *db_path) {
    EventManager *em = calloc(1, sizeof(EventManager));
    if (!em) return NULL;

    if (heap_init(&em->heap) != EM_OK) { free(em); return NULL; }
    em->next_id = 1;
    strncpy(em->db_path, db_path ? db_path : DEFAULT_DB_FILE,
            sizeof(em->db_path) - 1);

    em_load(em);   /* best-effort; ignore error if file missing */
    return em;
}

void em_destroy(EventManager *em) {
    if (!em) return;

    /* Free all event nodes */
    Event *ev = em->list.head;
    while (ev) {
        Event *next = ev->next;
        em_event_free(ev);
        ev = next;
    }

    heap_destroy(&em->heap);
    map_destroy(&em->map);
    free(em);
}

/* ════════════════════════════════════════════════════════════
 * EVENT MANAGER  –  CRUD
 * ════════════════════════════════════════════════════════════ */

EMStatus em_add_event(EventManager *em, Event *ev) {
    if (!em || !ev) return EM_ERR_INVALID;
    if (ev->start_time == 0 || !ev->title[0]) return EM_ERR_INVALID;

    ev->id         = em->next_id++;
    ev->created_at = time(NULL);
    ev->updated_at = ev->created_at;

    EMStatus s;
    if ((s = map_insert(&em->map, ev)) != EM_OK) return s;
    if ((s = heap_push(&em->heap, ev)) != EM_OK) {
        map_remove(&em->map, ev->id);
        return s;
    }
    list_append(&em->list, ev);
    return EM_OK;
}

EMStatus em_remove_event(EventManager *em, unsigned int id) {
    if (!em) return EM_ERR_INVALID;
    Event *ev = map_find(&em->map, id);
    if (!ev) return EM_ERR_NOT_FOUND;

    heap_remove_by_id(&em->heap, id);
    list_remove(&em->list, ev);
    map_remove(&em->map, id);
    em_event_free(ev);
    return EM_OK;
}

Event *em_find_by_id(const EventManager *em, unsigned int id) {
    if (!em) return NULL;
    return map_find(&em->map, id);
}

EMStatus em_update_event(EventManager *em, unsigned int id, const Event *updated) {
    if (!em || !updated) return EM_ERR_INVALID;
    Event *ev = map_find(&em->map, id);
    if (!ev) return EM_ERR_NOT_FOUND;

    /* Preserve structural pointers & id */
    Event *prev_ptr  = ev->prev;
    Event *next_ptr  = ev->next;
    unsigned int eid = ev->id;
    time_t created   = ev->created_at;

    memcpy(ev, updated, sizeof(Event));
    ev->id         = eid;
    ev->prev       = prev_ptr;
    ev->next       = next_ptr;
    ev->created_at = created;
    ev->updated_at = time(NULL);

    /* Re-heapify (start_time may have changed) */
    heap_remove_by_id(&em->heap, id);
    heap_push(&em->heap, ev);
    return EM_OK;
}

/* ════════════════════════════════════════════════════════════
 * EVENT MANAGER  –  QUERIES
 * ════════════════════════════════════════════════════════════ */

/* Helper: allocate a result list (events are NOT owned) */
static EventList *result_list_alloc(void) {
    EventList *l = calloc(1, sizeof(EventList));
    return l;
}

/* For result lists, nodes are clones so we free them */
void em_free_result_list(EventList *list) {
    if (!list) return;
    Event *ev = list->head;
    while (ev) {
        Event *n = ev->next;
        em_event_free(ev);
        ev = n;
    }
    free(list);
}

EventList *em_filter_by_category(const EventManager *em, Category cat) {
    EventList *res = result_list_alloc();
    if (!res) return NULL;
    for (Event *ev = em->list.head; ev; ev = ev->next) {
        if (!ev->is_cancelled && ev->category == cat) {
            Event *clone = em_event_clone(ev);
            if (clone) list_append(res, clone);
        }
    }
    return res;
}

EventList *em_filter_by_priority(const EventManager *em, Priority p) {
    EventList *res = result_list_alloc();
    if (!res) return NULL;
    for (Event *ev = em->list.head; ev; ev = ev->next) {
        if (!ev->is_cancelled && ev->priority == p) {
            Event *clone = em_event_clone(ev);
            if (clone) list_append(res, clone);
        }
    }
    return res;
}

EventList *em_search_by_title(const EventManager *em, const char *keyword) {
    EventList *res = result_list_alloc();
    if (!res || !keyword) return res;

    /* Case-insensitive substring search */
    char kw[MAX_TITLE_LEN];
    strncpy(kw, keyword, sizeof(kw) - 1);
    kw[sizeof(kw)-1] = '\0';
    for (char *p = kw; *p; p++) *p = (char)tolower((unsigned char)*p);

    for (Event *ev = em->list.head; ev; ev = ev->next) {
        char title_lc[MAX_TITLE_LEN];
        strncpy(title_lc, ev->title, sizeof(title_lc) - 1);
        title_lc[sizeof(title_lc)-1] = '\0';
        for (char *p = title_lc; *p; p++) *p = (char)tolower((unsigned char)*p);

        if (strstr(title_lc, kw)) {
            Event *clone = em_event_clone(ev);
            if (clone) list_append(res, clone);
        }
    }
    return res;
}

EventList *em_upcoming_events(const EventManager *em, int days) {
    time_t now    = time(NULL);
    time_t cutoff = now + (time_t)days * 86400;
    EventList *res = result_list_alloc();
    if (!res) return NULL;

    for (Event *ev = em->list.head; ev; ev = ev->next) {
        if (!ev->is_cancelled && ev->start_time >= now && ev->start_time <= cutoff) {
            Event *clone = em_event_clone(ev);
            if (clone) list_append(res, clone);
        }
    }
    return res;
}

Event *em_next_event(const EventManager *em) {
    if (!em || em->heap.size == 0) return NULL;
    time_t now = time(NULL);
    /* Scan heap for first non-cancelled future event */
    for (size_t i = 0; i < em->heap.size; i++) {
        Event *ev = em->heap.data[i];
        if (!ev->is_cancelled && ev->start_time >= now)
            return ev;   /* heap is approx-ordered; good enough for peek */
    }
    return NULL;
}
