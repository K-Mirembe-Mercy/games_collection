/*
 * ╔══════════════════════════════════════════════════════════════════════╗
 * ║  umg_extensions.c — Ubuntu Mind Garden Extension Module             ║
 * ║  "A tree is straightened while it is young." — Ugandan Proverb     ║
 * ║                                                                      ║
 * ║  HOW TO USE:                                                         ║
 * ║  1. Place this file in the same folder as ubuntu_mindgarden_v3.c    ║
 * ║  2. Place umg_shared.h in the same folder                           ║
 * ║  3. Add  #include "umg_shared.h"  at the top of v3.c               ║
 * ║  4. Add  ext_menu(&ext, &player);  as option 19 in the v3 menu     ║
 * ║  5. Compile everything:                                              ║
 * ║       gcc ubuntu_mindgarden_v3.c umg_extensions.c -o umg -lm       ║
 * ║  6. Run:  ./umg                                                     ║
 * ║                                                                      ║
 * ║  NEW FEATURES THIS MODULE ADDS:                                      ║
 * ║  • Mood Analytics  — 30-day charts, trend analysis                  ║
 * ║  • CBT Workbook    — Thought records with reframing                 ║
 * ║  • Habit Tracker   — Weekly habit streaks & goals                   ║
 * ║  • Safety Plan     — Personal crisis plan builder                   ║
 * ║  • Story Mode      — 12-chapter Ugandan healing narrative           ║
 * ║  • Challenges      — Daily/weekly community challenges               ║
 * ║  • Friends         — Local accountability partner system            ║
 * ║  • Panic Button    — Instant grounding for acute anxiety            ║
 * ║  • Sleep Hygiene   — Wind-down routine guide                        ║
 * ║  • Weekly Report   — Full wellness stats report                     ║
 * ║  • Personal Mantra — Custom affirmation creator                     ║
 * ╚══════════════════════════════════════════════════════════════════════╝
 */

#include "umg_shared.h"

/* ══════════════════════════ INTERNAL UTILITIES ════════════════════════ */
static void _cls(void)        { printf("\033[2J\033[H"); fflush(stdout); }
static void _ms(int t)        { usleep(t * 1000); }
static void _nl(void)         { printf("\n"); }

static void _div(const char *c, char ch, int w) {
    printf("%s", c);
    for (int i=0;i<w;i++) putchar(ch);
    printf(R "\n");
}

static void _tw(const char *col, const char *txt, int d) {
    printf("%s", col);
    while (*txt) { putchar(*txt++); fflush(stdout); _ms(d); }
    printf(R);
}

static void _enter(void) {
    printf(DIM "\n  [ Press ENTER ]" R); fflush(stdout);
    while (getchar() != '\n');
}

static int _int(int lo, int hi) {
    int v; char buf[32];
    while (1) {
        if (fgets(buf,sizeof(buf),stdin) && sscanf(buf,"%d",&v)==1
                && v>=lo && v<=hi) return v;
        printf(ORANGE "  Enter %d–%d: " R,lo,hi);
    }
}

static char _char(void) {
    char buf[8];
    if (fgets(buf,sizeof(buf),stdin)) return toupper((unsigned char)buf[0]);
    return '?';
}

static long _day(void) { return (long)(time(NULL)/86400); }

static const char *_fmt_day(long epoch_d) {
    static char buf[16];
    time_t t = (time_t)(epoch_d * 86400);
    struct tm *tm = localtime(&t);
    snprintf(buf,sizeof(buf),"%02d/%02d",tm->tm_mday,tm->tm_mon+1);
    return buf;
}

/* ══════════════════════════ SAVE / LOAD ═══════════════════════════════ */
void ext_load(ExtState *e) {
    memset(e,0,sizeof(ExtState));
    FILE *f = fopen(SAVE_FILE_EXT,"rb");
    if (f) { fread(e,sizeof(ExtState),1,f); fclose(f); }

    /* Init story chapters if fresh */
    if (!e->chapters[0].xp_reward) {
        const char *titles[MAX_STORY_CHAPTERS] = {
            "The Seed Awakens","First Rain on Red Earth","A Walk to the Nile",
            "The Gorilla's Lesson","Drumbeats at Dusk","Mist over Bwindi",
            "The Crane Takes Flight","Campfire Circle","Voice of the Ancestors",
            "Lake Victoria's Mirror","Rwenzori Summit","The Ancestor Light"
        };
        const char *summaries[MAX_STORY_CHAPTERS] = {
            "You are a seed buried in Ugandan soil. What do you need to grow?",
            "The first rains come. Something inside you begins to soften.",
            "You walk along the Nile. The river carries away what you no longer need.",
            "A mountain gorilla sits with you in silence. What does it teach you?",
            "Kampala drums echo at dusk. Your heartbeat syncs with the rhythm of belonging.",
            "Morning mist blankets Bwindi. In stillness, you finally hear yourself.",
            "The crested crane lifts off. You feel what it is to rise above your story.",
            "A fire is lit. Elders and kin gather. You share your truth for the first time.",
            "An ancestor speaks to you in a dream. They say: 'We are proud of you.'",
            "Lake Victoria reflects your face — but calmer, brighter, more at peace.",
            "You climb the Rwenzori mountains. Every step is a year of healing.",
            "You become the light you were always looking for. Ubuntu complete."
        };
        for (int i=0;i<MAX_STORY_CHAPTERS;i++) {
            strncpy(e->chapters[i].title,   titles[i],   63);
            strncpy(e->chapters[i].summary, summaries[i],255);
            e->chapters[i].xp_reward  = 80 + i * 30;
            e->chapters[i].unlocked   = (i == 0) ? 1 : 0;
            e->chapters[i].completed  = 0;
        }
    }
    /* Init daily challenges if fresh */
    if (e->challenge_count == 0) {
        const char *ct[] = {
            "Sunrise Check-In","Bwindi Breath","Griot's Entry",
            "Gorilla Focus","Ubuntu Stretch","Kampala Rhythm",
            "Nile Gratitude","Ancestor Reflection"
        };
        const char *cd[] = {
            "Complete a daily check-in today",
            "Do 2 breathing sessions today",
            "Write one journal entry",
            "Win a Gorilla Wisdom game",
            "Complete a grounding session",
            "Score 5/5 on Drum Rhythm",
            "Name 5 things you are grateful for (in journal)",
            "Complete one meditation session"
        };
        long today = _day();
        e->challenge_count = 8;
        for (int i=0;i<8;i++) {
            strncpy(e->challenges[i].title, ct[i], 63);
            strncpy(e->challenges[i].desc,  cd[i],127);
            e->challenges[i].target      = 1;
            e->challenges[i].xp_reward   = 60 + i*15;
            e->challenges[i].expires_day = today + 1;
        }
    }
}

