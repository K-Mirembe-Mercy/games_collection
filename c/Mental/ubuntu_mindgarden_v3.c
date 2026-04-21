/*
 * ╔══════════════════════════════════════════════════════════════════════╗
 * ║        UBUNTU MIND GARDEN — ADVANCED EDITION v3.0                   ║
 * ║        "Umuntu ngumuntu ngabantu" — I am because we are             ║
 * ║                                                                      ║
 * ║  20-Level Mental Wellness RPG | Ugandan & African Culture           ║
 * ║  Free forever to play | Premium unlocks bonus content               ║
 * ║                                                                      ║
 * ║  Compile:  gcc ubuntu_mindgarden_v3.c -o umg3 -lm                  ║
 * ║  Run:      ./umg3                                                   ║
 * ╚══════════════════════════════════════════════════════════════════════╝
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <math.h>
#include <unistd.h>
#include <ctype.h>

/* ══════════════════════════ ANSI COLORS ════════════════════════════ */
#define R        "\033[0m"
#define BOLD     "\033[1m"
#define DIM      "\033[2m"
#define ITALIC   "\033[3m"
#define BLINK    "\033[5m"
#define ULINE    "\033[4m"

#define GOLD     "\033[38;5;220m"
#define ORANGE   "\033[38;5;208m"
#define CRIMSON  "\033[38;5;160m"
#define RED      "\033[38;5;196m"
#define EARTH    "\033[38;5;130m"
#define SAVANNA  "\033[38;5;178m"
#define FOREST   "\033[38;5;34m"
#define JUNGLE   "\033[38;5;28m"
#define LAKE     "\033[38;5;33m"
#define SKY      "\033[38;5;117m"
#define IVORY    "\033[38;5;230m"
#define PURPLE   "\033[38;5;135m"
#define PINK     "\033[38;5;211m"
#define MIST     "\033[38;5;189m"
#define SILVER   "\033[38;5;250m"
#define CYAN     "\033[38;5;51m"
#define LIME     "\033[38;5;118m"
#define TEAL     "\033[38;5;37m"
#define ROSE     "\033[38;5;204m"
#define INDIGO   "\033[38;5;62m"
#define AMBER    "\033[38;5;214m"

/* ══════════════════════════ CONSTANTS ══════════════════════════════ */
#define MAX_NAME          64
#define MAX_JOURNAL       768
#define JOURNAL_CAP       20
#define MAX_LEVELS        20
#define MAX_QUESTS        12
#define MAX_BADGES        25
#define MAX_SKILLS         8
#define SAVE_FILE         "umg3_save.dat"
#define VERSION           "3.0"

/*
 * ══════════════════════ 20-LEVEL SYSTEM ═════════════════════════════
 *
 * FREE TIER  (Levels 1–8):  Full gameplay, all core features
 * PREMIUM    (Levels 9–20): Bonus content, advanced features, extra games
 *
 * Free players can play ALL activities at Levels 1–8 forever.
 * Premium unlocks deeper content, not harder walls.
 *
 *  Lv 1  — Seed of Intention       XP:0
 *  Lv 2  — First Sprout            XP:120
 *  Lv 3  — Young Sapling           XP:300
 *  Lv 4  — Rooted Sapling          XP:550
 *  Lv 5  — River Walker            XP:900
 *  Lv 6  — Savanna Keeper          XP:1350
 *  Lv 7  — Forest Guardian         XP:1900
 *  Lv 8  — Mountain Climber        XP:2600     ← Free cap
 *  ─────────────────────────────────────────────
 *  Lv 9  — Crested Crane     [★]  XP:3500
 *  Lv 10 — Nile Swimmer      [★]  XP:4600
 *  Lv 11 — Gorilla Kin       [★]  XP:6000
 *  Lv 12 — Bwindi Mystic     [★]  XP:7700
 *  Lv 13 — Drumbeat Elder    [★]  XP:9800
 *  Lv 14 — Savanna Shaman    [★]  XP:12300
 *  Lv 15 — Ancestor's Voice  [★]  XP:15300
 *  Lv 16 — Ubuntu Sage       [★]  XP:18900
 *  Lv 17 — Lake Spirit       [★]  XP:23200
 *  Lv 18 — Baobab Soul       [★]  XP:28300
 *  Lv 19 — Rwenzori Peak     [★]  XP:34400
 *  Lv 20 — Ancestor Light    [★]  XP:42000
 */

static const char *lvl_name[MAX_LEVELS+1] = {
    "",
    "Seed of Intention","First Sprout","Young Sapling","Rooted Sapling",
    "River Walker","Savanna Keeper","Forest Guardian","Mountain Climber",
    "Crested Crane","Nile Swimmer","Gorilla Kin","Bwindi Mystic",
    "Drumbeat Elder","Savanna Shaman","Ancestor's Voice","Ubuntu Sage",
    "Lake Spirit","Baobab Soul","Rwenzori Peak","Ancestor Light"
};
static const char *lvl_icon[MAX_LEVELS+1] = {
    "",
    "🌱","🌿","🌲","🌳","🌊","🌾","🌴","🏔",
    "🦩","🐟","🦍","🌫","🥁","🔮","👁","📿",
    "💧","🌳","⛰","✨"
};
static const int lvl_xp[MAX_LEVELS+1] = {
    0,
    0,120,300,550,900,1350,1900,2600,
    3500,4600,6000,7700,9800,12300,15300,18900,
    23200,28300,34400,42000
};
static const int lvl_premium[MAX_LEVELS+1] = {
    0, 0,0,0,0,0,0,0,0,  /* 1–8 free */
    1,1,1,1,1,1,1,1,1,1,1,1  /* 9–20 premium */
};

/* ══════════════════════════ SKILL TREE ════════════════════════════ */
/* Each skill has a level and contributes passive bonuses */
typedef struct {
    char name[32];
    int  level;      /* 0–5 */
    int  xp_bonus;   /* % bonus XP from this discipline */
} Skill;

static const char *skill_names[MAX_SKILLS] = {
    "Breathing Master","Grounding Adept","Journal Sage",
    "Affirmation Warrior","Meditation Monk","Mindful Mover",
    "Community Elder","Ancestral Wisdom"
};

/* ══════════════════════════ BADGES ════════════════════════════════ */
#define BADGE_FIRST_BREATH     0
#define BADGE_7_STREAK         1
#define BADGE_30_STREAK        2
#define BADGE_JOURNAL_3        3
#define BADGE_JOURNAL_10       4
#define BADGE_GROUNDED_1       5
#define BADGE_AFF_10           6
#define BADGE_AFF_50           7
#define BADGE_NILE_ACE         8
#define BADGE_GORILLA_MASTER   9
#define BADGE_DRUM_KING        10
#define BADGE_MEDITATION_1     11   /* premium */
#define BADGE_HEALER           12   /* premium */
#define BADGE_UBUNTU_50        13   /* premium */
#define BADGE_FULL_FOREST      14   /* premium */
#define BADGE_ANCESTOR_LIGHT   15   /* premium */
#define BADGE_14_STREAK        16
#define BADGE_COIN_100         17
#define BADGE_LV5              18
#define BADGE_LV10             19   /* premium */
#define BADGE_PERFECT_NILE     20
#define BADGE_NIGHT_OWL        21
#define BADGE_DAWN_RISER       22
#define BADGE_RESILIENCE       23   /* 3 check-ins with mood<=3 */
#define BADGE_UBUNTU_MASTER    24   /* premium */

static const char *badge_name[MAX_BADGES] = {
    "First Breath","7-Day Warrior","30-Day Legend","Griot Begins",
    "Griot Master","Grounded Once","Affirmation x10","Affirmation x50",
    "Nile Ace","Gorilla Master","Drum King","Meditation Adept",
    "Community Healer","Ubuntu 50","Full Forest","Ancestor Light",
    "14-Day Flame","Coin Collector","River Walker","Nile Swimmer",
    "Perfect Nile","Night Owl","Dawn Riser","Resilience",
    "Ubuntu Master"
};
static const char *badge_icon[MAX_BADGES] = {
    "🌬","🔥","🌋","📖","📚","🌿","🦩","👑",
    "🐟","🦍","🥁","🧘","💛","🤝","🌳","✨",
    "💎","💰","🌊","🏊","🎯","🦉","🌅","💪",
    "🌍"
};
static const int badge_premium[MAX_BADGES] = {
    0,0,0,0,0,0,0,0,
    0,0,0,1,1,1,1,1,
    0,0,0,1,0,0,0,0,1
};

/* ══════════════════════════ QUESTS ════════════════════════════════ */
#define QUEST_MAX 12
typedef struct {
    char title[64];
    char desc[128];
    int  target;
    int  progress;
    int  xp_reward;
    int  coin_reward;
    int  complete;
    int  premium;
    int  skill_idx;  /* which skill gets +1 on complete (-1=none) */
} Quest;

/* ══════════════════════════ MEDITATION STATE ══════════════════════ */
typedef struct {
    int sessions_done;
    int total_minutes;
    int unlocked;  /* 1 if premium or earned via coins */
} MeditationData;

/* ══════════════════════════ PLAYER ════════════════════════════════ */
typedef struct {
    char    name[MAX_NAME];
    int     level;
    int     xp;
    int     coins;
    int     mood;          /* 1–10 */
    int     anxiety;       /* 1–10 */
    int     energy;        /* 1–10 */
    int     streak;
    int     sessions;
    int     breaths;
    int     grounds;
    int     affirmations;
    int     journals;
    int     meditations;
    int     minigames_won;
    int     low_mood_checkins;  /* for resilience badge */
    int     premium;
    int     badges[MAX_BADGES];
    char    journal[JOURNAL_CAP][MAX_JOURNAL];
    int     journal_count;
    Quest   quests[QUEST_MAX];
    int     quest_count;
    Skill   skills[MAX_SKILLS];
    MeditationData med;
    long    last_day;
    int     total_xp_earned;
    int     total_coins_earned;
    int     perfect_games;
} Player;

/* ══════════════════════════ CONTENT ════════════════════════════════ */
typedef struct { const char *text, *author, *origin; } Proverb;

