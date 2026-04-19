/*
 * ================================================================
 *   UBUNTU MIND GARDEN - A Mental Wellness Game
 *   "Umuntu ngumuntu ngabantu" - I am because we are
 *
 *   Inspired by Ugandan & African culture, nature, and community
 *   A terminal game to support mental health through mindfulness,
 *   breathing exercises, affirmations, and grounding techniques.
 *
 *   Compile: gcc ubuntu_mindgarden.c -o ubuntu_mindgarden -lm
 *   Run:     ./ubuntu_mindgarden
 * ================================================================
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <math.h>
#include <unistd.h>

/* ──────────────────────── ANSI COLOR CODES ──────────────────────── */
#define RESET       "\033[0m"
#define BOLD        "\033[1m"
#define DIM         "\033[2m"
#define ITALIC      "\033[3m"

/* Ugandan sunrise palette */
#define GOLD        "\033[38;5;220m"
#define ORANGE      "\033[38;5;208m"
#define CRIMSON     "\033[38;5;160m"
#define EARTH       "\033[38;5;130m"
#define SAVANNA     "\033[38;5;178m"
#define FOREST      "\033[38;5;34m"
#define JUNGLE      "\033[38;5;28m"
#define LAKE        "\033[38;5;33m"
#define SKY         "\033[38;5;117m"
#define MIST        "\033[38;5;189m"
#define IVORY       "\033[38;5;230m"
#define PURPLE      "\033[38;5;135m"
#define PINK        "\033[38;5;211m"

/* Background accents */
#define BG_DARK     "\033[48;5;232m"
#define BG_EARTH    "\033[48;5;52m"
#define BG_FOREST   "\033[48;5;22m"

/* ──────────────────────── GAME CONSTANTS ──────────────────────── */
#define MAX_NAME        64
#define MAX_JOURNAL     512
#define JOURNAL_ENTRIES 5
#define AFFIRMATION_COUNT 20
#define PROVERB_COUNT     15
#define BREATHING_CYCLES  4

/* ──────────────────────── DATA STRUCTURES ──────────────────────── */
typedef struct {
    char name[MAX_NAME];
    int  mood;          /* 1-10 */
    int  anxiety;       /* 1-10 */
    int  energy;        /* 1-10 */
    int  sessions;
    int  seeds_planted; /* progress tokens */
    int  garden_stage;  /* 0=seeds 1=sprout 2=sapling 3=tree 4=forest */
    char journal[JOURNAL_ENTRIES][MAX_JOURNAL];
    int  journal_count;
    time_t last_visit;
} Player;

typedef struct {
    const char *text;
    const char *author;
    const char *origin;
} Proverb;

/* ──────────────────────── UGANDAN & AFRICAN CONTENT ──────────────────────── */

static const char *affirmations[AFFIRMATION_COUNT] = {
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
    "I rise again, like Africa — ancient, resilient, magnificent.",
    "My ancestors walked through storms; I carry their courage.",
    "Joy lives in me, like the drumbeat of a Kampala celebration.",
    "I am more powerful than my fear.",
    "Every breath I take is a gift from the Ugandan hills.",
    "I welcome calm the way the savanna welcomes the dawn.",
    "My mind is clearing like morning mist over the Nile.",
    "I am loved, I am enough, I am here.",
    "Growth takes time — the baobab did not become mighty overnight.",
    "I choose peace. I choose myself. I choose today."
};