void ext_save(ExtState *e) {
    FILE *f = fopen(SAVE_FILE_EXT,"wb");
    if (f) { fwrite(e,sizeof(ExtState),1,f); fclose(f); }
}

/* ══════════════════════════ MOOD HISTORY ══════════════════════════════ */
void ext_record_mood(ExtState *e, Player *p) {
    MoodHistory *h = &e->mood_hist;
    int idx = h->head % MAX_MOOD_HISTORY;
    h->mood[idx]       = p->mood;
    h->anxiety[idx]    = p->anxiety;
    h->energy[idx]     = p->energy;
    h->timestamps[idx] = _day();
    h->head            = (h->head + 1) % MAX_MOOD_HISTORY;
    if (h->count < MAX_MOOD_HISTORY) h->count++;
}

void ext_show_mood_chart(ExtState *e) {
    _cls();
    _div(CYAN,'═',60);
    printf(CYAN "  📊  " GOLD BOLD "MOOD ANALYTICS — 30-Day Tracker" R "\n");
    _div(CYAN,'═',60);

    MoodHistory *h = &e->mood_hist;
    if (h->count == 0) {
        printf(IVORY "\n  No mood data yet. Complete a check-in first!\n" R);
        _enter(); return;
    }

    int n = h->count > 20 ? 20 : h->count;
    printf(IVORY "\n  Last %d entries:\n\n" R, n);

    /* Bar chart */
    printf(GOLD "  %-8s  %-20s  %-20s  %-10s\n" R,"Date","Mood","Calm","Energy");
    _div(DIM,'-',62);

    int total_mood=0, total_calm=0, total_energy=0;
    for (int i=0;i<n;i++) {
        int idx = ((h->head - n + i) + MAX_MOOD_HISTORY) % MAX_MOOD_HISTORY;
        int m = h->mood[idx];
        int c = 10 - h->anxiety[idx];
        int en= h->energy[idx];
        total_mood   += m;
        total_calm   += c;
        total_energy += en;

        printf("  " DIM "%s" R "  ", _fmt_day(h->timestamps[idx]));

        /* Mood bar */
        printf(ORANGE "[" R);
        for (int j=0;j<10;j++) printf(j<m ? ORANGE "▓" R : DIM "░" R);
        printf(ORANGE "]%2d  " R, m);

        /* Calm bar */
        printf(LAKE "[" R);
        for (int j=0;j<10;j++) printf(j<c ? LAKE "▓" R : DIM "░" R);
        printf(LAKE "]%2d  " R, c);

        /* Energy bar */
        printf(LIME "[" R);
        for (int j=0;j<10;j++) printf(j<en ? LIME "▓" R : DIM "░" R);
        printf(LIME "]%2d\n" R, en);
    }

    _div(DIM,'-',62);
    printf(GOLD "  Avg Mood: %.1f   Avg Calm: %.1f   Avg Energy: %.1f\n" R,
           (float)total_mood/n, (float)total_calm/n, (float)total_energy/n);

    /* Trend */
    if (n >= 4) {
        int first_half = 0, second_half = 0;
        for (int i=0;i<n/2;i++) {
            int idx = ((h->head-n+i)+MAX_MOOD_HISTORY)%MAX_MOOD_HISTORY;
            first_half += h->mood[idx];
        }
        for (int i=n/2;i<n;i++) {
            int idx = ((h->head-n+i)+MAX_MOOD_HISTORY)%MAX_MOOD_HISTORY;
            second_half += h->mood[idx];
        }
        float t1 = (float)first_half/(n/2), t2 = (float)second_half/(n-n/2);
        printf("\n  Trend: ");
        if      (t2 > t1 + 0.5) printf(LIME "  📈 Mood is RISING — great progress!\n" R);
        else if (t2 < t1 - 0.5) printf(CRIMSON "  📉 Mood dipping — consider extra care.\n" R);
        else                     printf(SAVANNA "  ➡  Mood is stable — consistency is strength.\n" R);
    }
    _enter();
}

/* ══════════════════════════ CBT WORKBOOK ══════════════════════════════ */
/*
 * Cognitive Behavioural Therapy thought records.
 * Helps users identify cognitive distortions and reframe thoughts.
 */