static const char *aff_free[] = {
    "I am rooted like the fig tree of Bwindi — nothing uproots me.",
    "Like Lake Victoria, my depth is endless and full of life.",
    "I carry the quiet strength of the Rwenzori mountains.",
    "My spirit is as vast and free as the African savanna.",
    "I bloom like Uganda's flame tree after every dry season.",
    "Like the crested crane, I stand tall, elegant, and proud.",
    "Ubuntu lives in me — I am because we are, and we are strong.",
    "The Nile rises for me every single morning. So do I.",
    "My healing is as natural as rain on the Ugandan hills.",
    "I am worthy of deep peace, like the stillness of Lake Bunyonyi.",
    "I breathe in the forest air of Bwindi and breathe out fear.",
    "My roots go deeper than any storm can reach.",
};
#define AFF_FREE_N 12

static const char *aff_premium[] = {
    "I rise again — ancient, resilient, magnificent, like Africa herself.",
    "My ancestors walked through fire. I carry their unbreakable courage.",
    "Joy drums inside me like a Kampala celebration that never ends.",
    "I am more powerful than my fear. The lion in me awakens.",
    "Every breath is a gift wrapped in the green of Ugandan hills.",
    "I welcome calm the way the savanna welcomes the first rains.",
    "My mind clears like morning mist burning off the River Nile.",
    "I am loved, I am enough, I am here — completely and always.",
    "The baobab was not built overnight — my growth is sacred and sure.",
    "I choose peace. I choose growth. I choose my fullest self.",
    "The crested crane does not apologise for its crown. Neither do I.",
    "I am the storyteller of my own life — and this is a great story.",
    "My tears water the garden. My laughter is the sunshine. Both matter.",
    "I carry the mountain in my chest and the river in my voice.",
    "I am not broken — I am being rebuilt into something extraordinary.",
    "Every wound I have carried has taught me something the healed never knew.",
};
#define AFF_PREM_N 16

static const Proverb proverbs[] = {
    {"Umuntu ngumuntu ngabantu.", "Nguni/Buganda", "Uganda/S.Africa"},
    {"Rain does not fall on one roof alone.", "Zulu", "South Africa"},
    {"However long the night, the dawn will break.", "Pan-African", "Africa"},
    {"Sticks in a bundle are unbreakable.", "Bondei", "Tanzania"},
    {"He who learns, teaches.", "Ethiopian", "Ethiopia"},
    {"The forest would be silent if no bird sang except the best.", "W.African", "W.Africa"},
    {"When the music changes, so does the dance.", "Hausa", "Nigeria"},
    {"Do not look where you fell — look where you slipped.", "Pan-African", "Africa"},
    {"A child not embraced by the village will burn it down.", "African", "E.Africa"},
    {"Until the lion learns to write, every story glorifies the hunter.", "Pan-African","Africa"},
    {"The wise do not weep and sit still — they plant.", "Ugandan", "Uganda"},
    {"To get lost is to learn the way.", "Swahili", "E.Africa"},
    {"The one who tells the stories rules the world.", "Hopi/Pan-African","Africa"},
    {"If you want to go fast, go alone. To go far, go together.", "African", "Africa"},
    {"A tree is straightened while it is young.", "Ugandan", "Uganda"},
    {"Even the mightiest eagle begins as an egg.", "African", "Africa"},
    {"Speak softly and carry a big heart.", "Buganda", "Uganda"},
    {"The axe forgets; the tree remembers — and still it grows.", "Shona","Zimbabwe"},
};
#define PROVERB_N 18

static const char *meditation_scripts[] = {
    /* Script 0 — Basic Awareness */
    "Close your eyes. Breathe naturally.\n"
    "Notice your feet on the ground — the earth of Uganda beneath you.\n"
    "Feel your breath rise and fall like the Nile's gentle current.\n"
    "You are here. You are present. You are safe.\n"
    "Stay with this feeling for a few moments...",

    /* Script 1 — Savanna Visualization (Premium) */
    "You are standing at the edge of the African savanna at dawn.\n"
    "The air is golden. Acacia trees cast long shadows.\n"
    "A pride of lions rests in the distance — peaceful, unhurried.\n"
    "You belong here. This land knows your name.\n"
    "Breathe in the dawn. Breathe out everything that no longer serves you.",

    /* Script 2 — Bwindi Forest Walk (Premium) */
    "You walk slowly into the Bwindi Impenetrable Forest.\n"
    "Mist hangs between ancient trees. Ferns brush your hands.\n"
    "Somewhere deep in the forest, a mountain gorilla watches you.\n"
    "Not with fear — with recognition. You are kin.\n"
    "With every step, you leave behind a worry. The forest absorbs it.",

    /* Script 3 — Lake Victoria Reflection (Premium) */
    "Sit at the shore of Lake Victoria at twilight.\n"
    "The water is the colour of copper and rose.\n"
    "Your reflection looks back at you — but calmer, freer.\n"
    "That version of you already exists. You are walking toward them.\n"
    "Let the waves wash over your feet. Let them take what you don't need.",

    /* Script 4 — Ancestral Fire (Premium) */
    "A fire burns in a village compound under a sky full of stars.\n"
    "Around it sit your ancestors — healers, warriors, storytellers.\n"
    "They have been waiting to meet you. They are proud of you.\n"
    "One of them places a hand on your shoulder and says:\n"
    "'You carry our best. You are not alone. You never were.'"
};
#define MED_SCRIPTS_FREE 1
#define MED_SCRIPTS_TOTAL 5

/* ══════════════════════════ UTILITIES ══════════════════════════════ */
void cls(void)            { printf("\033[2J\033[H"); fflush(stdout); }
void ms(int t)            { usleep(t * 1000); }
void nl(void)             { printf("\n"); }

void divider(const char *c, char ch, int w) {
    printf("%s", c);
    for (int i=0;i<w;i++) putchar(ch);
    printf(R "\n");
}

void tw(const char *col, const char *txt, int d) {
    printf("%s", col);
    while (*txt) { putchar(*txt++); fflush(stdout); ms(d); }
    printf(R);
}

void press_enter(void) {
    printf(DIM "\n  [ Press ENTER ]" R);
    fflush(stdout);
    while (getchar() != '\n');
}

int get_int(int lo, int hi) {
    int v; char buf[32];
    while (1) {
        if (fgets(buf, sizeof(buf), stdin) && sscanf(buf, "%d", &v)==1
                && v>=lo && v<=hi) return v;
        printf(ORANGE "  Enter %d–%d: " R, lo, hi);
    }
}

char get_char(void) {
    char buf[8];
    if (fgets(buf, sizeof(buf), stdin)) return toupper(buf[0]);
    return '?';
}

long epoch_day(void) { return (long)(time(NULL)/86400); }

/* ══════════════════════════ XP & LEVELS ════════════════════════════ */
int skill_xp_bonus(Player *p) {
    int bonus = 0;
    for (int i=0;i<MAX_SKILLS;i++) bonus += p->skills[i].level * 2;
    return bonus; /* % bonus */
}

void award_xp(Player *p, int base_xp, const char *reason) {
    int bonus   = (base_xp * skill_xp_bonus(p)) / 100;
    int total   = base_xp + bonus;
    int coins   = total / 4;
    p->xp      += total;
    p->coins   += coins;
    p->total_xp_earned    += total;
    p->total_coins_earned += coins;

    printf(GOLD "  +" BOLD "%d XP" R GOLD, total);
    if (bonus > 0) printf(" (" LIME "+%d skill bonus" GOLD ")", bonus);
    printf(" — %s  💰+%d\n" R, reason, coins);

    /* Level-up loop */
    while (p->level < MAX_LEVELS) {
        int next = p->level + 1;
        if (lvl_premium[next] && !p->premium) break;
        if (p->xp < lvl_xp[next]) break;
        p->level = next;
        printf(GOLD BOLD "\n  ╔══════════════════════════════════════╗\n");
        printf(GOLD BOLD "  ║  ✨ LEVEL UP!  →  Lv.%2d %-18s║\n",
               p->level, lvl_name[p->level]);
        printf(GOLD BOLD "  ╚══════════════════════════════════════╝\n" R);
        printf(IVORY "     %s %s\n" R, lvl_icon[p->level], lvl_name[p->level]);
        sleep(1);
    }
}

void gain_skill_level(Player *p, int idx) {
    if (idx < 0 || idx >= MAX_SKILLS) return;
    if (p->skills[idx].level >= 5) return;
    p->skills[idx].level++;
    printf(CYAN "  📈 Skill upgraded: " BOLD "%s → Lv%d" R "\n",
           p->skills[idx].name, p->skills[idx].level);
}

/* ══════════════════════════ BADGES ════════════════════════════════ */
void give_badge(Player *p, int id) {
    if (id < 0 || id >= MAX_BADGES) return;
    if (p->badges[id]) return;
    if (badge_premium[id] && !p->premium) return;
    p->badges[id] = 1;
    printf(GOLD BLINK "  🏅 BADGE: " R GOLD BOLD "%s %s\n" R,
           badge_icon[id], badge_name[id]);
    award_xp(p, 60, "Badge Earned");
    sleep(1);
}

void check_badges(Player *p) {
    if (p->breaths   >= 1)  give_badge(p, BADGE_FIRST_BREATH);
    if (p->streak    >= 7)  give_badge(p, BADGE_7_STREAK);
    if (p->streak    >= 14) give_badge(p, BADGE_14_STREAK);
    if (p->streak    >= 30) give_badge(p, BADGE_30_STREAK);
    if (p->journals  >= 3)  give_badge(p, BADGE_JOURNAL_3);
    if (p->journals  >= 10) give_badge(p, BADGE_JOURNAL_10);
    if (p->grounds   >= 1)  give_badge(p, BADGE_GROUNDED_1);
    if (p->affirmations >= 10) give_badge(p, BADGE_AFF_10);
    if (p->affirmations >= 50) give_badge(p, BADGE_AFF_50);
    if (p->level     >= 5)  give_badge(p, BADGE_LV5);
    if (p->level     >= 10) give_badge(p, BADGE_LV10);
    if (p->coins     >= 100) give_badge(p, BADGE_COIN_100);
    if (p->low_mood_checkins >= 3) give_badge(p, BADGE_RESILIENCE);
    if (p->perfect_games >= 1) give_badge(p, BADGE_PERFECT_NILE);
    if (p->meditations >= 1) give_badge(p, BADGE_MEDITATION_1);
}

