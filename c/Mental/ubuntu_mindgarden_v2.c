/*
 * ╔══════════════════════════════════════════════════════════════════╗
 * ║         UBUNTU MIND GARDEN — FULL EDITION v2.0                  ║
 * ║         "Umuntu ngumuntu ngabantu" — I am because we are        ║
 * ║                                                                  ║
 * ║  A levelled mental wellness RPG rooted in Ugandan & African      ║
 * ║  culture. Breathe, reflect, grow — and unlock your forest.      ║
 * ║                                                                  ║
 * ║  Compile:  gcc ubuntu_mindgarden_v2.c -o umg -lm                ║
 * ║  Run:      ./umg                                                 ║
 * ╚══════════════════════════════════════════════════════════════════╝
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <math.h>
#include <unistd.h>

/* ═══════════════════════════ ANSI COLORS ═══════════════════════════ */
#define R         "\033[0m"
#define BOLD      "\033[1m"
#define DIM       "\033[2m"
#define ITALIC    "\033[3m"
#define BLINK     "\033[5m"

#define GOLD      "\033[38;5;220m"
#define ORANGE    "\033[38;5;208m"
#define CRIMSON   "\033[38;5;160m"
#define RED       "\033[38;5;196m"
#define EARTH     "\033[38;5;130m"
#define SAVANNA   "\033[38;5;178m"
#define FOREST    "\033[38;5;34m"
#define JUNGLE    "\033[38;5;28m"
#define LAKE      "\033[38;5;33m"
#define SKY       "\033[38;5;117m"
#define IVORY     "\033[38;5;230m"
#define PURPLE    "\033[38;5;135m"
#define PINK      "\033[38;5;211m"
#define MIST      "\033[38;5;189m"
#define SILVER    "\033[38;5;250m"
#define CYAN      "\033[38;5;51m"
#define LIME      "\033[38;5;118m"

/* ═══════════════════════════ CONSTANTS ═════════════════════════════ */
#define MAX_NAME         64
#define MAX_JOURNAL      512
#define JOURNAL_ENTRIES  10
#define MAX_LEVELS       10
#define MAX_QUESTS        5
#define MAX_BADGES       20
#define SAVE_FILE        "umg_save.dat"

/* ════════════════════════ LEVEL DEFINITIONS ════════════════════════ */
/*
  Level 1  — Seedling       (FREE)   XP:0
  Level 2  — Sprout         (FREE)   XP:100
  Level 3  — Sapling        (FREE)   XP:250
  Level 4  — Young Tree     (FREE)   XP:500
  Level 5  — Rooted Tree    (FREE)   XP:900       ← Free cap
  ─────────────────────────────────────────────────
  Level 6  — Elder Tree     (PREMIUM) XP:1400     ← Requires upgrade
  Level 7  — Grove Guardian (PREMIUM) XP:2000
  Level 8  — Forest Spirit  (PREMIUM) XP:3000
  Level 9  — Ubuntu Elder   (PREMIUM) XP:4500
  Level 10 — Ancestor Light (PREMIUM) XP:7000
*/

static const char *level_names[MAX_LEVELS + 1] = {
    "", "Seedling", "Sprout", "Sapling", "Young Tree", "Rooted Tree",
    "Elder Tree", "Grove Guardian", "Forest Spirit", "Ubuntu Elder", "Ancestor Light"
};
static const char *level_icons[MAX_LEVELS + 1] = {
    "", "🌱", "🌿", "🌲", "🌳", "🏔",
    "🌴", "🦁", "🦩", "👑", "✨"
};
static const int level_xp[MAX_LEVELS + 1] = {
    0, 0, 100, 250, 500, 900, 1400, 2000, 3000, 4500, 7000
};
static const int level_premium[MAX_LEVELS + 1] = {
    0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1
};

/* ═════════════════════════ BADGE SYSTEM ════════════════════════════ */
#define BADGE_FIRST_BREATH    0
#define BADGE_WEEK_STREAK     1
#define BADGE_JOURNAL_5       2
#define BADGE_GROUNDED        3
#define BADGE_AFFIRMATION_10  4
#define BADGE_GORILLA         5   /* Premium */
#define BADGE_NILE_SWIMMER    6   /* Premium */
#define BADGE_ANCESTOR        7   /* Premium */
#define BADGE_FULL_FOREST     8   /* Premium */
#define BADGE_UBUNTU_MASTER   9   /* Premium */

static const char *badge_names[] = {
    "First Breath",    "7-Day Streak",   "Griot (5 Journals)",
    "Grounded",        "Affirmation x10","Gorilla Spirit",
    "Nile Swimmer",    "Ancestor Voice", "Full Forest",
    "Ubuntu Master"
};
static const char *badge_icons[] = {
    "🌬","🔥","📖","🌿","🦩","🦍","🌊","👻","🌳","👑"
};

/* ═══════════════════════ QUEST SYSTEM ══════════════════════════════ */
typedef struct {
    char  title[64];
    char  desc[128];
    int   target;
    int   progress;
    int   xp_reward;
    int   complete;
    int   premium;
} Quest;

/* ═════════════════════ PLAYER DATA ═════════════════════════════════ */
typedef struct {
    char  name[MAX_NAME];
    int   level;
    int   xp;
    int   coins;        /* Ubuntu Coins — in-game currency  */
    int   mood;
    int   anxiety;
    int   energy;
    int   streak;       /* consecutive day streak           */
    int   sessions;
    int   breaths;      /* breathing sessions done          */
    int   grounds;      /* grounding sessions done          */
    int   affirmations; /* affirmations done                */
    int   journals;     /* journal entries written          */
    int   premium;      /* 0=free 1=premium                 */
    int   badges[MAX_BADGES];
    char  journal[JOURNAL_ENTRIES][MAX_JOURNAL];
    int   journal_count;
    Quest quests[MAX_QUESTS];
    int   quest_count;
    long  last_day;     /* epoch day for streak tracking    */
} Player;

/* ═══════════════════════ PROVERBS & CONTENT ════════════════════════ */
typedef struct { const char *text, *author, *origin; } Proverb;

static const char *affirmations_free[] = {
    "I am rooted like the mighty fig tree of Bwindi.",
    "Like Lake Victoria, my depth holds endless life.",
    "I carry the strength of the Rwenzori mountains.",
    "My spirit is as vast as the African savanna.",
    "I bloom like the Uganda flame tree after the dry season.",
    "Like the crested crane, I stand tall and graceful.",
    "I am part of a community that lifts me — Ubuntu.",
    "The sun rises on the Nile for me every morning.",
    "My healing is as natural as rain on Ugandan hills.",
    "I am worthy of peace, like the stillness of Lake Bunyonyi.",
};
#define AFF_FREE_COUNT 10

