/*
 * ═══════════════════════════════════════════════════════════════
 *   TILES  —  A Memory Matching Game in Many Scripts
 *   Supports: Latin, Greek, Cyrillic, Arabic, Hebrew, Devanagari,
 *             CJK, Katakana, Hiragana, Runic, Ethiopic, and more
 * ═══════════════════════════════════════════════════════════════
 *
 *  Compile:  gcc -o tiles tiles.c
 *  Run:      ./tiles
 *
 *  Requirements: A UTF-8 terminal (most modern terminals).
 *  On Windows:   Use Windows Terminal + "chcp 65001" first.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#ifdef _WIN32
  #include <windows.h>
  #define CLEAR "cls"
  static void setup_windows_utf8(void) {
      SetConsoleOutputCP(65001);
      SetConsoleCP(65001);
  }
#else
  #include <unistd.h>
  #define CLEAR "clear"
  static void setup_windows_utf8(void) {}
#endif

/* ── ANSI colour helpers ─────────────────────────────────────── */
#define RESET      "\033[0m"
#define BOLD       "\033[1m"
#define DIM        "\033[2m"

/* foreground */
#define FG_BLACK   "\033[30m"
#define FG_RED     "\033[31m"
#define FG_GREEN   "\033[32m"
#define FG_YELLOW  "\033[33m"
#define FG_BLUE    "\033[34m"
#define FG_MAGENTA "\033[35m"
#define FG_CYAN    "\033[36m"
#define FG_WHITE   "\033[37m"
#define FG_BRIGHT_WHITE "\033[97m"

/* background */
#define BG_BLACK   "\033[40m"
#define BG_RED     "\033[41m"
#define BG_GREEN   "\033[42m"
#define BG_YELLOW  "\033[43m"
#define BG_BLUE    "\033[44m"
#define BG_MAGENTA "\033[45m"
#define BG_CYAN    "\033[46m"
#define BG_WHITE   "\033[47m"
#define BG_BRIGHT_BLACK "\033[100m"
#define BG_BRIGHT_WHITE "\033[107m"

/* ── Game constants ──────────────────────────────────────────── */
#define MAX_TILES  36   /* max board size (6×6) */

typedef struct {
    const char *symbol;   /* UTF-8 glyph(s) shown when revealed */
    int         matched;  /* 1 if already matched */
    int         visible;  /* 1 if currently face-up (preview) */
} Tile;

/* ── Script sets ─────────────────────────────────────────────── */
/* Each set has at least 18 unique symbols so we can make 18 pairs */