/* ══════════════════════════ QUESTS ════════════════════════════════ */
void init_quests(Player *p) {
    p->quest_count = 12;
    int i = 0;

    /* FREE quests */
    strcpy(p->quests[i].title, "First Inhale");
    strcpy(p->quests[i].desc,  "Complete your first breathing session");
    p->quests[i].target=1; p->quests[i].xp_reward=50;
    p->quests[i].coin_reward=10; p->quests[i].premium=0;
    p->quests[i].skill_idx=0; i++;

    strcpy(p->quests[i].title, "Three Breaths");
    strcpy(p->quests[i].desc,  "Complete 3 breathing sessions");
    p->quests[i].target=3; p->quests[i].xp_reward=100;
    p->quests[i].coin_reward=20; p->quests[i].premium=0;
    p->quests[i].skill_idx=0; i++;

    strcpy(p->quests[i].title, "Griot of the Hills");
    strcpy(p->quests[i].desc,  "Write 5 journal entries");
    p->quests[i].target=5; p->quests[i].xp_reward=130;
    p->quests[i].coin_reward=25; p->quests[i].premium=0;
    p->quests[i].skill_idx=2; i++;

    strcpy(p->quests[i].title, "Bwindi Walker");
    strcpy(p->quests[i].desc,  "Complete 3 grounding sessions");
    p->quests[i].target=3; p->quests[i].xp_reward=110;
    p->quests[i].coin_reward=20; p->quests[i].premium=0;
    p->quests[i].skill_idx=1; i++;

    strcpy(p->quests[i].title, "Ancestor's Words");
    strcpy(p->quests[i].desc,  "Complete 10 affirmations");
    p->quests[i].target=10; p->quests[i].xp_reward=150;
    p->quests[i].coin_reward=30; p->quests[i].premium=0;
    p->quests[i].skill_idx=3; i++;

    strcpy(p->quests[i].title, "River Walker");
    strcpy(p->quests[i].desc,  "Reach Level 5");
    p->quests[i].target=5; p->quests[i].xp_reward=200;
    p->quests[i].coin_reward=50; p->quests[i].premium=0;
    p->quests[i].skill_idx=-1; i++;

    strcpy(p->quests[i].title, "7-Day Flame");
    strcpy(p->quests[i].desc,  "Maintain a 7-day streak");
    p->quests[i].target=7; p->quests[i].xp_reward=250;
    p->quests[i].coin_reward=60; p->quests[i].premium=0;
    p->quests[i].skill_idx=6; i++;

    strcpy(p->quests[i].title, "Nile Conqueror");
    strcpy(p->quests[i].desc,  "Win 5 Nile Rapids games");
    p->quests[i].target=5; p->quests[i].xp_reward=180;
    p->quests[i].coin_reward=35; p->quests[i].premium=0;
    p->quests[i].skill_idx=4; i++;

    /* PREMIUM quests */
    strcpy(p->quests[i].title, "Gorilla Keeper");
    strcpy(p->quests[i].desc,  "[★] Complete 5 Gorilla Wisdom games");
    p->quests[i].target=5; p->quests[i].xp_reward=300;
    p->quests[i].coin_reward=75; p->quests[i].premium=1;
    p->quests[i].skill_idx=7; i++;

    strcpy(p->quests[i].title, "Meditation Master");
    strcpy(p->quests[i].desc,  "[★] Complete 10 meditation sessions");
    p->quests[i].target=10; p->quests[i].xp_reward=400;
    p->quests[i].coin_reward=100; p->quests[i].premium=1;
    p->quests[i].skill_idx=5; i++;

    strcpy(p->quests[i].title, "Ubuntu 50");
    strcpy(p->quests[i].desc,  "[★] Complete 50 sessions total");
    p->quests[i].target=50; p->quests[i].xp_reward=800;
    p->quests[i].coin_reward=200; p->quests[i].premium=1;
    p->quests[i].skill_idx=6; i++;

    strcpy(p->quests[i].title, "30-Day Legend");
    strcpy(p->quests[i].desc,  "[★] Maintain a 30-day streak");
    p->quests[i].target=30; p->quests[i].xp_reward=1000;
    p->quests[i].coin_reward=250; p->quests[i].premium=1;
    p->quests[i].skill_idx=-1; i++;
}

void refresh_quests(Player *p) {
    for (int i=0;i<p->quest_count;i++) {
        Quest *q = &p->quests[i];
        if (q->complete) continue;
        if (q->premium && !p->premium) continue;

        int prog = 0;
        switch (i) {
            case 0: prog = p->breaths;      break;
            case 1: prog = p->breaths;      break;
            case 2: prog = p->journals;     break;
            case 3: prog = p->grounds;      break;
            case 4: prog = p->affirmations; break;
            case 5: prog = p->level;        break;
            case 6: prog = p->streak;       break;
            case 7: prog = p->minigames_won;break;
            case 8: prog = p->minigames_won;break;
            case 9: prog = p->meditations;  break;
            case 10:prog = p->sessions;     break;
            case 11:prog = p->streak;       break;
        }
        q->progress = prog;

        if (!q->complete && q->progress >= q->target) {
            q->complete = 1;
            printf(LIME BOLD "\n  ✅ QUEST COMPLETE: %s\n" R, q->title);
            award_xp(p, q->xp_reward, "Quest Complete");
            p->coins += q->coin_reward;
            printf(GOLD "  💰 +%d Ubuntu Coins!\n" R, q->coin_reward);
            if (q->skill_idx >= 0) gain_skill_level(p, q->skill_idx);
            sleep(1);
        }
    }
}

/* ══════════════════════════ STREAK ════════════════════════════════ */
void update_streak(Player *p) {
    long today = epoch_day();
    if (p->last_day == 0) { p->streak=1; p->last_day=today; return; }
    long d = today - p->last_day;
    if      (d == 1) { p->streak++; p->last_day=today; }
    else if (d >  1) { p->streak=1; p->last_day=today; }
    check_badges(p);
    if (p->streak >= 3)
        printf(CRIMSON BOLD "  🔥 %d-DAY STREAK! " R CRIMSON "Keep it up!\n" R, p->streak);
}

/* ══════════════════════════ SAVE / LOAD ════════════════════════════ */
void save_game(Player *p) {
    FILE *f = fopen(SAVE_FILE, "wb");
    if (f) { fwrite(p, sizeof(Player), 1, f); fclose(f); }
}
int load_game(Player *p) {
    FILE *f = fopen(SAVE_FILE, "rb");
    if (!f) return 0;
    int ok = (fread(p, sizeof(Player), 1, f) == 1);
    fclose(f);
    return ok;
}

/* ══════════════════════════ BANNER ════════════════════════════════ */
void draw_logo(void) {
    printf(GOLD  "  ╔══════════════════════════════════════════════════════════╗\n");
    printf(GOLD  "  ║" ORANGE BOLD "  ██╗   ██╗██████╗ ██╗   ██╗███╗  ██╗████████╗██╗      " GOLD "║\n");
    printf(GOLD  "  ║" ORANGE BOLD "  ██║   ██║██╔══██╗██║   ██║████╗ ██║╚══██╔══╝██║      " GOLD "║\n");
    printf(GOLD  "  ║" ORANGE BOLD "  ██║   ██║██████╔╝██║   ██║██╔██╗██║   ██║   ██║      " GOLD "║\n");
    printf(GOLD  "  ║" ORANGE BOLD "  ╚██████╔╝██╔══██╗╚██████╔╝██║╚████║   ██║   ██║      " GOLD "║\n");
    printf(GOLD  "  ║" ORANGE BOLD "   ╚═════╝ ╚═════╝  ╚═════╝ ╚═╝ ╚═══╝   ╚═╝   ╚═╝     " GOLD "║\n");
    printf(EARTH "  ║      " SAVANNA BOLD "M I N D   G A R D E N  v3.0" R EARTH "                  ║\n");
    printf(FOREST"  ║   " IVORY "20-Level Mental Wellness RPG | Uganda & Africa" FOREST "     ║\n");
    printf(GOLD  "  ╚══════════════════════════════════════════════════════════╝\n" R);
}

void draw_hud(Player *p) {
    int xp_next  = (p->level < MAX_LEVELS) ? lvl_xp[p->level+1] : p->xp+1;
    int xp_this  = p->xp - lvl_xp[p->level];
    int xp_range = xp_next - lvl_xp[p->level];
    int filled   = (xp_range > 0) ? (xp_this * 24 / xp_range) : 24;

    printf(GOLD "  ┌──────────────────────────────────────────────────────┐\n");
    printf(GOLD "  │ " BOLD "%s Lv.%d  %s %s" R GOLD, lvl_icon[p->level], p->level, p->name,
           p->premium ? GOLD BOLD " [★ PREMIUM]" R GOLD : "");
    /* pad */
    int pad = 44 - (int)strlen(p->name) - (p->premium?10:0);
    for (int i=0;i<pad;i++) putchar(' ');
    printf("│\n");

    printf(GOLD "  │ " SAVANNA "XP [" R);
    for (int i=0;i<24;i++)
        printf(i < filled ? LIME "█" R : DIM "░" R);
    printf(SAVANNA "] %d/%d" R, p->xp, xp_next);
    printf(GOLD "       │\n");

    printf(GOLD "  │ " GOLD "💰%d  " CRIMSON "🔥%d-streak  " CYAN "🎯%d/%d lvl  " ORANGE "⚡%d energy" R GOLD,
           p->coins, p->streak, p->level, MAX_LEVELS, p->energy);
    printf("    │\n");

    /* Mood/Calm bars */
    printf(GOLD "  │ " ORANGE "Mood[" R);
    for (int i=0;i<10;i++) printf(i < p->mood ? ORANGE "▓" R : DIM "░" R);
    printf(ORANGE "]%d  " LAKE "Calm[" R, p->mood);
    for (int i=0;i<10;i++) printf(i < (10-p->anxiety) ? LAKE "▓" R : DIM "░" R);
    printf(LAKE "]%d" R GOLD "              │\n", 10-p->anxiety);

    printf(GOLD "  └──────────────────────────────────────────────────────┘\n" R);
}