static const char *affirmations_premium[] = {
    "I rise again, like Africa — ancient, resilient, magnificent.",
    "My ancestors walked through storms; I carry their courage.",
    "Joy lives in me, like the drumbeat of a Kampala celebration.",
    "I am more powerful than my fear — the lion inside me roars.",
    "Every breath I take is a gift from the Ugandan hills.",
    "I welcome calm the way the savanna welcomes the dawn.",
    "My mind is clearing like morning mist over the Nile.",
    "I am loved, I am enough, I am here — always enough.",
    "Growth takes time — the baobab did not become mighty overnight.",
    "I choose peace. I choose myself. I choose today.",
};
#define AFF_PREMIUM_COUNT 10

static const Proverb proverbs[] = {
    {"Umuntu ngumuntu ngabantu.", "Nguni peoples", "Uganda/S. Africa"},
    {"Rain does not fall on one roof alone.", "Zulu", "South Africa"},
    {"However long the night, the dawn will break.", "Pan-African", "Africa"},
    {"Sticks in a bundle are unbreakable.", "Bondei", "Tanzania"},
    {"He who learns, teaches.", "Ethiopian", "Ethiopia"},
    {"The forest would be silent if no bird sang except the best.", "West African", "W. Africa"},
    {"When the music changes, so does the dance.", "Hausa", "Nigeria"},
    {"Do not look where you fell, but where you slipped.", "Pan-African", "Africa"},
    {"A child not embraced by the village will burn it down to feel its warmth.", "African", "E. Africa"},
    {"Until the lion learns to write, every story glorifies the hunter.", "Pan-African", "Africa"},
};
#define PROVERB_COUNT 10

/* ═══════════════════════ UTILITY ═══════════════════════════════════ */
void cls(void) { printf("\033[2J\033[H"); fflush(stdout); }
void sleep_ms(int ms) { usleep(ms * 1000); }

void divider(const char *col, char ch, int w) {
    printf("%s", col);
    for (int i = 0; i < w; i++) putchar(ch);
    printf(R "\n");
}

void typewrite(const char *col, const char *txt, int ms) {
    printf("%s", col);
    while (*txt) { putchar(*txt++); fflush(stdout); sleep_ms(ms); }
    printf(R);
}

void press_enter(void) {
    printf(DIM "\n  [ Press ENTER to continue ]" R "\n");
    while (getchar() != '\n');
}

int get_int(int lo, int hi) {
    int v; char buf[32];
    while (1) {
        if (fgets(buf, sizeof(buf), stdin) && sscanf(buf, "%d", &v) == 1
                && v >= lo && v <= hi) return v;
        printf(ORANGE "  Enter %d-%d: " R, lo, hi);
    }
}

long epoch_day(void) { return (long)(time(NULL) / 86400); }

/* ═══════════════════════ XP & LEVELS ══════════════════════════════ */
void award_xp(Player *p, int xp, const char *reason) {
    p->xp += xp;
    p->coins += xp / 5;
    printf(GOLD "  +" BOLD "%d XP" R GOLD " — %s  |  💰 +%d Ubuntu Coins\n" R,
           xp, reason, xp / 5);

    /* Level up check */
    while (p->level < MAX_LEVELS && p->xp >= level_xp[p->level + 1]) {
        if (level_premium[p->level + 1] && !p->premium) break;
        p->level++;
        printf(GOLD BOLD "\n  ✨ LEVEL UP! You are now Level %d — %s %s!\n" R,
               p->level, level_icons[p->level], level_names[p->level]);
        sleep(1);
    }
}

void check_badge(Player *p, int badge_id) {
    if (p->badges[badge_id]) return;
    if (badge_id >= 5 && !p->premium) return; /* premium badges */
    p->badges[badge_id] = 1;
    printf(GOLD BLINK "  🏅 BADGE UNLOCKED: " R GOLD BOLD "%s %s\n" R,
           badge_icons[badge_id], badge_names[badge_id]);
    award_xp(p, 50, "Badge Earned");
    sleep(1);
}

/* ═══════════════════════ SAVE / LOAD ═══════════════════════════════ */
void save_player(Player *p) {
    FILE *f = fopen(SAVE_FILE, "wb");
    if (f) { fwrite(p, sizeof(Player), 1, f); fclose(f); }
}

int load_player(Player *p) {
    FILE *f = fopen(SAVE_FILE, "rb");
    if (!f) return 0;
    int ok = (fread(p, sizeof(Player), 1, f) == 1);
    fclose(f);
    return ok;
}

/* ═══════════════════════ STREAK CHECK ══════════════════════════════ */
void update_streak(Player *p) {
    long today = epoch_day();
    if (p->last_day == 0) { p->streak = 1; p->last_day = today; return; }
    long diff = today - p->last_day;
    if (diff == 1) { p->streak++; p->last_day = today; }
    else if (diff > 1) { p->streak = 1; p->last_day = today; }
    /* diff==0: same day, no change */

    if (p->streak >= 7) check_badge(p, BADGE_WEEK_STREAK);
    if (p->streak >= 7)
        printf(CRIMSON BOLD "  🔥 %d-Day Streak! Keep going!\n" R, p->streak);
}

/* ═══════════════════════ QUEST INIT ════════════════════════════════ */
void init_quests(Player *p) {
    p->quest_count = 5;

    strcpy(p->quests[0].title,  "First Breath");
    strcpy(p->quests[0].desc,   "Complete 3 breathing sessions");
    p->quests[0].target = 3; p->quests[0].xp_reward = 80; p->quests[0].premium = 0;

    strcpy(p->quests[1].title,  "Griot of the Village");
    strcpy(p->quests[1].desc,   "Write 5 journal entries");
    p->quests[1].target = 5; p->quests[1].xp_reward = 120; p->quests[1].premium = 0;

    strcpy(p->quests[2].title,  "Walk with the Ancestors");
    strcpy(p->quests[2].desc,   "Complete 10 affirmations");
    p->quests[2].target = 10; p->quests[2].xp_reward = 150; p->quests[2].premium = 0;

    strcpy(p->quests[3].title,  "Bwindi Wanderer");
    strcpy(p->quests[3].desc,   "[PREMIUM] Ground yourself 5 times");
    p->quests[3].target = 5; p->quests[3].xp_reward = 200; p->quests[3].premium = 1;

    strcpy(p->quests[4].title,  "Ubuntu Champion");
    strcpy(p->quests[4].desc,   "[PREMIUM] Reach a 14-day streak");
    p->quests[4].target = 14; p->quests[4].xp_reward = 500; p->quests[4].premium = 1;
}

void check_quests(Player *p) {
    /* Update quest progress */
    p->quests[0].progress = p->breaths;
    p->quests[1].progress = p->journals;
    p->quests[2].progress = p->affirmations;
    p->quests[3].progress = p->grounds;
    p->quests[4].progress = p->streak;

    for (int i = 0; i < p->quest_count; i++) {
        if (!p->quests[i].complete && p->quests[i].progress >= p->quests[i].target) {
            if (p->quests[i].premium && !p->premium) continue;
            p->quests[i].complete = 1;
            printf(LIME BOLD "\n  ✅ QUEST COMPLETE: %s\n" R, p->quests[i].title);
            award_xp(p, p->quests[i].xp_reward, "Quest Reward");
        }
    }
}

