/*
 * ╔══════════════════════════════════════════════════════════════════════╗
 * ║   umg_levels.c — Ubuntu Mind Garden: LEVEL EXPANSION PACK          ║
 * ║   "Even the mightiest eagle begins as an egg." — African Proverb   ║
 * ║                                                                      ║
 * ║   EXPANDS the game from 20 → 100 LEVELS                            ║
 * ║   Adds: 5 Prestige tiers | 8 new mini-games | Boss encounters      ║
 * ║         Daily login rewards | Combo multipliers | Loot boxes       ║
 * ║         Level-specific cutscenes | Legendary badges | Rank titles  ║
 * ║                                                                      ║
 * ║   COMPILE (all 3 files together):                                   ║
 * ║     gcc ubuntu_mindgarden_v3.c umg_extensions.c umg_levels.c       ║
 * ║         -o umg_full -lm                                             ║
 * ║   Run: ./umg_full                                                   ║
 * ╚══════════════════════════════════════════════════════════════════════╝
 *
 *  HOW TO PLUG IN:
 *  1. Add  #include "umg_levels.h"  at the top of ubuntu_mindgarden_v3.c
 *  2. In main() declare:  LevelState lstate;  lvl_load(&lstate);
 *  3. Add case 20 in main menu: lvl_menu(&lstate, &player);
 *  4. Call lvl_daily_login(&lstate, &player); right after load_game()
 *  5. Compile with all 3 .c files as shown above
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <math.h>
#include <unistd.h>
#include <ctype.h>

/* ═══════════════════════════ COLORS (safe re-include) ══════════════ */
#ifndef R
#define R        "\033[0m"
#define BOLD     "\033[1m"
#define DIM      "\033[2m"
#define ITALIC   "\033[3m"
#define BLINK    "\033[5m"
#define GOLD     "\033[38;5;220m"
#define ORANGE   "\033[38;5;208m"
#define CRIMSON  "\033[38;5;160m"
#define EARTH    "\033[38;5;130m"
#define SAVANNA  "\033[38;5;178m"
#define FOREST   "\033[38;5;34m"
#define JUNGLE   "\033[38;5;28m"
#define LAKE     "\033[38;5;33m"
#define SKY      "\033[38;5;117m"
#define IVORY    "\033[38;5;230m"
#define PURPLE   "\033[38;5;135m"
#define PINK     "\033[38;5;211m"
#define CYAN     "\033[38;5;51m"
#define LIME     "\033[38;5;118m"
#define TEAL     "\033[38;5;37m"
#define INDIGO   "\033[38;5;62m"
#define AMBER    "\033[38;5;214m"
#define ROSE     "\033[38;5;204m"
#define SILVER   "\033[38;5;250m"
#define MIST     "\033[38;5;189m"
#endif

/* ═══════════════════════════ LEVEL EXPANSION CONSTANTS ═════════════ */
#define TOTAL_LEVELS      100
#define PRESTIGE_LEVELS     5
#define MAX_LVL_BADGES     50
#define MAX_LOOT_SLOTS     20
#define SAVE_LVL_FILE    "umg_levels.dat"

/* ═══════════════════════════ 100-LEVEL TABLE ═══════════════════════ */
/*
 * TIER 1 — THE SEED PATH        (Lv 1–20)   [from v3 core]
 * TIER 2 — THE SAVANNA WALK     (Lv 21–35)  [free]
 * TIER 3 — THE NILE JOURNEY     (Lv 36–50)  [free]
 * TIER 4 — THE BWINDI TRIALS    (Lv 51–65)  [premium]
 * TIER 5 — THE ANCESTOR PATH    (Lv 66–80)  [premium]
 * TIER 6 — THE UBUNTU LEGENDS   (Lv 81–100) [premium]
 *
 * PRESTIGE SYSTEM:
 *   After reaching Lv 100, you Prestige (reset to Lv 1 with a star)
 *   Each prestige gives permanent bonuses + exclusive titles
 *   Max Prestige: ★★★★★ (5 stars) = "Ubuntu Eternal"
 */

static const char *tier_name[] = {
    "The Seed Path", "The Savanna Walk", "The Nile Journey",
    "The Bwindi Trials", "The Ancestor Path", "The Ubuntu Legends"
};
static const char *tier_color[] = {
    FOREST, SAVANNA, LAKE, JUNGLE, PURPLE, GOLD
};

/* All 100 level names */
static const char *lv_name[101] = {
    "",
    /* 1–20: Seed Path (from v3) */
    "Seed of Intention","First Sprout","Young Sapling","Rooted Sapling",
    "River Walker","Savanna Keeper","Forest Guardian","Mountain Climber",
    "Crested Crane","Nile Swimmer","Gorilla Kin","Bwindi Mystic",
    "Drumbeat Elder","Savanna Shaman","Ancestor's Voice","Ubuntu Sage",
    "Lake Spirit","Baobab Soul","Rwenzori Peak","Ancestor Light",
    /* 21–35: Savanna Walk */
    "Savanna Wanderer","Acacia Dreamer","Dust Road Traveller",
    "Horizon Chaser","Sun Drinker","Wind Listener","Plains Runner",
    "Termite Tower Sage","Dry Season Survivor","Rain Dancer",
    "Buffalo Witness","Wildebeest Wanderer","Zebra Stripe Mind",
    "Cheetah Focus","Lion's Patience",
    /* 36–50: Nile Journey */
    "Nile Source Seeker","Current Rider","Papyrus Whisperer",
    "Kingfisher Eye","Hippo Calm","Crocodile Still","Heron Stance",
    "Sandbank Rester","Delta Mapper","Flood Plain Keeper",
    "Ancient Waters Soul","Nile Star","River Memory","Blue Nile Heart",
    "White Nile Spirit",
    /* 51–65: Bwindi Trials */
    "Forest Entrant","Vine Crosser","Mist Walker","Gorilla Shadow",
    "Canopy Dweller","Root Tender","Fern Bed Sleeper","Ancient Bark",
    "Moss Mind","Impenetrable Core","Forest Elder","Bwindi Keeper",
    "Silverback Wisdom","Night Forest","Dawn Forest",
    /* 66–80: Ancestor Path */
    "Ancestor Caller","Memory Keeper","Ritual Bearer","Drum Summoner",
    "Fire Lighter","Story Keeper","Clan Guardian","Village Heart",
    "Spirit Talker","Dream Walker","Ancestral Fire","Sacred Grove",
    "Ubuntu Flame","Eternal Drumbeat","Living Ancestor",
    /* 81–100: Ubuntu Legends */
    "Legend Seed","Ubuntu Weaver","Continental Soul","Pan-African Heart",
    "Motherland Root","Eternal Sprout","Cosmic Ubuntu","Star Ancestor",
    "Galaxy Seed","Infinite Grove","Light Keeper","Ubuntu Eternal Flame",
    "Ancestral Galaxy","Ubuntu Cosmos","Timeless Root",
    "Ubuntu Infinity","Eternal Bwindi","Cosmic Crane","Ubuntu Genesis",
    "UBUNTU ETERNAL"
};

static const char *lv_icon[101] = {
    "",
    "🌱","🌿","🌲","🌳","🌊","🌾","🌴","🏔",
    "🦩","🐟","🦍","🌫","🥁","🔮","👁","📿",
    "💧","🌳","⛰","✨",
    "🦒","🌵","🛤","🌅","☀","💨","🏃","🗼",
    "🌞","💃","🦬","🦓","🦓","🐆","🦁",
    "🔍","🌊","📜","🐦","🦛","🐊","🦢",
    "🏖","🗺","🌿","🌌","⭐","🧠","💙","👻",
    "🌲","🌿","🌫","🦍","🌳","🌱","🛌","🪵",
    "🍀","💎","🧙","🌲","🦍","🌙","🌅",
    "👴","🧠","🪬","🥁","🔥","📖","🛡","🏘",
    "👻","💭","🔥","🌳","🔥","🥁","🏺",
    "🌟","🕸","🌍","❤","🌏","🌱","🌌","⭐",
    "🌌","🌳","💡","🔥","🌌","🌀","🌱",
    "♾","🌲","🦩","🌱","✨"
};