void ext_cbt_session(ExtState *e, Player *p) {
    _cls();
    _div(PURPLE,'═',60);
    printf(PURPLE "  🧠  " PINK BOLD "CBT THOUGHT RECORD — Reframe Your Mind" R "\n");
    _div(PURPLE,'═',60);
    printf(IVORY "\n  Cognitive Behavioural Therapy (CBT) helps you identify\n");
    printf("  unhelpful thoughts and replace them with balanced ones.\n");
    printf("  Like the Nile — we redirect, not dam.\n\n" R);

    if (e->cbt_count >= MAX_CBT_ENTRIES) {
        printf(ORANGE "  Thought journal full! Here are your recent records:\n\n" R);
        int start = e->cbt_count >= 3 ? e->cbt_count-3 : 0;
        for (int i=start;i<e->cbt_count;i++) {
            printf(GOLD "  [%d] Situation: " IVORY "%s\n" R, i+1, e->cbt[i].situation);
            printf(GOLD "       Balanced: " LIME  "%s\n\n" R, e->cbt[i].balanced_thought);
        }
        _enter(); return;
    }

    CBTRecord *rec = &e->cbt[e->cbt_count];
    rec->timestamp = _day();

    /* Common distortions guide */
    printf(GOLD "  Common thought traps to watch for:\n" R);
    printf(DIM  "  • All-or-Nothing  • Catastrophising  • Mind Reading\n");
    printf("  • Emotional Reasoning  • Personalisation  • Should Statements\n\n" R);

    printf(CYAN "  Step 1: What is the SITUATION? (What happened?)\n  > " R);
    if (fgets(rec->situation, 255, stdin)) {
        int l=strlen(rec->situation); if(l>0&&rec->situation[l-1]=='\n') rec->situation[l-1]='\0';
    }

    printf(CYAN "\n  Step 2: What THOUGHT went through your mind?\n  > " R);
    if (fgets(rec->thought, 255, stdin)) {
        int l=strlen(rec->thought); if(l>0&&rec->thought[l-1]=='\n') rec->thought[l-1]='\0';
    }

    printf(CYAN "\n  Step 3: What EMOTION did you feel? (e.g. anxious, sad, angry)\n  > " R);
    if (fgets(rec->emotion, 63, stdin)) {
        int l=strlen(rec->emotion); if(l>0&&rec->emotion[l-1]=='\n') rec->emotion[l-1]='\0';
    }

    printf(ORANGE "\n  Step 4: What EVIDENCE supports this thought?\n  > " R);
    if (fgets(rec->evidence_for, 255, stdin)) {
        int l=strlen(rec->evidence_for); if(l>0&&rec->evidence_for[l-1]=='\n') rec->evidence_for[l-1]='\0';
    }

    printf(FOREST "\n  Step 5: What EVIDENCE goes AGAINST this thought?\n  > " R);
    if (fgets(rec->evidence_against, 255, stdin)) {
        int l=strlen(rec->evidence_against); if(l>0&&rec->evidence_against[l-1]=='\n') rec->evidence_against[l-1]='\0';
    }

    printf(GOLD "\n  Step 6: Write a BALANCED, kinder thought:\n");
    printf(ITALIC "  (e.g. 'Even if this is hard, I have handled hard things before.')\n" R);
    printf(GOLD "  > " R);
    if (fgets(rec->balanced_thought, 255, stdin)) {
        int l=strlen(rec->balanced_thought); if(l>0&&rec->balanced_thought[l-1]=='\n') rec->balanced_thought[l-1]='\0';
    }

    e->cbt_count++;
    _div(GOLD,'─',60);
    printf(LIME "\n  ✅ Thought record saved.\n\n" R);
    printf(IVORY "  Your balanced thought:\n");
    printf(GOLD BOLD "  « %s »\n\n" R, rec->balanced_thought);
    printf(SAVANNA "  Read this again before you sleep tonight.\n" R);

    award_xp(p, 70, "CBT Thought Record");
    _enter();
}

/* ══════════════════════════ HABIT TRACKER ═════════════════════════════ */
void _show_habits(ExtState *e) {
    printf(GOLD "  %-22s  %-7s  Mon Tue Wed Thu Fri Sat Sun\n" R,"Habit","Goal/wk");
    _div(DIM,'-',64);
    const char *days[] = {"M","T","W","T","F","S","S"};
    for (int i=0;i<e->habit_count;i++) {
        Habit *h = &e->habits[i];
        if (!h->active) continue;
        int done_this_week = 0;
        for (int d=0;d<7;d++) done_this_week += h->completions[d];
        printf("  %-22s  %d/%-5d ", h->name, done_this_week, h->target_per_week);
        for (int d=0;d<7;d++) {
            if (h->completions[d]) printf(LIME " ✓%s  " R, days[d]);
            else printf(DIM "  %s  " R, days[d]);
        }
        printf("  🔥%d\n", h->streak);
    }
}

void ext_habit_tracker(ExtState *e, Player *p) {
    _cls();
    _div(TEAL,'═',60);
    printf(TEAL "  📋  " CYAN BOLD "HABIT TRACKER — Weekly Wellness Habits" R "\n");
    _div(TEAL,'═',60);

    /* Weekly reset check */
    long today = _day();
    int dow = (int)((today + 3) % 7); /* 0=Mon */
    if (e->last_weekly_reset == 0 || today - e->last_weekly_reset >= 7) {
        for (int i=0;i<e->habit_count;i++) {
            Habit *h = &e->habits[i];
            int done = 0;
            for (int d=0;d<7;d++) done += h->completions[d];
            if (done >= h->target_per_week) h->streak++;
            else if (h->total_done > 0)     h->streak = 0;
            memset(h->completions, 0, sizeof(h->completions));
        }
        e->last_weekly_reset = today;
        printf(LIME "  Weekly habits reset!\n\n" R);
    }

    if (e->habit_count == 0) {
        printf(IVORY "\n  No habits set yet. Let's create your first one.\n" R);
    } else {
        _nl();
        _show_habits(e);
        _nl();
    }

    printf(GOLD "  ┌─────────────────────────────────────────────┐\n");
    printf(GOLD "  │ 1. Mark a habit as done today               │\n");
    printf(GOLD "  │ 2. Add a new habit                          │\n");
    printf(GOLD "  │ 3. Remove a habit                           │\n");
    printf(GOLD "  │ 4. Back                                     │\n");
    printf(GOLD "  └─────────────────────────────────────────────┘\n");
    printf(SAVANNA "  > " R);

    int c = _int(1,4);

    if (c == 1 && e->habit_count > 0) {
        printf(IVORY "\n  Which habit? (1-%d): " R, e->habit_count);
        int hi = _int(1, e->habit_count) - 1;
        if (e->habits[hi].completions[dow]) {
            printf(LIME "  Already done today! Keep it up 🌿\n" R);
        } else {
            e->habits[hi].completions[dow] = 1;
            e->habits[hi].total_done++;
            printf(LIME "  ✓ Marked done! Keep the streak alive!\n" R);
            award_xp(p, 20, "Habit Completed");
        }
    } else if (c == 2) {
        if (e->habit_count >= MAX_HABIT_SLOTS) {
            printf(ORANGE "  Habit slots full! Remove one first.\n" R);
        } else {
            Habit *h = &e->habits[e->habit_count];
            memset(h, 0, sizeof(Habit));
            printf(CYAN "\n  Habit name (e.g. 'Morning walk', 'Read 10 mins'):\n  > " R);
            if (fgets(h->name, 63, stdin)) {
                int l=strlen(h->name); if(l>0&&h->name[l-1]=='\n') h->name[l-1]='\0';
            }
            printf(CYAN "  Times per week target (1-7): " R);
            h->target_per_week = _int(1,7);
            h->active = 1;
            e->habit_count++;
            printf(LIME "  Habit '%s' added! 🌱\n" R, h->name);
        }
    } else if (c == 3 && e->habit_count > 0) {
        printf(IVORY "\n  Remove which? (1-%d): " R, e->habit_count);
        int ri = _int(1, e->habit_count) - 1;
        printf(CRIMSON "  Removed '%s'.\n" R, e->habits[ri].name);
        for (int i=ri;i<e->habit_count-1;i++) e->habits[i]=e->habits[i+1];
        e->habit_count--;
    }
    _enter();
}