/* ═══════════════════════ HEADER / BANNER ═══════════════════════════ */
void draw_logo(void) {
    printf(GOLD   "  ╔══════════════════════════════════════════════════════╗\n");
    printf(GOLD   "  ║" ORANGE BOLD "  ██╗   ██╗██████╗ ██╗   ██╗███╗  ██╗████████╗██╗  " GOLD "  ║\n");
    printf(GOLD   "  ║" ORANGE BOLD "  ██║   ██║██╔══██╗██║   ██║████╗ ██║╚══██╔══╝██║  " GOLD "  ║\n");
    printf(GOLD   "  ║" ORANGE BOLD "  ██║   ██║██████╔╝██║   ██║██╔██╗██║   ██║   ██║  " GOLD "  ║\n");
    printf(GOLD   "  ║" ORANGE BOLD "  ╚██████╔╝██╔══██╗╚██████╔╝██║╚████║   ██║   ██║  " GOLD "  ║\n");
    printf(GOLD   "  ║" ORANGE BOLD "   ╚═════╝ ╚═════╝  ╚═════╝ ╚═╝ ╚═══╝   ╚═╝   ╚═╝ " GOLD "  ║\n");
    printf(EARTH  "  ║      " SAVANNA BOLD "M I N D   G A R D E N  v2.0" EARTH "               ║\n");
    printf(FOREST "  ║   " IVORY "Uganda & Africa — Levelled Mental Wellness RPG" FOREST "  ║\n");
    printf(GOLD   "  ╚══════════════════════════════════════════════════════╝\n" R);
}

/* ═══════════════════════ HUD BAR ════════════════════════════════════ */
void draw_hud(Player *p) {
    int xp_needed = (p->level < MAX_LEVELS) ? level_xp[p->level + 1] : p->xp;
    int xp_this   = p->xp - level_xp[p->level];
    int xp_range  = xp_needed - level_xp[p->level];
    int filled    = (xp_range > 0) ? (xp_this * 20 / xp_range) : 20;

    printf(GOLD "  ╔════ " R BOLD "%s %s %s" R GOLD " ════ Lv.%d ════════════════╗\n" R,
           level_icons[p->level], p->name, level_icons[p->level], p->level);

    printf(GOLD "  ║ " SAVANNA "XP [" R);
    for (int i = 0; i < 20; i++)
        printf(i < filled ? LIME "█" R : DIM "░" R);
    printf(SAVANNA "] %d/%d" R "\n", p->xp, xp_needed);

    printf(GOLD "  ║ " GOLD "💰 %d Coins  " CRIMSON "🔥 %d Streak  " CYAN "🎯 Lv%d/%d  " R,
           p->coins, p->streak, p->level, MAX_LEVELS);
    printf(p->premium ? GOLD BOLD "[★ PREMIUM]" R "\n" : DIM "[FREE]\n" R);

    /* Mood bars */
    printf(GOLD "  ║ " ORANGE "Mood  [" R);
    for (int i = 0; i < 10; i++) printf(i < p->mood ? ORANGE "▓" R : DIM "░" R);
    printf(ORANGE "] %d  " LAKE "Calm [" R);
    for (int i = 0; i < 10; i++) printf(i < (10 - p->anxiety) ? LAKE "▓" R : DIM "░" R);
    printf(LAKE "] %d\n" R, 10 - p->anxiety);

    printf(GOLD "  ╚══════════════════════════════════════════════════╝\n" R);
}

/* ═══════════════════════ LEVEL MAP ══════════════════════════════════ */
void show_level_map(Player *p) {
    cls();
    printf("\n");
    divider(GOLD, '═', 56);
    printf(GOLD "  🗺  " SAVANNA BOLD "THE PATH OF UBUNTU — Level Map" R "\n");
    divider(GOLD, '═', 56);
    printf("\n");

    for (int i = 1; i <= MAX_LEVELS; i++) {
        int unlocked   = (p->level >= i);
        int is_current = (p->level == i);
        int is_premium = level_premium[i];
        const char *col = is_current ? GOLD :
                          (unlocked ? FOREST : (is_premium ? PURPLE : DIM));

        printf("  %s", col);
        if (is_current) printf(BOLD "▶ ");
        else printf("  ");

        printf("Lv.%2d %s %-18s", i, level_icons[i], level_names[i]);

        if (is_current)       printf(GOLD BOLD " ← YOU ARE HERE" R);
        else if (!unlocked && !is_premium) printf(DIM " [%d XP]" R, level_xp[i]);
        else if (!unlocked && is_premium)  printf(PURPLE " [★ PREMIUM — %d XP]" R, level_xp[i]);
        else                               printf(LIME " ✓ Complete" R);

        printf("\n");
        if (i == 5 && p->level <= 5) {
            printf(PURPLE "  ─────── ★ PREMIUM ZONE BELOW ───────\n" R);
        }
    }
    printf("\n");
    divider(EARTH, '─', 56);
    printf(IVORY "  Each level unlocks new activities, affirmations,\n");
    printf("  mini-games, and deeper healing tools.\n" R);
    press_enter();
}

/* ═══════════════════════ BREATHING ══════════════════════════════════ */
void breathing_exercise(Player *p) {
    cls();
    printf("\n");
    divider(LAKE, '═', 56);
    printf(LAKE "  🌊  " SKY BOLD "BREATHING — Lake Victoria Tide" R "\n");
    divider(LAKE, '═', 56);

    int cycles = (p->level >= 3) ? 6 : 4; /* more cycles at higher level */
    printf(IVORY "\n  Inhale 4s → Hold 7s → Exhale 8s\n");
    printf(DIM "  Cycles: %d (increases with level)\n\n" R, cycles);
    press_enter();

    for (int c = 1; c <= cycles; c++) {
        for (int s = 4; s >= 1; s--) {
            printf("\r  " FOREST "🌬  INHALE — %ds...            " R, s);
            fflush(stdout); sleep(1);
        }
        for (int s = 7; s >= 1; s--) {
            printf("\r  " GOLD "✨  HOLD   — %ds...            " R, s);
            fflush(stdout); sleep(1);
        }
        for (int s = 8; s >= 1; s--) {
            printf("\r  " LAKE "🌊  EXHALE — %ds...            " R, s);
            fflush(stdout); sleep(1);
        }
        printf("\n  " SAVANNA "Cycle %d/%d ✓\n" R, c, cycles);
    }

    p->anxiety = (p->anxiety > 2) ? p->anxiety - 2 : 1;
    p->mood    = (p->mood < 10)   ? p->mood + 1    : 10;
    p->breaths++;

    award_xp(p, 30, "Breathing Session");
    if (p->breaths == 1) check_badge(p, BADGE_FIRST_BREATH);
    check_quests(p);
    press_enter();
}