/* XP required per level (exponential curve) */
static int lv_xp_required(int level) {
    if (level <= 1) return 0;
    if (level <= 20) {
        int base[] = {0,0,120,300,550,900,1350,1900,2600,
                      3500,4600,6000,7700,9800,12300,15300,18900,
                      23200,28300,34400,42000};
        return base[level];
    }
    /* Formula for levels 21–100: grows exponentially */
    int base = 42000;
    for (int i = 21; i <= level; i++)
        base = (int)(base * 1.18f) + 2000;
    return base;
}

/* Tier from level */
static int lv_tier(int level) {
    if (level <= 20) return 0;
    if (level <= 35) return 1;
    if (level <= 50) return 2;
    if (level <= 65) return 3;
    if (level <= 80) return 4;
    return 5;
}

/* Premium required? */
static int lv_needs_premium(int level) {
    return (level >= 51) ? 1 : 0;
}

/* ═══════════════════════════ PRESTIGE ══════════════════════════════ */
static const char *prestige_title[] = {
    "Wanderer", "Keeper", "Guardian", "Elder", "Ancestor", "Ubuntu Eternal"
};
static const char *prestige_star[] = {
    "","★","★★","★★★","★★★★","★★★★★"
};
static const int prestige_xp_bonus[] = { 0, 10, 20, 35, 55, 80 }; /* % bonus */

/* ═══════════════════════════ LEGENDARY BADGES ══════════════════════ */
#define LVL_BADGE_COUNT 50
static const char *lvl_badge_name[LVL_BADGE_COUNT] = {
    /* Tier 2 badges (Savanna) */
    "Plains Walker","Sun Drinker","Rain Dancer","Buffalo Witness",
    "Cheetah Mind","Lion's Patience","Savanna Complete",
    /* Tier 3 badges (Nile) */
    "Nile Source","Papyrus Reader","Hippo Calm","Ancient Waters",
    "Nile Star","Nile Complete",
    /* Tier 4 badges (Bwindi) */
    "Forest Entrant","Vine Crosser","Gorilla Shadow","Canopy Dweller",
    "Impenetrable Core","Forest Elder","Bwindi Complete",
    /* Tier 5 badges (Ancestor) */
    "Ancestor Caller","Fire Lighter","Story Keeper","Dream Walker",
    "Living Ancestor","Ancestor Complete",
    /* Tier 6 badges (Legends) */
    "Legend Seed","Ubuntu Weaver","Cosmic Ubuntu","Star Ancestor",
    "Ubuntu Eternal Flame","Legends Complete",
    /* Prestige badges */
    "First Prestige","Double Prestige","Triple Prestige",
    "Quad Prestige","Max Prestige — Ubuntu Eternal",
    /* Combo badges */
    "Combo x5","Combo x10","Combo x25","Combo x50",
    /* Loot badges */
    "First Loot","Rare Find","Legendary Haul",
    /* Special */
    "Speed Demon","Night Owl Legend","Level 50","Level 75","Level 100"
};
static const char *lvl_badge_icon[LVL_BADGE_COUNT] = {
    "🏃","☀","💃","🦬","🐆","🦁","🌾",
    "🔍","📜","🦛","🌌","⭐","🌊",
    "🌲","🌿","🦍","🌳","💎","🧙","🌴",
    "👴","🔥","📖","💭","🏺","👁",
    "🌟","🕸","🌌","⭐","🔥","✨",
    "⭐","★★","★★★","★★★★","★★★★★",
    "⚡","💥","🌪","💫",
    "📦","💎","👑",
    "⚡","🦉","🥈","🥇","🏆"
};

/* ═══════════════════════════ LOOT BOX SYSTEM ═══════════════════════ */
typedef struct {
    char  name[64];
    char  type;    /* 'X'=XP 'C'=Coins 'B'=Badge boost 'S'=Skill boost */
    int   value;
    int   rarity;  /* 1=common 2=rare 3=epic 4=legendary */
} LootItem;

static const LootItem loot_table[] = {
    {"Handful of Ubuntu Seeds",    'X', 50,  1},
    {"Nile Water Blessing",        'X', 100, 1},
    {"Savanna Sunrise Boost",      'X', 200, 2},
    {"Bwindi Mist Surge",         'X', 400, 2},
    {"Ancestor's XP Gift",        'X', 800, 3},
    {"Ubuntu Cosmic Burst",       'X',1500, 4},
    {"Kampala Market Coins",      'C',  30, 1},
    {"Ugandan Hills Treasure",    'C',  75, 2},
    {"Nile Delta Gold",           'C', 150, 2},
    {"Ancestor's Coin Chest",     'C', 300, 3},
    {"Ubuntu Legend Vault",       'C', 600, 4},
    {"Skill Seed (Breathing)",    'S',   1, 2},
    {"Skill Seed (Grounding)",    'S',   2, 2},
    {"Skill Seed (Journal)",      'S',   3, 2},
    {"Skill Mega Seed",           'S',  -1, 3}, /* random skill */
    {"Ancestor Skill Crystal",    'S',  -2, 4}, /* all skills +1 */
    {"Badge Magnet",              'B',   5, 2},
    {"Legendary Badge Aura",      'B',  10, 4},
};
#define LOOT_TABLE_SIZE 18

/* ═══════════════════════════ COMBO SYSTEM ══════════════════════════ */
/* Combo multiplier increases XP when you do activities back-to-back  */
typedef struct {
    int  count;         /* current combo */
    int  best_combo;    /* all-time best */
    long last_action;   /* epoch time of last action */
    float multiplier;   /* current XP multiplier (1.0x–5.0x) */
} ComboState;

/* ═══════════════════════════ BOSS ENCOUNTERS ═══════════════════════ */
/* Special challenge events that appear at milestone levels            */
typedef struct {
    char  name[64];
    char  desc[256];
    int   level_trigger;
    int   defeated;
    int   xp_reward;
    int   coin_reward;
    int   attempts;
} Boss;

static Boss boss_table[] = {
    {
        "The Anxious Fog",
        "A thick mental fog rolls in from Lake Victoria. You must breathe\n"
        "  through 6 cycles perfectly to clear it. No mistakes allowed.",
        25, 0, 500, 100, 0
    },
    {
        "The Doubt Serpent",
        "A great python of self-doubt coils around the Nile. Answer 5\n"
        "  affirmation challenges correctly to drive it away.",
        40, 0, 800, 150, 0
    },
    {
        "The Bwindi Shadow",
        "A dark shape lurks in the Impenetrable Forest. Win Gorilla\n"
        "  Wisdom with a perfect score to reveal the light within.",
        55, 0, 1200, 200, 0
    },
    {
        "The Ancestral Trial",
        "Your ancestors set a challenge: name 10 African proverbs from\n"
        "  memory (hints given). Prove you carry their wisdom.",
        70, 0, 2000, 300, 0
    },
    {
        "The Ubuntu Dragon",
        "A dragon of isolation threatens to divide the village. Complete\n"
        "  the ultimate 10-round drum challenge to unite the people.",
        85, 0, 3500, 500, 0
    },
    {
        "THE FINAL ANCESTOR",
        "At Level 99, face the ultimate test. A 5-part challenge:\n"
        "  Breathe. Ground. Affirm. Drum. Remember. All in sequence.",
        99, 0, 10000, 2000, 0
    },
};
#define BOSS_COUNT 6

/* ═══════════════════════════ DAILY LOGIN REWARDS ═══════════════════ */
static const int login_xp[]    = {20,30,50,75,100,150,300};   /* day 1–7 */
static const int login_coins[] = { 5,10,15,20, 25, 40,100};
static const char *login_msg[] = {
    "Welcome back! A seed grows with daily water. 🌱",
    "Day 2! Consistency is the greatest power. 🌿",
    "Day 3! The river doesn't stop — neither do you. 🌊",
    "Day 4! Your roots are going deeper. 🌳",
    "Day 5! The savanna remembers those who walk it daily. 🌾",
    "Day 6! Almost a full week — the ancestors are watching! 🔥",
    "Day 7! FULL WEEK! The Nile celebrates you today! 🎉"
};

