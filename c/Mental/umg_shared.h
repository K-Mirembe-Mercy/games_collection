/*
 * ╔══════════════════════════════════════════════════════════════════════╗
 * ║  umg_shared.h — Ubuntu Mind Garden Shared Header                    ║
 * ║  Links ubuntu_mindgarden_v3.c  +  umg_extensions.c together         ║
 * ║                                                                      ║
 * ║  HOW TO COMPILE BOTH FILES TOGETHER:                                 ║
 * ║    gcc ubuntu_mindgarden_v3.c umg_extensions.c -o umg -lm           ║
 * ║  Then run:  ./umg                                                    ║
 * ╚══════════════════════════════════════════════════════════════════════╝
 */

#ifndef UMG_SHARED_H
#define UMG_SHARED_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <math.h>
#include <unistd.h>
#include <ctype.h>

/* ═══════════════════════ VERSION ═══════════════════════════════════ */
#define UMG_VERSION       "3.1"
#define SAVE_FILE_EXT     "umg_ext.dat"   /* extension save file */

/* ═══════════════════════ ANSI COLORS ═══════════════════════════════ */
#ifndef R
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
#endif

/* ═══════════════════════ SHARED CONSTANTS ══════════════════════════ */
#define MAX_NAME          64
#define MAX_JOURNAL       768
#define JOURNAL_CAP       20
#define MAX_LEVELS        20
#define MAX_QUESTS        12
#define MAX_BADGES        25
#define MAX_SKILLS         8
#define SAVE_FILE         "umg3_save.dat"

/* ═══════════════════════ EXTENSION CONSTANTS ═══════════════════════ */
#define MAX_FRIENDS        10
#define MAX_CHALLENGES      8
#define MAX_STORY_CHAPTERS 12
#define MAX_MOOD_HISTORY   30
#define MAX_CBT_ENTRIES    15
#define MAX_HABIT_SLOTS    10
#define MAX_CRISIS_PLAN    10

/* ═══════════════════════ SHARED STRUCTS (mirror v3) ════════════════ */

typedef struct {
    char name[32];
    int  level;
    int  xp_bonus;
} Skill;

typedef struct {
    char title[64];
    char desc[128];
    int  target;
    int  progress;
    int  xp_reward;
    int  coin_reward;
    int  complete;
    int  premium;
    int  skill_idx;
} Quest;

typedef struct {
    int sessions_done;
    int total_minutes;
    int unlocked;
} MeditationData;

typedef struct {
    char    name[MAX_NAME];
    int     level;
    int     xp;
    int     coins;
    int     mood;
    int     anxiety;
    int     energy;
    int     streak;
    int     sessions;
    int     breaths;
    int     grounds;
    int     affirmations;
    int     journals;
    int     meditations;
    int     minigames_won;
    int     low_mood_checkins;
    int     premium;
    int     badges[MAX_BADGES];
    char    journal[JOURNAL_CAP][MAX_JOURNAL];
    int     journal_count;
    Quest   quests[MAX_QUESTS];
    int     quest_count;
    Skill   skills[MAX_SKILLS];
    MeditationData med;
    long    last_day;
    int     total_xp_earned;
    int     total_coins_earned;
    int     perfect_games;
} Player;

/* ═══════════════════════ EXTENSION DATA STRUCTS ════════════════════ */

/* Mood History — track mood over 30 days for trend analysis */
typedef struct {
    int  mood[MAX_MOOD_HISTORY];
    int  anxiety[MAX_MOOD_HISTORY];
    int  energy[MAX_MOOD_HISTORY];
    long timestamps[MAX_MOOD_HISTORY];
    int  count;
    int  head;   /* circular buffer head */
} MoodHistory;

/* CBT (Cognitive Behavioural Therapy) thought record */
typedef struct {
    char situation[256];
    char thought[256];
    char emotion[64];
    char evidence_for[256];
    char evidence_against[256];
    char balanced_thought[256];
    long timestamp;
} CBTRecord;

/* Habit Tracker */
typedef struct {
    char  name[64];
    int   target_per_week;
    int   completions[7];   /* Mon–Sun */
    int   total_done;
    int   streak;
    int   active;
} Habit;

/* Safety / Crisis Plan */
typedef struct {
    char warning_signs[MAX_CRISIS_PLAN][128];
    int  warning_count;
    char coping_strategies[MAX_CRISIS_PLAN][128];
    int  coping_count;
    char support_people[5][64];
    int  support_count;
    char emergency_contacts[3][128];
    int  contact_count;
    int  created;
} CrisisPlan;

/* Story Mode chapter */
typedef struct {
    char  title[64];
    char  summary[256];
    int   unlocked;
    int   completed;
    int   xp_reward;
} StoryChapter;

/* Social / Community Challenge */
typedef struct {
    char title[64];
    char desc[128];
    int  target;
    int  progress;
    int  xp_reward;
    int  expires_day;  /* epoch day */
    int  complete;
} Challenge;

/* Friend entry (local co-op / accountability) */
typedef struct {
    char name[MAX_NAME];
    int  level;
    int  streak;
    int  active;
} Friend;

/* Full Extension State — saved separately */
typedef struct {
    MoodHistory  mood_hist;
    CBTRecord    cbt[MAX_CBT_ENTRIES];
    int          cbt_count;
    Habit        habits[MAX_HABIT_SLOTS];
    int          habit_count;
    CrisisPlan   crisis_plan;
    StoryChapter chapters[MAX_STORY_CHAPTERS];
    Challenge    challenges[MAX_CHALLENGES];
    int          challenge_count;
    Friend       friends[MAX_FRIENDS];
    int          friend_count;
    int          total_ext_sessions;
    long         last_weekly_reset;
    int          night_mode;         /* 1 if user prefers calm colour scheme */
    char         personal_mantra[256];
    int          mantra_set;
} ExtState;

/* ═══════════════════════ SHARED UTILITY PROTOTYPES ═════════════════ */
/* Implemented in umg_extensions.c, callable from main game           */

void ext_load(ExtState *e);
void ext_save(ExtState *e);

void ext_record_mood(ExtState *e, Player *p);
void ext_show_mood_chart(ExtState *e);
void ext_cbt_session(ExtState *e, Player *p);
void ext_habit_tracker(ExtState *e, Player *p);
void ext_crisis_plan(ExtState *e, Player *p);
void ext_story_mode(ExtState *e, Player *p);
void ext_challenges(ExtState *e, Player *p);
void ext_friends(ExtState *e, Player *p);
void ext_set_mantra(ExtState *e, Player *p);
void ext_sleep_hygiene(Player *p);
void ext_panic_button(Player *p);
void ext_weekly_report(ExtState *e, Player *p);
void ext_menu(ExtState *e, Player *p);

/* Award XP — defined in main game, used by extensions */
void award_xp(Player *p, int base_xp, const char *reason);

#endif /* UMG_SHARED_H */