/* ═══════════════════════ MOOD CHECK-IN ══════════════════════════════ */
void mood_checkin(Player *p) {
    cls();
    printf("\n");
    divider(ORANGE, '═', 56);
    printf(ORANGE "  🌅  " GOLD BOLD "DAILY CHECK-IN — Sunrise Reflection" R "\n");
    divider(ORANGE, '═', 56);
    printf(IVORY "\n  How is your MOOD today? (1=very low, 10=wonderful)\n  > " R);
    p->mood = get_int(1, 10);

    printf(ORANGE "\n  ANXIETY level? (1=very calm, 10=very anxious)\n  > " R);
    p->anxiety = get_int(1, 10);

    printf(FOREST "\n  ENERGY level? (1=exhausted, 10=full of life)\n  > " R);
    p->energy = get_int(1, 10);

    if (p->mood <= 3) {
        typewrite(CRIMSON, "\n  The baobab survives the longest drought.\n", 18);
        typewrite(IVORY,   "  You are not alone. Try breathing next.\n", 18);
    } else if (p->mood <= 6) {
        typewrite(SAVANNA, "\n  You are mid-journey — keep walking forward.\n", 18);
    } else {
        typewrite(GOLD, "\n  Your energy shines like a Kampala sunrise!\n", 18);
    }

    update_streak(p);
    p->sessions++;
    award_xp(p, 20, "Daily Check-In");
    check_quests(p);
    press_enter();
}

/* ═══════════════════════ AFFIRMATION ════════════════════════════════ */
void daily_affirmation(Player *p) {
    cls();
    printf("\n");
    divider(PURPLE, '═', 56);
    printf(PURPLE "  🦩  " PINK BOLD "AFFIRMATION — Words of the Ancestors" R "\n");
    divider(PURPLE, '═', 56);

    srand((unsigned)time(NULL));

    const char *aff;
    if (p->premium) {
        int idx = rand() % (AFF_FREE_COUNT + AFF_PREMIUM_COUNT);
        aff = (idx < AFF_FREE_COUNT) ? affirmations_free[idx]
                                     : affirmations_premium[idx - AFF_FREE_COUNT];
    } else {
        aff = affirmations_free[rand() % AFF_FREE_COUNT];
    }

    printf(IVORY "\n  Close your eyes. Breathe deeply.\n\n" R);
    sleep(2);
    divider(GOLD, '─', 56);
    printf("\n  " GOLD BOLD "«  ");
    typewrite(GOLD BOLD, aff, 25);
    printf("  »\n\n" R);
    divider(GOLD, '─', 56);

    /* Bonus proverb for Lv3+ */
    if (p->level >= 3) {
        int pi = rand() % PROVERB_COUNT;
        printf(EARTH "\n  ─── African Wisdom (Lv3+ Bonus) ─────────\n");
        printf(SAVANNA "  \"%s\"\n" DIM "  — %s (%s)\n\n" R,
               proverbs[pi].text, proverbs[pi].author, proverbs[pi].origin);
    }

    /* Lv5+: reflection prompt */
    if (p->level >= 5) {
        printf(CYAN "\n  [Lv5+ Feature] How does this affirmation apply to your life?\n  > " R);
        char buf[128];
        if (fgets(buf, sizeof(buf), stdin)) {
            printf(FOREST "  Your reflection has been heard. 🌿\n" R);
        }
    }

    p->affirmations++;
    if (p->affirmations >= 10) check_badge(p, BADGE_AFFIRMATION_10);
    award_xp(p, 25, "Affirmation");
    check_quests(p);
    press_enter();
}

/* ═══════════════════════ GROUNDING ══════════════════════════════════ */
void grounding_exercise(Player *p) {
    cls();
    printf("\n");
    divider(FOREST, '═', 56);
    printf(FOREST "  🌿  " JUNGLE BOLD "GROUNDING — Bwindi Forest (5-4-3-2-1)" R "\n");
    divider(FOREST, '═', 56);
    printf(IVORY "\n  You are in the Bwindi Impenetrable Forest.\n");
    printf("  Mountain gorillas rest nearby. You are safe.\n\n" R);

    struct { int n; const char *sense; const char *col; } s[] = {
        {5,"things you SEE",   LAKE},
        {4,"things you TOUCH", EARTH},
        {3,"things you HEAR",  FOREST},
        {2,"things you SMELL", SAVANNA},
        {1,"thing you TASTE",  ORANGE}
    };

    for (int i = 0; i < 5; i++) {
        printf("%s\n  %d — Name %s:\n" R, s[i].col, s[i].n, s[i].sense);
        char buf[128];
        for (int j = 0; j < s[i].n; j++) {
            printf(GOLD "  %d. " R, j + 1); fflush(stdout);
            if (fgets(buf, sizeof(buf), stdin))
                printf(FOREST "     ✓\n" R);
        }
    }

    /* Lv4+ bonus: body scan */
    if (p->level >= 4) {
        printf(CYAN "\n  [Lv4+ Bonus] Body Scan — where do you feel tension? > " R);
        char buf[128];
        if (fgets(buf, sizeof(buf), stdin)) {
            printf(FOREST "  Breathe into that spot. Let it soften. 🌿\n" R);
            sleep(2);
        }
    }

    printf(GOLD "\n  ✅ You are HERE. You are PRESENT. You are SAFE.\n" R);
    p->anxiety = (p->anxiety > 3) ? p->anxiety - 3 : 1;
    p->mood    = (p->mood < 10)   ? p->mood + 2    : 10;
    p->grounds++;
    if (p->grounds == 1) check_badge(p, BADGE_GROUNDED);
    award_xp(p, 40, "Grounding Session");
    check_quests(p);
    press_enter();
}