static const Proverb proverbs[PROVERB_COUNT] = {
    {"Umuntu ngumuntu ngabantu.",         "Nguni peoples",   "Uganda/Southern Africa"},
    {"I am because we are.",              "Ubuntu philosophy","Pan-African"},
    {"The axe forgets, but the tree remembers — and still grows.", "Shona proverb", "Zimbabwe"},
    {"Rain does not fall on one roof alone.",  "Zulu proverb",   "South Africa"},
    {"Speak softly and carry a big stick — of patience.", "Buganda saying", "Uganda"},
    {"The forest would be silent if no bird sang except the best.", "African proverb", "West Africa"},
    {"A child who is not embraced by the village will burn it down.", "African proverb", "East Africa"},
    {"Sticks in a bundle are unbreakable.",   "Bondei proverb", "Tanzania"},
    {"Until the lion learns to write, every story glorifies the hunter — rewrite yours.", "African proverb", "Pan-African"},
    {"He who learns, teaches.",               "Ethiopian proverb","Ethiopia"},
    {"The wise do not sit still and weep — they plant.",     "Ugandan saying", "Uganda"},
    {"However long the night, the dawn will break.",         "African proverb", "Pan-African"},
    {"When the music changes, so does the dance.",           "Hausa proverb",  "Nigeria"},
    {"A boat cannot go forward if each rows his own way.",   "Swahili proverb","East Africa"},
    {"Do not look where you fell, but where you slipped.",   "African proverb", "Pan-African"}
};

static const char *uganda_animals[] = {
    "🦍 Mountain Gorilla", "🦩 Grey Crowned Crane",
    "🐘 African Elephant", "🦛 White Rhino",
    "🐆 Leopard",          "🦜 African Grey Parrot",
    "🐊 Nile Crocodile",   "🦁 Lion",
    "🦒 Giraffe",          "🦋 African Swallowtail"
};

/* ──────────────────────── UTILITY FUNCTIONS ──────────────────────── */

void clear_screen(void) {
    printf("\033[2J\033[H");
    fflush(stdout);
}

void sleep_ms(int ms) {
    usleep(ms * 1000);
}

void print_divider(const char *color, char ch, int width) {
    printf("%s", color);
    for (int i = 0; i < width; i++) putchar(ch);
    printf("%s\n", RESET);
}

void typewrite(const char *color, const char *text, int delay_ms) {
    printf("%s", color);
    while (*text) {
        putchar(*text++);
        fflush(stdout);
        sleep_ms(delay_ms);
    }
    printf("%s", RESET);
}

void press_enter(void) {
    printf("\n%s  [ Press ENTER to continue ]%s\n", DIM, RESET);
    while (getchar() != '\n');
}

int get_int_input(int min, int max) {
    int val;
    char buf[32];
    while (1) {
        if (fgets(buf, sizeof(buf), stdin)) {
            if (sscanf(buf, "%d", &val) == 1 && val >= min && val <= max)
                return val;
        }
        printf("%s  Invalid. Enter a number %d-%d: %s", ORANGE, min, max, RESET);
    }
}

/* ──────────────────────── BANNER & VISUALS ──────────────────────── */

void draw_sun(void) {
    printf(GOLD "          \\   |   /\n");
    printf(GOLD "       ─── " ORANGE "☀" GOLD " ───\n");
    printf(GOLD "          /   |   \\\n" RESET);
}

void draw_title_screen(void) {
    clear_screen();
    printf("\n\n");
    printf(GOLD   "  ╔══════════════════════════════════════════════════════╗\n");
    printf(GOLD   "  ║                                                      ║\n");
    printf(ORANGE "  ║   " GOLD BOLD "  ██╗   ██╗██████╗ ██╗   ██╗███╗  ██╗████████╗██╗   " ORANGE "║\n");
    printf(ORANGE "  ║   " GOLD BOLD "  ██║   ██║██╔══██╗██║   ██║████╗ ██║╚══██╔══╝██║   " ORANGE "║\n");
    printf(ORANGE "  ║   " GOLD BOLD "  ██║   ██║██████╔╝██║   ██║██╔██╗██║   ██║   ██║   " ORANGE "║\n");
    printf(ORANGE "  ║   " GOLD BOLD "  ██║   ██║██╔══██╗██║   ██║██║╚████║   ██║   ██║   " ORANGE "║\n");
    printf(ORANGE "  ║   " GOLD BOLD "  ╚██████╔╝██████╔╝╚██████╔╝██║ ╚███║   ██║   ██║   " ORANGE "║\n");
    printf(ORANGE "  ║   " GOLD BOLD "   ╚═════╝ ╚═════╝  ╚═════╝ ╚═╝  ╚══╝   ╚═╝   ╚═╝  " ORANGE "║\n");
    printf(GOLD   "  ║                                                      ║\n");
    printf(EARTH  "  ║          " SAVANNA BOLD "M I N D   G A R D E N" EARTH "                       ║\n");
    printf(FOREST "  ║    " IVORY "A Mental Wellness Journey through Uganda & Africa" FOREST "   ║\n");
    printf(GOLD   "  ║                                                      ║\n");
    printf(GOLD   "  ╚══════════════════════════════════════════════════════╝\n\n" RESET);

    draw_sun();
    printf("\n");
    printf(FOREST "  🌿  " JUNGLE "🌳  " FOREST "🦩  " SAVANNA "🌾  " EARTH "🏔  " LAKE "🌊  " FOREST "🌿\n\n" RESET);

    printf(ITALIC MIST "  \"Umuntu ngumuntu ngabantu\"\n");
    printf("   I am because we are — Ubuntu Philosophy\n\n" RESET);
}

