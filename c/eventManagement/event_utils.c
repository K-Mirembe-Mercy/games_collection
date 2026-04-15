/* ============================================================
 * event_utils.c  –  Persistence, utilities, statistics
 * ============================================================ */

/* strptime requires _XOPEN_SOURCE or _GNU_SOURCE */
#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif

#include "event_manager.h"

/* ════════════════════════════════════════════════════════════
 * STRING / DATE UTILITIES
 * ════════════════════════════════════════════════════════════ */

const char *em_priority_str(Priority p) {
    switch (p) {
        case PRIORITY_LOW:    return "Low";
        case PRIORITY_MEDIUM: return "Medium";
        case PRIORITY_HIGH:   return "High";
        case PRIORITY_URGENT: return "Urgent";
        default:              return "Unknown";
    }
}

const char *em_category_str(Category c) {
    static const char *names[CAT_COUNT] = {
        "Conference", "Meeting", "Workshop", "Social", "Webinar", "Other"
    };
    if (c >= 0 && c < CAT_COUNT) return names[c];
    return "Other";
}

const char *em_status_str(EMStatus s) {
    switch (s) {
        case EM_OK:            return "OK";
        case EM_ERR_ALLOC:     return "Memory allocation failed";
        case EM_ERR_NOT_FOUND: return "Event not found";
        case EM_ERR_DUPLICATE: return "Duplicate ID";
        case EM_ERR_INVALID:   return "Invalid argument";
        case EM_ERR_IO:        return "I/O error";
        case EM_ERR_FULL:      return "Capacity exceeded";
        default:               return "Unknown error";
    }
}

bool em_parse_datetime(const char *s, time_t *out) {
    if (!s || !out) return false;
    struct tm tm = {0};
    if (!strptime(s, DATE_FMT, &tm)) return false;
    tm.tm_isdst = -1;
    *out = mktime(&tm);
    return (*out != (time_t)-1);
}

void em_format_datetime(time_t t, char *buf, size_t n) {
    if (!buf || n == 0) return;
    if (t == 0) { snprintf(buf, n, "N/A"); return; }
    struct tm *tm = localtime(&t);
    strftime(buf, n, DATE_FMT, tm);
}

/* Escape commas/newlines in a field for CSV */
static void csv_escape(const char *src, char *dst, size_t n) {
    size_t j = 0;
    for (size_t i = 0; src[i] && j + 2 < n; i++) {
        if (src[i] == ',' || src[i] == '\n' || src[i] == '\\')
            dst[j++] = '\\';
        dst[j++] = src[i];
    }
    dst[j] = '\0';
}

/* Unescape CSV field in-place */
static void csv_unescape(char *s) {
    char *r = s, *w = s;
    while (*r) {
        if (*r == '\\' && *(r+1)) { r++; }
        *w++ = *r++;
    }
    *w = '\0';
}

/* ════════════════════════════════════════════════════════════
 * PERSISTENCE  –  CSV format
 *
 * One event per line; fields separated by commas.
 * Commas inside field values are backslash-escaped.
 *
 * Column order:
 *  0  id
 *  1  title
 *  2  description
 *  3  location
 *  4  organizer
 *  5  start_time  (unix timestamp)
 *  6  end_time
 *  7  priority
 *  8  category
 *  9  is_recurring
 * 10  recur_days
 * 11  tags         (pipe-separated)
 * 12  attendees    (pipe-separated)
 * 13  is_cancelled
 * 14  created_at
 * 15  next_id      (written only in header comment line)
 * ════════════════════════════════════════════════════════════ */

#define CSV_SEP ","
#define FIELD_SEP "|"

EMStatus em_save(const EventManager *em) {
    if (!em) return EM_ERR_INVALID;

    FILE *fp = fopen(em->db_path, "w");
    if (!fp) return EM_ERR_IO;

    /* Header: store next_id so IDs survive across sessions */
    fprintf(fp, "#next_id=%u\n", em->next_id);
    fprintf(fp, "#id,title,description,location,organizer,"
                "start_time,end_time,priority,category,"
                "is_recurring,recur_days,tags,attendees,"
                "is_cancelled,created_at\n");

    char esc[MAX_DESC_LEN * 2];

    for (Event *ev = em->list.head; ev; ev = ev->next) {
        /* id */
        fprintf(fp, "%u,", ev->id);

        /* title */
        csv_escape(ev->title, esc, sizeof(esc));
        fprintf(fp, "%s,", esc);

        /* description */
        csv_escape(ev->description, esc, sizeof(esc));
        fprintf(fp, "%s,", esc);

        /* location */
        csv_escape(ev->location, esc, sizeof(esc));
        fprintf(fp, "%s,", esc);

        /* organizer */
        csv_escape(ev->organizer, esc, sizeof(esc));
        fprintf(fp, "%s,", esc);

        /* times */
        fprintf(fp, "%ld,%ld,", (long)ev->start_time, (long)ev->end_time);

        /* priority, category, recurring, recur_days */
        fprintf(fp, "%d,%d,%d,%d,",
                (int)ev->priority, (int)ev->category,
                (int)ev->is_recurring, ev->recur_days);

        /* tags (pipe-joined) */
        for (int i = 0; i < ev->tag_count; i++) {
            csv_escape(ev->tags[i], esc, sizeof(esc));
            fprintf(fp, "%s%s", esc, (i < ev->tag_count - 1) ? FIELD_SEP : "");
        }
        fprintf(fp, ",");

        /* attendees (pipe-joined) */
        for (int i = 0; i < ev->attendee_count; i++) {
            csv_escape(ev->attendees[i], esc, sizeof(esc));
            fprintf(fp, "%s%s", esc, (i < ev->attendee_count - 1) ? FIELD_SEP : "");
        }
        fprintf(fp, ",");

        /* is_cancelled, created_at */
        fprintf(fp, "%d,%ld\n", (int)ev->is_cancelled, (long)ev->created_at);
    }

    fclose(fp);
    return EM_OK;
}

