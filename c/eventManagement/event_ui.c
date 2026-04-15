/* ============================================================
 * event_ui.c  –  Interactive terminal user interface
 * ============================================================ */

#include "event_manager.h"

/* ── ANSI / display macros ─────────────────────────────────── */
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

/* ── Safe input helpers ─────────────────────────────────────── */

__attribute__((unused))
static void flush_stdin(void) {
    int c;
    while ((c = getchar()) != '\n' && c != EOF);
}

/* Read a non-empty string; returns false on empty optional fields */
static bool read_string(const char *prompt, char *buf, size_t n, bool required) {
    printf("  %s", prompt);
    if (!fgets(buf, (int)n, stdin)) { buf[0] = '\0'; return false; }
    buf[strcspn(buf, "\r\n")] = '\0';
    if (required && !buf[0]) {
        printf(C_RED "  [Required field cannot be empty]\n" C_RESET);
        return false;
    }
    return buf[0] != '\0';
}

static int read_int(const char *prompt, int min, int max) {
    char buf[32];
    int val;
    while (1) {
        printf("  %s", prompt);
        if (!fgets(buf, sizeof(buf), stdin)) continue;
        if (sscanf(buf, "%d", &val) == 1 && val >= min && val <= max)
            return val;
        printf(C_RED "  Please enter a number between %d and %d.\n" C_RESET, min, max);
    }
}

static bool read_datetime(const char *prompt, time_t *out) {
    char buf[32];
    while (1) {
        printf("  %s (YYYY-MM-DD HH:MM): ", prompt);
        if (!fgets(buf, sizeof(buf), stdin)) continue;
        buf[strcspn(buf, "\r\n")] = '\0';
        if (!buf[0]) { *out = 0; return false; }
        if (em_parse_datetime(buf, out)) return true;
        printf(C_RED "  Invalid format. Use YYYY-MM-DD HH:MM\n" C_RESET);
    }
}

static bool read_bool(const char *prompt) {
    char buf[8];
    printf("  %s (y/n): ", prompt);
    if (!fgets(buf, sizeof(buf), stdin)) return false;
    return tolower((unsigned char)buf[0]) == 'y';
}

/* ── Banner ─────────────────────────────────────────────────── */

static void print_banner(void) {
    printf("\n" C_BOLD C_CYAN
    "  ╔══════════════════════════════════════════════════╗\n"
    "  ║         E V E N T   M A N A G E R  v%-10s ║\n"
    "  ╚══════════════════════════════════════════════════╝\n"
    C_RESET, APP_VERSION);
}

static void print_separator(void) {
    printf(C_DIM "  ──────────────────────────────────────────────────\n" C_RESET);
}

/* ── Main menu ──────────────────────────────────────────────── */

static void print_menu(void) {
    printf("\n" C_BOLD "  MAIN MENU\n" C_RESET);
    print_separator();
    printf("  " C_GREEN "1" C_RESET " Add event\n");
    printf("  " C_GREEN "2" C_RESET " List all events\n");
    printf("  " C_GREEN "3" C_RESET " View event details\n");
    printf("  " C_GREEN "4" C_RESET " Edit event\n");
    printf("  " C_GREEN "5" C_RESET " Cancel / delete event\n");
    printf("  " C_GREEN "6" C_RESET " Search by title\n");
    printf("  " C_GREEN "7" C_RESET " Filter events\n");
    printf("  " C_GREEN "8" C_RESET " Upcoming events\n");
    printf("  " C_GREEN "9" C_RESET " Next event\n");
    printf("  " C_GREEN "S" C_RESET " Statistics\n");
    printf("  " C_GREEN "W" C_RESET " Save to disk\n");
    printf("  " C_RED   "Q" C_RESET " Quit\n");
    print_separator();
    printf("  Choice: ");
}

/* ── Event form ─────────────────────────────────────────────── */