void draw_garden(int stage) {
    printf("\n  %sYour Mind Garden:%s\n", GOLD, RESET);
    if (stage == 0) {
        printf(EARTH "  [ 🌱 🌱 🌱 — Seeds planted, waiting to grow ]\n" RESET);
    } else if (stage == 1) {
        printf(FOREST "  [ 🌿 🌿 🌿 — Tiny sprouts reaching for light ]\n" RESET);
    } else if (stage == 2) {
        printf(JUNGLE "  [ 🌲 🌲 🌲 — Saplings growing strong ]\n" RESET);
    } else if (stage == 3) {
        printf(FOREST "  [ 🌳 🌳 🌳 — A mighty tree stands ]\n" RESET);
    } else {
        printf(JUNGLE "  [ 🌳🌿🦋🌸🦜🌳 — Your forest is thriving! ]\n" RESET);
    }
}

void draw_mood_bar(const char *label, const char *color, int value, int max) {
    printf("  %s%-12s%s [", color, label, RESET);
    int filled = (value * 20) / max;
    for (int i = 0; i < 20; i++) {
        if (i < filled) printf("%s█%s", color, RESET);
        else printf(DIM "░" RESET);
    }
    printf("] %s%d/%d%s\n", BOLD, value, max, RESET);
}

void draw_dashboard(Player *p) {
    clear_screen();
    printf("\n");
    print_divider(GOLD, '═', 56);
    printf(GOLD "  🌄  " ORANGE BOLD "UBUNTU MIND GARDEN" RESET GOLD " — Welcome back, %s!\n", p->name);
    print_divider(GOLD, '═', 56);

    draw_garden(p->garden_stage);
    printf("\n");
    draw_mood_bar("Mood",    ORANGE, p->mood,    10);
    draw_mood_bar("Calm",    LAKE,   10-p->anxiety, 10);
    draw_mood_bar("Energy",  GOLD,   p->energy,  10);
    printf("\n");
    printf("  %s🌱 Seeds Planted: %d   📅 Sessions: %d%s\n",
           SAVANNA, p->seeds_planted, p->sessions, RESET);
    print_divider(EARTH, '─', 56);
    printf("\n");
}

/* ──────────────────────── BREATHING EXERCISE ──────────────────────── */