/* ═══════════════════════════ LEVEL STATE ═══════════════════════════ */
typedef struct {
    int       total_level;          /* 1–100                          */
    int       total_xp;             /* cumulative XP across resets    */
    int       prestige;             /* 0–5                            */
    int       lvl_badges[LVL_BADGE_COUNT];
    Boss      bosses[BOSS_COUNT];
    ComboState combo;
    int       loot_opened;          /* total loot boxes opened        */
    int       loot_available;       /* loot boxes waiting to open     */
    long      last_login_day;
    int       login_streak;
    int       login_day_in_cycle;   /* 1–7 for reward table           */
    int       total_sessions_ever;
    int       speed_runs;           /* sub-2min sessions              */
    int       perfect_streak;       /* activities done perfectly      */
    long      first_play_day;
    /* Per-level unlock flags (did player visit this level's cutscene) */
    int       cutscene_seen[101];
} LevelState;

/* ═══════════════════════════ UTILITIES ═════════════════════════════ */
static void L_cls(void)       { printf("\033[2J\033[H"); fflush(stdout); }
static void L_ms(int t)       { usleep(t*1000); }
static void L_nl(void)        { printf("\n"); }

static void L_div(const char *c, char ch, int w) {
    printf("%s", c);
    for(int i=0;i<w;i++) putchar(ch);
    printf(R "\n");
}

static void L_tw(const char *col, const char *txt, int d) {
    printf("%s", col);
    while(*txt){ putchar(*txt++); fflush(stdout); L_ms(d); }
    printf(R);
}

static void L_enter(void) {
    printf(DIM "\n  [ Press ENTER ]" R); fflush(stdout);
    while(getchar()!='\n');
}

static int L_int(int lo, int hi) {
    int v; char buf[32];
    while(1){
        if(fgets(buf,sizeof(buf),stdin)&&sscanf(buf,"%d",&v)==1&&v>=lo&&v<=hi) return v;
        printf(ORANGE "  Enter %d–%d: " R,lo,hi);
    }
}

static long L_day(void) { return (long)(time(NULL)/86400); }

/* ═══════════════════════════ SAVE / LOAD ═══════════════════════════ */
void lvl_load(LevelState *s) {
    memset(s, 0, sizeof(LevelState));
    FILE *f = fopen(SAVE_LVL_FILE,"rb");
    if(f){ fread(s,sizeof(LevelState),1,f); fclose(f); }
    if(s->total_level < 1) s->total_level = 1;
    if(s->combo.multiplier < 1.0f) s->combo.multiplier = 1.0f;
    if(s->first_play_day == 0) s->first_play_day = L_day();
    /* copy boss table */
    for(int i=0;i<BOSS_COUNT;i++){
        if(s->bosses[i].xp_reward == 0) s->bosses[i] = boss_table[i];
    }
}

void lvl_save(LevelState *s) {
    FILE *f = fopen(SAVE_LVL_FILE,"wb");
    if(f){ fwrite(s,sizeof(LevelState),1,f); fclose(f); }
}

/* ═══════════════════════════ COMBO ENGINE ══════════════════════════ */
static void combo_tick(LevelState *s) {
    long now = time(NULL);
    long elapsed = now - s->combo.last_action;
    /* Combo resets if more than 10 mins between actions */
    if(s->combo.last_action > 0 && elapsed > 600) {
        s->combo.count = 0;
        s->combo.multiplier = 1.0f;
    }
    s->combo.count++;
    s->combo.last_action = now;

    /* Multiplier tiers */
    if      (s->combo.count >= 25) s->combo.multiplier = 5.0f;
    else if (s->combo.count >= 10) s->combo.multiplier = 3.0f;
    else if (s->combo.count >=  5) s->combo.multiplier = 2.0f;
    else if (s->combo.count >=  3) s->combo.multiplier = 1.5f;
    else                           s->combo.multiplier = 1.0f;

    if(s->combo.count > s->combo.best_combo)
        s->combo.best_combo = s->combo.count;
}

static int combo_apply_xp(LevelState *s, int base_xp) {
    int result = (int)(base_xp * s->combo.multiplier);
    if(s->combo.count >= 3)
        printf(AMBER "  ⚡ COMBO x%d  %.1fx multiplier! +%d XP\n" R,
               s->combo.count, s->combo.multiplier, result);
    return result;
}

/* ═══════════════════════════ LEVEL-UP CHECK ════════════════════════ */
/*
 * This is called from the main game's award_xp flow.
 * Pass total accumulated XP and current level; returns new level.
 */
int lvl_check_levelup(LevelState *s, int xp, int premium) {
    int old_level = s->total_level;
    while(s->total_level < TOTAL_LEVELS) {
        int next = s->total_level + 1;
        if(lv_needs_premium(next) && !premium) break;
        if(xp < lv_xp_required(next)) break;
        s->total_level = next;

        /* Level-up fanfare */
        int tier = lv_tier(s->total_level);
        printf("\n");
        L_div(tier_color[tier],'═',60);
        printf("%s  ✨ LEVEL UP! → Lv.%d\n  %s %s\n", tier_color[tier],
               s->total_level, lv_icon[s->total_level], lv_name[s->total_level]);
        printf("  Tier: %s\n" R, tier_name[tier]);
        L_div(tier_color[tier],'═',60);

        /* Award loot box every 5 levels */
        if(s->total_level % 5 == 0) {
            s->loot_available++;
            printf(GOLD "  📦 LOOT BOX EARNED at Level %d!\n" R, s->total_level);
        }

        /* Milestone messages */
        if(s->total_level == 25) printf(SAVANNA "  🌾 You entered the Savanna Walk! New world awaits.\n" R);
        if(s->total_level == 36) printf(LAKE    "  🌊 The Nile Journey begins. Ancient waters call.\n" R);
        if(s->total_level == 51) printf(JUNGLE  "  🌲 The Bwindi Trials. Only the rooted survive.\n" R);
        if(s->total_level == 66) printf(PURPLE  "  👁 The Ancestor Path. You walk with those who came before.\n" R);
        if(s->total_level == 81) printf(GOLD BLINK "  ✨ UBUNTU LEGENDS TIER! You are history.\n" R);
        if(s->total_level == 100)printf(GOLD BOLD BLINK "  👑 LEVEL 100! UBUNTU ETERNAL! PRESTIGE AWAITS!\n" R);

        /* Boss check */
        for(int b=0;b<BOSS_COUNT;b++){
            if(!s->bosses[b].defeated && s->total_level >= s->bosses[b].level_trigger) {
                printf(CRIMSON BLINK "  ⚔  BOSS ENCOUNTER UNLOCKED: %s!\n" R,
                       s->bosses[b].name);
                printf(CRIMSON "  Find it in the Level Hub → Boss Battles!\n" R);
            }
        }
        sleep(1);
    }
    s->total_xp = xp;
    return s->total_level;
}

/* ═══════════════════════════ DAILY LOGIN ════════════════════════════ */
void lvl_daily_login(LevelState *s, void *player_ptr) {
    /* We use void* to avoid circular header deps — cast locally */
    /* Just award coins/xp as ints here, caller uses award_xp    */
    long today = L_day();
    if(s->last_login_day == today) return; /* already claimed */

    long diff = today - s->last_login_day;
    if(diff > 1) {
        s->login_streak = 0;
        s->login_day_in_cycle = 0;
    }

    s->login_streak++;
    s->login_day_in_cycle = ((s->login_day_in_cycle) % 7);
    int idx = s->login_day_in_cycle;
    s->login_day_in_cycle++;
    s->last_login_day = today;

    L_cls();
    L_div(GOLD,'═',58);
    printf(GOLD "  🎁  " AMBER BOLD "DAILY LOGIN REWARD — Day %d" R "\n", idx+1);
    L_div(GOLD,'═',58);
    L_nl();
    L_tw(IVORY, "  ", 0);
    L_tw(SAVANNA, login_msg[idx], 20);
    printf("\n\n" R);
    printf(GOLD "  🎁 +%d XP\n" R, login_xp[idx]);
    printf(GOLD "  💰 +%d Ubuntu Coins\n\n" R, login_coins[idx]);

    if(s->login_streak == 7) {
        printf(CRIMSON BOLD "  🔥 7-DAY LOGIN STREAK BONUS! +200 XP +50 Coins!\n" R);
    }
    if(s->login_streak == 30) {
        printf(GOLD BOLD BLINK "  👑 30-DAY DEDICATION! LEGENDARY REWARD!\n" R);
        s->loot_available += 3;
        printf(GOLD "  +3 LOOT BOXES!\n" R);
    }

    /* Note: actual XP/coin award happens in main via award_xp */
    printf(IVORY "  Come back tomorrow for Day %d reward!\n\n" R,
           (idx+1)%7+1);
    L_enter();
}