/* ══════════════════════════ LEVEL MAP ══════════════════════════════ */
void show_level_map(Player *p) {
    cls();
    divider(GOLD,'═',60);
    printf(GOLD "  🗺  " SAVANNA BOLD "THE UBUNTU PATH — 20 Levels" R "\n");
    divider(GOLD,'═',60);
    printf(IVORY "  FREE: Levels 1–8  |  PREMIUM ★: Levels 9–20\n\n" R);

    for (int i=1;i<=MAX_LEVELS;i++) {
        int done    = (p->level > i);
        int current = (p->level == i);
        int locked  = (!done && !current && lvl_premium[i] && !p->premium);
        const char *col = current ? GOLD BOLD :
                          (done   ? LIME :
                          (locked ? PURPLE : IVORY));

        if (i == 9)
            printf(PURPLE "\n  ──── ★ PREMIUM ZONE ────────────────────────────\n\n" R);

        printf("  %s", col);
        printf("%s Lv.%2d  %-22s", current ? "▶" : " ", i, lvl_name[i]);
        printf(" %s", lvl_icon[i]);
        if (current)       printf(GOLD BOLD " ← YOU" R);
        else if (done)     printf(LIME " ✓" R);
        else if (locked)   printf(PURPLE " [★ %d XP]" R, lvl_xp[i]);
        else               printf(DIM " [%d XP]" R, lvl_xp[i]);
        printf("\n" R);
    }
    printf("\n");
    press_enter();
}

/* ══════════════════════════ GARDEN VIEW ════════════════════════════ */
void show_garden(Player *p) {
    cls();
    divider(FOREST,'═',60);
    printf(FOREST "  🌳  " JUNGLE BOLD "YOUR MIND GARDEN" R "\n");
    divider(FOREST,'═',60);

    /* ASCII art scaled to level */
    int stage = p->level / 4;
    if (stage > 4) stage = 4;

    const char *art[] = {
        EARTH  "\n       .  *  🌱  *  .\n   .   🌱  .   🌱  .   🌱\n",
        FOREST "\n    🌿   🌿   🌿   🌿\n   🌱  ·  🌿  ·  🌱  ·  🌿\n",
        JUNGLE "\n  🌲   🌲   🌲   🌲   🌲\n 🌿 🌱 🌿 🦋 🌿 🌱 🌿 🌿\n",
        FOREST "\n 🌳  🌳  🌳  🌳  🌳  🌳\n🌺 🦜 🌸 🦋 🌼 🌺 🦩 🌸 🦜\n",
        JUNGLE "\n✨🌳🦁🌳🦩🌳🌺🌳🦜🌳✨\n🌊🌿🦋🌸🥁🌸🦋🌿🎭🌿🌊\n"
    };
    printf("%s" R "\n\n", art[stage]);

    printf(GOLD "  Lv:  %d — %s %s\n" R, p->level, lvl_icon[p->level], lvl_name[p->level]);
    printf(SAVANNA "  XP:  %d  (total earned: %d)\n" R, p->xp, p->total_xp_earned);
    printf(LAKE   "  💰  %d coins  (total earned: %d)\n" R, p->coins, p->total_coins_earned);
    printf(CRIMSON "  🔥  %d-day streak\n" R, p->streak);
    printf(FOREST "  📅  %d sessions total\n" R, p->sessions);
    printf(CYAN   "  🧘  %d meditations  🌬 %d breaths  🌿 %d grounds\n" R,
           p->meditations, p->breaths, p->grounds);

    /* Skills */
    printf(GOLD "\n  Skill Tree:\n" R);
    for (int i=0;i<MAX_SKILLS;i++) {
        printf("  %-22s [", skill_names[i]);
        for (int j=0;j<5;j++)
            printf(j < p->skills[i].level ? LIME "★" R : DIM "☆" R);
        printf("]\n");
    }

    /* XP progress bar */
    int xp_next  = (p->level < MAX_LEVELS) ? lvl_xp[p->level+1] : p->xp+1;
    int xp_this  = p->xp - lvl_xp[p->level];
    int xp_range = xp_next - lvl_xp[p->level];
    int pct      = xp_range > 0 ? (xp_this * 100 / xp_range) : 100;
    int filled   = pct * 30 / 100;

    printf(SAVANNA "\n  Next Level [" R);
    for (int i=0;i<30;i++) printf(i < filled ? LIME "█" R : DIM "░" R);
    printf(SAVANNA "] %d%%\n\n" R, pct);
    press_enter();
}

/* ══════════════════════════ MOOD CHECK-IN ══════════════════════════ */
void mood_checkin(Player *p) {
    cls();
    divider(ORANGE,'═',58);
    printf(ORANGE "  🌅  " GOLD BOLD "DAILY CHECK-IN — Sunrise on the Nile" R "\n");
    divider(ORANGE,'═',58);

    printf(IVORY "\n  MOOD today? (1=very low, 10=wonderful)\n  > " R);
    int prev_mood = p->mood;
    p->mood = get_int(1,10);

    printf(ORANGE "\n  ANXIETY? (1=very calm, 10=very anxious)\n  > " R);
    p->anxiety = get_int(1,10);

    printf(FOREST "\n  ENERGY? (1=exhausted, 10=full of life)\n  > " R);
    p->energy = get_int(1,10);

    /* Level-specific bonus prompts */
    if (p->level >= 3) {
        printf(CYAN "\n  [Lv3+] What is ONE thing you are grateful for today?\n  > " R);
        char buf[128]; if (fgets(buf,sizeof(buf),stdin))
            printf(FOREST "  Gratitude plants a seed 🌱\n" R);
    }
    if (p->level >= 6) {
        printf(CYAN "\n  [Lv6+] Name one intention for today:\n  > " R);
        char buf[128]; if (fgets(buf,sizeof(buf),stdin))
            printf(FOREST "  Your intention is set. The savanna aligns with you.\n" R);
    }

    /* Mood trend feedback */
    if (p->mood <= 3) {
        printf(CRIMSON "\n  The baobab stands through the harshest drought.\n" R);
        printf(IVORY   "  Your feelings are valid. You are not alone.\n" R);
        p->low_mood_checkins++;
    } else if (p->mood > prev_mood) {
        printf(LIME "\n  You're rising — like the Nile in the wet season! 📈\n" R);
    } else {
        printf(SAVANNA "\n  You are on the path. Keep walking. 🌾\n" R);
    }

    update_streak(p);
    p->sessions++;
    award_xp(p, 25 + p->level, "Daily Check-In");
    check_badges(p);
    refresh_quests(p);
    press_enter();
}

/* ══════════════════════════ BREATHING ══════════════════════════════ */
void breathing_exercise(Player *p) {
    cls();
    divider(LAKE,'═',58);
    printf(LAKE "  🌊  " SKY BOLD "BREATHING — Lake Victoria Tide" R "\n");
    divider(LAKE,'═',58);

    /* Choose technique based on level */
    int technique = 1; /* default: 4-7-8 */
    if (p->level >= 5) {
        printf(IVORY "\n  Choose breathing technique:\n");
        printf(LAKE  "  1. 4-7-8 (Anxiety relief)\n");
        printf(CYAN  "  2. Box Breathing (Focus & calm)\n");
        if (p->level >= 7)
            printf(FOREST "  3. Coherent Breathing (Deep balance) [Lv7+]\n");
        printf(SAVANNA "  > " R);
        technique = get_int(1, p->level >= 7 ? 3 : 2);
    }

    int cycles = 4 + (p->level / 3);
    printf(IVORY "\n  Cycles: %d\n\n" R, cycles);
    press_enter();

    for (int c=1;c<=cycles;c++) {
        printf(LAKE "  — Cycle %d/%d —\n" R, c, cycles);

        if (technique == 1) { /* 4-7-8 */
            for (int s=4;s>=1;s--) { printf("\r  " FOREST "🌬 INHALE  %ds   " R,s); fflush(stdout); sleep(1); }
            for (int s=7;s>=1;s--) { printf("\r  " GOLD   "✨ HOLD    %ds   " R,s); fflush(stdout); sleep(1); }
            for (int s=8;s>=1;s--) { printf("\r  " LAKE   "🌊 EXHALE  %ds   " R,s); fflush(stdout); sleep(1); }
        } else if (technique == 2) { /* Box */
            for (int s=4;s>=1;s--) { printf("\r  " FOREST "🌬 INHALE  %ds   " R,s); fflush(stdout); sleep(1); }
            for (int s=4;s>=1;s--) { printf("\r  " GOLD   "✨ HOLD IN %ds   " R,s); fflush(stdout); sleep(1); }
            for (int s=4;s>=1;s--) { printf("\r  " LAKE   "🌊 EXHALE  %ds   " R,s); fflush(stdout); sleep(1); }
            for (int s=4;s>=1;s--) { printf("\r  " PURPLE "⬜ HOLD OUT%ds   " R,s); fflush(stdout); sleep(1); }
        } else { /* Coherent: 5-5 */
            for (int s=5;s>=1;s--) { printf("\r  " FOREST "🌬 INHALE  %ds   " R,s); fflush(stdout); sleep(1); }
            for (int s=5;s>=1;s--) { printf("\r  " LAKE   "🌊 EXHALE  %ds   " R,s); fflush(stdout); sleep(1); }
        }
        printf("\n  " LIME "✓ Cycle %d complete\n" R, c);
    }

    p->anxiety = (p->anxiety>2) ? p->anxiety-2 : 1;
    p->mood    = (p->mood<10)   ? p->mood+1    : 10;
    p->breaths++;
    p->sessions++;
    award_xp(p, 35 + p->level, "Breathing Session");
    gain_skill_level(p, 0); /* breathing skill */
    check_badges(p);
    refresh_quests(p);
    press_enter();
}