static const char *script_latin[] = {
    "A","B","C","D","E","F","G","H","I","J","K","L","M",
    "N","O","P","Q","R","S","T","U","V","W","X","Y","Z",
    "Æ","Ø","Å","Ñ","Ü","Ç","Ö","ß","Ð","Þ"
};
static const char *script_greek[] = {
    "Α","Β","Γ","Δ","Ε","Ζ","Η","Θ","Ι","Κ","Λ","Μ","Ν",
    "Ξ","Ο","Π","Ρ","Σ","Τ","Υ","Φ","Χ","Ψ","Ω",
    "α","β","γ","δ","ε","ζ","η","θ","ι","κ","λ","μ"
};
static const char *script_cyrillic[] = {
    "А","Б","В","Г","Д","Е","Ж","З","И","К","Л","М","Н",
    "О","П","Р","С","Т","У","Ф","Х","Ц","Ч","Ш","Щ",
    "Ъ","Ы","Ь","Э","Ю","Я","Ё","Ї","Є","Ґ","І"
};
static const char *script_hebrew[] = {
    "א","ב","ג","ד","ה","ו","ז","ח","ט","י","כ","ל","מ",
    "נ","ס","ע","פ","צ","ק","ר","ש","ת","ך","ם","ן","ף",
    "ץ","בּ","כּ","פּ","שׁ","שׂ","וּ","יִ","אָ","אֵ"
};
static const char *script_arabic[] = {
    "ا","ب","ت","ث","ج","ح","خ","د","ذ","ر","ز","س","ش",
    "ص","ض","ط","ظ","ع","غ","ف","ق","ك","ل","م","ن",
    "ه","و","ي","ة","ى","ء","ؤ","ئ","إ","أ","آ"
};
static const char *script_devanagari[] = {
    "अ","आ","इ","ई","उ","ऊ","ए","ऐ","ओ","औ","क","ख","ग",
    "घ","च","छ","ज","झ","ट","ठ","ड","ढ","त","थ","द",
    "ध","न","प","फ","ब","भ","म","य","र","ल","व"
};
static const char *script_katakana[] = {
    "ア","イ","ウ","エ","オ","カ","キ","ク","ケ","コ","サ","シ","ス",
    "セ","ソ","タ","チ","ツ","テ","ト","ナ","ニ","ヌ","ネ","ノ",
    "ハ","ヒ","フ","ヘ","ホ","マ","ミ","ム","メ","モ","ヤ"
};
static const char *script_hiragana[] = {
    "あ","い","う","え","お","か","き","く","け","こ","さ","し","す",
    "せ","そ","た","ち","つ","て","と","な","に","ぬ","ね","の",
    "は","ひ","ふ","へ","ほ","ま","み","む","め","も","や","ゆ"
};
static const char *script_cjk[] = {
    "山","水","火","木","金","土","日","月","星","風","雨","雪","花",
    "鳥","魚","龍","虎","馬","牛","羊","猫","犬","竹","梅","桜",
    "道","心","愛","夢","空","海","川","森","石","光","影"
};
static const char *script_runic[] = {
    "ᚠ","ᚢ","ᚦ","ᚨ","ᚱ","ᚲ","ᚷ","ᚹ","ᚺ","ᚾ","ᛁ","ᛃ","ᛇ",
    "ᛈ","ᛉ","ᛊ","ᛏ","ᛒ","ᛖ","ᛗ","ᛚ","ᛜ","ᛞ","ᛟ","ᚩ",
    "ᚫ","ᚣ","ᛡ","ᛠ","ᚳ","ᚴ","ᛣ","ᛤ","ᛥ","ᚸ","ᚻ"
};
static const char *script_ethiopic[] = {
    "አ","ሀ","ለ","ሐ","መ","ሠ","ረ","ሰ","ሸ","ቀ","ቐ","በ","ቨ",
    "ተ","ቸ","ኀ","ነ","ኘ","አ","ከ","ኸ","ወ","ዐ","ዘ","ዠ",
    "የ","ደ","ዸ","ጀ","ገ","ጘ","ጠ","ጨ","ጰ","ጸ","ፀ"
};
static const char *script_symbols[] = {
    "★","☀","☁","☂","☃","♠","♣","♥","♦","♪","♫","✈","⚓",
    "⚡","⚽","✿","❀","❤","☯","☮","☭","☪","✡","⚛","✝",
    "⚜","♞","♟","⚔","🜂","🜃","🜄","🜁","☿","♄","♃"
};

typedef struct {
    const char  *name;
    const char **glyphs;
    int          count;
    const char  *fg_color;
    const char  *bg_color;
} ScriptInfo;

#define NSCRIPTS 12
static ScriptInfo scripts[NSCRIPTS] = {
    { "Latin",      script_latin,      36, FG_BRIGHT_WHITE, BG_BLUE      },
    { "Greek",      script_greek,      36, FG_BRIGHT_WHITE, BG_MAGENTA   },
    { "Cyrillic",   script_cyrillic,   36, FG_BRIGHT_WHITE, BG_RED       },
    { "Hebrew",     script_hebrew,     36, FG_BRIGHT_WHITE, BG_BRIGHT_BLACK },
    { "Arabic",     script_arabic,     36, FG_BRIGHT_WHITE, BG_GREEN     },
    { "Devanagari", script_devanagari, 36, FG_BLACK,        BG_YELLOW    },
    { "Katakana",   script_katakana,   36, FG_BLACK,        BG_CYAN      },
    { "Hiragana",   script_hiragana,   36, FG_BRIGHT_WHITE, BG_BLUE      },
    { "CJK",        script_cjk,        36, FG_BLACK,        BG_WHITE     },
    { "Runic",      script_runic,      36, FG_BRIGHT_WHITE, BG_RED       },
    { "Ethiopic",   script_ethiopic,   36, FG_BLACK,        BG_YELLOW    },
    { "Symbols",    script_symbols,    36, FG_BRIGHT_WHITE, BG_MAGENTA   },
};