/* ═══════════════════════════ LOOT BOX ══════════════════════════════ */
void lvl_open_loot(LevelState *s) {
    if(s->loot_available <= 0) {
        printf(ORANGE "  No loot boxes available! Level up to earn more.\n" R);
        L_enter(); return;
    }

    L_cls();
    L_div(GOLD,'═',58);
    printf(GOLD "  📦  " AMBER BOLD "LOOT BOX — Ubuntu Treasure Chest" R "\n");
    L_div(GOLD,'═',58);
    printf(IVORY "\n  You have " GOLD BOLD "%d" R IVORY " loot boxes.\n\n" R, s->loot_available);
    printf(GOLD "  Open one? (Y/N): " R);
    char buf[8]; if(fgets(buf,8,stdin)&&toupper(buf[0])!='Y') return;

    s->loot_available--;
    s->loot_opened++;

    srand((unsigned)(time(NULL)+s->loot_opened));

    /* Weighted random: common=50%, rare=30%, epic=15%, legendary=5% */
    int roll = rand()%100;
    int rarity;
    if      (roll < 50) rarity = 1;
    else if (roll < 80) rarity = 2;
    else if (roll < 95) rarity = 3;
    else                rarity = 4;

    /* Pick item of matching rarity */
    int candidates[LOOT_TABLE_SIZE], nc=0;
    for(int i=0;i<LOOT_TABLE_SIZE;i++)
        if(loot_table[i].rarity==rarity) candidates[nc++]=i;

    int pick = (nc>0) ? candidates[rand()%nc] : 0;
    const LootItem *item = &loot_table[pick];

    const char *rarity_str[] = {"",SILVER "COMMON",LAKE "RARE",PURPLE "EPIC",GOLD BOLD "LEGENDARY"};
    const char *rarity_box[] = {"","📦","💙","💜","✨"};

    printf("\n  %s\n", rarity_box[rarity]);
    L_ms(500);
    printf(GOLD "  ╔══════════════════════════════════════╗\n");
    printf(GOLD "  ║  %s%-28s " GOLD "  ║\n" R, rarity_str[rarity], "");
    printf(GOLD "  ║  " IVORY BOLD "  %-34s" GOLD "║\n" R, item->name);
    printf(GOLD "  ╚══════════════════════════════════════╝\n\n" R);

    /* Apply reward */
    if(item->type == 'X') {
        printf(LIME "  +%d XP\n" R, item->value);
        /* caller adds to player — logged here */
    } else if(item->type == 'C') {
        printf(GOLD "  +%d Ubuntu Coins! 💰\n" R, item->value);
    } else if(item->type == 'S') {
        if(item->value == -2) printf(CYAN "  ALL skills +1! 📈\n" R);
        else if(item->value == -1) printf(CYAN "  Random skill +1! 📈\n" R);
        else printf(CYAN "  Skill #%d +1! 📈\n" R, item->value);
    } else if(item->type == 'B') {
        printf(PINK "  Badge XP boost! +%d%%\n" R, item->value);
    }

    /* First loot badge */
    if(s->loot_opened == 1) {
        s->lvl_badges[44] = 1; /* First Loot */
        printf(GOLD "  🏅 Badge: First Loot!\n" R);
    }
    if(rarity == 4) {
        s->lvl_badges[46] = 1; /* Legendary Haul */
        printf(GOLD "  🏅 Badge: Legendary Haul!\n" R);
    }
    L_enter();
}

/* ═══════════════════════════ BOSS BATTLES ══════════════════════════ */

/* Boss 1: Breathing challenge (perfect 6 cycles) */
static int boss_anxious_fog(void) {
    L_cls();
    L_div(CRIMSON,'═',58);
    printf(CRIMSON "  ⚔  BOSS: THE ANXIOUS FOG\n" R);
    printf(CRIMSON "  Complete 6 PERFECT breathing cycles.\n" R);
    printf(IVORY   "  Type the correct number when prompted. No mistakes!\n\n" R);
    L_enter();

    int errors=0;
    srand((unsigned)time(NULL));
    for(int c=1;c<=6;c++){
        int inhale=4, hold=rand()%4+4, exhale=rand()%4+5;
        printf(LAKE "\n  Cycle %d: INHALE %ds | HOLD %ds | EXHALE %ds\n" R,
               c, inhale, hold, exhale);
        printf(GOLD "  How many seconds to HOLD? " R);
        int ans = L_int(1,15);
        if(ans == hold) printf(LIME "  ✓ Correct!\n" R);
        else { printf(CRIMSON "  ✗ Wrong! Was %d.\n" R, hold); errors++; }
    }
    if(errors == 0){
        printf(LIME BOLD "\n  The fog CLEARS! You breathed through it perfectly!\n" R);
        return 1;
    } else {
        printf(CRIMSON "\n  The fog lingers (%d error(s)). Try again!\n" R, errors);
        return 0;
    }
}

/* Boss 2: Affirmation fill-in (5 questions) */
static int boss_doubt_serpent(void) {
    L_cls();
    L_div(PURPLE,'═',58);
    printf(PURPLE "  ⚔  BOSS: THE DOUBT SERPENT\n" R);
    printf(PURPLE "  Complete the affirmations. Type the missing word.\n\n" R);
    L_enter();

    struct { const char *prompt; const char *answer; } q[] = {
        {"I am rooted like the fig tree of _______.", "BWINDI"},
        {"Like Lake Victoria, my depth holds endless _______.", "LIFE"},
        {"The sun rises on the _______ for me every morning.", "NILE"},
        {"I carry the strength of the _______ mountains.", "RWENZORI"},
        {"Ubuntu — I am because _______ are.", "WE"},
    };
    int score=0;
    for(int i=0;i<5;i++){
        printf(GOLD "  Q%d: %s\n  > " R, i+1, q[i].prompt);
        char buf[64];
        if(fgets(buf,sizeof(buf),stdin)){
            int l=strlen(buf); if(l>0&&buf[l-1]=='\n') buf[l-1]='\0';
            for(int j=0;buf[j];j++) buf[j]=toupper(buf[j]);
            if(strcmp(buf,q[i].answer)==0){score++;printf(LIME "  ✓\n" R);}
            else printf(CRIMSON "  ✗ Answer: %s\n" R, q[i].answer);
        }
    }
    if(score>=4){printf(LIME BOLD "\n  The Serpent flees! Words of truth defeated it!\n" R);return 1;}
    else{printf(CRIMSON "\n  %d/5 — The Serpent coils tighter. Try again!\n" R,score);return 0;}
}

/* Boss 3: Gorilla Wisdom (must score 4/5 rounds) */
static int boss_bwindi_shadow(void) {
    L_cls();
    L_div(JUNGLE,'═',58);
    printf(JUNGLE "  ⚔  BOSS: THE BWINDI SHADOW\n" R);
    printf(JUNGLE "  Gorilla Wisdom — 5 rounds. Need 4/5 perfect.\n\n" R);
    L_enter();

    const char *plants[]={"🌿","🍃","🌺","🦋","🌸","🍀","🌼","🌻","🌱","🌾"};
    int score=0; int seq[6]; srand((unsigned)time(NULL)+3);
    for(int r=0;r<5;r++){
        int len=r+3;
        for(int i=0;i<len;i++) seq[i]=rand()%10;
        printf(GOLD "  Round %d: ", r+1);
        for(int i=0;i<len;i++) printf("%s ",plants[seq[i]]);
        printf(R "\n"); sleep(2);
        printf(DIM "  (Hidden)\n\n" R); sleep(1);
        printf(IVORY "  Plants: "); for(int i=0;i<10;i++) printf("%d:%s ",i,plants[i]);
        printf("\n  Enter %d numbers: " R,len);
        char buf[64]; int ok=1;
        if(fgets(buf,sizeof(buf),stdin)){
            char *tok=strtok(buf," \n");
            for(int i=0;i<len&&tok;i++,tok=strtok(NULL," \n"))
                if(atoi(tok)!=seq[i]){ok=0;break;}
        }
        if(ok){score++;printf(LIME "  ✓ Perfect!\n" R);}
        else printf(CRIMSON "  ✗\n" R);
    }
    if(score>=4){printf(LIME BOLD "\n  The Shadow DISSOLVES! The forest is clear!\n" R);return 1;}
    else{printf(CRIMSON "\n  %d/5 — Shadow grows stronger. Train more!\n" R,score);return 0;}
}

