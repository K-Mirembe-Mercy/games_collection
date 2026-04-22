/*
 * umg_levels.h — Header for the Level Expansion Module
 * Include this in ubuntu_mindgarden_v3.c to plug in 100 levels
 *
 * COMPILE:
 *   gcc ubuntu_mindgarden_v3.c umg_extensions.c umg_levels.c -o umg_full -lm
 * RUN:
 *   ./umg_full
 */

#ifndef UMG_LEVELS_H
#define UMG_LEVELS_H

#define TOTAL_LEVELS    100
#define PRESTIGE_LEVELS   5
#define MAX_LVL_BADGES   50
#define BOSS_COUNT        6
#define SAVE_LVL_FILE "umg_levels.dat"

typedef struct {
    int  count;
    int  best_combo;
    long last_action;
    float multiplier;
} ComboState;

typedef struct {
    char name[64];
    char desc[256];
    int  level_trigger;
    int  defeated;
    int  xp_reward;
    int  coin_reward;
    int  attempts;
} Boss;

typedef struct {
    int        total_level;
    int        total_xp;
    int        prestige;
    int        lvl_badges[MAX_LVL_BADGES];
    Boss       bosses[BOSS_COUNT];
    ComboState combo;
    int        loot_opened;
    int        loot_available;
    long       last_login_day;
    int        login_streak;
    int        login_day_in_cycle;
    int        total_sessions_ever;
    int        speed_runs;
    int        perfect_streak;
    long       first_play_day;
    int        cutscene_seen[101];
} LevelState;

/* Public API */
void lvl_load(LevelState *s);
void lvl_save(LevelState *s);
void lvl_daily_login(LevelState *s, void *player_ptr);
void lvl_menu(LevelState *s, void *player_ptr);
int  lvl_check_levelup(LevelState *s, int xp, int premium);

#endif /* UMG_LEVELS_H */

/*
 * ══ HOW TO ADD TO ubuntu_mindgarden_v3.c (4 steps) ══════════════
 *
 * STEP 1 — Add at the top of v3.c (after other includes):
 *   #include "umg_levels.h"
 *
 * STEP 2 — In main(), after load_game(&player);  add:
 *   LevelState lstate;
 *   lvl_load(&lstate);
 *   lvl_daily_login(&lstate, &player);
 *
 * STEP 3 — In main_menu() add option 20:
 *   printf(GOLD "  ║ " AMBER "20." IVORY " ★ Level Hub (100 Levels) " GOLD "║\n");
 *   Change: return get_int(1, 20);
 *
 * STEP 4 — In main() switch add:
 *   case 20: lvl_menu(&lstate, &player); break;
 *   Also add after save_game(&player):
 *   lvl_save(&lstate);
 *
 * COMPILE ALL 3 FILES:
 *   gcc ubuntu_mindgarden_v3.c umg_extensions.c umg_levels.c -o umg_full -lm
 *
 * RUN:  ./umg_full
 * ═════════════════════════════════════════════════════════════════
 */