/* ══════════════════════════ SAFETY / CRISIS PLAN ══════════════════════ */
void ext_crisis_plan(ExtState *e, Player *p) {
    _cls();
    _div(CRIMSON,'═',60);
    printf(CRIMSON "  🛡  " ORANGE BOLD "MY SAFETY PLAN — Personal Crisis Plan" R "\n");
    _div(CRIMSON,'═',60);
    printf(IVORY "\n  A safety plan is a personal toolkit for your hardest moments.\n");
    printf("  Built now, so it's ready when you need it most.\n\n" R);

    CrisisPlan *cp = &e->crisis_plan;

    if (cp->created) {
        printf(GOLD "  Your current safety plan:\n\n" R);

        printf(ORANGE "  ⚠  Warning Signs I notice:\n" R);
        for (int i=0;i<cp->warning_count;i++)
            printf(IVORY "     %d. %s\n" R, i+1, cp->warning_signs[i]);

        printf(FOREST "\n  🌿 Coping Strategies that help me:\n" R);
        for (int i=0;i<cp->coping_count;i++)
            printf(IVORY "     %d. %s\n" R, i+1, cp->coping_strategies[i]);

        printf(LAKE "\n  🤝 People I can reach out to:\n" R);
        for (int i=0;i<cp->support_count;i++)
            printf(IVORY "     %d. %s\n" R, i+1, cp->support_people[i]);

        printf(CRIMSON "\n  📞 Emergency Contacts:\n" R);
        for (int i=0;i<cp->contact_count;i++)
            printf(IVORY "     %d. %s\n" R, i+1, cp->emergency_contacts[i]);

        printf(GOLD "\n  Update plan? (Y/N): " R);
        char ch = _char();
        if (ch != 'Y') { _enter(); return; }
    }

    /* Build the plan */
    memset(cp, 0, sizeof(CrisisPlan));

    printf(ORANGE "\n  Step 1: Warning Signs (up to %d — things that tell you you're struggling)\n" R, MAX_CRISIS_PLAN);
    printf(DIM "  (e.g. 'I stop eating', 'I isolate myself', 'I can't sleep')\n" R);
    while (cp->warning_count < MAX_CRISIS_PLAN) {
        printf(ORANGE "  [%d] Add a sign (or press ENTER to continue): " R, cp->warning_count+1);
        char buf[127];
        if (fgets(buf,127,stdin)) {
            if (buf[0]=='\n') break;
            int l=strlen(buf); if(l>0&&buf[l-1]=='\n') buf[l-1]='\0';
            strncpy(cp->warning_signs[cp->warning_count++], buf, 127);
        }
    }

    printf(FOREST "\n  Step 2: Coping Strategies (things that help you)\n" R);
    printf(DIM "  (e.g. 'Call a friend', 'Go for a walk', 'Breathe for 5 mins')\n" R);
    while (cp->coping_count < MAX_CRISIS_PLAN) {
        printf(FOREST "  [%d] Add strategy (or ENTER to continue): " R, cp->coping_count+1);
        char buf[127];
        if (fgets(buf,127,stdin)) {
            if (buf[0]=='\n') break;
            int l=strlen(buf); if(l>0&&buf[l-1]=='\n') buf[l-1]='\0';
            strncpy(cp->coping_strategies[cp->coping_count++], buf, 127);
        }
    }

    printf(LAKE "\n  Step 3: Support People (name + how to reach them, up to 5)\n" R);
    while (cp->support_count < 5) {
        printf(LAKE "  [%d] Person (or ENTER to continue): " R, cp->support_count+1);
        char buf[63];
        if (fgets(buf,63,stdin)) {
            if (buf[0]=='\n') break;
            int l=strlen(buf); if(l>0&&buf[l-1]=='\n') buf[l-1]='\0';
            strncpy(cp->support_people[cp->support_count++], buf, 63);
        }
    }

    printf(CRIMSON "\n  Step 4: Emergency Contacts (e.g. Butabika: +256 414 504 375)\n" R);
    /* Pre-fill with Butabika */
    strncpy(cp->emergency_contacts[0], "Butabika Hospital: +256 414 504 375", 127);
    strncpy(cp->emergency_contacts[1], "Butabika Toll-Free: 0800 211 306", 127);
    cp->contact_count = 2;
    printf(SAVANNA "  (Butabika contacts pre-filled. Add more?)\n" R);
    while (cp->contact_count < 3) {
        printf(CRIMSON "  [%d] Contact (or ENTER to skip): " R, cp->contact_count+1);
        char buf[127];
        if (fgets(buf,127,stdin)) {
            if (buf[0]=='\n') break;
            int l=strlen(buf); if(l>0&&buf[l-1]=='\n') buf[l-1]='\0';
            strncpy(cp->emergency_contacts[cp->contact_count++], buf, 127);
        }
    }

    cp->created = 1;
    printf(LIME "\n  ✅ Safety plan saved.\n");
    printf(GOLD "  This plan belongs to you. It is ready when you need it.\n" R);
    award_xp(p, 100, "Safety Plan Created");
    _enter();
}