/* ══════════════════════════ AFFIRMATION ════════════════════════════ */
void daily_affirmation(Player *p) {
    cls();
    divider(PURPLE,'═',58);
    printf(PURPLE "  🦩  " PINK BOLD "AFFIRMATION — Words of the Ancestors" R "\n");
    divider(PURPLE,'═',58);

    srand((unsigned)time(NULL) + p->affirmations);

    const char *aff;
    if (p->premium) {
        int total = AFF_FREE_N + AFF_PREM_N;
        int idx   = rand() % total;
        aff = (idx < AFF_FREE_N) ? aff_free[idx] : aff_premium[idx - AFF_FREE_N];
    } else {
        aff = aff_free[rand() % AFF_FREE_N];
    }

    printf(IVORY "\n  Close your eyes. Breathe in.\n\n" R);
    sleep(2);
    divider(GOLD,'─',58);
    printf("\n  " GOLD BOLD "«  "); tw(GOLD BOLD, aff, 22); printf("  »\n\n" R);
    divider(GOLD,'─',58);

    /* Say it 3 times practice */
    if (p->level >= 2) {
        printf(IVORY "\n  Repeat it 3 times — aloud if you can:\n" R);
        for (int i=1;i<=3;i++) {
            printf(GOLD "  [%d] " R, i); press_enter();
            printf(LIME "  ✓ You spoke it into existence.\n" R);
        }
    }

    /* Proverb bonus for Lv4+ */
    if (p->level >= 4) {
        int pi = rand() % PROVERB_N;
        printf(EARTH "\n  ── African Wisdom (Lv4+ bonus) ──────────────\n");
        printf(SAVANNA "  \"%s\"\n" DIM "  — %s (%s)\n\n" R,
               proverbs[pi].text, proverbs[pi].author, proverbs[pi].origin);
    }

    /* Lv6+: write a reflection */
    if (p->level >= 6) {
        printf(CYAN "\n  [Lv6+] How does this affirmation apply to your life today?\n  > " R);
        char buf[256]; if (fgets(buf,sizeof(buf),stdin))
            printf(FOREST "  Your reflection honours your journey. 🌿\n" R);
    }

    p->affirmations++;
    p->sessions++;
    award_xp(p, 28 + p->level, "Affirmation");
    gain_skill_level(p, 3);
    check_badges(p);
    refresh_quests(p);
    press_enter();
}

/* ══════════════════════════ GROUNDING ══════════════════════════════ */
void grounding_exercise(Player *p) {
    cls();
    divider(FOREST,'═',58);
    printf(FOREST "  🌿  " JUNGLE BOLD "GROUNDING — Bwindi Forest (5-4-3-2-1)" R "\n");
    divider(FOREST,'═',58);
    printf(IVORY "\n  You are in the Bwindi Impenetrable Forest.\n");
    printf(       "  Mountain gorillas rest nearby. You are safe.\n\n" R);

    struct { int n; const char *s; const char *c; } senses[] = {
        {5,"things you SEE",   LAKE  },
        {4,"things you TOUCH", EARTH },
        {3,"things you HEAR",  FOREST},
        {2,"things you SMELL", SAVANNA},
        {1,"thing you TASTE",  ORANGE}
    };
    for (int i=0;i<5;i++) {
        printf("%s\n  %d — Name %s:\n" R, senses[i].c, senses[i].n, senses[i].s);
        char buf[128];
        for (int j=0;j<senses[i].n;j++) {
            printf(GOLD "  %d. " R, j+1); fflush(stdout);
            if (fgets(buf,sizeof(buf),stdin)) printf(FOREST "     ✓ Noticed.\n" R);
        }
    }

    /* Lv4+: body scan */
    if (p->level >= 4) {
        printf(CYAN "\n  [Lv4+] Body Scan — where do you feel tension?\n  > " R);
        char buf[128]; if (fgets(buf,sizeof(buf),stdin)) {
            printf(FOREST "  Breathe into that area. Let it soften with each exhale.\n" R);
            sleep(3);
        }
    }

    /* Lv7+: Positive memory anchor */
    if (p->level >= 7) {
        printf(CYAN "\n  [Lv7+] Recall a memory where you felt truly safe and happy.\n");
        printf("  Describe it briefly:\n  > " R);
        char buf[256]; if (fgets(buf,sizeof(buf),stdin)) {
            printf(GOLD "  That feeling is always available to you. It lives inside you.\n" R);
            sleep(2);
        }
    }

    printf(GOLD "\n  ✅ You are HERE. You are PRESENT. You are SAFE.\n\n" R);
    p->anxiety = (p->anxiety>3) ? p->anxiety-3 : 1;
    p->mood    = (p->mood<10)   ? p->mood+2    : 10;
    p->grounds++;
    p->sessions++;
    award_xp(p, 45 + p->level, "Grounding Session");
    gain_skill_level(p, 1);
    check_badges(p);
    refresh_quests(p);
    press_enter();
}

/* ══════════════════════════ MEDITATION (Lv3+ free / all premium) ═══ */
void meditation_session(Player *p) {
    if (p->level < 3 && !p->premium) {
        printf(PURPLE "\n  🔒 Meditation unlocks at Level 3.\n" R);
        sleep(1); return;
    }
    cls();
    divider(INDIGO,'═',58);
    printf(INDIGO "  🧘  " PURPLE BOLD "MEDITATION — Stillness of Lake Bunyonyi" R "\n");
    divider(INDIGO,'═',58);

    int max_script = p->premium ? MED_SCRIPTS_TOTAL : MED_SCRIPTS_FREE;
    printf(IVORY "\n  Choose a meditation:\n");
    printf(INDIGO "  1. Basic Awareness (all players)\n" R);
    if (p->premium) {
        printf(PURPLE "  2. Savanna at Dawn          [★]\n");
        printf(PURPLE "  3. Bwindi Forest Walk       [★]\n");
        printf(PURPLE "  4. Lake Victoria Reflection [★]\n");
        printf(PURPLE "  5. Ancestral Fire           [★]\n");
    } else {
        printf(DIM   "  2–5. Unlock with Premium ★\n" R);
    }
    printf(SAVANNA "  > " R);
    int choice = get_int(1, max_script);

    /* Duration */
    int mins = 3;
    if (p->level >= 5) {
        printf(IVORY "\n  Duration? (1=3min  2=5min  3=10min): " R);
        int d = get_int(1,3);
        mins = (d==1)?3:(d==2)?5:10;
    }

    cls();
    printf(INDIGO "\n  🧘 Beginning %d-minute meditation...\n\n" R, mins);
    printf(IVORY "  Read slowly. Breathe with each line.\n\n" R);
    divider(PURPLE,'─',58);

    /* Print the chosen script slowly */
    const char *script = meditation_scripts[choice-1];
    while (*script) {
        if (*script == '\n') { printf("\n"); sleep(1); }
        else { putchar(*script); fflush(stdout); ms(30); }
        script++;
    }
    printf("\n");
    divider(PURPLE,'─',58);

    /* Timer countdown */
    printf(INDIGO "\n  Sit in stillness now...\n" R);
    for (int s = mins * 60; s > 0; s--) {
        int m2 = s / 60, s2 = s % 60;
        printf("\r  " PURPLE "⏱  %02d:%02d remaining  " R, m2, s2);
        fflush(stdout);
        sleep(1);
    }
    printf("\r  " LIME "✅ Meditation complete.                 \n\n" R);

    p->meditations++;
    p->sessions++;
    p->med.sessions_done++;
    p->med.total_minutes += mins;
    p->anxiety = (p->anxiety>1) ? p->anxiety-1 : 1;
    p->mood    = (p->mood<10)   ? p->mood+1    : 10;
    award_xp(p, 50 + mins*5 + p->level, "Meditation Session");
    gain_skill_level(p, 5);
    check_badges(p);
    refresh_quests(p);
    press_enter();
}

/* ══════════════════════════ MINI-GAME: NILE RAPIDS ════════════════ */
void minigame_nile(Player *p) {
    if (p->level < 3) {
        printf(PURPLE "\n  🔒 Nile Rapids unlocks at Level 3!\n" R);
        sleep(1); return;
    }
    cls();
    divider(LAKE,'═',58);
    printf(LAKE "  🌊  " CYAN BOLD "NILE RAPIDS — Word Focus Game [Lv3+]" R "\n");
    divider(LAKE,'═',58);
    printf(IVORY "\n  The Nile flows fast. Type each word exactly.\n");
    printf("  Higher levels unlock harder rounds!\n\n" R);
    press_enter();

    const char *easy[]  = {"BREATHE","PEACE","UBUNTU","NILE","CRANE",
                            "FOREST","CALM","RISE","HEAL","HOPE"};
    const char *medium[]= {"BWINDI","KAMPALA","SAVANNA","GORILLA","MOUNTAIN",
                            "VICTORIA","RWENZORI","ANCESTOR","RESILIENT","UBUNTU"};
    const char *hard[]  = {"IMPENETRABLE","AFFIRMATION","GROUNDING","MINDFULNESS",
                            "PERSEVERANCE","RWENZORIMTS","BUNYONYILAKE","TRANSFORMATION"};

    int rounds = 6 + (p->level / 3);
    int score  = 0;
    srand((unsigned)time(NULL));

    for (int r=0;r<rounds;r++) {
        const char *w;
        if      (p->level <= 4) w = easy  [rand()%10];
        else if (p->level <= 8) w = medium[rand()%10];
        else                    w = hard  [rand()%8];

        printf(GOLD "\n  Round %d/%d — Type: " BOLD CYAN "%s" R GOLD " →  " R, r+1, rounds, w);
        fflush(stdout);

        char buf[64];
        if (fgets(buf,sizeof(buf),stdin)) {
            int len = strlen(buf);
            if (len>0 && buf[len-1]=='\n') buf[len-1]='\0';
            if (strcmp(buf,w)==0) {
                score++;
                printf(LIME "  ✓ Correct!\n" R);
            } else {
                printf(CRIMSON "  ✗ It was: %s\n" R, w);
            }
        }
    }

    int perfect = (score == rounds);
    printf(GOLD "\n  Score: %d/%d\n" R, score, rounds);
    if (perfect) {
        printf(LIME BOLD "  🎯 PERFECT! You ride the Nile like a master!\n" R);
        p->perfect_games++;
    }

    int xp = score * 18 + (perfect ? 80 : 0);
    if (score > 0) p->minigames_won++;
    p->sessions++;
    award_xp(p, xp, "Nile Rapids");
    check_badges(p);
    refresh_quests(p);
    press_enter();
}