/* Split a raw CSV line into fields; returns field count */
static int csv_split(char *line, char **fields, int max_fields) {
    int n = 0;
    char *p = line;
    while (n < max_fields) {
        fields[n++] = p;
        /* Advance past escaped chars and commas */
        while (*p) {
            if (*p == '\\') { p += 2; continue; }
            if (*p == ',')  { *p++ = '\0'; break; }
            p++;
        }
        if (!*p && *(p-1) != ',') break;   /* last field, no trailing comma */
    }
    return n;
}

EMStatus em_load(EventManager *em) {
    if (!em) return EM_ERR_INVALID;

    FILE *fp = fopen(em->db_path, "r");
    if (!fp) return EM_ERR_IO;

    char line[4096];
    while (fgets(line, sizeof(line), fp)) {
        /* strip trailing newline */
        line[strcspn(line, "\r\n")] = '\0';

        if (line[0] == '#') {
            /* Parse next_id from header */
            if (strncmp(line, "#next_id=", 9) == 0)
                em->next_id = (unsigned int)strtoul(line + 9, NULL, 10);
            continue;
        }
        if (!line[0]) continue;

        char *fields[20];
        int fc = csv_split(line, fields, 20);
        if (fc < 15) continue;   /* malformed row */

        Event *ev = em_event_alloc();
        if (!ev) { fclose(fp); return EM_ERR_ALLOC; }

        ev->id = (unsigned int)strtoul(fields[0], NULL, 10);

        strncpy(ev->title,       fields[1],  MAX_TITLE_LEN - 1);
        strncpy(ev->description, fields[2],  MAX_DESC_LEN - 1);
        strncpy(ev->location,    fields[3],  MAX_LOCATION_LEN - 1);
        strncpy(ev->organizer,   fields[4],  MAX_ORGANIZER_LEN - 1);
        csv_unescape(ev->title);
        csv_unescape(ev->description);
        csv_unescape(ev->location);
        csv_unescape(ev->organizer);

        ev->start_time   = (time_t)strtol(fields[5], NULL, 10);
        ev->end_time     = (time_t)strtol(fields[6], NULL, 10);
        ev->priority     = (Priority)atoi(fields[7]);
        ev->category     = (Category)atoi(fields[8]);
        ev->is_recurring = (bool)atoi(fields[9]);
        ev->recur_days   = atoi(fields[10]);

        /* tags */
        if (fields[11][0]) {
            char tags_buf[MAX_TAGS * MAX_TAG_LEN];
            strncpy(tags_buf, fields[11], sizeof(tags_buf) - 1);
            char *tok = strtok(tags_buf, FIELD_SEP);
            while (tok && ev->tag_count < MAX_TAGS) {
                csv_unescape(tok);
                strncpy(ev->tags[ev->tag_count++], tok, MAX_TAG_LEN - 1);
                tok = strtok(NULL, FIELD_SEP);
            }
        }

        /* attendees */
        if (fields[12][0]) {
            char att_buf[MAX_ATTENDEES * MAX_ATTENDEE_LEN];
            strncpy(att_buf, fields[12], sizeof(att_buf) - 1);
            char *tok = strtok(att_buf, FIELD_SEP);
            while (tok && ev->attendee_count < MAX_ATTENDEES) {
                csv_unescape(tok);
                strncpy(ev->attendees[ev->attendee_count++], tok, MAX_ATTENDEE_LEN - 1);
                tok = strtok(NULL, FIELD_SEP);
            }
        }

        ev->is_cancelled = (bool)atoi(fields[13]);
        ev->created_at   = (time_t)strtol(fields[14], NULL, 10);
        ev->updated_at   = ev->created_at;

        /* Insert without auto-incrementing next_id */
        unsigned int saved_id = ev->id;
        /* Temporarily set next_id so em_add_event doesn't skip saved id */
        unsigned int old_next = em->next_id;
        em->next_id = saved_id;          /* will be set to saved_id + 1 inside */
        em_add_event(em, ev);
        if (em->next_id <= old_next) em->next_id = old_next;
    }

    fclose(fp);
    return EM_OK;
}