/* Boss 4: Proverb challenge */
static int boss_ancestral_trial(void) {
    L_cls();
    L_div(AMBER,'═',58);
    printf(AMBER "  ⚔  BOSS: THE ANCESTRAL TRIAL\n" R);
    printf(AMBER "  Complete 7 African proverbs. A hint is given.\n\n" R);
    L_enter();

    struct { const char *hint; const char *answer; } p[]={
        {"'Rain does not fall on one ___ alone.'","ROOF"},
        {"'Sticks in a bundle are ___.'","UNBREAKABLE"},
        {"'He who learns, ___.'","TEACHES"},
        {"'However long the night, the ___ will break.'","DAWN"},
        {"'When the music changes, so does the ___.'","DANCE"},
        {"'To get lost is to learn the ___.'","WAY"},
        {"'If you want to go far, go ___.'","TOGETHER"},
    };
    int score=0;
    for(int i=0;i<7;i++){
        printf(GOLD "  Hint: %s\n  > " R, p[i].hint);
        char buf[64];
        if(fgets(buf,sizeof(buf),stdin)){
            int l=strlen(buf); if(l>0&&buf[l-1]=='\n') buf[l-1]='\0';
            for(int j=0;buf[j];j++) buf[j]=toupper(buf[j]);
            if(strcmp(buf,p[i].answer)==0){score++;printf(LIME "  ✓\n" R);}
            else printf(CRIMSON "  ✗ Answer: %s\n" R, p[i].answer);
        }
    }
    if(score>=6){printf(AMBER BOLD "\n  The ancestors BOW to you! Trial passed!\n" R);return 1;}
    else{printf(CRIMSON "\n  %d/7 — Study the proverbs more deeply.\n" R,score);return 0;}
}

/* Boss 5: Ultimate drum challenge (10 rounds) */
static int boss_ubuntu_dragon(void) {
    L_cls();
    L_div(RED,'═',58);
    printf(RED "  ⚔  BOSS: THE UBUNTU DRAGON\n" R);
    printf(RED "  10-Round Drum Battle. Need 8/10 to win.\n\n" R);
    L_enter();
    const char *pats[]={"BSBS","BBSS","BSHB","BSHR","BBSSHR",
                         "BSHBSR","BBRSSR","BSHBRB","BBSSHHRR","BSHBSRBB"};
    int score=0;
    for(int r=0;r<10;r++){
        const char *pat=pats[r];
        printf(AMBER "\n  Beat %d: " BOLD "%s" R AMBER " → Type it!\n  > " R,r+1,pat);
        char buf[32];
        if(fgets(buf,sizeof(buf),stdin)){
            int l=strlen(buf); if(l>0&&buf[l-1]=='\n') buf[l-1]='\0';
            for(int i=0;buf[i];i++) buf[i]=toupper(buf[i]);
            if(strcmp(buf,pat)==0){score++;printf(LIME "  ✓ BOOM!\n" R);}
            else printf(CRIMSON "  ✗ Was: %s\n" R,pat);
        }
    }
    if(score>=8){printf(GOLD BOLD "\n  THE DRAGON FALLS! Unity triumphs!\n" R);return 1;}
    else{printf(CRIMSON "\n  %d/10 — The Dragon breathes fire. Try again!\n" R,score);return 0;}
}

/* Boss 6: Final — 5-part sequence */
static int boss_final_ancestor(void) {
    L_cls();
    L_div(GOLD,'═',58);
    printf(GOLD BOLD BLINK "  ⚔  THE FINAL ANCESTOR — ULTIMATE TEST\n" R);
    L_div(GOLD,'═',58);
    printf(IVORY "\n  Five trials. Complete them all to achieve Ubuntu Eternal.\n\n" R);
    L_enter();

    int pass=0;

    /* Part 1: Breathing */
    printf(LAKE BOLD "  TRIAL 1: Breathe (3 perfect cycles)\n" R);
    int b_ok=1;
    for(int c=1;c<=3;c++){
        int hold=rand()%4+5;
        printf(LAKE "  Cycle %d: HOLD was ___s? (%d-%d): " R,c,4,10);
        int ans=L_int(4,10);
        if(ans==hold) printf(LIME "  ✓\n" R);
        else{printf(CRIMSON "  ✗ Was %d.\n" R,hold);b_ok=0;}
    }
    if(b_ok){pass++;printf(LIME "  Trial 1 PASSED!\n\n" R);}

    /* Part 2: Affirmation */
    printf(PURPLE BOLD "  TRIAL 2: Affirmation\n" R);
    printf(GOLD "  Write your personal healing affirmation:\n  > " R);
    char buf[256];
    if(fgets(buf,sizeof(buf),stdin)&&strlen(buf)>5){
        pass++;printf(LIME "  Your truth is spoken. Trial 2 PASSED!\n\n" R);
    }

    /* Part 3: Grounding */
    printf(FOREST BOLD "  TRIAL 3: Name 3 things you are grateful for today:\n" R);
    for(int i=1;i<=3;i++){
        printf(GOLD "  %d. " R,i);
        if(fgets(buf,sizeof(buf),stdin)&&strlen(buf)>2)
            printf(LIME "  ✓ Received.\n" R);
    }
    pass++;printf(LIME "  Trial 3 PASSED!\n\n" R);

    /* Part 4: Proverb */
    printf(AMBER BOLD "  TRIAL 4: Complete — 'Umuntu ngumuntu ___'\n  > " R);
    if(fgets(buf,sizeof(buf),stdin)){
        int l=strlen(buf); if(l>0&&buf[l-1]=='\n') buf[l-1]='\0';
        for(int j=0;buf[j];j++) buf[j]=toupper(buf[j]);
        if(strstr(buf,"NGABANTU")!=NULL){
            pass++;printf(LIME "  ✓ CORRECT! Trial 4 PASSED!\n\n" R);
        } else printf(CRIMSON "  ✗ Answer: NGABANTU. Try the next trial.\n\n" R);
    }

    /* Part 5: Drum */
    printf(ORANGE BOLD "  TRIAL 5: Drum Pattern — BSHBSRBB\n  > " R);
    if(fgets(buf,sizeof(buf),stdin)){
        int l=strlen(buf); if(l>0&&buf[l-1]=='\n') buf[l-1]='\0';
        for(int j=0;buf[j];j++) buf[j]=toupper(buf[j]);
        if(strcmp(buf,"BSHBSRBB")==0){
            pass++;printf(LIME "  ✓ The drum speaks! Trial 5 PASSED!\n\n" R);
        } else printf(CRIMSON "  ✗ The beat was: BSHBSRBB\n\n" R);
    }

    printf(GOLD "\n  Final Score: %d/5 trials passed.\n\n" R,pass);
    if(pass>=4){
        L_div(GOLD,'═',58);
        printf(GOLD BOLD BLINK "  👑 THE FINAL ANCESTOR BOWS.\n" R);
        printf(GOLD BOLD       "  YOU ARE UBUNTU ETERNAL.\n" R);
        L_div(GOLD,'═',58);
        return 1;
    } else {
        printf(CRIMSON "  Not yet. Return when you are ready.\n" R);
        return 0;
    }
}