void breathing_exercise(Player *p) {
    clear_screen();
    printf("\n");
    print_divider(LAKE, '═', 56);
    printf(LAKE "  🌊  " SKY BOLD "4-7-8 BREATHING — Lake Victoria Tide" RESET "\n");
    print_divider(LAKE, '═', 56);
    printf("\n");
    printf(IVORY "  Breathe with the rhythm of Lake Victoria.\n");
    printf("  This technique calms your nervous system.\n\n" RESET);
    printf(DIM "  Inhale 4s  →  Hold 7s  →  Exhale 8s\n\n" RESET);
    press_enter();

    for (int cycle = 1; cycle <= BREATHING_CYCLES; cycle++) {
        /* INHALE */
        printf("\r  " FOREST "🌬  INHALE... breathe in the forest air   " RESET);
        fflush(stdout);
        for (int s = 4; s >= 1; s--) {
            printf("\r  " FOREST "🌬  INHALE — %d seconds...              " RESET, s);
            fflush(stdout);
            sleep(1);
        }

        /* HOLD */
        printf("\r  " GOLD "✨  HOLD — feel the stillness inside...   " RESET);
        fflush(stdout);
        for (int s = 7; s >= 1; s--) {
            printf("\r  " GOLD "✨  HOLD — %d seconds...                " RESET, s);
            fflush(stdout);
            sleep(1);
        }

        /* EXHALE */
        printf("\r  " LAKE "🌊  EXHALE — let the Nile carry it away  " RESET);
        fflush(stdout);
        for (int s = 8; s >= 1; s--) {
            printf("\r  " LAKE "🌊  EXHALE — %d seconds...              " RESET, s);
            fflush(stdout);
            sleep(1);
        }

        printf("\n  " SAVANNA "Cycle %d of %d complete 🌿\n\n" RESET, cycle, BREATHING_CYCLES);
    }

    printf(GOLD "\n  ✅ Breathing complete. Your nervous system is calmer.\n" RESET);
    p->anxiety = (p->anxiety > 2) ? p->anxiety - 2 : 1;
    p->mood    = (p->mood < 10)   ? p->mood + 1    : 10;
    p->seeds_planted++;
    printf(FOREST "  🌱 A seed of calm was planted in your garden!\n" RESET);
    press_enter();
}

/* ──────────────────────── MOOD CHECK-IN ──────────────────────── */

void mood_checkin(Player *p) {
    clear_screen();
    printf("\n");
    print_divider(ORANGE, '═', 56);
    printf(ORANGE "  🌅  " GOLD BOLD "DAILY CHECK-IN — Sunrise Reflection" RESET "\n");
    print_divider(ORANGE, '═', 56);
    printf("\n");
    typewrite(IVORY, "  Like watching the sun rise over Kampala, let us\n", 18);
    typewrite(IVORY, "  check in honestly — no judgment, only truth.\n\n", 18);

    printf(GOLD "  How is your MOOD today? (1=very low, 10=wonderful)\n  > " RESET);
    p->mood = get_int_input(1, 10);

    printf(ORANGE "\n  How is your ANXIETY level? (1=very anxious, 10=very calm)\n  > " RESET);
    int calm = get_int_input(1, 10);
    p->anxiety = 11 - calm;

    printf(FOREST "\n  How is your ENERGY? (1=exhausted, 10=full of life)\n  > " RESET);
    p->energy = get_int_input(1, 10);

    printf("\n");
    print_divider(EARTH, '─', 56);

    /* Personalized response */
    if (p->mood <= 3) {
        typewrite(CRIMSON, "\n  The baobab tree survives the longest drought.\n", 20);
        typewrite(IVORY,   "  Your feelings are valid. You are not alone.\n", 20);
        typewrite(FOREST,  "  Try the breathing exercise — it will help.\n", 20);
    } else if (p->mood <= 6) {
        typewrite(SAVANNA, "\n  You are in the middle of your journey — keep walking.\n", 20);
        typewrite(IVORY,   "  Even the Nile rests in eddies before flowing forward.\n", 20);
    } else {
        typewrite(GOLD,   "\n  Your energy shines like an African sunrise!\n", 20);
        typewrite(FOREST, "  Share this light with your community today.\n", 20);
    }

    p->seeds_planted++;
    p->sessions++;
    printf(FOREST "\n  🌱 Check-in complete. Garden seed planted!\n" RESET);
    press_enter();
}

/* ──────────────────────── AFFIRMATION ──────────────────────── */