/* ══════════════════════════ MINI-GAME: GORILLA MEMORY (Lv5+) ══════ */
void minigame_gorilla(Player *p) {
    if (p->level < 5) {
        printf(PURPLE "\n  🔒 Gorilla Wisdom unlocks at Level 5!\n" R);
        sleep(1); return;
    }
    cls();
    divider(EARTH,'═',58);
    printf(EARTH "  🦍  " SAVANNA BOLD "GORILLA WISDOM — Memory Game [Lv5+]" R "\n");
    divider(EARTH,'═',58);
    printf(IVORY "\n  Remember the plant sequence shown by the gorilla!\n");
    printf("  (Premium players get longer sequences)\n\n" R);
    press_enter();

    const char *plants[] = {"🌿","🍃","🌺","🦋","🌸","🍀","🌼","🌻","🌱","🌾"};
    int rounds     = p->premium ? 5 : 3;
    int start_len  = p->premium ? 3 : 2;
    int score      = 0;
    int seq[12];
    srand((unsigned)time(NULL)+1);

    for (int r=0;r<rounds;r++) {
        int len = start_len + r;
        for (int i=0;i<len;i++) seq[i] = rand()%10;

        printf(GOLD "\n  Round %d — Sequence: ", r+1);
        for (int i=0;i<len;i++) printf("%s ", plants[seq[i]]);
        printf(R "\n");
        sleep(2);
        printf(DIM "  (Hidden now...)\n\n" R);
        sleep(1);

        printf(IVORY "  Plants: ");
        for (int i=0;i<10;i++) printf("%d:%s ", i, plants[i]);
        printf("\n");
        printf(CYAN "  Enter %d numbers (space-separated): " R, len);

        char buf[64]; int ok=1;
        if (fgets(buf,sizeof(buf),stdin)) {
            char *tok = strtok(buf," \n");
            for (int i=0;i<len&&tok;i++,tok=strtok(NULL," \n"))
                if (atoi(tok)!=seq[i]) { ok=0; break; }
            if (!tok && ok) ok=1; else if (!tok) ok=0;
        }
        if (ok) { score++; printf(LIME "  ✓ Perfect recall!\n" R); }
        else         printf(CRIMSON "  ✗ The gorilla remembers better!\n" R);
    }

    int perfect = (score == rounds);
    printf(GOLD "\n  Score: %d/%d\n" R, score, rounds);
    if (perfect && p->premium) {
        give_badge(p, BADGE_GORILLA_MASTER);
        printf(LIME BOLD "  🦍 You think like a mountain gorilla!\n" R);
    }
    int xp = score * 25 + (perfect ? 100 : 0);
    if (score > 0) p->minigames_won++;
    p->sessions++;
    award_xp(p, xp, "Gorilla Wisdom");
    refresh_quests(p);
    press_enter();
}

/* ══════════════════════════ MINI-GAME: DRUM RHYTHM (Lv6+) ═════════ */
void minigame_drum(Player *p) {
    if (p->level < 6) {
        printf(PURPLE "\n  🔒 Drum Rhythm unlocks at Level 6!\n" R);
        sleep(1); return;
    }
    cls();
    divider(EARTH,'═',58);
    printf(EARTH "  🥁  " AMBER BOLD "DRUM RHYTHM — Kampala Beat [Lv6+]" R "\n");
    divider(EARTH,'═',58);
    printf(IVORY "\n  Match the Kampala drum pattern!\n");
    printf(DIM   "  B=Bass  S=Snare  H=HiHat  R=Rest\n\n" R);
    press_enter();

    /* Patterns of increasing complexity */
    const char *patterns_easy[]  = {"BSBS","BBSS","BSHB","BSHR"};
    const char *patterns_hard[]  = {"BBSSHR","BSHBSR","BBRSSR","BSHBRB"};
    int rounds = 5, score = 0;
    srand((unsigned)time(NULL)+2);

    for (int r=0;r<rounds;r++) {
        const char *pat = (p->level <= 8) ?
            patterns_easy[rand()%4] : patterns_hard[rand()%4];

        printf(AMBER "\n  Beat %d: " BOLD "%s" R AMBER " — Type it!\n  > " R, r+1, pat);
        fflush(stdout);

        char buf[32];
        if (fgets(buf,sizeof(buf),stdin)) {
            int len = strlen(buf);
            if (len>0&&buf[len-1]=='\n') buf[len-1]='\0';
            /* make uppercase */
            for (int i=0;buf[i];i++) buf[i]=toupper(buf[i]);
            if (strcmp(buf,pat)==0) {
                score++; printf(LIME "  🥁 You've got rhythm!\n" R);
            } else {
                printf(CRIMSON "  ✗ Pattern was: %s\n" R, pat);
            }
        }
    }

    int perfect = (score==rounds);
    printf(GOLD "\n  Score: %d/%d\n" R, score, rounds);
    if (perfect) {
        give_badge(p, BADGE_DRUM_KING);
        printf(AMBER BOLD "  🥁 DRUM KING! Kampala sings your name!\n" R);
    }
    int xp = score * 20 + (perfect ? 90 : 0);
    p->sessions++;
    award_xp(p, xp, "Drum Rhythm");
    check_badges(p);
    refresh_quests(p);
    press_enter();
}

/* ══════════════════════════ JOURNAL ════════════════════════════════ */
void journal_entry(Player *p) {
    cls();
    divider(EARTH,'═',58);
    printf(EARTH "  📖  " SAVANNA BOLD "JOURNAL — Your Story Matters" R "\n");
    divider(EARTH,'═',58);

    const char *prompts_free[] = {
        "What moment today felt like sunlight on Ugandan hills?",
        "What is one thing you would tell your younger self?",
        "Describe someone who embodies Ubuntu for you.",
        "What fear would you release to the River Nile today?",
        "How have you changed since you started this garden?"
    };
    const char *prompts_premium[] = {
        "If your mind were a landscape in Uganda, what would it look like right now?",
        "Write a letter from your future, healed self to who you are today.",
        "What story about yourself no longer serves you? How would you rewrite it?",
        "Describe a wound that taught you strength.",
        "What does Ubuntu mean in your own relationships today?"
    };

    srand((unsigned)time(NULL)+p->journals);

    if (p->level >= 2) {
        const char *prompt;
        if (p->premium && rand()%2)
            prompt = prompts_premium[rand()%5];
        else
            prompt = prompts_free[rand()%5];
        printf(CYAN "\n  Prompt: " ITALIC "\"%s\"\n\n" R, prompt);
    }

    if (p->journal_count >= JOURNAL_CAP) {
        printf(ORANGE "\n  Journal full! Recent entries:\n\n" R);
        int start = p->journal_count >= 5 ? p->journal_count-5 : 0;
        for (int i=start;i<p->journal_count&&i<JOURNAL_CAP;i++)
            printf(GOLD "  [%d] " IVORY "%s\n" R, i+1, p->journal[i]);
        press_enter(); return;
    }

    printf(SAVANNA "\n  Write freely (press Enter when done):\n  > " R);
    if (fgets(p->journal[p->journal_count], MAX_JOURNAL, stdin)) {
        int len = strlen(p->journal[p->journal_count]);
        if (len>0 && p->journal[p->journal_count][len-1]=='\n')
            p->journal[p->journal_count][len-1]='\0';
        p->journal_count++;
        p->journals++;
    }

    p->sessions++;
    award_xp(p, 40 + p->level*2, "Journal Entry");
    gain_skill_level(p, 2);
    check_badges(p);
    refresh_quests(p);
    printf(FOREST "\n  The ancestors hear every word. 🌿\n" R);
    press_enter();
}

/* ══════════════════════════ QUESTS SCREEN ══════════════════════════ */
void show_quests(Player *p) {
    cls();
    divider(CYAN,'═',58);
    printf(CYAN "  🎯  " GOLD BOLD "QUESTS — Path of the Ubuntu Warrior" R "\n");
    divider(CYAN,'═',58);
    printf("\n");
    refresh_quests(p);

    for (int i=0;i<p->quest_count;i++) {
        Quest *q = &p->quests[i];
        if (q->premium && !p->premium) {
            printf(PURPLE "  🔒 [★] %-30s PREMIUM\n" R, q->title);
            continue;
        }
        const char *col = q->complete ? LIME : GOLD;
        printf("%s  %s %-30s" R, col, q->complete?"✅":"🎯", q->title);
        printf(DIM " [%d/%d]\n" R, q->progress, q->target);
        printf(IVORY "     %s  " GOLD "+%dXP  💰+%d\n\n" R,
               q->desc, q->xp_reward, q->coin_reward);
    }
    press_enter();
}

/* ══════════════════════════ BADGES SCREEN ══════════════════════════ */
void show_badges(Player *p) {
    cls();
    divider(GOLD,'═',58);
    printf(GOLD "  🏅  " SAVANNA BOLD "BADGES — Honours of the Journey" R "\n");
    divider(GOLD,'═',58);
    printf("\n");
    int earned=0;
    for (int i=0;i<MAX_BADGES;i++) {
        if (p->badges[i]) earned++;
        int locked = (badge_premium[i] && !p->premium);
        if (locked && !p->badges[i]) {
            printf(PURPLE "  🔒 %-22s [★ Premium]\n" R, badge_name[i]);
        } else {
            printf("%s  %s %-22s  %s\n" R,
                   p->badges[i] ? GOLD : DIM,
                   badge_icon[i], badge_name[i],
                   p->badges[i] ? "✓" : "○");
        }
    }
    printf(SAVANNA "\n  Badges earned: %d/%d\n" R, earned, MAX_BADGES);
    press_enter();
}

