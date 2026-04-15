/* ============================================================
 * main.c  –  Entry point
 * ============================================================ */

#include "event_manager.h"

int main(int argc, char *argv[]) {
    const char *db = (argc > 1) ? argv[1] : DEFAULT_DB_FILE;

    EventManager *em = em_create(db);
    if (!em) {
        fprintf(stderr, "Fatal: could not initialise event manager.\n");
        return EXIT_FAILURE;
    }

    em_run_ui(em);
    em_destroy(em);
    return EXIT_SUCCESS;
}