void daily_affirmation(Player *p) {
    clear_screen();
    printf("\n");
    print_divider(PURPLE, '═', 56);
    printf(PURPLE "  🦩  " PINK BOLD "AFFIRMATION — Words of the Ancestors" RESET "\n");
    print_divider(PURPLE, '═', 56);
    printf("\n");

    srand((unsigned)time(NULL));
    int idx = rand() % AFFIRMATION_COUNT;

    printf(IVORY "  Close your eyes. Breathe deeply.\n");
    printf("  When you are ready, read your affirmation:\n\n" RESET);
    sleep(2);

    print_divider(GOLD, '─', 56);
    printf("\n");
    printf(GOLD BOLD "  « ");
    typewrite(GOLD BOLD, affirmations[idx], 30);
    printf(" »\n\n" RESET);
    print_divider(GOLD, '─', 56);

    printf(IVORY "\n  Say this aloud three times.\n");
    printf("  Feel it. Believe it. It is true.\n\n" RESET);

    sleep(3);

    /* Bonus proverb */
    int pidx = rand() % PROVERB_COUNT;
    printf(EARTH "  ─── African Wisdom ───────────────────────────\n");
    printf(SAVANNA "  \"%s\"\n", proverbs[pidx].text);
    printf(DIM "   — %s (%s)\n\n" RESET, proverbs[pidx].author, proverbs[pidx].origin);

    p->seeds_planted++;
    p->mood = (p->mood < 10) ? p->mood + 1 : 10;
    printf(FOREST "  🌱 Affirmation complete. Confidence seed planted!\n" RESET);
    press_enter();
}

/* ──────────────────────── 5-4-3-2-1 GROUNDING ──────────────────────── */

void grounding_exercise(Player *p) {
    clear_screen();
    printf("\n");
    print_divider(FOREST, '═', 56);
    printf(FOREST "  🌿  " JUNGLE BOLD "GROUNDING — Bwindi Forest Technique" RESET "\n");
    print_divider(FOREST, '═', 56);
    printf("\n");
    printf(IVORY "  Imagine you are in the Bwindi Impenetrable Forest.\n");
    printf("  The mountain gorillas rest nearby. You are safe.\n\n" RESET);
    printf(DIM "  This 5-4-3-2-1 exercise anchors you to the present.\n\n" RESET);

    struct { int count; const char *sense; const char *prompt; const char *color; } senses[] = {
        {5, "SEE",   "Name 5 things you can SEE right now:", LAKE},
        {4, "TOUCH", "Name 4 things you can TOUCH or FEEL:", EARTH},
        {3, "HEAR",  "Name 3 things you can HEAR:", FOREST},
        {2, "SMELL", "Name 2 things you can SMELL:", SAVANNA},
        {1, "TASTE", "Name 1 thing you can TASTE:", ORANGE}
    };

    for (int i = 0; i < 5; i++) {
        printf("%s\n  %d — %s\n%s", senses[i].color, senses[i].count, senses[i].prompt, RESET);
        printf(DIM "  (Take your time. Type each one and press ENTER.)\n" RESET);
        char buf[128];
        for (int j = 0; j < senses[i].count; j++) {
            printf(GOLD "  %d. " RESET, j + 1);
            fflush(stdout);
            if (fgets(buf, sizeof(buf), stdin)) {
                /* just acknowledge */
                printf(FOREST "     ✓ Noticed.\n" RESET);
            }
        }
        printf("\n");
    }

    printf(GOLD "  ✅ You are HERE. You are PRESENT. You are SAFE.\n" RESET);
    printf(FOREST "  You walked through the forest and came back to yourself.\n\n" RESET);

    p->anxiety = (p->anxiety > 3) ? p->anxiety - 3 : 1;
    p->mood    = (p->mood < 10)   ? p->mood + 2    : 10;
    p->seeds_planted += 2;
    printf(FOREST "  🌱🌱 Two seeds of presence planted in your garden!\n" RESET);
    press_enter();
}

/* ──────────────────────── ANIMAL SPIRIT GUIDE ──────────────────────── */