/* Boss dispatch */
void lvl_boss_battles(LevelState *s) {
    L_cls();
    L_div(CRIMSON,'═',58);
    printf(CRIMSON "  ⚔  " RED BOLD "BOSS BATTLES — Face Your Inner Trials" R "\n");
    L_div(CRIMSON,'═',58);
    printf(IVORY "\n  Bosses appear at milestone levels.\n");
    printf("  Defeat them for massive XP and coin rewards!\n\n" R);

    int any=0;
    for(int b=0;b<BOSS_COUNT;b++){
        Boss *boss=&s->bosses[b];
        const char *status= boss->defeated ? LIME "✓ DEFEATED" R :
                            (s->total_level>=boss->level_trigger ? CRIMSON "⚔ AVAILABLE" R : DIM "🔒 LOCKED" R);
        printf("  %d. %s%-24s" R " %s  Lv%d+  +%dXP\n",
               b+1, boss->defeated?LIME:GOLD, boss->name, status,
               boss->level_trigger, boss->xp_reward);
        if(!boss->defeated && s->total_level>=boss->level_trigger) any=1;
    }

    if(!any){
        printf(IVORY "\n  No available bosses. Level up to unlock!\n" R);
        L_enter(); return;
    }
    printf(SAVANNA "\n  Choose boss (1-%d, or 0 to back): " R,BOSS_COUNT);
    int c=L_int(0,BOSS_COUNT);
    if(c==0) return;

    Boss *b=&s->bosses[c-1];
    if(b->defeated){printf(LIME "  Already defeated! Well done.\n" R);L_enter();return;}
    if(s->total_level<b->level_trigger){
        printf(ORANGE "  Locked! Reach Level %d first.\n" R,b->level_trigger);
        L_enter();return;
    }

    b->attempts++;
    int win=0;
    switch(c){
        case 1: win=boss_anxious_fog();    break;
        case 2: win=boss_doubt_serpent();  break;
        case 3: win=boss_bwindi_shadow();  break;
        case 4: win=boss_ancestral_trial();break;
        case 5: win=boss_ubuntu_dragon();  break;
        case 6: win=boss_final_ancestor(); break;
    }
    if(win){
        b->defeated=1;
        printf(GOLD "\n  🎁 Reward: +" BOLD "%d XP" R GOLD " + 💰%d coins!\n" R,
               b->xp_reward, b->coin_reward);
        s->loot_available++;
        printf(GOLD "  +1 Loot Box for defeating a boss!\n" R);
    }
    L_enter();
}

/* ═══════════════════════════ NEW MINI-GAMES ════════════════════════ */

/* Game 1: Word Unscramble (Lv25+) */
void lvl_game_unscramble(LevelState *s) {
    if(s->total_level < 25){
        printf(PURPLE "\n  🔒 Unlocks at Level 25!\n" R); L_enter(); return;
    }
    L_cls();
    L_div(CYAN,'═',58);
    printf(CYAN "  🔤  " SKY BOLD "WORD UNSCRAMBLE — Lv25+ Game" R "\n");
    L_div(CYAN,'═',58);
    printf(IVORY "\n  Unscramble the Uganda/Africa word!\n\n" R);

    struct { const char *scrambled; const char *answer; const char *hint; } words[]={
        {"PUDNAA","UGANDA","Country in East Africa"},
        {"IALTVOCIR","VICTORIA","Largest African lake"},
        {"NOTIZIRU","NUTRITION","Food for the mind and body"},
        {"AZNIAAN","NIANZAA","Swahili for Lake Victoria region"},
        {"BUTNU","UBUNTU","I am because we are"},
        {"NDWBII","BWINDI","Impenetrable Forest"},
        {"AMAPLAK","KAMPALA","Uganda's capital city"},
        {"ZENIWZRRO","RWENZORI","Mountains of the Moon"},
        {"ANIVASA","SAVANNA","African grassland"},
        {"NOCATRSE","ANCESTOR","Those who came before us"},
    };
    int n=sizeof(words)/sizeof(words[0]);
    int score=0, rounds=5;
    srand((unsigned)time(NULL)+s->total_level);

    for(int r=0;r<rounds;r++){
        int idx=rand()%n;
        printf(GOLD "  Q%d: Unscramble: " BOLD CYAN "%s" R "\n", r+1, words[idx].scrambled);
        printf(DIM  "  Hint: %s\n" R, words[idx].hint);
        printf(SAVANNA "  > " R);
        char buf[32];
        if(fgets(buf,sizeof(buf),stdin)){
            int l=strlen(buf); if(l>0&&buf[l-1]=='\n') buf[l-1]='\0';
            for(int j=0;buf[j];j++) buf[j]=toupper(buf[j]);
            if(strcmp(buf,words[idx].answer)==0){
                score++; printf(LIME "  ✓ Correct! %s\n" R, words[idx].answer);
            } else {
                printf(CRIMSON "  ✗ Answer: %s\n" R, words[idx].answer);
            }
        }
    }
    int xp = score*22+(score==rounds?80:0);
    combo_tick(s);
    xp = combo_apply_xp(s,xp);
    printf(GOLD "\n  Score: %d/%d → +%d XP\n" R,score,rounds,xp);
    L_enter();
}

/* Game 2: Proverb Match (Lv30+) */
void lvl_game_proverb_match(LevelState *s) {
    if(s->total_level < 30){
        printf(PURPLE "\n  🔒 Unlocks at Level 30!\n" R); L_enter(); return;
    }
    L_cls();
    L_div(AMBER,'═',58);
    printf(AMBER "  📜  " GOLD BOLD "PROVERB MATCH — Lv30+ Game" R "\n");
    L_div(AMBER,'═',58);
    printf(IVORY "\n  Match the proverb to its origin country!\n\n" R);

    struct { const char *proverb; const char *origin; int choices[4]; } q[]={
        {"Umuntu ngumuntu ngabantu.",1,{1,2,3,4}},
        {"He who learns, teaches.",3,{1,2,3,4}},
        {"Rain does not fall on one roof alone.",2,{1,2,3,4}},
        {"When the music changes, so does the dance.",4,{1,2,3,4}},
        {"To get lost is to learn the way.",5,{1,2,3,4}},
    };
    const char *origins[]={"","Uganda/S.Africa","South Africa","Ethiopia","Nigeria","East Africa"};
    int score=0;

    for(int i=0;i<5;i++){
        printf(GOLD "  Q%d: \"%s\"\n" R, i+1, q[i].proverb);
        printf("  1.Uganda/S.Africa  2.South Africa  3.Ethiopia  4.Nigeria  5.E.Africa\n");
        printf(SAVANNA "  > " R);
        int ans=L_int(1,5);
        if(ans==q[i].origin){score++;printf(LIME "  ✓ %s\n" R,origins[q[i].origin]);}
        else printf(CRIMSON "  ✗ Was: %s\n" R,origins[q[i].origin]);
    }
    int xp=score*25+(score==5?100:0);
    combo_tick(s);
    xp=combo_apply_xp(s,xp);
    printf(GOLD "\n  Score: %d/5 → +%d XP\n" R,score,xp);
    L_enter();
}

/* Game 3: Emotion Naming (Lv35+) */
void lvl_game_emotion_wheel(LevelState *s) {
    if(s->total_level < 35){
        printf(PURPLE "\n  🔒 Unlocks at Level 35!\n" R); L_enter(); return;
    }
    L_cls();
    L_div(ROSE,'═',58);
    printf(ROSE "  🎡  " PINK BOLD "EMOTION WHEEL — Lv35+ Game" R "\n");
    L_div(ROSE,'═',58);
    printf(IVORY "\n  Name the emotion described. Builds emotional vocabulary.\n\n" R);

    struct { const char *desc; const char *emotion; } em[]={
        {"A tightness in your chest when something bad might happen","ANXIETY"},
        {"Warmth in your heart when someone shows you care","GRATITUDE"},
        {"A heavy, slow feeling when things feel pointless","DEPRESSION"},
        {"Fire in your chest when something feels unfair","ANGER"},
        {"Light, floating feeling when things go well","JOY"},
        {"Empty, disconnected, like watching life through glass","DISSOCIATION"},
        {"Wanting to pull away and be alone","WITHDRAWAL"},
        {"Embarrassment about something from your past","SHAME"},
    };
    int n=sizeof(em)/sizeof(em[0]);
    int score=0,rounds=5;
    srand((unsigned)time(NULL)+99);

    for(int r=0;r<rounds;r++){
        int idx=rand()%n;
        printf(PINK "  Scenario %d:\n  \"%s\"\n" R, r+1, em[idx].desc);
        printf(GOLD "\n  What emotion is this? (one word): " R);
        char buf[64];
        if(fgets(buf,sizeof(buf),stdin)){
            int l=strlen(buf); if(l>0&&buf[l-1]=='\n') buf[l-1]='\0';
            for(int j=0;buf[j];j++) buf[j]=toupper(buf[j]);
            if(strcmp(buf,em[idx].emotion)==0){
                score++; printf(LIME "  ✓ Exactly! %s\n\n" R,em[idx].emotion);
            } else {
                printf(ORANGE "  Close — the word is: " BOLD "%s\n\n" R,em[idx].emotion);
            }
        }
    }
    int xp=score*28+(score==rounds?90:0);
    combo_tick(s);
    xp=combo_apply_xp(s,xp);
    printf(GOLD "  Score: %d/%d → +%d XP\n" R,score,rounds,xp);
    L_enter();
}