/* ── Difficulty ──────────────────────────────────────────────── */
typedef struct { int rows; int cols; const char *name; } Difficulty;
static Difficulty difficulties[] = {
    { 2, 4,  "Easy   (2×4,  8 tiles)" },
    { 3, 4,  "Medium (3×4, 12 tiles)" },
    { 4, 4,  "Hard   (4×4, 16 tiles)" },
    { 4, 5,  "Expert (4×5, 20 tiles)" },
    { 5, 6,  "Master (5×6, 30 tiles)" },
    { 6, 6,  "Legend (6×6, 36 tiles)" },
};
#define NDIFF 6

/* ── Game state ──────────────────────────────────────────────── */
static Tile   board[MAX_TILES];
static int    rows, cols, total_tiles;
static int    script_idx;
static int    moves, matches_found, total_pairs;
static time_t start_time;
static ScriptInfo *S; /* pointer to current script */

/* ── Utilities ───────────────────────────────────────────────── */
static void wait_enter(void) {
    printf("  Press ENTER to continue...");
    fflush(stdout);
    /* consume any leftover newline then wait */
    int c;
    while ((c = getchar()) != '\n' && c != EOF);
    while ((c = getchar()) != '\n' && c != EOF);
}

static int read_int(int lo, int hi) {
    int n; char buf[64];
    while (1) {
        if (!fgets(buf, sizeof(buf), stdin)) continue;
        if (sscanf(buf, "%d", &n) == 1 && n >= lo && n <= hi) return n;
        printf("  Please enter a number %d–%d: ", lo, hi); fflush(stdout);
    }
}

static void shuffle(int *arr, int n) {
    for (int i = n - 1; i > 0; i--) {
        int j = rand() % (i + 1);
        int tmp = arr[i]; arr[i] = arr[j]; arr[j] = tmp;
    }
}

/* ── Banner & title screen ───────────────────────────────────── */
static void print_banner(void) {
    printf(BOLD FG_CYAN
        "\n"
        "  ╔══════════════════════════════════════════════════╗\n"
        "  ║                                                  ║\n"
        "  ║   T I L E S  ·  A  M E M O R Y  G A M E         ║\n"
        "  ║        in  many  scripts  of  the  world         ║\n"
        "  ║                                                  ║\n"
        "  ╚══════════════════════════════════════════════════╝\n"
        RESET "\n");
}

/* ── Board rendering ─────────────────────────────────────────── */
static void render_board(int sel1, int sel2) {
    system(CLEAR);
    print_banner();

    /* status line */
    time_t elapsed = time(NULL) - start_time;
    int mins = (int)(elapsed / 60), secs = (int)(elapsed % 60);
    printf("  " BOLD FG_YELLOW "Script: %-11s" RESET
           "  " BOLD FG_CYAN "Moves: %-5d" RESET
           "  " BOLD FG_GREEN "Pairs: %d/%d" RESET
           "  " BOLD FG_MAGENTA "Time: %02d:%02d\n\n" RESET,
           S->name, moves, matches_found, total_pairs, mins, secs);

    /* column indices */
    printf("     ");
    for (int c = 0; c < cols; c++) printf("  %2d ", c + 1);
    printf("\n");
    printf("     ");
    for (int c = 0; c < cols; c++) printf("┌────");
    printf("┐\n");

    for (int r = 0; r < rows; r++) {
        printf("  %2d ", r + 1);
        for (int c = 0; c < cols; c++) {
            int idx = r * cols + c;
            Tile *t = &board[idx];
            int is_sel = (idx == sel1 || idx == sel2);

            printf("│");

            if (t->matched) {
                /* matched tiles: dim green */
                printf(DIM FG_GREEN " %-2s " RESET, t->symbol);
            } else if (t->visible || is_sel) {
                /* face-up: coloured with the script's colours */
                printf("%s%s " BOLD " %s " RESET, S->bg_color, S->fg_color, t->symbol);
            } else {
                /* face-down: grey block with tile number */
                printf(BG_BRIGHT_BLACK FG_WHITE " %2d " RESET, idx + 1);
            }
        }
        printf("│\n");

        if (r < rows - 1) {
            printf("     ");
            for (int c = 0; c < cols; c++) printf("├────");
            printf("┤\n");
        }
    }
    printf("     ");
    for (int c = 0; c < cols; c++) printf("└────");
    printf("┘\n\n");
}