/* ═══════════════════════ MINI-GAME: NILE RAPIDS ════════════════════ */
/* A simple reaction/focus game unlocked at Lv3                        */
void minigame_nile(Player *p) {
    if (p->level < 3) {
        printf(PURPLE "\n  🔒 Unlock at Level 3!\n" R);
        sleep(1); return;
    }
    cls();
    printf("\n");
    divider(LAKE, '═', 56);
    printf(LAKE "  🌊  " CYAN BOLD "NILE RAPIDS — Focus Mini-Game (Lv3+)" R "\n");
    divider(LAKE, '═', 56);
    printf(IVORY "\n  The Nile flows fast. Type the word shown\n");
    printf("  within 3 seconds to stay afloat!\n\n" R);
    press_enter();

    const char *words[] = { "BREATHE","PEACE","UBUNTU","NILE","CRANE",
                             "FOREST","GORILLA","KAMPALA","CALM","RISE" };
    int score = 0, rounds = 6;

    srand((unsigned)time(NULL));
    for (int r = 0; r < rounds; r++) {
        const char *w = words[rand() % 10];
        printf(GOLD "\n  » Type: " BOLD CYAN "%s" R GOLD " «\n  > " R, w);
        fflush(stdout);

        /* Simple timed input using alarm — fallback: just check match */
        char buf[32];
        if (fgets(buf, sizeof(buf), stdin)) {
            /* strip newline */
            int len = strlen(buf);
            if (len > 0 && buf[len-1] == '\n') buf[len-1] = '\0';
            if (strcmp(buf, w) == 0) {
                score++;
                printf(LIME "  ✓ Correct! Stay focused!\n" R);
            } else {
                printf(CRIMSON "  ✗ The rapids took you — keep breathing.\n" R);
            }
        }
    }

    int xp = score * 15;
    printf(GOLD "\n  Score: %d/%d 🎯\n" R, score, rounds);
    if (score == rounds) {
        printf(LIME BOLD "  PERFECT! You mastered the Nile!\n" R);
        xp += 50;
    }
    award_xp(p, xp, "Nile Rapids Mini-Game");
    press_enter();
}

/* ═══════════════════════ MINI-GAME: GORILLA WISDOM (Premium Lv6+) == */
void minigame_gorilla(Player *p) {
    if (!p->premium || p->level < 6) {
        printf(PURPLE "\n  🔒 Premium + Level 6 required!\n" R);
        sleep(1); return;
    }
    cls();
    printf("\n");
    divider(EARTH, '═', 56);
    printf(EARTH "  🦍  " SAVANNA BOLD "GORILLA WISDOM — Memory Game (Premium Lv6+)" R "\n");
    divider(EARTH, '═', 56);
    printf(IVORY "\n  The mountain gorilla remembers every plant\n");
    printf("  in Bwindi. Remember the sequence!\n\n" R);
    press_enter();

    const char *items[] = {"🌿","🍃","🌺","🦋","🌸","🍀","🌼","🌻"};
    int seq[8], len = 3, score = 0;
    srand((unsigned)time(NULL));

    for (int round = 0; round < 4; round++, len++) {
        /* Generate sequence */
        for (int i = 0; i < len; i++) seq[i] = rand() % 8;

        printf(GOLD "  Round %d — Remember: ", round + 1);
        for (int i = 0; i < len; i++) printf("%s ", items[seq[i]]);
        printf(R "\n");
        sleep(2);
        printf("  " DIM "(Sequence hidden...)" R "\n\n");
        sleep(1);

        printf(CYAN "  Enter the numbers (0-7) separated by spaces:\n");
        printf("  Plants: ");
        for (int i = 0; i < 8; i++) printf("%d:%s ", i, items[i]);
        printf("\n  > " R);

        int got[8]; char buf[64];
        if (fgets(buf, sizeof(buf), stdin)) {
            int ok = 1;
            char *tok = strtok(buf, " \n");
            for (int i = 0; i < len && tok; i++, tok = strtok(NULL, " \n")) {
                if (atoi(tok) != seq[i]) { ok = 0; break; }
            }
            if (ok) { score++; printf(LIME "  ✓ Perfect memory!\n" R); }
            else       printf(CRIMSON "  ✗ The gorilla remembers better — try again!\n" R);
        }
    }

    printf(GOLD "\n  Score: %d/4\n" R, score);
    int xp = score * 30 + 40;
    if (score == 4) { check_badge(p, BADGE_GORILLA); xp += 100; }
    award_xp(p, xp, "Gorilla Wisdom Game");
    press_enter();
}

/* ═══════════════════════ JOURNAL ════════════════════════════════════ */
void journal_entry(Player *p) {
    cls();
    printf("\n");
    divider(EARTH, '═', 56);
    printf(EARTH "  📖  " SAVANNA BOLD "JOURNAL — Your Story Matters" R "\n");
    divider(EARTH, '═', 56);

    /* Lv2+ gets a prompt */
    if (p->level >= 2) {
        const char *prompts[] = {
            "What moment today felt like a ray of Ugandan sunshine?",
            "What is one thing you would tell your younger self?",
            "Describe a person who makes you feel Ubuntu.",
            "What fear would you release to the Nile today?",
            "How have you grown since you started this garden?"
        };
        srand((unsigned)time(NULL));
        printf(CYAN "\n  [Lv2+ Prompt] %s\n" R, prompts[rand() % 5]);
    }

    if (p->journal_count >= JOURNAL_ENTRIES) {
        printf(ORANGE "\n  Journal full! Here are your entries:\n\n" R);
        for (int i = 0; i < JOURNAL_ENTRIES; i++)
            printf(GOLD "  [%d] " IVORY "%s\n" R, i + 1, p->journal[i]);
        press_enter(); return;
    }

    printf(SAVANNA "\n  Write freely. No judgment. Only truth.\n  > " R);
    if (fgets(p->journal[p->journal_count], MAX_JOURNAL, stdin)) {
        int len = strlen(p->journal[p->journal_count]);
        if (len > 0 && p->journal[p->journal_count][len-1] == '\n')
            p->journal[p->journal_count][len-1] = '\0';
        p->journal_count++;
        p->journals++;
    }

    if (p->journals >= 5) check_badge(p, BADGE_JOURNAL_5);
    award_xp(p, 35, "Journal Entry");
    check_quests(p);
    printf(FOREST "\n  Your words are seeds. The forest hears you. 🌿\n" R);
    press_enter();
}

/* ═══════════════════════ QUESTS SCREEN ══════════════════════════════ */
void show_quests(Player *p) {
    cls();
    printf("\n");
    divider(CYAN, '═', 56);
    printf(CYAN "  🎯  " GOLD BOLD "QUESTS — Path of the Ubuntu Warrior" R "\n");
    divider(CYAN, '═', 56);
    printf("\n");

    check_quests(p);

    for (int i = 0; i < p->quest_count; i++) {
        Quest *q = &p->quests[i];
        if (q->premium && !p->premium) {
            printf(PURPLE "  🔒 [★] %-25s — PREMIUM\n" R, q->title);
            continue;
        }
        const char *col = q->complete ? LIME : GOLD;
        printf("%s  %s %-25s" R, col, q->complete ? "✅" : "🎯", q->title);
        printf(DIM " [%d/%d] +%dXP\n" R, q->progress, q->target, q->xp_reward);
        printf(IVORY "     %s\n\n" R, q->desc);
    }
    press_enter();
}

/* ═══════════════════════ BADGES SCREEN ══════════════════════════════ */
void show_badges(Player *p) {
    cls();
    printf("\n");
    divider(GOLD, '═', 56);
    printf(GOLD "  🏅  " SAVANNA BOLD "YOUR BADGES — Honours of the Journey" R "\n");
    divider(GOLD, '═', 56);
    printf("\n");

    for (int i = 0; i < 10; i++) {
        int earned = p->badges[i];
        int prem   = (i >= 5);
        if (prem && !p->premium && !earned) {
            printf(PURPLE "  🔒 %-22s  [★ Premium]\n" R, badge_names[i]);
        } else {
            printf("%s  %s %-22s  %s\n" R,
                   earned ? GOLD : DIM,
                   badge_icons[i],
                   badge_names[i],
                   earned ? "✓ EARNED" : "(locked)");
        }
    }
    printf("\n");
    press_enter();
}