/* ══════════════════════════ SKILLS SCREEN ══════════════════════════ */
void show_skills(Player *p) {
    cls();
    divider(TEAL,'═',58);
    printf(TEAL "  📈  " CYAN BOLD "SKILL TREE — Your Healing Disciplines" R "\n");
    divider(TEAL,'═',58);
    printf(IVORY "\n  Each skill levels up by completing its activity.\n");
    printf("  Higher skill level = more XP from that activity.\n\n" R);

    for (int i=0;i<MAX_SKILLS;i++) {
        printf(CYAN "  %-24s Lv.%d  [" R, skill_names[i], p->skills[i].level);
        for (int j=0;j<5;j++)
            printf(j < p->skills[i].level ? LIME "★" R : DIM "☆" R);
        printf("]  +" LIME "%d%%" R " XP bonus\n", p->skills[i].level*2);
    }
    printf(GOLD "\n  Total XP bonus from skills: +%d%%\n" R, skill_xp_bonus(p));
    press_enter();
}

/* ══════════════════════════ COIN SHOP ══════════════════════════════ */
void coin_shop(Player *p) {
    cls();
    divider(GOLD,'═',58);
    printf(GOLD "  💰  " SAVANNA BOLD "UBUNTU COIN SHOP" R "\n");
    divider(GOLD,'═',58);
    printf(IVORY "\n  You have " GOLD BOLD "%d Ubuntu Coins" R ".\n\n", p->coins);

    printf(GOLD "  ┌───────────────────────────────────────────────┐\n");
    printf(GOLD "  │ " ORANGE "1. Mood Boost      " IVORY "— 30 coins  (+2 mood)   " GOLD "│\n");
    printf(GOLD "  │ " LAKE   "2. Calm Wave       " IVORY "— 30 coins  (-2 anxiety)" GOLD "│\n");
    printf(GOLD "  │ " LIME   "3. Energy Surge    " IVORY "— 40 coins  (+3 energy) " GOLD "│\n");
    printf(GOLD "  │ " CYAN   "4. XP Boost        " IVORY "— 60 coins  (+80 XP)   " GOLD "│\n");
    printf(GOLD "  │ " PINK   "5. Skill Boost     " IVORY "— 80 coins  (skill +1)  " GOLD "│\n");
    printf(GOLD "  │ " PURPLE "6. Unlock Meditation" IVORY "— 200 coins (free only)" GOLD "│\n");
    printf(GOLD "  │ " DIM    "7. Back" GOLD "                                    │\n");
    printf(GOLD "  └───────────────────────────────────────────────┘\n\n" R);
    printf(SAVANNA "  Choose: " R);

    int c = get_int(1,7);
    switch (c) {
        case 1:
            if (p->coins>=30){p->coins-=30;p->mood=(p->mood+2>10)?10:p->mood+2;
            printf(LIME "  Mood boosted! 🌅\n" R);}
            else printf(CRIMSON "  Not enough coins!\n" R); break;
        case 2:
            if (p->coins>=30){p->coins-=30;p->anxiety=(p->anxiety-2<1)?1:p->anxiety-2;
            printf(LIME "  Calm wave! 🌊\n" R);}
            else printf(CRIMSON "  Not enough coins!\n" R); break;
        case 3:
            if (p->coins>=40){p->coins-=40;p->energy=(p->energy+3>10)?10:p->energy+3;
            printf(LIME "  Energy surge! ⚡\n" R);}
            else printf(CRIMSON "  Not enough coins!\n" R); break;
        case 4:
            if (p->coins>=60){p->coins-=60;award_xp(p,80,"XP Boost Item");}
            else printf(CRIMSON "  Not enough coins!\n" R); break;
        case 5:
            if (p->coins>=80){
                p->coins-=80;
                printf(PINK "  Which skill? (1-8):\n" R);
                for (int i=0;i<MAX_SKILLS;i++) printf("  %d. %s\n",i+1,skill_names[i]);
                printf("  > " R);
                int si = get_int(1,8)-1;
                gain_skill_level(p,si);
            } else printf(CRIMSON "  Not enough coins!\n" R); break;
        case 6:
            if (p->premium){printf(GOLD "  You already have Premium! All meditations unlocked.\n" R);}
            else if (p->coins>=200){
                p->coins-=200; p->med.unlocked=1;
                printf(LIME "  Basic meditation unlocked via coins! 🧘\n" R);
            } else printf(CRIMSON "  Need 200 coins. Keep playing!\n" R); break;
        case 7: break;
    }
    sleep(1);
}

/* ══════════════════════════ CRISIS ══════════════════════════════════ */
void crisis_support(void) {
    cls();
    divider(CRIMSON,'═',58);
    printf(CRIMSON "  💛  " ORANGE BOLD "YOU ARE NOT ALONE" R "\n");
    divider(CRIMSON,'═',58);
    printf(GOLD   "\n  🇺🇬 UGANDA:\n" R);
    printf(IVORY  "  • Butabika National Referral Hospital:\n");
    printf(SAVANNA "    +256 414 504 375 (Direct)\n");
    printf(SAVANNA "    0800 211 306 (Toll Free)\n\n" R);
    printf(IVORY  "  • TASO Uganda: taso.go.ug\n\n" R);
    printf(GOLD   "  🌍 INTERNATIONAL:\n" R);
    printf(IVORY  "  • Befrienders Worldwide: befrienders.org\n");
    printf(IVORY  "  • Crisis Text Line: crisistextline.org\n");
    printf(IVORY  "  • WHO Mental Health: who.int/mental_health\n\n" R);
    printf(CRIMSON "  If in immediate danger — call emergency services NOW.\n\n" R);
    printf(GOLD   "  You are worthy of help. Reaching out is courage.\n\n" R);
    press_enter();
}

/* ══════════════════════════ PAYMENT / PREMIUM ══════════════════════ */
void payment_screen(Player *p) {
    cls();
    divider(PURPLE,'═',58);
    printf(PURPLE "  ★  " GOLD BOLD "UBUNTU MIND GARDEN — PREMIUM" R "\n");
    divider(PURPLE,'═',58);
    printf("\n");

    if (p->premium) {
        printf(GOLD BOLD "  ✨ You are a PREMIUM member!\n\n" R);
        printf(IVORY "  Your premium status unlocks:\n");
        printf(LIME  "  ✓ Levels 9–20  ✓ All meditations  ✓ All badges\n");
        printf(LIME  "  ✓ 16 premium affirmations  ✓ Gorilla full game\n");
        printf(LIME  "  ✓ Advanced journal prompts  ✓ All quests\n\n" R);
        printf(FOREST "  10%% of your subscription supports mental health\n");
        printf("  programs across Uganda. Thank you. 🌍\n\n" R);
        press_enter(); return;
    }

    printf(IVORY "  The game is FREE forever for Levels 1–8.\n");
    printf("  Premium unlocks Levels 9–20 and bonus content.\n");
    printf(DIM   "  No pressure — play free as long as you like.\n\n" R);

    printf(GOLD BOLD "  What Premium adds:\n" R);
    printf(LIME "  ✓ 12 more levels (9–20)\n");
    printf(LIME "  ✓ 4 meditation scripts (Savanna, Bwindi, Lake, Fire)\n");
    printf(LIME "  ✓ 16 extra affirmations from the ancestors\n");
    printf(LIME "  ✓ Full Gorilla Wisdom memory game (5 rounds)\n");
    printf(LIME "  ✓ Advanced journal prompts\n");
    printf(LIME "  ✓ Premium badges and quests\n");
    printf(LIME "  ✓ Ubuntu Sage features at Lv16+\n\n" R);

    divider(GOLD,'─',58);
    printf(GOLD "\n  PLANS:\n\n");
    printf(GOLD "  ┌─────────────────────────────────────────────────┐\n");
    printf(GOLD "  │ 1. " LIME  BOLD "Monthly   — UGX 15,000  / USD 4.99  " GOLD "         │\n");
    printf(GOLD "  │ 2. " CYAN  BOLD "Annual    — UGX 120,000 / USD 34.99 " GOLD " (save40%)│\n");
    printf(GOLD "  │ 3. " AMBER BOLD "Lifetime  — UGX 250,000 / USD 69.99 " GOLD "         │\n");
    printf(GOLD "  │ 4. " DIM   "No thanks — continue free" GOLD "                  │\n");
    printf(GOLD "  └─────────────────────────────────────────────────┘\n\n" R);
    printf(SAVANNA "  Choose: " R);

    int plan = get_int(1,4);
    if (plan==4) {
        printf(IVORY "\n  No problem! The garden is still yours. 🌿\n" R);
        sleep(1); return;
    }

    const char *pname[] = {"","Monthly","Annual","Lifetime"};
    const char *price[] = {"","UGX 15,000 / USD 4.99","UGX 120,000 / USD 34.99",
                              "UGX 250,000 / USD 69.99"};
    cls();
    divider(GOLD,'═',58);
    printf(GOLD "  ★  CHECKOUT — %s Plan\n" R, pname[plan]);
    divider(GOLD,'═',58);
    printf(IVORY "\n  Plan:   " GOLD BOLD "%s\n" R, pname[plan]);
    printf(IVORY "  Price:  " GOLD BOLD "%s\n\n" R, price[plan]);

    printf(SAVANNA "  PAYMENT METHODS:\n\n");
    printf(GOLD "  ┌──────────────────────────────────────────────────┐\n");
    printf(GOLD "  │ A. " ORANGE "Mobile Money — MTN / Airtel Uganda        " GOLD " │\n");
    printf(GOLD "  │ B. " CYAN   "Card — Visa / Mastercard                  " GOLD " │\n");
    printf(GOLD "  │ C. " LIME   "PayPal / Stripe                           " GOLD " │\n");
    printf(GOLD "  │ D. " PURPLE "Flutterwave — Pan-Africa (M-Pesa, etc.)   " GOLD " │\n");
    printf(GOLD "  │ E. " DIM    "Cancel" GOLD "                                     │\n");
    printf(GOLD "  └──────────────────────────────────────────────────┘\n\n" R);
    printf(IVORY "  Choose (A/B/C/D/E): " R);

    char ch = get_char();
    if (ch=='E') { printf(ORANGE "\n  Cancelled. Come back anytime.\n" R); sleep(1); return; }

    printf("\n");
    if      (ch=='A') {
        printf(ORANGE "  📱 MOBILE MONEY\n\n" R);
        printf(IVORY  "  MTN:    *165*3*1# → Ref: UMG-%s\n", p->name);
        printf(IVORY  "  Airtel: *185*9#   → Ref: UMG-%s\n\n", p->name);
        printf(IVORY  "  Price: " GOLD "%s\n\n" R, price[plan]);
    } else if (ch=='B') {
        printf(CYAN   "  💳 CARD PAYMENT\n\n" R);
        printf(IVORY  "  Visit: " CYAN "https://pay.ubuntumindgarden.ug\n" R);
        printf(IVORY  "  Code: " GOLD "UMG-%s-%d\n\n" R, p->name, (int)time(NULL)%9999);
    } else if (ch=='C') {
        printf(LIME   "  🌐 PAYPAL / STRIPE\n\n" R);
        printf(IVORY  "  PayPal: paypal.me/UbuntuMindGarden\n");
        printf(IVORY  "  Stripe: https://buy.stripe.com/ubuntumindgarden\n\n" R);
    } else {
        printf(PURPLE "  🌍 FLUTTERWAVE (Pan-Africa)\n\n" R);
        printf(IVORY  "  Supports: M-Pesa, MTN, Airtel, Zamtel,\n");
        printf(IVORY  "            Bank Transfer, Card\n");
        printf(IVORY  "  Link: " PURPLE "https://flutterwave.com/pay/ubuntumindgarden\n\n" R);
    }

    printf(IVORY "  Enter transaction/confirmation ID (or CANCEL):\n  > " R);
    char txn[64];
    if (fgets(txn,sizeof(txn),stdin) && strlen(txn)>1
        && strncmp(txn,"CANCEL",6)!=0) {
        int len=strlen(txn); if(len>0&&txn[len-1]=='\n') txn[len-1]='\0';

        printf(GOLD BOLD "\n  ╔══════════════════════════════════════╗\n");
        printf(GOLD BOLD "  ║  ✨ PAYMENT RECEIVED — THANK YOU!  ✨║\n");
        printf(GOLD BOLD "  ╚══════════════════════════════════════╝\n\n" R);
        printf(IVORY "  Ref: " GOLD "%s\n" R, txn);
        printf(IVORY "  Account upgraded to " GOLD BOLD "★ PREMIUM\n\n" R);
        printf(FOREST "  🌍 10%% supports mental health in Uganda.\n" R);
        sleep(2);

        p->premium = 1;
        award_xp(p, 300, "Premium Welcome Bonus");
        p->coins += 150;
        printf(GOLD "  🎁 +150 Welcome Coins added!\n" R);
        sleep(1);
    } else {
        printf(ORANGE "\n  Payment cancelled. Free mode continues. 🌿\n" R);
    }
    press_enter();
}