static bool fill_event_form(Event *ev, bool is_edit) {
    printf("\n" C_BOLD "  %s\n" C_RESET, is_edit ? "── EDIT EVENT ──" : "── NEW EVENT ──");
    print_separator();

    /* Title */
    if (!read_string("Title* : ", ev->title, MAX_TITLE_LEN, true)) return false;

    /* Description */
    read_string("Description (optional): ", ev->description, MAX_DESC_LEN, false);

    /* Location */
    read_string("Location : ", ev->location, MAX_LOCATION_LEN, false);

    /* Organizer */
    read_string("Organizer: ", ev->organizer, MAX_ORGANIZER_LEN, false);

    /* Start time */
    if (!read_datetime("Start time*", &ev->start_time)) return false;

    /* End time */
    printf("  End time  ");
    time_t end_t;
    if (read_datetime("", &end_t) && end_t > ev->start_time)
        ev->end_time = end_t;
    else
        ev->end_time = ev->start_time + 3600;  /* default: 1 hour */

    /* Priority */
    printf("\n  Priority:\n");
    printf("    1) Low   2) Medium   3) High   4) Urgent\n");
    ev->priority = (Priority)read_int("Choice [1-4]: ", 1, 4);

    /* Category */
    printf("\n  Category:\n");
    for (int i = 0; i < CAT_COUNT; i++)
        printf("    %d) %s\n", i + 1, em_category_str((Category)i));
    ev->category = (Category)(read_int("Choice [1-6]: ", 1, CAT_COUNT) - 1);

    /* Recurring */
    ev->is_recurring = read_bool("Recurring event?");
    if (ev->is_recurring)
        ev->recur_days = read_int("Repeat every N days: ", 1, 365);

    /* Tags */
    printf("\n  Tags (press Enter to skip each slot; max %d):\n", MAX_TAGS);
    ev->tag_count = 0;
    for (int i = 0; i < MAX_TAGS; i++) {
        char tmp[MAX_TAG_LEN] = {0};
        printf("  Tag %d: ", i + 1);
        if (!fgets(tmp, sizeof(tmp), stdin)) break;
        tmp[strcspn(tmp, "\r\n")] = '\0';
        if (!tmp[0]) break;
        snprintf(ev->tags[ev->tag_count++], MAX_TAG_LEN, "%s", tmp);
    }

    /* Attendees */
    printf("\n  Attendees (press Enter to skip; max %d):\n", MAX_ATTENDEES);
    ev->attendee_count = 0;
    for (int i = 0; i < MAX_ATTENDEES; i++) {
        char tmp[MAX_ATTENDEE_LEN] = {0};
        printf("  Attendee %d: ", i + 1);
        if (!fgets(tmp, sizeof(tmp), stdin)) break;
        tmp[strcspn(tmp, "\r\n")] = '\0';
        if (!tmp[0]) break;
        snprintf(ev->attendees[ev->attendee_count++], MAX_ATTENDEE_LEN, "%s", tmp);
    }

    return true;
}

/* ── Action handlers ────────────────────────────────────────── */

static void ui_add_event(EventManager *em) {
    Event *ev = em_event_alloc();
    if (!ev) { printf(C_RED "  Memory error.\n" C_RESET); return; }

    if (!fill_event_form(ev, false)) {
        em_event_free(ev);
        printf(C_YELLOW "  Event creation cancelled.\n" C_RESET);
        return;
    }

    EMStatus s = em_add_event(em, ev);
    if (s == EM_OK)
        printf(C_GREEN "  ✓ Event #%u added.\n" C_RESET, ev->id);
    else {
        printf(C_RED "  Error: %s\n" C_RESET, em_status_str(s));
        em_event_free(ev);
    }
}

static void ui_list_events(const EventManager *em) {
    printf("\n" C_BOLD "  ALL EVENTS (%zu)\n" C_RESET, em->list.count);
    print_separator();
    if (!em->list.count) { printf("  (none)\n"); return; }
    printf("  %-6s %-40s %-9s %-13s  Start\n", "ID", "Title", "Priority", "Category");
    print_separator();
    for (Event *ev = em->list.head; ev; ev = ev->next)
        em_print_event(ev, false);
}

static void ui_view_event(const EventManager *em) {
    int id = read_int("Event ID: ", 1, 999999);
    Event *ev = em_find_by_id(em, (unsigned int)id);
    if (!ev) { printf(C_RED "  Event #%d not found.\n" C_RESET, id); return; }
    printf("\n");
    em_print_event(ev, true);
}

static void ui_edit_event(EventManager *em) {
    int id = read_int("Event ID to edit: ", 1, 999999);
    Event *ev = em_find_by_id(em, (unsigned int)id);
    if (!ev) { printf(C_RED "  Event #%d not found.\n" C_RESET, id); return; }

    /* Clone, fill form, then update */
    Event updated;
    memcpy(&updated, ev, sizeof(Event));

    if (!fill_event_form(&updated, true)) {
        printf(C_YELLOW "  Edit cancelled.\n" C_RESET);
        return;
    }

    EMStatus s = em_update_event(em, (unsigned int)id, &updated);
    if (s == EM_OK) printf(C_GREEN "  ✓ Event #%u updated.\n" C_RESET, (unsigned int)id);
    else            printf(C_RED "  Error: %s\n" C_RESET, em_status_str(s));
}