/* ═══════════════════════ GARDEN VIEW ════════════════════════════════ */
void show_garden(Player *p) {
    cls();
    printf("\n");
    divider(FOREST, '═', 56);
    printf(FOREST "  🌳  " JUNGLE BOLD "YOUR MIND GARDEN" R "\n");
    divider(FOREST, '═', 56);

    const char *art[] = {
        EARTH  "      .  *  .\n  .  🌱  .  🌱  .\n",
        FOREST "    🌿   🌿   🌿\n   .  ·  🌱  ·  .\n",
        JUNGLE "   🌲  🌲  🌲\n  🌿  🌱  🌿  🌱\n",
        FOREST "  🌳  🌳  🌳\n 🌿 🦋 🌺 🦜 🌿\n",
        JUNGLE " 🌳🌳 🦁 🌳🌳\n🌺🦋🌸🦜🌿🌼🌻\n",
        GOLD   "✨🌳🦩🌳🌺🌳🦩🌳✨\n🌊🌿🦋🌸👑🌸🦋🌿🌊\n"
    };

    int stage = p->level - 1;
    if (stage > 5) stage = 5;
    printf("\n%s" R "\n", art[stage]);

    printf(GOLD "  Level:         %d — %s %s\n" R, p->level, level_icons[p->level], level_names[p->level]);
    printf(SAVANNA "  XP:            %d\n", p->xp);
    printf(LAKE   "  Ubuntu Coins:  💰 %d\n", p->coins);
    printf(CRIMSON "  Streak:        🔥 %d days\n", p->streak);
    printf(FOREST "  Sessions:      %d\n" R, p->sessions);
    printf("\n");

    /* XP bar */
    int xp_next  = (p->level < MAX_LEVELS) ? level_xp[p->level+1] : p->xp;
    int xp_this  = p->xp - level_xp[p->level];
    int xp_range = xp_next - level_xp[p->level];
    int filled   = (xp_range > 0) ? (xp_this * 30 / xp_range) : 30;
    printf(SAVANNA "  Progress to Lv%d: [", p->level + 1);
    for (int i = 0; i < 30; i++)
        printf(i < filled ? LIME "█" R : DIM "░" R);
    printf(SAVANNA "] %d%%\n\n" R, xp_range > 0 ? (xp_this * 100 / xp_range) : 100);

    press_enter();
}

/* ═══════════════════════ SHOP — UBUNTU COINS ════════════════════════ */
void coin_shop(Player *p) {
    cls();
    printf("\n");
    divider(GOLD, '═', 56);
    printf(GOLD "  💰  " SAVANNA BOLD "UBUNTU COIN SHOP" R "\n");
    divider(GOLD, '═', 56);
    printf(IVORY "\n  You have " GOLD BOLD "%d Ubuntu Coins" R IVORY ".\n\n" R, p->coins);

    printf(GOLD "  ┌───────────────────────────────────────┐\n");
    printf(GOLD "  │ 1. Mood Boost       " ORANGE "— 30 coins (+2 mood)" GOLD "  │\n");
    printf(GOLD "  │ 2. Calm Wave        " LAKE   "— 30 coins (-2 anxiety)" GOLD "│\n");
    printf(GOLD "  │ 3. Energy Surge     " LIME   "— 40 coins (+3 energy)" GOLD " │\n");
    printf(GOLD "  │ 4. XP Boost         " CYAN   "— 60 coins (+50 XP)" GOLD "    │\n");
    printf(GOLD "  │ 5. Back             " DIM    "— return to menu" GOLD "       │\n");
    printf(GOLD "  └───────────────────────────────────────┘\n\n" R);
    printf(SAVANNA "  Choose: " R);

    int c = get_int(1, 5);
    switch (c) {
        case 1:
            if (p->coins >= 30) { p->coins -= 30; p->mood = (p->mood + 2 > 10) ? 10 : p->mood + 2;
                printf(LIME "  Mood boosted! 🌅\n" R); }
            else printf(CRIMSON "  Not enough coins!\n" R);
            break;
        case 2:
            if (p->coins >= 30) { p->coins -= 30; p->anxiety = (p->anxiety - 2 < 1) ? 1 : p->anxiety - 2;
                printf(LIME "  Calm wave received! 🌊\n" R); }
            else printf(CRIMSON "  Not enough coins!\n" R);
            break;
        case 3:
            if (p->coins >= 40) { p->coins -= 40; p->energy = (p->energy + 3 > 10) ? 10 : p->energy + 3;
                printf(LIME "  Energy surging! ⚡\n" R); }
            else printf(CRIMSON "  Not enough coins!\n" R);
            break;
        case 4:
            if (p->coins >= 60) { p->coins -= 60; award_xp(p, 50, "XP Boost Item"); }
            else printf(CRIMSON "  Not enough coins!\n" R);
            break;
        case 5: break;
    }
    sleep(1);
}