/* ════════════════════════════════════════════════════════════
 * PRINT HELPERS
 * ════════════════════════════════════════════════════════════ */

/* ANSI colour codes */
#define C_RESET   "\033[0m"
#define C_BOLD    "\033[1m"
#define C_DIM     "\033[2m"
#define C_RED     "\033[31m"
#define C_GREEN   "\033[32m"
#define C_YELLOW  "\033[33m"
#define C_BLUE    "\033[34m"
#define C_MAGENTA "\033[35m"
#define C_CYAN    "\033[36m"
#define C_WHITE   "\033[37m"

static const char *priority_color(Priority p) {
    switch (p) {
        case PRIORITY_LOW:    return C_GREEN;
        case PRIORITY_MEDIUM: return C_YELLOW;
        case PRIORITY_HIGH:   return C_MAGENTA;
        case PRIORITY_URGENT: return C_RED;
        default:              return C_RESET;
    }
}

void em_print_event(const Event *ev, bool detailed) {
    if (!ev) return;
    char start[32], end[32], created[32];
    em_format_datetime(ev->start_time, start,   sizeof(start));
    em_format_datetime(ev->end_time,   end,     sizeof(end));
    em_format_datetime(ev->created_at, created, sizeof(created));

    printf(C_BOLD "  [#%-4u]" C_RESET " %s%-40s" C_RESET
           " %s%-8s" C_RESET " %s%-12s" C_RESET "\n",
           ev->id,
           ev->is_cancelled ? C_DIM : C_WHITE, ev->title,
           priority_color(ev->priority), em_priority_str(ev->priority),
           C_CYAN, em_category_str(ev->category));

    if (detailed) {
        printf("         " C_DIM "Start   : " C_RESET "%s\n", start);
        printf("         " C_DIM "End     : " C_RESET "%s\n", end);
        printf("         " C_DIM "Location: " C_RESET "%s\n", ev->location[0] ? ev->location : "-");
        printf("         " C_DIM "Organizer: " C_RESET "%s\n", ev->organizer[0] ? ev->organizer : "-");
        if (ev->description[0])
            printf("         " C_DIM "Desc    : " C_RESET "%s\n", ev->description);
        if (ev->is_recurring)
            printf("         " C_DIM "Recurring" C_RESET " every %d day(s)\n", ev->recur_days);
        if (ev->tag_count) {
            printf("         " C_DIM "Tags    : " C_RESET);
            for (int i = 0; i < ev->tag_count; i++)
                printf(C_YELLOW "#%s" C_RESET "%s", ev->tags[i],
                       i < ev->tag_count - 1 ? "  " : "");
            printf("\n");
        }
        if (ev->attendee_count) {
            printf("         " C_DIM "Attendees (%d): " C_RESET, ev->attendee_count);
            for (int i = 0; i < ev->attendee_count; i++)
                printf("%s%s", ev->attendees[i],
                       i < ev->attendee_count - 1 ? ", " : "");
            printf("\n");
        }
        if (ev->is_cancelled) printf("         " C_RED "** CANCELLED **" C_RESET "\n");
        printf("         " C_DIM "Created : " C_RESET "%s\n", created);
        printf("\n");
    }
}

/* ════════════════════════════════════════════════════════════
 * STATISTICS
 * ════════════════════════════════════════════════════════════ */

void em_print_stats(const EventManager *em) {
    if (!em) return;

    size_t total     = em->list.count;
    size_t active    = 0, cancelled = 0, upcoming = 0, past = 0;
    size_t by_cat[CAT_COUNT]  = {0};
    size_t by_pri[5]          = {0};
    time_t now = time(NULL);

    for (Event *ev = em->list.head; ev; ev = ev->next) {
        if (ev->is_cancelled) cancelled++;
        else {
            active++;
            if (ev->start_time >= now) upcoming++;
            else                       past++;
        }
        if (ev->category >= 0 && ev->category < CAT_COUNT)
            by_cat[ev->category]++;
        if (ev->priority >= 1 && ev->priority <= 4)
            by_pri[ev->priority]++;
    }

    printf("\n" C_BOLD C_CYAN "  ── Statistics ──────────────────────────────────" C_RESET "\n");
    printf("  Total events   : " C_BOLD "%zu" C_RESET "\n", total);
    printf("  Active         : " C_GREEN "%zu" C_RESET
           "   Cancelled : " C_RED "%zu" C_RESET "\n", active, cancelled);
    printf("  Upcoming       : " C_YELLOW "%zu" C_RESET
           "   Past      : " C_DIM "%zu" C_RESET "\n", upcoming, past);

    printf("\n  " C_BOLD "By Category:" C_RESET "\n");
    for (int i = 0; i < CAT_COUNT; i++) {
        printf("    %-12s : %zu\n", em_category_str((Category)i), by_cat[i]);
    }

    printf("\n  " C_BOLD "By Priority:" C_RESET "\n");
    for (int p = 1; p <= 4; p++) {
        printf("    %s%-8s" C_RESET " : %zu\n",
               priority_color((Priority)p),
               em_priority_str((Priority)p), by_pri[p]);
    }
    printf("\n");
}