void animal_spirit(void) {
    clear_screen();
    printf("\n");
    print_divider(SAVANNA, '═', 56);
    printf(SAVANNA "  🦁  " GOLD BOLD "ANIMAL SPIRIT — Uganda Wildlife Guide" RESET "\n");
    print_divider(SAVANNA, '═', 56);
    printf("\n");
    printf(IVORY "  The wildlife of Uganda carries ancient wisdom.\n");
    printf("  Let one find you today...\n\n" RESET);

    sleep(2);
    srand((unsigned)time(NULL) + 42);
    int idx = rand() % 10;

    printf(GOLD BOLD "\n  Your spirit animal today: %s\n\n" RESET, uganda_animals[idx]);

    const char *messages[] = {
        "The Mountain Gorilla reminds you: strength comes from gentleness and community.",
        "The Grey Crowned Crane, Uganda's symbol, says: stand tall — you are royalty.",
        "The Elephant teaches: your memory holds wisdom, your size holds power — use both kindly.",
        "The White Rhino shows you: survival takes resilience and adaptability.",
        "The Leopard whispers: you can thrive in the darkness as much as the light.",
        "The African Grey Parrot says: your voice matters — speak your truth today.",
        "The Nile Crocodile teaches: patience and stillness are forms of strength.",
        "The Lion roars: courage is not the absence of fear — it is moving forward anyway.",
        "The Giraffe shows you: rise above — your unique perspective is your gift.",
        "The African Swallowtail butterfly says: transformation is beautiful, not painful."
    };

    print_divider(EARTH, '─', 56);
    printf("\n");
    typewrite(SAVANNA, "  ", 0);
    typewrite(IVORY, messages[idx], 25);
    printf("\n\n" RESET);
    print_divider(EARTH, '─', 56);

    press_enter();
}

/* ──────────────────────── JOURNAL ──────────────────────── */

void journal_entry(Player *p) {
    clear_screen();
    printf("\n");
    print_divider(EARTH, '═', 56);
    printf(EARTH "  📖  " SAVANNA BOLD "JOURNAL — Your Story Matters" RESET "\n");
    print_divider(EARTH, '═', 56);
    printf("\n");
    printf(IVORY "  In Uganda, griots carry the stories of the people.\n");
    printf("  You are the griot of your own life.\n\n" RESET);

    if (p->journal_count >= JOURNAL_ENTRIES) {
        printf(ORANGE "  Your journal is full. Here are your recent entries:\n\n" RESET);
        for (int i = 0; i < JOURNAL_ENTRIES; i++) {
            printf(GOLD "  Entry %d:\n" RESET, i + 1);
            printf(IVORY "  %s\n\n" RESET, p->journal[i]);
        }
        press_enter();
        return;
    }

    printf(GOLD "  Write how you feel right now.\n");
    printf("  Be honest. Be kind to yourself.\n\n" RESET);
    printf(SAVANNA "  > " RESET);
    fflush(stdout);

    if (fgets(p->journal[p->journal_count], MAX_JOURNAL, stdin)) {
        /* Remove trailing newline */
        int len = strlen(p->journal[p->journal_count]);
        if (len > 0 && p->journal[p->journal_count][len-1] == '\n')
            p->journal[p->journal_count][len-1] = '\0';
        p->journal_count++;
        p->seeds_planted++;
    }

    printf(FOREST "\n  Your words have been heard by the ancestors.\n");
    printf("  Writing is healing. 🌱\n" RESET);
    press_enter();
}

/* ──────────────────────── GARDEN PROGRESS ──────────────────────── */

void update_garden(Player *p) {
    int s = p->seeds_planted;
    if      (s >= 30) p->garden_stage = 4;
    else if (s >= 20) p->garden_stage = 3;
    else if (s >= 10) p->garden_stage = 2;
    else if (s >= 5)  p->garden_stage = 1;
    else              p->garden_stage = 0;
}