/* ══════════════════════════ MAIN MENU ══════════════════════════════ */
int main_menu(Player *p) {
    cls();
    draw_hud(p);
    nl();
    printf(GOLD "  ╔═══════════════════════════════════════════════════╗\n");
    printf(GOLD "  ║  " ORANGE BOLD "UBUNTU MIND GARDEN v3 — MAIN MENU" R GOLD "           ║\n");
    printf(GOLD "  ╠═════════════════════════════╦═════════════════════╣\n");
    printf(GOLD "  ║ " ORANGE "1." IVORY " Daily Check-In          " GOLD "║ " CYAN "10." IVORY " Quests           " GOLD "║\n");
    printf(GOLD "  ║ " LAKE   "2." IVORY " Breathing (3 modes)     " GOLD "║ " SAVANNA"11." IVORY " Badges           " GOLD "║\n");
    printf(GOLD "  ║ " PURPLE "3." IVORY " Affirmation             " GOLD "║ " TEAL  "12." IVORY " Skills           " GOLD "║\n");
    printf(GOLD "  ║ " FOREST "4." IVORY " Grounding (5-4-3-2-1)  " GOLD "║ " LIME  "13." IVORY " Level Map        " GOLD "║\n");
    printf(GOLD "  ║ " INDIGO "5." IVORY " Meditation              " GOLD "║ " FOREST"14." IVORY " My Garden        " GOLD "║\n");
    printf(GOLD "  ║ " EARTH  "6." IVORY " Journal                 " GOLD "║ " ORANGE"15." IVORY " Coin Shop        " GOLD "║\n");
    printf(GOLD "  ╠═════════════════════════════╣                     ║\n");
    printf(GOLD "  ║ " CYAN   "7." IVORY " Nile Rapids   " DIM "[Lv3+]" GOLD "  ║ " PURPLE"16." IVORY " ★ Premium        " GOLD "║\n");
    printf(GOLD "  ║ " EARTH  "8." IVORY " Gorilla Wisdom" DIM "[Lv5+]" GOLD "  ║ " CRIMSON"17." IVORY " Crisis Support   " GOLD "║\n");
    printf(GOLD "  ║ " AMBER  "9." IVORY " Drum Rhythm   " DIM "[Lv6+]" GOLD "  ║ " DIM   "18." RESET IVORY " Save & Exit      " GOLD "║\n");
    printf(GOLD "  ╚═════════════════════════════╩═════════════════════╝\n\n" R);
    printf(SAVANNA "  Choose (1-18): " R);
    return get_int(1,18);
}

/* ══════════════════════════ PLAYER SETUP ═══════════════════════════ */
void setup_new_player(Player *p) {
    memset(p,0,sizeof(Player));
    p->level=1; p->mood=5; p->anxiety=5; p->energy=5;
    p->premium=0;
    for (int i=0;i<MAX_SKILLS;i++) {
        strncpy(p->skills[i].name, skill_names[i], 31);
        p->skills[i].level=0;
    }
    init_quests(p);

    cls();
    draw_logo();
    printf(GOLD "\n  Karibu! What is your name, traveler?\n  > " R);
    char buf[MAX_NAME+8];
    if (fgets(buf,sizeof(buf),stdin)) {
        int len=strlen(buf);
        if (len>0&&buf[len-1]=='\n') buf[len-1]='\0';
        strncpy(p->name, buf, MAX_NAME-1);
    }
    if (!strlen(p->name)) strcpy(p->name,"Friend");

    printf(FOREST "\n  Karibu, " GOLD BOLD "%s" R FOREST "! Welcome to Ubuntu Mind Garden.\n\n" R, p->name);
    printf(IVORY  "  This garden is FREE forever at Levels 1–8.\n");
    printf(        "  Premium unlocks 12 more levels and bonus content\n");
    printf(        "  — but you can grow a magnificent forest without it.\n\n" R);
    printf(SAVANNA "  Every session plants a seed. Watch your mind grow.\n");
    printf(         "  Reach Level 20 to become the Ancestor Light. ✨\n\n" R);
    press_enter();
}

/* ══════════════════════════ MAIN ═══════════════════════════════════ */
int main(void) {
    Player player;
    int loaded = load_game(&player);

    if (!loaded) {
        setup_new_player(&player);
    } else {
        /* re-link skill names after load */
        for (int i=0;i<MAX_SKILLS;i++)
            strncpy(player.skills[i].name, skill_names[i], 31);
        cls();
        draw_logo();
        printf(FOREST "\n  Welcome back, " GOLD BOLD "%s" R FOREST "!\n" R, player.name);
        printf(GOLD   "  Level %d — %s %s\n" R, player.level,
               lvl_icon[player.level], lvl_name[player.level]);
        printf(CRIMSON "  🔥 Streak: %d days\n\n" R, player.streak);
        press_enter();
    }

    int running=1;
    while (running) {
        int c = main_menu(&player);
        switch(c) {
            case  1: mood_checkin(&player);      break;
            case  2: breathing_exercise(&player);break;
            case  3: daily_affirmation(&player); break;
            case  4: grounding_exercise(&player);break;
            case  5: meditation_session(&player);break;
            case  6: journal_entry(&player);     break;
            case  7: minigame_nile(&player);     break;
            case  8: minigame_gorilla(&player);  break;
            case  9: minigame_drum(&player);     break;
            case 10: show_quests(&player);       break;
            case 11: show_badges(&player);       break;
            case 12: show_skills(&player);       break;
            case 13: show_level_map(&player);    break;
            case 14: show_garden(&player);       break;
            case 15: coin_shop(&player);         break;
            case 16: payment_screen(&player);    break;
            case 17: crisis_support();           break;
            case 18: running=0;                  break;
        }
        save_game(&player);
        refresh_quests(&player);
        check_badges(&player);
    }

    /* Farewell */
    cls();
    divider(GOLD,'═',58);
    printf(GOLD "\n  Goodbye, " ORANGE BOLD "%s" R GOLD ".\n\n" R, player.name);
    printf(IVORY "  Level " GOLD BOLD "%d/%d" R IVORY " — %s %s\n",
           player.level, MAX_LEVELS, lvl_icon[player.level], lvl_name[player.level]);
    printf(IVORY "  XP: " GOLD "%d  " IVORY "Coins: 💰%d  Streak: 🔥%d\n\n" R,
           player.xp, player.coins, player.streak);
    printf(SAVANNA "  \"However long the night, the dawn will break.\"\n");
    printf(DIM     "   — African Proverb\n\n" R);
    printf(FOREST  "  Your garden grows even while you sleep. 🌿\n\n" R);
    divider(GOLD,'═',58);
    printf("\n");
    return 0;
}

/*
 * ════════════════════════════════════════════════════════════════════
 *  UBUNTU MIND GARDEN v3.0 — Advanced Edition
 *  ~1400 lines of C | 20 Levels | 3 Mini-Games | 5 Meditations
 *  8-Skill Tree | 25 Badges | 12 Quests | Save/Load | Ubuntu Coins
 *  FREE levels 1–8 forever | Premium levels 9–20 (optional)
 *  "Umuntu ngumuntu ngabantu" — I am because we are 🌍
 * ════════════════════════════════════════════════════════════════════
 */