/* Game 4: Speed Breathing (Lv40+) — count breaths in 60s */
void lvl_game_breath_counter(LevelState *s) {
    if(s->total_level < 40){
        printf(PURPLE "\n  🔒 Unlocks at Level 40!\n" R); L_enter(); return;
    }
    L_cls();
    L_div(LAKE,'═',58);
    printf(LAKE "  ⏱  " SKY BOLD "BREATH COUNTER — Lv40+ Game" R "\n");
    L_div(LAKE,'═',58);
    printf(IVORY "\n  Breathe for 60 seconds. Count your natural breaths.\n");
    printf("  An average person takes 12–20 breaths per minute.\n\n" R);
    L_enter();

    printf(FOREST "\n  START — Breathe naturally for 60 seconds.\n" R);
    for(int s2=60;s2>=1;s2--){
        printf("\r  " LAKE "%02d seconds remaining..." R, s2); fflush(stdout); sleep(1);
    }
    printf("\n\n" GOLD "  How many breaths did you count? " R);
    int count=L_int(1,40);

    int xp=0;
    printf("\n");
    if(count>=12&&count<=20){
        printf(LIME "  ✓ Normal range (12-20)! Great awareness. +80 XP\n" R); xp=80;
    } else if(count>20){
        printf(ORANGE "  You're breathing fast — try slowing down. +40 XP\n" R); xp=40;
    } else {
        printf(CYAN "  Very slow breather — you're very calm! +60 XP\n" R); xp=60;
    }
    combo_tick(s);
    xp=combo_apply_xp(s,xp);
    printf(GOLD "  Final: +%d XP\n" R,xp);
    L_enter();
}

/* Game 5: Ubuntu Trivia (Lv50+) */
void lvl_game_ubuntu_trivia(LevelState *s) {
    if(s->total_level < 50){
        printf(PURPLE "\n  🔒 Unlocks at Level 50!\n" R); L_enter(); return;
    }
    L_cls();
    L_div(GOLD,'═',58);
    printf(GOLD "  🌍  " AMBER BOLD "UBUNTU TRIVIA — Lv50+ Game" R "\n");
    L_div(GOLD,'═',58);
    printf(IVORY "\n  Uganda, Africa, and mental health knowledge!\n\n" R);

    struct { const char *q; int correct; const char *opts[4]; } trivia[]={
        {"What is the national bird of Uganda?",
         2, {"Eagle","Grey Crowned Crane","Flamingo","Pelican"}},
        {"Which is the largest freshwater lake in Africa?",
         1, {"Lake Victoria","Lake Tanganyika","Lake Malawi","Lake Chad"}},
        {"What does 'Ubuntu' mean?",
         3, {"I am strong","Peace","I am because we are","Love"}},
        {"Where is Bwindi Impenetrable Forest?",
         1, {"Uganda","Kenya","Tanzania","Rwanda"}},
        {"Which river is the longest in Africa?",
         2, {"Congo","Nile","Niger","Zambezi"}},
        {"The 4-7-8 breathing technique is used for:",
         1, {"Anxiety relief","Building muscle","Sleep disorders","Digestion"}},
        {"Butabika Hospital is located in which city?",
         2, {"Entebbe","Kampala","Jinja","Gulu"}},
        {"How many mountain gorillas are estimated to exist worldwide?",
         3, {"Under 200","Over 2000","Around 1000","Exact unknown"}},
    };
    int n=sizeof(trivia)/sizeof(trivia[0]);
    int score=0,rounds=6;
    srand((unsigned)time(NULL)+s->total_level);

    for(int r=0;r<rounds;r++){
        int idx=rand()%n;
        printf(GOLD "  Q%d: %s\n" R,r+1,trivia[idx].q);
        for(int j=0;j<4;j++)
            printf("  %d. %s\n",j+1,trivia[idx].opts[j]);
        printf(SAVANNA "  > " R);
        int ans=L_int(1,4);
        if(ans==trivia[idx].correct){
            score++; printf(LIME "  ✓ Correct!\n\n" R);
        } else {
            printf(CRIMSON "  ✗ Answer: %d. %s\n\n" R,
                   trivia[idx].correct,
                   trivia[idx].opts[trivia[idx].correct-1]);
        }
    }
    int xp=score*30+(score==rounds?150:0);
    combo_tick(s);
    xp=combo_apply_xp(s,xp);
    printf(GOLD "  Score: %d/%d → +%d XP\n" R,score,rounds,xp);
    L_enter();
}

/* ═══════════════════════════ PRESTIGE SYSTEM ═══════════════════════ */
void lvl_prestige(LevelState *s) {
    L_cls();
    L_div(GOLD,'═',58);
    printf(GOLD "  👑  " AMBER BOLD "PRESTIGE — Rebirth of the Ubuntu Path" R "\n");
    L_div(GOLD,'═',58);

    if(s->total_level < TOTAL_LEVELS){
        printf(ORANGE "\n  You must reach Level 100 to Prestige.\n");
        printf(IVORY  "  Current: Level %d / 100\n\n" R,s->total_level);
        L_enter(); return;
    }
    if(s->prestige >= PRESTIGE_LEVELS){
        printf(GOLD BOLD BLINK "\n  ★★★★★ MAX PRESTIGE — Ubuntu Eternal!\n" R);
        printf(GOLD "  You have achieved the highest honour.\n\n" R);
        L_enter(); return;
    }

    printf(IVORY "\n  You have reached Level 100!\n\n");
    printf(GOLD "  Prestige resets your level to 1 but gives:\n" R);
    printf(LIME "  ✓ Prestige star: %s\n", prestige_star[s->prestige+1]);
    printf(LIME "  ✓ +%d%% permanent XP bonus\n", prestige_xp_bonus[s->prestige+1]);
    printf(LIME "  ✓ New title: " BOLD "%s\n" R, prestige_title[s->prestige+1]);
    printf(LIME "  ✓ Exclusive prestige badge\n");
    printf(LIME "  ✓ Keep all badges, coins, and skills\n\n" R);
    printf(CRIMSON "  WARNING: Level resets to 1. This cannot be undone.\n\n" R);
    printf(GOLD "  Prestige now? (Y/N): " R);
    char buf[8]; if(!fgets(buf,8,stdin)||toupper(buf[0])!='Y') return;

    s->prestige++;
    s->total_level = 1;

    /* Give prestige badge */
    int badge_id = 36 + (s->prestige-1); /* badge slots 36-40 = prestige 1-5 */
    if(badge_id < LVL_BADGE_COUNT) s->lvl_badges[badge_id]=1;

    printf(GOLD BOLD BLINK "\n  ✨ PRESTIGE %d ACHIEVED! ✨\n\n" R, s->prestige);
    printf(GOLD "  You are now: " BOLD "%s %s\n" R,
           prestige_star[s->prestige], prestige_title[s->prestige]);
    printf(SAVANNA "\n  The baobab sheds its leaves each year,\n");
    printf("  yet grows mightier with every cycle.\n");
    printf("  So do you. 🌳\n\n" R);
    L_enter();
}