/* ══════════════════════════ STORY MODE ════════════════════════════════ */
void ext_story_mode(ExtState *e, Player *p) {
    _cls();
    _div(AMBER,'═',60);
    printf(AMBER "  📜  " GOLD BOLD "STORY MODE — The Ubuntu Healing Journey" R "\n");
    _div(AMBER,'═',60);
    printf(IVORY "\n  A 12-chapter healing narrative set across Uganda.\n");
    printf("  Each chapter unlocks after completing activities.\n\n" R);

    /* Unlock chapters based on level */
    for (int i=0;i<MAX_STORY_CHAPTERS;i++) {
        int req_level = 1 + i * (MAX_LEVELS / MAX_STORY_CHAPTERS);
        if (p->level >= req_level) e->chapters[i].unlocked = 1;
    }

    /* List chapters */
    for (int i=0;i<MAX_STORY_CHAPTERS;i++) {
        StoryChapter *ch = &e->chapters[i];
        if (!ch->unlocked) {
            printf(DIM "  %2d. %-30s [Locked]\n" R, i+1, ch->title);
        } else if (ch->completed) {
            printf(LIME "  %2d. %-30s ✓\n" R, i+1, ch->title);
        } else {
            printf(GOLD "  %2d. " IVORY "%-30s" GOLD " ▶ Read (+%dXP)\n" R,
                   i+1, ch->title, ch->xp_reward);
        }
    }

    printf(SAVANNA "\n  Read a chapter (1-12, or 0 to go back): " R);
    int c = _int(0, 12);
    if (c == 0) return;

    StoryChapter *ch = &e->chapters[c-1];
    if (!ch->unlocked) {
        printf(ORANGE "\n  Chapter locked. Keep playing to unlock.\n" R);
        _enter(); return;
    }

    _cls();
    _div(AMBER,'─',60);
    printf(AMBER "  Chapter %d: " GOLD BOLD "%s\n" R, c, ch->title);
    _div(AMBER,'─',60);
    printf("\n");

    /* Display the narrative for this chapter */
    _tw(IVORY, "  ", 0);
    _tw(SAVANNA, ch->summary, 22);
    printf("\n\n" R);

    /* Interactive element for each chapter */
    const char *reflections[] = {
        "What do YOU need to grow right now?",
        "What has been softening inside you lately?",
        "What are you ready to let the river carry away?",
        "What lesson has silence been teaching you?",
        "Where in your life do you feel you belong?",
        "What have you heard in your own stillness?",
        "What story about yourself are you ready to rise above?",
        "What truth have you been afraid to share?",
        "What would your proudest ancestor say to you right now?",
        "Who is the calmer, brighter version of you becoming?",
        "What has been your hardest step in this healing journey?",
        "What light do you carry that others might not see yet?"
    };

    printf(CYAN "  Reflection: %s\n  > " R, reflections[c-1]);
    char buf[256];
    if (fgets(buf, 255, stdin) && strlen(buf) > 1) {
        printf(FOREST "\n  Your reflection has been woven into the story. 🌿\n" R);
        sleep(1);
    }

    if (!ch->completed) {
        ch->completed = 1;
        award_xp(p, ch->xp_reward, "Story Chapter Complete");
        printf(GOLD "  Chapter complete! 📜\n" R);
    } else {
        printf(SAVANNA "  (Already completed — no XP, but reflection is always good.)\n" R);
    }
    _enter();
}

/* ══════════════════════════ CHALLENGES ════════════════════════════════ */
void ext_challenges(ExtState *e, Player *p) {
    _cls();
    _div(ROSE,'═',60);
    printf(ROSE "  🏆  " PINK BOLD "DAILY CHALLENGES — Ubuntu Community Goals" R "\n");
    _div(ROSE,'═',60);

    /* Reset expired challenges */
    long today = _day();
    for (int i=0;i<e->challenge_count;i++) {
        if (e->challenges[i].expires_day <= today && !e->challenges[i].complete) {
            e->challenges[i].progress  = 0;
            e->challenges[i].complete  = 0;
            e->challenges[i].expires_day = today + 1;
        }
    }

    printf(IVORY "\n  Today's challenges:\n\n" R);
    int any_active = 0;
    for (int i=0;i<e->challenge_count;i++) {
        Challenge *ch = &e->challenges[i];
        if (ch->complete) {
            printf(LIME "  ✅ %-30s +%dXP\n" R, ch->title, ch->xp_reward);
        } else {
            printf(GOLD "  🎯 %-30s [%d/%d] +%dXP\n" R,
                   ch->title, ch->progress, ch->target, ch->xp_reward);
            printf(DIM  "     %s\n" R, ch->desc);
            any_active = 1;
        }
    }

    if (any_active) {
        printf(CYAN "\n  Mark a challenge done? (number, or 0 to back): " R);
        int c = _int(0, e->challenge_count);
        if (c > 0) {
            Challenge *ch = &e->challenges[c-1];
            if (ch->complete) {
                printf(ORANGE "  Already complete!\n" R);
            } else {
                ch->progress++;
                if (ch->progress >= ch->target) {
                    ch->complete = 1;
                    printf(LIME BOLD "  🏆 CHALLENGE COMPLETE: %s!\n" R, ch->title);
                    award_xp(p, ch->xp_reward, "Challenge Complete");
                } else {
                    printf(SAVANNA "  Progress: %d/%d\n" R, ch->progress, ch->target);
                }
            }
        }
    }
    _enter();
}