/* ── Sleep helper (cross-platform, ~1s) ─────────────────────── */
static void sleep_sec(int s) {
#ifdef _WIN32
    Sleep(s * 1000);
#else
    sleep(s);
#endif
}

/* ── Core game loop ──────────────────────────────────────────── */
static void play_game(void) {
    int first = -1, second = -1;

    while (matches_found < total_pairs) {
        render_board(first, second);

        /* pick first tile */
        printf("  Enter tile number (1-%d), or 0 to quit: ", total_tiles);
        fflush(stdout);
        int choice;
        char buf[64];
        if (!fgets(buf, sizeof(buf), stdin)) continue;
        if (sscanf(buf, "%d", &choice) != 1) continue;
        if (choice == 0) {
            printf(BOLD FG_RED "\n  Game abandoned.\n" RESET);
            return;
        }
        choice--; /* 0-indexed */
        if (choice < 0 || choice >= total_tiles) continue;
        if (board[choice].matched || board[choice].visible) continue;

        first = choice;
        board[first].visible = 1;
        render_board(first, -1);

        /* pick second tile */
        printf("  Enter second tile (1-%d): ", total_tiles);
        fflush(stdout);
        if (!fgets(buf, sizeof(buf), stdin)) continue;
        if (sscanf(buf, "%d", &choice) != 1) continue;
        choice--;
        if (choice < 0 || choice >= total_tiles) continue;
        if (choice == first || board[choice].matched || board[choice].visible) continue;

        second = choice;
        board[second].visible = 1;
        moves++;

        render_board(first, second);

        /* check match */
        if (strcmp(board[first].symbol, board[second].symbol) == 0) {
            board[first].matched  = board[second].matched  = 1;
            board[first].visible  = board[second].visible  = 0;
            matches_found++;
            printf(BOLD FG_GREEN "  ✓  Match!  %d pair%s found.\n" RESET,
                   matches_found, matches_found == 1 ? "" : "s");
            sleep_sec(1);
        } else {
            printf(FG_RED "  ✗  No match.\n" RESET);
            sleep_sec(1);
            board[first].visible  = board[second].visible  = 0;
        }
        first = second = -1;
    }

    /* Victory! */
    render_board(-1, -1);
    time_t elapsed = time(NULL) - start_time;
    int mins = (int)(elapsed / 60), secs = (int)(elapsed % 60);
    printf(BOLD FG_YELLOW
        "  ╔══════════════════════════════════════╗\n"
        "  ║   🎉  Congratulations! You won!  🎉   ║\n"
        "  ╚══════════════════════════════════════╝\n" RESET);
    printf(BOLD "  Moves: %d   Time: %02d:%02d\n\n" RESET, moves, mins, secs);

    /* score (lower is better) */
    int score = moves * 10 + (int)elapsed;
    printf(BOLD FG_CYAN "  Score: %d  (lower is better)\n\n" RESET, score);
}