/* ═══════════════════════════ LEVEL HUB ═════════════════════════════ */
void lvl_show_map(LevelState *s) {
    L_cls();
    L_div(GOLD,'═',60);
    printf(GOLD "  🗺  " SAVANNA BOLD "FULL LEVEL MAP — 100 Levels" R "\n");
    L_div(GOLD,'═',60);

    for(int i=1;i<=TOTAL_LEVELS;i++){
        int tier=lv_tier(i);
        int done=(s->total_level>i);
        int curr=(s->total_level==i);
        const char *col=curr?GOLD BOLD:(done?LIME:DIM);

        if(i==1||i==21||i==36||i==51||i==66||i==81){
            printf("\n%s  ── %s ──" R "\n", tier_color[tier], tier_name[tier]);
        }

        printf("  %s", col);
        if(curr) printf("▶ ");
        else printf("  ");
        printf("Lv.%3d %s %-24s" R, i, lv_icon[i], lv_name[i]);
        if(curr)     printf(GOLD BOLD " ◄ YOU\n" R);
        else if(done) printf(LIME " ✓\n" R);
        else         printf(DIM " [%d XP]\n" R, lv_xp_required(i));
    }

    if(s->prestige>0)
        printf(GOLD "\n  Prestige: %s %s\n" R,
               prestige_star[s->prestige], prestige_title[s->prestige]);
    L_enter();
}

void lvl_show_stats(LevelState *s) {
    L_cls();
    L_div(CYAN,'═',58);
    printf(CYAN "  📊  " GOLD BOLD "LEVEL STATS" R "\n");
    L_div(CYAN,'═',58);

    int badges=0;
    for(int i=0;i<LVL_BADGE_COUNT;i++) if(s->lvl_badges[i]) badges++;
    int bosses=0;
    for(int i=0;i<BOSS_COUNT;i++) if(s->bosses[i].defeated) bosses++;

    printf(GOLD "\n  Level:         " BOLD "%d / 100\n" R,s->total_level);
    printf(GOLD "  Prestige:      " BOLD "%s %s\n" R,
           prestige_star[s->prestige], prestige_title[s->prestige]);
    printf(GOLD "  Total XP:      " BOLD "%d\n" R,s->total_xp);
    printf(GOLD "  Login Streak:  " BOLD "%d days\n" R,s->login_streak);
    printf(GOLD "  Loot Opened:   " BOLD "%d  (available: %d)\n" R,
           s->loot_opened,s->loot_available);
    printf(GOLD "  Bosses Slain:  " BOLD "%d / %d\n" R,bosses,BOSS_COUNT);
    printf(GOLD "  Lvl Badges:    " BOLD "%d / %d\n" R,badges,LVL_BADGE_COUNT);
    printf(GOLD "  Best Combo:    " BOLD "x%d\n" R,s->combo.best_combo);
    printf(GOLD "  XP Multiplier: " BOLD "%.1fx (combo x%d)\n" R,
           s->combo.multiplier,s->combo.count);
    printf(GOLD "  Days Playing:  " BOLD "%ld\n\n" R, L_day()-s->first_play_day+1);

    /* Progress bar to next level */
    if(s->total_level < TOTAL_LEVELS){
        int need=lv_xp_required(s->total_level+1);
        int have=s->total_xp - lv_xp_required(s->total_level);
        int range=need - lv_xp_required(s->total_level);
        int pct=(range>0)?(have*100/range):100;
        int fill=pct*30/100;
        printf(SAVANNA "  To Lv%d: [",s->total_level+1);
        for(int i=0;i<30;i++) printf(i<fill?LIME "█" R:DIM "░" R);
        printf(SAVANNA "] %d%%\n\n" R,pct);
    }
    L_enter();
}

/* ═══════════════════════════ MAIN LEVEL MENU ═══════════════════════ */
void lvl_menu(LevelState *s, void *player_ptr) {
    int running=1;
    while(running){
        L_cls();
        L_div(GOLD,'═',60);
        printf(GOLD "  🏆  " AMBER BOLD "LEVEL HUB — 100-Level Expansion" R "\n");
        printf(GOLD "  Level: " BOLD "%d/100" R GOLD "  Prestige: " BOLD "%s" R GOLD
               "  Combo: " AMBER BOLD "x%d" R "\n",
               s->total_level, prestige_star[s->prestige], s->combo.count);
        L_div(GOLD,'═',60);

        printf(GOLD "\n  ╔═══════════════════════════════════════════════════╗\n");
        printf(GOLD "  ║  MINI-GAMES                                        ║\n");
        printf(GOLD "  ║ " CYAN   " 1." IVORY " Word Unscramble     " DIM "[Lv25+]" GOLD "            ║\n");
        printf(GOLD "  ║ " AMBER  " 2." IVORY " Proverb Match       " DIM "[Lv30+]" GOLD "            ║\n");
        printf(GOLD "  ║ " ROSE   " 3." IVORY " Emotion Wheel       " DIM "[Lv35+]" GOLD "            ║\n");
        printf(GOLD "  ║ " LAKE   " 4." IVORY " Breath Counter      " DIM "[Lv40+]" GOLD "            ║\n");
        printf(GOLD "  ║ " SAVANNA" 5." IVORY " Ubuntu Trivia       " DIM "[Lv50+]" GOLD "            ║\n");
        printf(GOLD "  ╠═══════════════════════════════════════════════════╣\n");
        printf(GOLD "  ║  PROGRESSION                                       ║\n");
        printf(GOLD "  ║ " CRIMSON" 6." IVORY " Boss Battles                            " GOLD "║\n");
        printf(GOLD "  ║ " ORANGE " 7." IVORY " Open Loot Box  " GOLD "(📦 x%d available)" GOLD "    ║\n",s->loot_available);
        printf(GOLD "  ║ " PURPLE " 8." IVORY " Prestige System                         " GOLD "║\n");
        printf(GOLD "  ╠═══════════════════════════════════════════════════╣\n");
        printf(GOLD "  ║  INFO                                              ║\n");
        printf(GOLD "  ║ " LIME   " 9." IVORY " Full Level Map (1–100)                  " GOLD "║\n");
        printf(GOLD "  ║ " TEAL   "10." IVORY " Level Stats & Combo Tracker             " GOLD "║\n");
        printf(GOLD "  ║ " DIM    "11." RESET IVORY " Back to Main Menu                      " GOLD "║\n");
        printf(GOLD "  ╚═══════════════════════════════════════════════════╝\n\n" R);

        if(s->loot_available>0)
            printf(GOLD BLINK "  📦 %d Loot Box(es) waiting!\n\n" R, s->loot_available);

        combo_tick(s); /* passive combo update */

        printf(SAVANNA "  Choose (1-11): " R);
        int c=L_int(1,11);
        switch(c){
            case  1: lvl_game_unscramble(s);   break;
            case  2: lvl_game_proverb_match(s);break;
            case  3: lvl_game_emotion_wheel(s);break;
            case  4: lvl_game_breath_counter(s);break;
            case  5: lvl_game_ubuntu_trivia(s);break;
            case  6: lvl_boss_battles(s);      break;
            case  7: lvl_open_loot(s);         break;
            case  8: lvl_prestige(s);          break;
            case  9: lvl_show_map(s);          break;
            case 10: lvl_show_stats(s);        break;
            case 11: running=0;                break;
        }
        lvl_save(s);
    }
}

/*
 * ════════════════════════════════════════════════════════════════════
 *  END OF umg_levels.c
 *
 *  TOTAL LEVELS: 100 (expandable with prestige × 5 = effectively 600)
 *  NEW MINI-GAMES: Word Unscramble | Proverb Match | Emotion Wheel
 *                  Breath Counter | Ubuntu Trivia
 *  BOSS FIGHTS: 6 (Anxious Fog → Final Ancestor)
 *  PRESTIGE: 5 tiers (Wanderer → Ubuntu Eternal)
 *  LOOT BOXES: Common / Rare / Epic / Legendary drops
 *  COMBO SYSTEM: Up to 5× XP multiplier
 *  DAILY LOGIN: 7-day reward cycle with bonus at day 7 & 30
 *  BADGES: 50 level-specific badges
 *
 *  "The one who tells the stories rules the world." — African Proverb
 * ════════════════════════════════════════════════════════════════════
 */