void show_garden(Player *p) {
    clear_screen();
    printf("\n");
    print_divider(FOREST, '═', 56);
    printf(FOREST "  🌳  " JUNGLE BOLD "YOUR MIND GARDEN — Uganda Nature Reserve" RESET "\n");
    print_divider(FOREST, '═', 56);
    printf("\n");

    const char *stages[] = {
        "  🌱 🌱 🌱\n  Seeds of healing have been planted.\n  Keep nurturing them every day.",
        "  🌿 🌿 🌿\n  Sprouts are appearing!\n  Your consistency is creating change.",
        "  🌲 🌲 🌲\n  Saplings growing strong like the forest of Bwindi!\n  You are becoming resilient.",
        "  🌳 🌳 🌳\n  A mighty tree stands!\n  Like the Mvule tree of Uganda — ancient and strong.",
        "  🌳🌿🦋🌸🦜🌳\n  YOUR FOREST IS THRIVING!\n  You have built something beautiful and real."
    };

    printf(FOREST "%s\n\n" RESET, stages[p->garden_stage]);
    printf(GOLD "  Seeds planted: %d / 30\n", p->seeds_planted);
    printf(SAVANNA "  Sessions completed: %d\n" RESET, p->sessions);

    printf("\n  " IVORY "Progress:\n");
    printf("  [");
    int pct = (p->seeds_planted * 20) / 30;
    for (int i = 0; i < 20; i++) {
        if (i < pct) printf(FOREST "█" RESET);
        else printf(DIM "░" RESET);
    }
    printf("] %d%%\n\n" RESET, (p->seeds_planted * 100) / 30);

    if (p->garden_stage == 4) {
        printf(GOLD BOLD "  ✨ You have reached FULL BLOOM — the highest stage!\n");
        printf("  You carry the spirit of Ugandan nature in your healing.\n" RESET);
    }
    press_enter();
}

/* ──────────────────────── CRISIS RESOURCES ──────────────────────── */

void crisis_support(void) {
    clear_screen();
    printf("\n");
    print_divider(CRIMSON, '═', 56);
    printf(CRIMSON "  💛  " ORANGE BOLD "YOU ARE NOT ALONE — Support Resources" RESET "\n");
    print_divider(CRIMSON, '═', 56);
    printf("\n");

    printf(IVORY "  If you are in crisis or need immediate support,\n");
    printf("  please reach out. You deserve care.\n\n" RESET);

    printf(GOLD   "  🇺🇬 UGANDA:\n" RESET);
    printf(IVORY  "  • Butabika National Referral Hospital:\n");
    printf(SAVANNA "    +256 (0)414 505 000\n\n" RESET);
    printf(IVORY  "  • TASO Uganda (mental health support):\n");
    printf(SAVANNA "    taso.go.ug\n\n" RESET);

    printf(GOLD   "  🌍 AFRICA / INTERNATIONAL:\n" RESET);
    printf(IVORY  "  • Befrienders Worldwide: befrienders.org\n");
    printf(IVORY  "  • Crisis Text Line (global): crisistextline.org\n");
    printf(IVORY  "  • WHO Mental Health: who.int/mental_health\n\n" RESET);

    printf(CRIMSON "  If you are in immediate danger, call your\n");
    printf("  local emergency services NOW.\n\n" RESET);

    print_divider(GOLD, '─', 56);
    printf(GOLD "\n  Remember: Seeking help is the bravest thing\n");
    printf("  a person can do. Ubuntu — we are here together.\n\n" RESET);
    press_enter();
}

/* ──────────────────────── PLAYER SETUP ──────────────────────── */

void setup_player(Player *p) {
    memset(p, 0, sizeof(Player));
    p->mood         = 5;
    p->anxiety      = 5;
    p->energy       = 5;
    p->sessions     = 0;
    p->seeds_planted= 0;
    p->garden_stage = 0;
    p->journal_count= 0;
    p->last_visit   = time(NULL);

    draw_title_screen();
    printf(GOLD "  Welcome, traveler. What is your name?\n  > " RESET);
    fflush(stdout);

    char buf[MAX_NAME + 32];
    if (fgets(buf, sizeof(buf), stdin)) {
        int len = strlen(buf);
        if (len > 0 && buf[len-1] == '\n') buf[len-1] = '\0';
        strncpy(p->name, buf, MAX_NAME - 1);
    }
    if (strlen(p->name) == 0) strcpy(p->name, "Friend");

    printf("\n");
    printf(FOREST "  Karibu, " GOLD BOLD "%s" RESET FOREST "! (Welcome in Swahili)\n\n", p->name);
    printf(IVORY  "  This garden is yours. Every time you breathe,\n");
    printf(        "  reflect, or journal — you plant a seed.\n");
    printf(        "  Watch your mind grow into a beautiful forest.\n\n" RESET);
    press_enter();
}