/* ══════════════════════════ FRIENDS / ACCOUNTABILITY ═════════════════ */
void ext_friends(ExtState *e, Player *p) {
    _cls();
    _div(LAKE,'═',60);
    printf(LAKE "  🤝  " SKY BOLD "ACCOUNTABILITY CIRCLE — Ubuntu Partners" R "\n");
    _div(LAKE,'═',60);
    printf(IVORY "\n  \"If you want to go far, go together.\" — African Proverb\n\n" R);

    if (e->friend_count > 0) {
        printf(GOLD "  Your Ubuntu Circle:\n\n" R);
        for (int i=0;i<e->friend_count;i++) {
            Friend *f = &e->friends[i];
            if (!f->active) continue;
            printf(LAKE "  %d. " BOLD "%-20s" R LAKE " Lv.%d  🔥%d-streak\n" R,
                   i+1, f->name, f->level, f->streak);
        }
        _nl();
    }

    printf(GOLD "  ┌───────────────────────────────────────────┐\n");
    printf(GOLD "  │ 1. Add an accountability partner         │\n");
    printf(GOLD "  │ 2. Update a partner's progress           │\n");
    printf(GOLD "  │ 3. Send encouragement (log message)      │\n");
    printf(GOLD "  │ 4. Remove a partner                      │\n");
    printf(GOLD "  │ 5. Back                                  │\n");
    printf(GOLD "  └───────────────────────────────────────────┘\n" R);
    printf(SAVANNA "  > " R);

    int c = _int(1,5);

    if (c == 1) {
        if (e->friend_count >= MAX_FRIENDS) {
            printf(ORANGE "  Circle full (max %d).\n" R, MAX_FRIENDS);
        } else {
            Friend *f = &e->friends[e->friend_count];
            memset(f, 0, sizeof(Friend));
            printf(CYAN "\n  Partner name: " R);
            if (fgets(f->name, MAX_NAME-1, stdin)) {
                int l=strlen(f->name); if(l>0&&f->name[l-1]=='\n') f->name[l-1]='\0';
            }
            printf(CYAN "  Their level (1-20): " R); f->level = _int(1,20);
            printf(CYAN "  Their streak (days): " R); f->streak = _int(0,365);
            f->active = 1;
            e->friend_count++;
            printf(LIME "  %s added to your Ubuntu Circle! 🤝\n" R, f->name);
            award_xp(p, 30, "Added Accountability Partner");
        }
    } else if (c == 2 && e->friend_count > 0) {
        printf(IVORY "\n  Update which partner? (1-%d): " R, e->friend_count);
        int fi = _int(1, e->friend_count) - 1;
        printf(CYAN "  New level: " R); e->friends[fi].level  = _int(1,20);
        printf(CYAN "  New streak: " R); e->friends[fi].streak = _int(0,365);
        printf(LIME "  Updated!\n" R);
    } else if (c == 3 && e->friend_count > 0) {
        printf(IVORY "\n  Which partner? (1-%d): " R, e->friend_count);
        int fi = _int(1,e->friend_count)-1;
        printf(CYAN "  Message of encouragement:\n  > " R);
        char msg[256];
        if (fgets(msg,255,stdin)) {
            printf(LIME "  Logged: \"" ITALIC "%s" R LIME "\" — sent with Ubuntu love. 💛\n" R, msg);
            award_xp(p, 15, "Encouraged a Partner");
        }
    } else if (c == 4 && e->friend_count > 0) {
        printf(IVORY "\n  Remove which? (1-%d): " R, e->friend_count);
        int ri = _int(1,e->friend_count)-1;
        printf(CRIMSON "  Removed %s.\n" R, e->friends[ri].name);
        for (int i=ri;i<e->friend_count-1;i++) e->friends[i]=e->friends[i+1];
        e->friend_count--;
    }
    _enter();
}

/* ══════════════════════════ PERSONAL MANTRA ═══════════════════════════ */
void ext_set_mantra(ExtState *e, Player *p) {
    _cls();
    _div(PURPLE,'═',60);
    printf(PURPLE "  🔮  " PINK BOLD "PERSONAL MANTRA — Your Sacred Phrase" R "\n");
    _div(PURPLE,'═',60);
    printf(IVORY "\n  A mantra is a personal affirmation that is entirely yours.\n");
    printf("  Unlike pre-written affirmations, this is your own truth.\n\n" R);

    if (e->mantra_set) {
        printf(GOLD "  Your current mantra:\n\n");
        printf(GOLD BOLD "  « %s »\n\n" R, e->personal_mantra);
        printf(IVORY "  Read it aloud. Let it settle into your bones.\n" R);
        printf(CYAN "\n  Change mantra? (Y/N): " R);
        char ch = _char();
        if (ch != 'Y') { _enter(); return; }
    }

    printf(GOLD "\n  Examples for inspiration:\n" R);
    printf(DIM "  • 'I rise like the Nile — slow, powerful, unstoppable.'\n");
    printf("  • 'I am rooted in love. Nothing breaks my roots.'\n");
    printf("  • 'Every day I choose to grow.'\n\n" R);

    printf(GOLD "  Write YOUR mantra:\n  > " R);
    if (fgets(e->personal_mantra, 255, stdin)) {
        int l=strlen(e->personal_mantra);
        if (l>0&&e->personal_mantra[l-1]=='\n') e->personal_mantra[l-1]='\0';
    }

    e->mantra_set = 1;
    printf(PURPLE "\n  Your mantra has been set:\n\n");
    printf(GOLD BOLD "  « %s »\n\n" R, e->personal_mantra);
    printf(SAVANNA "  Speak this every morning like the drum speaks at dawn.\n" R);
    award_xp(p, 40, "Personal Mantra Set");
    _enter();
}