/* ── Setup: choose script & difficulty ───────────────────────── */
static void setup_game(void) {
    system(CLEAR);
    print_banner();

    printf(BOLD "  Choose a script:\n\n" RESET);
    for (int i = 0; i < NSCRIPTS; i++) {
        /* show a sample of the glyphs */
        printf("  %s%2d%s  %-12s  ",
               BOLD FG_CYAN, i + 1, RESET, scripts[i].name);
        printf("%s%s", scripts[i].bg_color, scripts[i].fg_color);
        for (int g = 0; g < 8 && g < scripts[i].count; g++)
            printf(" %s", scripts[i].glyphs[g]);
        printf("%s", RESET);
        printf("  …\n");
    }
    printf("\n  Enter script number (1-%d): ", NSCRIPTS);
    fflush(stdout);
    script_idx = read_int(1, NSCRIPTS) - 1;
    S = &scripts[script_idx];

    printf("\n" BOLD "  Choose difficulty:\n\n" RESET);
    for (int i = 0; i < NDIFF; i++)
        printf("  " BOLD FG_CYAN "%d" RESET "  %s\n", i + 1, difficulties[i].name);
    printf("\n  Enter difficulty (1-%d): ", NDIFF);
    fflush(stdout);
    int diff = read_int(1, NDIFF) - 1;
    rows = difficulties[diff].rows;
    cols = difficulties[diff].cols;
    total_tiles = rows * cols;
    total_pairs = total_tiles / 2;

    /* build board: pick total_pairs unique symbols, duplicate & shuffle */
    int avail[36];
    for (int i = 0; i < S->count; i++) avail[i] = i;
    shuffle(avail, S->count);

    const char *chosen[MAX_TILES / 2];
    for (int i = 0; i < total_pairs; i++) chosen[i] = S->glyphs[avail[i]];

    /* fill board array: each symbol appears twice */
    int order[MAX_TILES];
    for (int i = 0; i < total_tiles; i++) order[i] = i;
    shuffle(order, total_tiles);

    for (int i = 0; i < total_tiles; i++) {
        /* assign symbol: tile i → symbol at order[i]/2 */
        board[i].symbol  = chosen[order[i] / 2];
        board[i].matched = 0;
        board[i].visible = 0;
    }
    /* Fix: shuffle pairs properly */
    /* Re-do: place 2 copies of each symbol into random positions */
    int positions[MAX_TILES];
    for (int i = 0; i < total_tiles; i++) positions[i] = i;
    shuffle(positions, total_tiles);
    for (int p = 0; p < total_pairs; p++) {
        board[positions[p * 2    ]].symbol = chosen[p];
        board[positions[p * 2 + 1]].symbol = chosen[p];
        board[positions[p * 2    ]].matched = 0;
        board[positions[p * 2    ]].visible = 0;
        board[positions[p * 2 + 1]].matched = 0;
        board[positions[p * 2 + 1]].visible = 0;
    }

    moves = 0;
    matches_found = 0;
    start_time = time(NULL);
}

/* ── Brief preview (show all tiles for 3s) ───────────────────── */
static void preview(void) {
    printf("\n" BOLD FG_YELLOW "  Memorise the tiles! (3 seconds…)\n" RESET);
    for (int i = 0; i < total_tiles; i++) board[i].visible = 1;
    render_board(-1, -1);
    sleep_sec(3);
    for (int i = 0; i < total_tiles; i++) board[i].visible = 0;
}

/* ── How to play ─────────────────────────────────────────────── */
static void show_help(void) {
    system(CLEAR);
    print_banner();
    printf(BOLD "  HOW TO PLAY\n\n" RESET
           "  All tiles start face-down.  On each turn:\n\n"
           "  1. Enter the number of any face-down tile.\n"
           "  2. Enter the number of a second face-down tile.\n\n"
           "  If both tiles show the same symbol → they are MATCHED\n"
           "  and stay face-up (dimmed green).\n\n"
           "  Otherwise they flip back face-down.\n\n"
           "  Goal: match every pair in as few moves as possible.\n\n"
           "  • A 3-second preview is shown at the start.\n"
           "  • You can enter 0 at any time to quit.\n\n");
    wait_enter();
}

/* ── Main ────────────────────────────────────────────────────── */
int main(void) {
    setup_windows_utf8();
    srand((unsigned)time(NULL));

    while (1) {
        system(CLEAR);
        print_banner();
        printf("  " BOLD FG_CYAN "1" RESET "  New game\n"
               "  " BOLD FG_CYAN "2" RESET "  How to play\n"
               "  " BOLD FG_CYAN "3" RESET "  Quit\n\n"
               "  Choose: ");
        fflush(stdout);
        int c = read_int(1, 3);
        if (c == 3) break;
        if (c == 2) { show_help(); continue; }

        setup_game();
        preview();
        play_game();

        printf(BOLD "  Play again? (1=yes / 2=no): " RESET);
        fflush(stdout);
        if (read_int(1, 2) == 2) break;
    }

    system(CLEAR);
    print_banner();
    printf(BOLD FG_CYAN "  Thanks for playing TILES!\n\n" RESET);
    return 0;
}