/* ═══════════════════════ PAYMENT / PREMIUM ══════════════════════════ */
void payment_screen(Player *p) {
    cls();
    printf("\n");
    divider(PURPLE, '═', 56);
    printf(PURPLE "  ★  " GOLD BOLD "UBUNTU MIND GARDEN — PREMIUM UPGRADE" R "\n");
    divider(PURPLE, '═', 56);
    printf("\n");

    if (p->premium) {
        printf(GOLD BOLD "  ✨ You are already a PREMIUM member!\n\n" R);
        printf(IVORY "  Thank you for supporting Ubuntu Mind Garden.\n");
        printf("  Your purchase helps fund mental health resources\n");
        printf("  across Uganda and East Africa. 🌍\n\n" R);
        press_enter(); return;
    }

    printf(IVORY "  FREE Plan (Current):\n");
    printf(DIM "  ✓ Levels 1–5\n");
    printf(DIM "  ✓ 10 affirmations\n");
    printf(DIM "  ✓ Basic breathing, grounding, journal\n");
    printf(DIM "  ✓ Nile Rapids mini-game\n\n" R);

    printf(GOLD BOLD "  ★ PREMIUM Plan — Unlock Everything:\n" R);
    printf(LIME "  ✓ ALL 10 Levels\n");
    printf(LIME "  ✓ 20 affirmations (double the wisdom)\n");
    printf(LIME "  ✓ Gorilla Wisdom memory game\n");
    printf(LIME "  ✓ Advanced journal prompts\n");
    printf(LIME "  ✓ Body scan at Level 4+\n");
    printf(LIME "  ✓ Premium badges & quests\n");
    printf(LIME "  ✓ Affirmation reflections at Level 5+\n\n" R);

    divider(GOLD, '─', 56);

    printf(GOLD "\n  PRICING OPTIONS:\n\n");
    printf(GOLD "  ┌─────────────────────────────────────────┐\n");
    printf(GOLD "  │ 1. " LIME BOLD "Monthly   — UGX 15,000 / USD 4.99" GOLD "     │\n");
    printf(GOLD "  │ 2. " CYAN BOLD "Annual    — UGX 120,000 / USD 34.99" GOLD "   │\n");
    printf(GOLD "  │              " IVORY "(Save 40%!)" GOLD "                   │\n");
    printf(GOLD "  │ 3. " ORANGE BOLD "Lifetime  — UGX 250,000 / USD 69.99" GOLD "  │\n");
    printf(GOLD "  │ 4. " DIM "Back to menu" GOLD "                          │\n");
    printf(GOLD "  └─────────────────────────────────────────┘\n\n" R);

    printf(SAVANNA "  Choose a plan: " R);
    int plan = get_int(1, 4);
    if (plan == 4) return;

    const char *plan_names[] = {"", "Monthly", "Annual", "Lifetime"};
    const char *prices[]     = {"", "UGX 15,000 / USD 4.99",
                                     "UGX 120,000 / USD 34.99",
                                     "UGX 250,000 / USD 69.99"};

    cls();
    printf("\n");
    divider(GOLD, '═', 56);
    printf(GOLD "  ★  " SAVANNA BOLD "CHECKOUT — %s Plan" R "\n", plan_names[plan]);
    divider(GOLD, '═', 56);
    printf("\n");
    printf(IVORY "  Plan:    " GOLD BOLD "%s\n" R, plan_names[plan]);
    printf(IVORY "  Price:   " GOLD BOLD "%s\n\n" R, prices[plan]);

    printf(SAVANNA "  PAYMENT METHODS:\n\n");
    printf(GOLD "  ┌─────────────────────────────────────────┐\n");
    printf(GOLD "  │ A. " ORANGE "Mobile Money (MTN / Airtel Uganda)" GOLD "   │\n");
    printf(GOLD "  │ B. " CYAN   "Visa / Mastercard (Credit/Debit)" GOLD "     │\n");
    printf(GOLD "  │ C. " LIME   "PayPal / Stripe" GOLD "                      │\n");
    printf(GOLD "  │ D. " PURPLE "Flutterwave (Pan-Africa)" GOLD "             │\n");
    printf(GOLD "  └─────────────────────────────────────────┘\n\n" R);

    printf(IVORY "  Choose payment (A/B/C/D): " R);
    char ch[8];
    if (fgets(ch, sizeof(ch), stdin)) {
        char method = ch[0];

        printf("\n");
        divider(GOLD, '─', 56);

        if (method == 'A' || method == 'a') {
            printf(ORANGE "\n  📱 MOBILE MONEY PAYMENT\n\n" R);
            printf(IVORY "  Send " GOLD BOLD "%s" R IVORY " to:\n", prices[plan]);
            printf(ORANGE "  MTN:    *165*3*1# → Enter Code: UMG2024\n");
            printf(ORANGE "  Airtel: *185*9#   → Enter Code: UMG2024\n\n" R);
            printf(IVORY "  Enter transaction ID after payment:\n  > " R);
        } else if (method == 'B' || method == 'b') {
            printf(CYAN "\n  💳 CARD PAYMENT\n\n" R);
            printf(IVORY "  Visit: " CYAN BOLD "https://pay.ubuntumindgarden.ug\n" R);
            printf(IVORY "  Your session code: " GOLD BOLD "UMG-%s-%d\n\n" R,
                   p->name, (int)time(NULL) % 10000);
            printf(IVORY "  Enter confirmation code after payment:\n  > " R);
        } else if (method == 'C' || method == 'c') {
            printf(LIME "\n  🌐 PAYPAL / STRIPE\n\n" R);
            printf(IVORY "  PayPal:  paypal.me/UbuntuMindGarden\n");
            printf(IVORY "  Stripe:  " LIME "https://buy.stripe.com/ubuntumindgarden\n\n" R);
            printf(IVORY "  Enter PayPal/Stripe order ID:\n  > " R);
        } else {
            printf(PURPLE "\n  🌍 FLUTTERWAVE (Pan-Africa)\n\n" R);
            printf(IVORY "  Supported: M-Pesa, MTN, Airtel, Zamtel, Tigo,\n");
            printf(IVORY "             Bank Transfer, Card across Africa\n\n");
            printf(IVORY "  Link: " PURPLE "https://flutterwave.com/pay/ubuntumindgarden\n\n" R);
            printf(IVORY "  Enter Flutterwave ref number:\n  > " R);
        }

        char txn[64];
        if (fgets(txn, sizeof(txn), stdin) && strlen(txn) > 1) {
            /* In production: verify txn with payment gateway API */
            /* For this demo: any non-empty input unlocks premium  */
            printf(GOLD BOLD "\n  ✨ PAYMENT RECEIVED — THANK YOU! ✨\n\n" R);
            printf(LIME "  Transaction: %s" R, txn);
            printf(IVORY "\n  Your account has been upgraded to " GOLD BOLD "★ PREMIUM" R "\n\n");

            p->premium = 1;
            printf(FOREST "  🌍 10%% of your subscription goes towards\n");
            printf("  mental health programs in Uganda. Ubuntu!\n\n" R);
            sleep(2);

            /* Unlock premium rewards */
            award_xp(p, 200, "Premium Upgrade Bonus");
            p->coins += 100;
            printf(GOLD "  +100 Ubuntu Coins added as welcome gift! 💰\n" R);
        } else {
            printf(ORANGE "\n  Payment cancelled. You can upgrade anytime.\n" R);
        }
    }
    press_enter();
}

/* ═══════════════════════ CRISIS SUPPORT ════════════════════════════ */
void crisis_support(void) {
    cls();
    printf("\n");
    divider(CRIMSON, '═', 56);
    printf(CRIMSON "  💛  " ORANGE BOLD "YOU ARE NOT ALONE — Support Resources" R "\n");
    divider(CRIMSON, '═', 56);
    printf("\n");
    printf(GOLD   "  🇺🇬 UGANDA:\n" R);
    printf(IVORY  "  • Butabika National Referral Hospital:\n");
    printf(SAVANNA "    +256 414 504 375 (Direct)\n");
    printf(SAVANNA "    0800 211 306 (Toll Free)\n\n" R);
    printf(IVORY  "  • TASO Uganda: taso.go.ug\n\n" R);
    printf(GOLD   "  🌍 AFRICA / INTERNATIONAL:\n" R);
    printf(IVORY  "  • Befrienders: befrienders.org\n");
    printf(IVORY  "  • Crisis Text Line: crisistextline.org\n");
    printf(IVORY  "  • WHO Mental Health: who.int/mental_health\n\n" R);
    printf(CRIMSON "  Emergency? Call your local services NOW.\n\n" R);
    press_enter();
}