/* ══════════════════════════ SLEEP HYGIENE ═════════════════════════════ */
void ext_sleep_hygiene(Player *p) {
    _cls();
    _div(INDIGO,'═',60);
    printf(INDIGO "  🌙  " MIST BOLD "SLEEP HYGIENE — Wind-Down Routine" R "\n");
    _div(INDIGO,'═',60);
    printf(IVORY "\n  The Rwenzori mountains are at peace in the night.\n");
    printf("  Let your mind find that same mountain stillness.\n\n" R);

    /* Time-of-day personalisation */
    time_t now = time(NULL);
    struct tm *t = localtime(&now);
    int hour = t->tm_hour;

    if (hour >= 20 || hour < 5) {
        printf(INDIGO "  🌙 Evening Mode — Wind-Down Routine\n\n" R);
        printf(MIST "  Your 5-step wind-down:\n\n" R);
        const char *steps[] = {
            "📵  Put your phone face-down — the Nile flows without Wi-Fi",
            "💡  Dim your lights or use candlelight for 30 mins",
            "📖  Read something calming — a proverb, a poem, a story",
            "🧘  3 minutes of gentle breathing (4-4-4-4 box breath)",
            "📝  Write one thing that went well today in your journal"
        };
        for (int i=0;i<5;i++) {
            printf(INDIGO "  Step %d: " IVORY "%s\n" R, i+1, steps[i]);
            _ms(300);
        }
    } else if (hour >= 5 && hour < 10) {
        printf(LIME "  🌅 Morning Mode — Rise & Shine Routine\n\n" R);
        const char *steps[] = {
            "🌊  Drink a full glass of water — the Nile starts the day",
            "🌬  5 deep breaths before touching your phone",
            "🌞  Step outside for 2 minutes of sunlight if you can",
            "📝  Set one intention for the day in your journal",
            "🌱  Say your personal mantra aloud"
        };
        for (int i=0;i<5;i++) {
            printf(LIME "  Step %d: " IVORY "%s\n" R, i+1, steps[i]);
            _ms(300);
        }
    } else {
        printf(SAVANNA "  ☀  Daytime Reset — Midday Recharge\n\n" R);
        const char *steps[] = {
            "🚶  5-minute walk — even around the room",
            "💧  Drink water — Lake Victoria sustains everything",
            "👁  Look away from screens for 2 minutes (20-20-20 rule)",
            "🌬  4 slow belly breaths to reset the nervous system",
            "📝  One thing going well today — write it or say it"
        };
        for (int i=0;i<5;i++) {
            printf(SAVANNA "  Step %d: " IVORY "%s\n" R, i+1, steps[i]);
            _ms(300);
        }
    }

    printf(INDIGO "\n  Sleep tips for tonight:\n" R);
    printf(DIM "  • Keep a consistent bedtime — even weekends\n");
    printf("  • Cool the room if possible — the Ugandan highland air\n");
    printf("  • No screens 30 mins before bed\n");
    printf("  • If you wake at night — try the 4-7-8 breath\n\n" R);

    award_xp(p, 25, "Sleep Hygiene Routine");
    _enter();
}

/* ══════════════════════════ PANIC BUTTON ══════════════════════════════ */
/*
 * Instant grounding for acute anxiety or panic attacks.
 * No menus — straight into action.
 */
void ext_panic_button(Player *p) {
    _cls();
    printf(CRIMSON BOLD "\n\n  🆘  PANIC BUTTON — You Are Safe.\n\n" R);
    printf(IVORY "  You are " BOLD "not" R IVORY " in danger. Your body is trying to protect you.\n");
    printf("  Let's slow everything down. Follow these steps.\n\n" R);
    sleep(1);

    printf(ORANGE BOLD "  STEP 1: BREATHE\n" R);
    printf(IVORY "  Put one hand on your chest. One on your belly.\n\n" R);
    for (int s=5;s>=1;s--) {
        printf("\r  " FOREST "🌬 BREATHE IN — %d...\t" R, s); fflush(stdout); sleep(1);
    }
    printf("\n");
    for (int s=5;s>=1;s--) {
        printf("\r  " LAKE "🌊 BREATHE OUT — %d...\t" R, s); fflush(stdout); sleep(1);
    }
    printf("\n");
    for (int s=5;s>=1;s--) {
        printf("\r  " FOREST "🌬 BREATHE IN — %d...\t" R, s); fflush(stdout); sleep(1);
    }
    printf("\n");
    for (int s=7;s>=1;s--) {
        printf("\r  " LAKE "🌊 BREATHE OUT slowly — %d...\t" R, s); fflush(stdout); sleep(1);
    }
    printf("\n\n");

    printf(ORANGE BOLD "  STEP 2: GROUND — Name these aloud:\n" R);
    printf(LAKE   "  5 things you can SEE right now.\n" R);   sleep(4);
    printf(EARTH  "  4 things you can TOUCH.\n" R);           sleep(4);
    printf(FOREST "  3 things you can HEAR.\n" R);            sleep(3);
    printf(SAVANNA"  2 things you can SMELL.\n" R);           sleep(3);
    printf(ORANGE "  1 thing you can TASTE.\n\n" R);          sleep(2);

    printf(ORANGE BOLD "  STEP 3: REMEMBER\n" R);
    _tw(IVORY, "  You have survived every difficult moment before this one.\n", 20);
    _tw(IVORY, "  This feeling will pass — it always does.\n", 20);
    _tw(GOLD,  "  You are stronger than this moment.\n\n", 20);

    printf(CRIMSON "  📞 If you need to talk to someone:\n" R);
    printf(SAVANNA "     Butabika: +256 414 504 375\n");
    printf(SAVANNA "     Toll Free: 0800 211 306\n\n" R);

    printf(FOREST "  You made it through. Breathe. 🌿\n\n" R);
    _enter();
}