/* ──────────────────────── MAIN MENU ──────────────────────── */

int main_menu(Player *p) {
    draw_dashboard(p);
    printf(GOLD   "  ┌─────────────────────────────────────┐\n");
    printf(GOLD   "  │  " ORANGE BOLD "UBUNTU MIND GARDEN — MAIN MENU" RESET GOLD "     │\n");
    printf(GOLD   "  ├─────────────────────────────────────┤\n");
    printf(GOLD   "  │ " ORANGE "1." IVORY " Daily Check-In (Sunrise)       " GOLD "   │\n");
    printf(GOLD   "  │ " LAKE   "2." IVORY " Breathing Exercise (Lake Tide)  " GOLD "   │\n");
    printf(GOLD   "  │ " PURPLE "3." IVORY " Affirmation (Ancestors)         " GOLD "   │\n");
    printf(GOLD   "  │ " FOREST "4." IVORY " Grounding (Bwindi Forest)       " GOLD "   │\n");
    printf(GOLD   "  │ " SAVANNA"5." IVORY " Animal Spirit Guide (Wildlife)  " GOLD "   │\n");
    printf(GOLD   "  │ " EARTH  "6." IVORY " Journal Entry (Your Story)      " GOLD "   │\n");
    printf(GOLD   "  │ " JUNGLE "7." IVORY " View My Garden (Progress)       " GOLD "   │\n");
    printf(GOLD   "  │ " CRIMSON"8." IVORY " Crisis Support Resources        " GOLD "   │\n");
    printf(GOLD   "  │ " DIM    "9." RESET IVORY " Exit Garden                    " GOLD "   │\n");
    printf(GOLD   "  └─────────────────────────────────────┘\n\n" RESET);
    printf(SAVANNA "  Choose: " RESET);

    return get_int_input(1, 9);
}

/* ──────────────────────── MAIN ──────────────────────── */

int main(void) {
    Player player;

    setup_player(&player);

    int choice;
    int running = 1;

    while (running) {
        choice = main_menu(&player);
        update_garden(&player);

        switch (choice) {
            case 1: mood_checkin(&player);      break;
            case 2: breathing_exercise(&player);break;
            case 3: daily_affirmation(&player); break;
            case 4: grounding_exercise(&player);break;
            case 5: animal_spirit();            break;
            case 6: journal_entry(&player);     break;
            case 7: show_garden(&player);       break;
            case 8: crisis_support();           break;
            case 9: running = 0;               break;
        }
    }

    /* Farewell */
    clear_screen();
    printf("\n");
    print_divider(GOLD, '═', 56);
    printf(GOLD "\n  Farewell, " ORANGE BOLD "%s" RESET GOLD ".\n\n" RESET, player.name);
    printf(IVORY "  You planted " FOREST BOLD "%d seeds" RESET IVORY " in your mind garden today.\n\n", player.seeds_planted);
    printf(SAVANNA "  \"However long the night, the dawn will break.\"\n");
    printf(DIM "   — African Proverb\n\n" RESET);
    printf(FOREST "  Come back tomorrow. Your garden needs you. 🌿\n\n" RESET);
    print_divider(GOLD, '═', 56);
    printf("\n");

    return 0;
}

/* ────────────────────────────────────────────────────────────────── */
/*  END OF UBUNTU MIND GARDEN                                         */
/*  Lines of C: ~500+ | Mental wellness features: 8 | 🌍 Ubuntu      */
/* ────────────────────────────────────────────────────────────────── */