/* ═══════════════════════ PLAYER SETUP ═══════════════════════════════ */
void setup_new_player(Player *p) {
    memset(p, 0, sizeof(Player));
    p->level   = 1;
    p->mood    = 5; p->anxiety = 5; p->energy = 5;
    p->streak  = 0; p->last_day = 0;
    p->premium = 0;
    init_quests(p);

    cls();
    draw_logo();
    printf(GOLD "\n  Welcome, traveler. What is your name?\n  > " R);
    char buf[MAX_NAME + 8];
    if (fgets(buf, sizeof(buf), stdin)) {
        int len = strlen(buf);
        if (len > 0 && buf[len-1] == '\n') buf[len-1] = '\0';
        strncpy(p->name, buf, MAX_NAME - 1);
    }
    if (!strlen(p->name)) strcpy(p->name, "Friend");

    printf(FOREST "\n  Karibu, " GOLD BOLD "%s" R FOREST "! (Welcome in Swahili)\n\n" R, p->name);
    printf(IVORY  "  This is your Mind Garden. Every session you complete\n");
    printf(        "  earns XP, Ubuntu Coins, and grows your garden.\n");
    printf(        "  Reach Level 10 to become an Ancestor Light. 🌟\n\n" R);
    press_enter();
}

/* ═══════════════════════ MAIN MENU ══════════════════════════════════ */
int main_menu(Player *p) {
    cls();
    draw_hud(p);
    printf("\n");
    printf(GOLD   "  ╔═══════════════════════════════════════════╗\n");
    printf(GOLD   "  ║  " ORANGE BOLD "UBUNTU MIND GARDEN — MAIN MENU" R GOLD "        ║\n");
    printf(GOLD   "  ╠═══════════════════════════════════════════╣\n");
    printf(GOLD   "  ║ " ORANGE "1." IVORY " Daily Check-In          " GOLD "            ║\n");
    printf(GOLD   "  ║ " LAKE   "2." IVORY " Breathing Exercise       " GOLD "            ║\n");
    printf(GOLD   "  ║ " PURPLE "3." IVORY " Affirmation              " GOLD "            ║\n");
    printf(GOLD   "  ║ " FOREST "4." IVORY " Grounding (5-4-3-2-1)   " GOLD "            ║\n");
    printf(GOLD   "  ║ " CYAN   "5." IVORY " Nile Rapids Mini-Game   " GOLD DIM " [Lv3+]" GOLD "   ║\n");
    printf(GOLD   "  ║ " PURPLE "6." IVORY " Gorilla Wisdom Game     " GOLD DIM " [★Lv6+]" GOLD "  ║\n");
    printf(GOLD   "  ║ " EARTH  "7." IVORY " Journal Entry            " GOLD "            ║\n");
    printf(GOLD   "  ╠═══════════════════════════════════════════╣\n");
    printf(GOLD   "  ║ " CYAN   "8." IVORY " Quests                  " GOLD "            ║\n");
    printf(GOLD   "  ║ " SAVANNA"9." IVORY " Badges                  " GOLD "            ║\n");
    printf(GOLD   "  ║ " LIME   "10." IVORY " Level Map              " GOLD "            ║\n");
    printf(GOLD   "  ║ " GOLD   "11." IVORY " My Garden (Progress)   " GOLD "            ║\n");
    printf(GOLD   "  ║ " ORANGE "12." IVORY " Ubuntu Coin Shop       " GOLD "            ║\n");
    printf(GOLD   "  ╠═══════════════════════════════════════════╣\n");
    printf(GOLD   "  ║ " PURPLE "13." IVORY " " BOLD "★ Upgrade to Premium " R GOLD "            ║\n");
    printf(GOLD   "  ║ " CRIMSON"14." IVORY " Crisis Support          " GOLD "            ║\n");
    printf(GOLD   "  ║ " DIM    "15." RESET IVORY " Save & Exit             " GOLD "            ║\n");
    printf(GOLD   "  ╚═══════════════════════════════════════════╝\n\n" R);
    printf(SAVANNA "  Choose (1-15): " R);
    return get_int(1, 15);
}

/* ═══════════════════════ MAIN ═══════════════════════════════════════ */
int main(void) {
    Player player;
    int loaded = load_player(&player);

    if (!loaded) {
        setup_new_player(&player);
    } else {
        cls();
        draw_logo();
        printf(FOREST "\n  Welcome back, " GOLD BOLD "%s" R FOREST "! (Save loaded)\n" R, player.name);
        printf(GOLD "  Level %d — %s %s\n" R, player.level, level_icons[player.level], level_names[player.level]);
        sleep(1);
        press_enter();
    }

    int running = 1;
    while (running) {
        int choice = main_menu(&player);
        switch (choice) {
            case  1: mood_checkin(&player);      break;
            case  2: breathing_exercise(&player);break;
            case  3: daily_affirmation(&player); break;
            case  4: grounding_exercise(&player);break;
            case  5: minigame_nile(&player);     break;
            case  6: minigame_gorilla(&player);  break;
            case  7: journal_entry(&player);     break;
            case  8: show_quests(&player);       break;
            case  9: show_badges(&player);       break;
            case 10: show_level_map(&player);    break;
            case 11: show_garden(&player);       break;
            case 12: coin_shop(&player);         break;
            case 13: payment_screen(&player);    break;
            case 14: crisis_support();           break;
            case 15: running = 0;                break;
        }
        save_player(&player);
        check_quests(&player);
    }

    /* Farewell */
    cls();
    printf("\n");
    divider(GOLD, '═', 56);
    printf(GOLD "\n  Goodbye, " ORANGE BOLD "%s" R GOLD ".\n\n" R, player.name);
    printf(IVORY "  Level " GOLD BOLD "%d" R IVORY " — %s %s\n", player.level, level_icons[player.level], level_names[player.level]);
    printf(IVORY "  XP: " GOLD "%d  " IVORY "Coins: 💰%d  Streak: 🔥%d\n\n" R,
           player.xp, player.coins, player.streak);
    printf(SAVANNA "  \"However long the night, the dawn will break.\"\n" DIM "   — African Proverb\n\n" R);
    printf(FOREST "  Your garden grows even while you rest. 🌿\n\n" R);
    divider(GOLD, '═', 56);
    printf("\n");
    return 0;
}

/*
 * ══════════════════════════════════════════════════════════════════
 *  END OF UBUNTU MIND GARDEN v2.0
 *  ~900 lines of C | 10 Levels | 2 Mini-Games | Payment System
 *  Quests | Badges | Ubuntu Coins | Save/Load | Streak Tracking
 *  "Umuntu ngumuntu ngabantu" — I am because we are 🌍
 * ══════════════════════════════════════════════════════════════════
 */