/* ══════════════════════════ WEEKLY REPORT ═════════════════════════════ */
void ext_weekly_report(ExtState *e, Player *p) {
    _cls();
    _div(GOLD,'═',60);
    printf(GOLD "  📊  " SAVANNA BOLD "WEEKLY WELLNESS REPORT" R "\n");
    _div(GOLD,'═',60);

    /* Count completed habits */
    int habits_done = 0;
    for (int i=0;i<e->habit_count;i++) {
        int done = 0;
        for (int d=0;d<7;d++) done += e->habits[i].completions[d];
        if (done >= e->habits[i].target_per_week) habits_done++;
    }

    /* Count completed challenges */
    int challenges_done = 0;
    for (int i=0;i<e->challenge_count;i++)
        if (e->challenges[i].complete) challenges_done++;

    /* Count story chapters */
    int chapters_done = 0;
    for (int i=0;i<MAX_STORY_CHAPTERS;i++)
        if (e->chapters[i].completed) chapters_done++;

    /* Count badges */
    int badges_total = 0;
    for (int i=0;i<MAX_BADGES;i++) if (p->badges[i]) badges_total++;

    printf(GOLD "\n  Player: " BOLD "%s" R GOLD "  |  Level: " BOLD "%d/20\n\n" R,
           p->name, p->level);

    _div(SAVANNA,'─',60);
    printf(ORANGE "  📅 Streak:              " BOLD "%d days\n" R, p->streak);
    printf(LAKE   "  🧘 Meditations:         " BOLD "%d\n" R, p->meditations);
    printf(FOREST "  🌬 Breathing sessions:  " BOLD "%d\n" R, p->breaths);
    printf(JUNGLE "  🌿 Grounding sessions:  " BOLD "%d\n" R, p->grounds);
    printf(EARTH  "  📖 Journal entries:     " BOLD "%d\n" R, p->journals);
    printf(PURPLE "  🦩 Affirmations:        " BOLD "%d\n" R, p->affirmations);
    printf(CYAN   "  🎮 Mini-games won:      " BOLD "%d\n" R, p->minigames_won);
    printf(GOLD   "  💰 Ubuntu Coins:        " BOLD "%d\n" R, p->coins);
    printf(LIME   "  ✅ Habits on track:     " BOLD "%d/%d\n" R, habits_done, e->habit_count);
    printf(ROSE   "  🏆 Challenges done:     " BOLD "%d/%d\n" R, challenges_done, e->challenge_count);
    printf(AMBER  "  📜 Story chapters:      " BOLD "%d/%d\n" R, chapters_done, MAX_STORY_CHAPTERS);
    printf(GOLD   "  🏅 Badges earned:       " BOLD "%d/%d\n" R, badges_total, MAX_BADGES);
    _div(SAVANNA,'─',60);

    /* Mood average if data available */
    if (e->mood_hist.count > 0) {
        int n = e->mood_hist.count > 7 ? 7 : e->mood_hist.count;
        int sum = 0;
        for (int i=0;i<n;i++) {
            int idx = ((e->mood_hist.head-n+i)+MAX_MOOD_HISTORY)%MAX_MOOD_HISTORY;
            sum += e->mood_hist.mood[idx];
        }
        printf(ORANGE "\n  Avg mood (last 7 days): " BOLD "%.1f/10\n" R, (float)sum/n);
    }

    /* Personalised message */
    printf(IVORY "\n  ");
    if (p->streak >= 7)
        _tw(CRIMSON, "🔥 7+ day streak! You are building something real.\n  ", 18);
    if (p->journals >= 5)
        _tw(GOLD, "📖 Consistent journalling. Your inner griot is growing.\n  ", 18);
    if (p->anxiety <= 3)
        _tw(LAKE, "🌊 Low anxiety reported — your breathing work is paying off.\n  ", 18);
    if (p->level >= 10 && p->premium)
        _tw(PURPLE, "★ Premium Lv10+ — you are in deep healing territory.\n  ", 18);

    printf(FOREST "\n  Well done this week, %s. Ubuntu. 🌍\n\n" R, p->name);
    award_xp(p, 50, "Weekly Report Viewed");
    _enter();
}

/* ══════════════════════════ EXTENSION MENU ════════════════════════════ */
void ext_menu(ExtState *e, Player *p) {
    int running = 1;
    while (running) {
        _cls();
        _div(GOLD,'═',60);
        printf(GOLD "  🌍  " AMBER BOLD "UBUNTU EXTENSIONS — Advanced Features" R "\n");
        _div(GOLD,'═',60);
        printf(GOLD "\n  ╔══════════════════════════════════════════════════╗\n");
        printf(GOLD "  ║ " CYAN   " 1." IVORY " Mood Analytics      " DIM "(30-day chart)" GOLD "          ║\n");
        printf(GOLD "  ║ " PURPLE " 2." IVORY " CBT Workbook        " DIM "(thought records)" GOLD "       ║\n");
        printf(GOLD "  ║ " TEAL   " 3." IVORY " Habit Tracker       " DIM "(weekly habits)" GOLD "         ║\n");
        printf(GOLD "  ║ " CRIMSON" 4." IVORY " My Safety Plan      " DIM "(crisis toolkit)" GOLD "        ║\n");
        printf(GOLD "  ║ " AMBER  " 5." IVORY " Story Mode          " DIM "(12 chapters)" GOLD "           ║\n");
        printf(GOLD "  ║ " ROSE   " 6." IVORY " Daily Challenges    " DIM "(community goals)" GOLD "       ║\n");
        printf(GOLD "  ║ " LAKE   " 7." IVORY " Accountability Circle" DIM "(friends)" GOLD "              ║\n");
        printf(GOLD "  ╠══════════════════════════════════════════════════╣\n");
        printf(GOLD "  ║ " PINK   " 8." IVORY " Personal Mantra     " DIM "(your sacred phrase)" GOLD "    ║\n");
        printf(GOLD "  ║ " INDIGO " 9." IVORY " Sleep Hygiene Guide " DIM "(wind-down routine)" GOLD "     ║\n");
        printf(GOLD "  ║ " RED    "10." IVORY " 🆘 PANIC BUTTON    " DIM "(instant grounding)" GOLD "      ║\n");
        printf(GOLD "  ║ " SAVANNA"11." IVORY " Weekly Report       " DIM "(full stats)" GOLD "            ║\n");
        printf(GOLD "  ║ " DIM    "12." RESET IVORY " Back to Main Menu" GOLD "                            ║\n");
        printf(GOLD "  ╚══════════════════════════════════════════════════╝\n\n" R);

        /* Show mantra if set */
        if (e->mantra_set)
            printf(ITALIC PURPLE "  \"%s\"\n\n" R, e->personal_mantra);

        printf(SAVANNA "  Choose (1-12): " R);
        int c = _int(1,12);

        ext_record_mood(e, p); /* record mood snapshot on each ext action */

        switch (c) {
            case  1: ext_show_mood_chart(e);        break;
            case  2: ext_cbt_session(e, p);         break;
            case  3: ext_habit_tracker(e, p);       break;
            case  4: ext_crisis_plan(e, p);         break;
            case  5: ext_story_mode(e, p);          break;
            case  6: ext_challenges(e, p);          break;
            case  7: ext_friends(e, p);             break;
            case  8: ext_set_mantra(e, p);          break;
            case  9: ext_sleep_hygiene(p);          break;
            case 10: ext_panic_button(p);           break;
            case 11: ext_weekly_report(e, p);       break;
            case 12: running = 0;                   break;
        }
        ext_save(e);
    }
}

/*
 * ════════════════════════════════════════════════════════════════════
 *  END OF umg_extensions.c
 *  New features: Mood Analytics | CBT Workbook | Habit Tracker
 *  Safety Plan | Story Mode | Challenges | Accountability Circle
 *  Panic Button | Sleep Guide | Weekly Report | Personal Mantra
 *  "If you want to go far, go together." — African Proverb 🌍
 * ════════════════════════════════════════════════════════════════════
 */