static void ui_cancel_event(EventManager *em) {
    printf("\n  1) Cancel event (mark as cancelled)\n");
    printf("  2) Delete event permanently\n");
    int choice = read_int("Choice: ", 1, 2);
    int id     = read_int("Event ID: ", 1, 999999);

    if (choice == 1) {
        Event *ev = em_find_by_id(em, (unsigned int)id);
        if (!ev) { printf(C_RED "  Not found.\n" C_RESET); return; }
        ev->is_cancelled = true;
        ev->updated_at   = time(NULL);
        printf(C_YELLOW "  Event #%u marked as cancelled.\n" C_RESET, (unsigned int)id);
    } else {
        if (!read_bool("Permanently delete this event?")) { printf("  Aborted.\n"); return; }
        EMStatus s = em_remove_event(em, (unsigned int)id);
        if (s == EM_OK) printf(C_GREEN "  ✓ Event #%u deleted.\n" C_RESET, (unsigned int)id);
        else            printf(C_RED "  Error: %s\n" C_RESET, em_status_str(s));
    }
}

static void ui_search(const EventManager *em) {
    char kw[MAX_TITLE_LEN];
    if (!read_string("Search keyword: ", kw, sizeof(kw), true)) return;
    EventList *results = em_search_by_title(em, kw);
    if (!results) return;
    printf("\n  Results (%zu):\n", results->count);
    print_separator();
    for (Event *ev = results->head; ev; ev = ev->next)
        em_print_event(ev, false);
    em_free_result_list(results);
}

static void ui_filter(const EventManager *em) {
    printf("\n  Filter by:\n");
    printf("  1) Category   2) Priority\n");
    int choice = read_int("Choice: ", 1, 2);
    EventList *results = NULL;

    if (choice == 1) {
        printf("  Category:\n");
        for (int i = 0; i < CAT_COUNT; i++)
            printf("    %d) %s\n", i + 1, em_category_str((Category)i));
        int cat = read_int("Choice [1-6]: ", 1, CAT_COUNT) - 1;
        results = em_filter_by_category(em, (Category)cat);
    } else {
        printf("  Priority:\n");
        printf("    1) Low  2) Medium  3) High  4) Urgent\n");
        int pri = read_int("Choice [1-4]: ", 1, 4);
        results = em_filter_by_priority(em, (Priority)pri);
    }

    if (!results) return;
    printf("\n  Matching events (%zu):\n", results->count);
    print_separator();
    for (Event *ev = results->head; ev; ev = ev->next)
        em_print_event(ev, false);
    em_free_result_list(results);
}

static void ui_upcoming(const EventManager *em) {
    int days = read_int("Show events in next N days [1-365]: ", 1, 365);
    EventList *results = em_upcoming_events(em, days);
    if (!results) return;
    printf("\n  Upcoming in next %d day(s): %zu event(s)\n", days, results->count);
    print_separator();
    for (Event *ev = results->head; ev; ev = ev->next)
        em_print_event(ev, false);
    em_free_result_list(results);
}

static void ui_next_event(const EventManager *em) {
    Event *ev = em_next_event(em);
    if (!ev) { printf(C_YELLOW "  No upcoming events.\n" C_RESET); return; }
    printf("\n" C_BOLD "  Next event:\n" C_RESET);
    em_print_event(ev, true);
}

/* ════════════════════════════════════════════════════════════
 * MAIN UI LOOP
 * ════════════════════════════════════════════════════════════ */

void em_run_ui(EventManager *em) {
    print_banner();

    Event *next = em_next_event(em);
    if (next) {
        char buf[32];
        em_format_datetime(next->start_time, buf, sizeof(buf));
        printf("  " C_YELLOW "Next upcoming: " C_BOLD "%s" C_RESET
               " " C_DIM "(%s)" C_RESET "\n", next->title, buf);
    }

    char buf[8];
    while (1) {
        print_menu();
        if (!fgets(buf, sizeof(buf), stdin)) break;
        char ch = toupper((unsigned char)buf[0]);

        switch (ch) {
            case '1': ui_add_event(em);             break;
            case '2': ui_list_events(em);           break;
            case '3': ui_view_event(em);            break;
            case '4': ui_edit_event(em);            break;
            case '5': ui_cancel_event(em);          break;
            case '6': ui_search(em);                break;
            case '7': ui_filter(em);                break;
            case '8': ui_upcoming(em);              break;
            case '9': ui_next_event(em);            break;
            case 'S': em_print_stats(em);           break;
            case 'W':
                if (em_save(em) == EM_OK)
                    printf(C_GREEN "  ✓ Saved to %s\n" C_RESET, em->db_path);
                else
                    printf(C_RED "  Save failed.\n" C_RESET);
                break;
            case 'Q':
                printf("  Saving...\n");
                if (em_save(em) == EM_OK)
                    printf(C_GREEN "  ✓ Saved. Goodbye!\n\n" C_RESET);
                return;
            default:
                printf(C_YELLOW "  Unknown option.\n" C_RESET);
        }
    }
}
