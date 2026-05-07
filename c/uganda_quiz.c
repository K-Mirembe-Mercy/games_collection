#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define MAX_OPTIONS 4
#define TOTAL_QUESTIONS 15

// DATA STRUCTURES

typedef struct {
    char question[300];
    char options[MAX_OPTIONS][150];
    int correct; // 0-indexed
    char explanation[300];
} Question;

typedef struct {
    char name[50];
    char region[80];
    char language[50];
    char description[400];
} Tribe;

// ─────────────────────────────────────────────
// QUIZ DATA
// ─────────────────────────────────────────────

Question questions[TOTAL_QUESTIONS] = {
    {
        "What is the capital city of Uganda?",
        {"Entebbe", "Kampala", "Jinja", "Gulu"},
        1,
        "Kampala is Uganda's capital and largest city, sitting on several hills in central Uganda."
    },
    {
        "In which year did Uganda gain independence from Britain?",
        {"1960", "1961", "1962", "1963"},
        2,
        "Uganda gained independence on October 9, 1962. October 9 is celebrated as Independence Day."
    },
    {
        "What is the national language of Uganda?",
        {"Luganda", "Swahili", "English", "Runyakitara"},
        2,
        "English is Uganda's official national language, a legacy of British colonial rule."
    },
    {
        "Which lake forms part of Uganda's southern border?",
        {"Lake Tanganyika", "Lake Malawi", "Lake Victoria", "Lake Albert"},
        2,
        "Lake Victoria, the largest lake in Africa, borders Uganda, Kenya, and Tanzania."
    },
    {
        "What is Uganda's national bird?",
        {"African Eagle", "Grey Crowned Crane", "Marabou Stork", "African Fish Eagle"},
        1,
        "The Grey Crowned Crane (Crested Crane) is Uganda's national bird and appears on the flag."
    },
    {
        "Who was Uganda's first Prime Minister at independence?",
        {"Idi Amin", "Yoweri Museveni", "Milton Obote", "Benedicto Kiwanuka"},
        2,
        "Milton Obote became Uganda's first Prime Minister when the country gained independence in 1962."
    },
    {
        "What is the name of Uganda's currency?",
        {"Uganda Dollar", "Uganda Pound", "Uganda Shilling", "Uganda Franc"},
        2,
        "The Uganda Shilling (UGX) is the official currency of Uganda."
    },
    {
        "The source of the Nile River is located in which Ugandan city?",
        {"Kampala", "Gulu", "Jinja", "Mbale"},
        2,
        "Jinja is known as the 'Source of the Nile' where the Nile begins its journey from Lake Victoria."
    },
    {
        "Bwindi Impenetrable Forest is famous for which animal?",
        {"Lions", "Mountain Gorillas", "Chimpanzees", "Elephants"},
        1,
        "Bwindi is home to nearly half the world's remaining Mountain Gorillas and is a UNESCO World Heritage Site."
    },
    {
        "What percentage of Uganda's flag is black?",
        {"One third", "Half", "One sixth", "Two thirds"},
        0,
        "Uganda's flag has six equal horizontal bands: black, yellow, red repeated twice. Black is one third."
    },
    {
        "Which Ugandan dish is made from steamed green bananas?",
        {"Posho", "Rolex", "Matooke", "Katogo"},
        2,
        "Matooke is Uganda's national dish — steamed or boiled green bananas often served with groundnut sauce."
    },
    {
        "The Rwenzori Mountains are also called what?",
        {"Mountains of Fire", "Mountains of the Moon", "Mountains of the Sun", "Mountains of the Nile"},
        1,
        "The Rwenzori Mountains are called 'Mountains of the Moon', known for their snow-capped peaks near the equator."
    },
    {
        "Which Ugandan leader ruled from 1971 to 1979 and was known for brutal dictatorship?",
        {"Milton Obote", "Tito Okello", "Idi Amin Dada", "Yoweri Museveni"},
        2,
        "Idi Amin ruled Uganda from 1971-1979 and is one of history's most notorious dictators."
    },
    {
        "What is a 'Rolex' in Ugandan street food culture?",
        {"A type of beef stew", "An egg omelette rolled in chapati", "A grilled maize dish", "A banana dessert"},
        1,
        "A Rolex is a popular Ugandan street food — an omelette with vegetables rolled inside a chapati. The name comes from 'Rolled Eggs'."
    },
    {
        "Uganda is sometimes called the 'Pearl of Africa'. Who first used this phrase?",
        {"Nelson Mandela", "Winston Churchill", "Queen Elizabeth II", "David Livingstone"},
        1,
        "Winston Churchill called Uganda the 'Pearl of Africa' in his 1908 book, praising its beauty and richness."
    }
};

// ─────────────────────────────────────────────
// TRIBES DATA
// ─────────────────────────────────────────────

Tribe tribes[TOTAL_TRIBES] = {
    {
        "Baganda",
        "Central Uganda (Buganda Kingdom)",
        "Luganda",
        "The largest ethnic group in Uganda, forming the Buganda Kingdom. Known for their rich cultural heritage, the Kabaka (king) system, and the Kasubi Tombs. They played a central role in Uganda's colonial and post-independence history."
    },
    {
        "Banyankole",
        "Western Uganda (Ankole region)",
        "Runyankole",
        "Known for the Ankole long-horned cattle, which are a symbol of wealth and status. The Banyankole have a strong pastoral tradition and were historically divided into the Bairu (farmers) and Bahima (herders) social classes."
    },
    {
        "Basoga",
        "Eastern Uganda (Busoga region)",
        "Lusoga",
        "The second largest group, located near the source of the Nile. The Basoga are known for their music, especially the amadinda xylophone. They have a chieftaincy system led by the Kyabazinga."
    },
    {
        "Acholi",
        "Northern Uganda",
        "Acholi (Luo)",
        "A Nilotic people known for their strong warrior tradition, vibrant dance (Bwola), and music. The Acholi were heavily affected by the LRA conflict but have a rich cultural revival underway. Their traditional leader is the Rwot."
    },
    {
        "Langi",
        "North-Central Uganda",
        "Lango",
        "A Nilotic group closely related to the Acholi. Known as skilled farmers and warriors. Former President Milton Obote was Langi. They are known for the Myel Orak dance and strong communal values."
    },
    {
        "Batoro",
        "Western Uganda (Tooro Kingdom)",
        "Rutooro",
        "One of Uganda's ancient kingdoms, the Tooro Kingdom. Known for their famous queen mothers (Batebe) and the Rwenzori Mountains. Their king holds the title Omukama. They are known for dignified culture and cattle keeping."
    },
    {
        "Bafumbira",
        "Southwestern Uganda (Kigezi region)",
        "Rufumbira (Kinyarwanda)",
        "Related to the Banyarwanda, living in the highlands of Kisoro near the Virunga volcanoes. Known for intore dance, terraced farming, and proximity to gorilla habitats. They share culture with Rwanda and DR Congo."
    },
    {
        "Iteso",
        "Eastern Uganda and parts of Kenya",
        "Ateso",
        "A Nilotic people known for their agricultural skills and the Akiriket (council of elders) governance system. The Iteso are famous for their emong dance and skill in iron smelting. They form one of Uganda's largest ethnic groups."
    },
    {
        "Karamojong",
        "Northeastern Uganda (Karamoja region)",
        "Ngakarimojong",
        "A semi-nomadic pastoralist people known for their distinct culture, cattle raiding traditions, and traditional dress. They live in one of Uganda's most remote regions. Elders wear distinctive ochre-covered hairstyles and the people are known for their resilience."
    },
    {
        "Banyoro",
        "Western Uganda (Bunyoro-Kitara Kingdom)",
        "Runyoro",
        "One of Uganda's oldest and most powerful kingdoms — the Bunyoro-Kitara Empire once dominated the Great Lakes region. The Banyoro are known for their historical resistance to British colonialism and their Omukama (king). Rich in iron working and trade traditions."
    }
};

// ─────────────────────────────────────────────
// UTILITY FUNCTIONS
// ─────────────────────────────────────────────

void clear_screen() {
    #ifdef _WIN32
        system("cls");
    #else
        system("clear");
    #endif
}

void print_line(char ch, int length) {
    for (int i = 0; i < length; i++) printf("%c", ch);
    printf("\n");
}

void print_centered(const char *text, int width) {
    int len = strlen(text);
    int pad = (width - len) / 2;
    for (int i = 0; i < pad; i++) printf(" ");
    printf("%s\n", text);
}

void pause_screen() {
    printf("\n  Press ENTER to continue...");
    while (getchar() != '\n');
    getchar();
}

void print_header() {
    printf("\n");
    print_line('=', 60);
    print_centered("🇺🇬  UGANDA — HISTORY, CULTURE & HERITAGE  🇺🇬", 60);
    print_line('=', 60);
    printf("\n");
}

// ─────────────────────────────────────────────
// QUIZ FUNCTION
// ─────────────────────────────────────────────

void run_quiz() {
    clear_screen();
    print_header();
    printf("  Welcome to the Uganda History & Culture Quiz!\n");
    printf("  Answer 15 questions about Uganda.\n");
    printf("  Type 1, 2, 3, or 4 to choose your answer.\n\n");
    pause_screen();

    int score = 0;
    int answer;
    char input[10];

    for (int i = 0; i < TOTAL_QUESTIONS; i++) {
        clear_screen();
        printf("\n");
        print_line('-', 60);
        printf("  QUESTION %d of %d          SCORE: %d/%d\n", i+1, TOTAL_QUESTIONS, score, i);
        print_line('-', 60);
        printf("\n  %s\n\n", questions[i].question);

        for (int j = 0; j < MAX_OPTIONS; j++) {
            printf("    [%d] %s\n", j+1, questions[i].options[j]);
        }

        printf("\n  Your answer: ");
        fgets(input, sizeof(input), stdin);
        answer = atoi(input) - 1;

        printf("\n");
        print_line('-', 60);

        if (answer == questions[i].correct) {
            printf("  ✓  CORRECT!\n");
            score++;
        } else {
            printf("  ✗  WRONG! The correct answer was:\n");
            printf("     [%d] %s\n", questions[i].correct + 1, questions[i].options[questions[i].correct]);
        }

        printf("\n  💡 %s\n", questions[i].explanation);
        print_line('-', 60);
        pause_screen();
    }

    // Final Results
    clear_screen();
    printf("\n");
    print_line('*', 60);
    print_centered("QUIZ COMPLETE!", 60);
    print_line('*', 60);
    printf("\n");

    printf("  Your Final Score: %d / %d\n\n", score, TOTAL_QUESTIONS);

    int percent = (score * 100) / TOTAL_QUESTIONS;

    if (percent == 100) {
        printf("  🏆 PERFECT SCORE! You are a Uganda expert!\n");
    } else if (percent >= 80) {
        printf("  🌟 Excellent! You know Uganda very well!\n");
    } else if (percent >= 60) {
        printf("  👍 Good job! Keep learning about Uganda!\n");
    } else if (percent >= 40) {
        printf("  📚 Not bad, but there's more to discover!\n");
    } else {
        printf("  🇺🇬 Keep exploring Uganda's rich history!\n");
    }

    printf("\n  Score breakdown: %d%% correct\n", percent);

    // Visual bar
    printf("\n  [");
    int filled = percent / 5;
    for (int i = 0; i < 20; i++) {
        if (i < filled) printf("█");
        else printf("░");
    }
    printf("] %d%%\n\n", percent);

    print_line('*', 60);
    pause_screen();
}

// ─────────────────────────────────────────────
// TRIBES FUNCTION
// ─────────────────────────────────────────────

void show_tribes() {
    clear_screen();
    print_header();

    printf("  Uganda has over 65 recognized ethnic groups and tribes.\n");
    printf("  Below are 10 of the most prominent tribes.\n\n");
    print_line('=', 60);
    printf("  Press ENTER after each tribe to see the next.\n");
    print_line('=', 60);
    pause_screen();

    for (int i = 0; i < TOTAL_TRIBES; i++) {
        clear_screen();
        printf("\n");
        print_line('-', 60);
        printf("  TRIBE %d of %d\n", i+1, TOTAL_TRIBES);
        print_line('-', 60);

        printf("\n  NAME:     %s\n", tribes[i].name);
        printf("  REGION:   %s\n", tribes[i].region);
        printf("  LANGUAGE: %s\n", tribes[i].language);
        printf("\n");

        // Word wrap description at ~55 chars
        printf("  ABOUT:\n  ");
        int len = strlen(tribes[i].description);
        int col = 0;
        for (int j = 0; j < len; j++) {
            if (col >= 54 && tribes[i].description[j] == ' ') {
                printf("\n  ");
                col = 0;
            } else {
                printf("%c", tribes[i].description[j]);
                col++;
            }
        }
        printf("\n");

        print_line('-', 60);
        printf("  [%d/%d tribes viewed]\n", i+1, TOTAL_TRIBES);
        pause_screen();
    }

    clear_screen();
    printf("\n");
    print_line('=', 60);
    print_centered("ALL TRIBES EXPLORED!", 60);
    print_line('=', 60);
    printf("\n");
    printf("  Uganda's diversity is its greatest strength.\n");
    printf("  65+ tribes, one Pearl of Africa. 🇺🇬\n\n");
    print_line('=', 60);
    pause_screen();
}

// ─────────────────────────────────────────────
// MAIN MENU
// ─────────────────────────────────────────────

int main() {
    char input[10];
    int choice;

    while (1) {
        clear_screen();
        print_header();

        printf("  MAIN MENU\n\n");
        printf("    [1]  Take the Uganda History & Culture Quiz\n");
        printf("    [2]  Explore Uganda's Tribes (10 featured)\n");
        printf("    [3]  About Uganda (Quick Facts)\n");
        printf("    [0]  Exit\n\n");
        print_line('-', 60);
        printf("  Choose an option: ");

        fgets(input, sizeof(input), stdin);
        choice = atoi(input);

        if (choice == 1) {
            run_quiz();
        } else if (choice == 2) {
            show_tribes();
        } else if (choice == 3) {
            clear_screen();
            print_header();
            printf("  QUICK FACTS ABOUT UGANDA\n\n");
            print_line('-', 60);
            printf("  Official Name : Republic of Uganda\n");
            printf("  Capital       : Kampala\n");
            printf("  Independence  : October 9, 1962\n");
            printf("  Population    : ~49 million (2024 est.)\n");
            printf("  Area          : 241,038 sq km\n");
            printf("  Currency      : Uganda Shilling (UGX)\n");
            printf("  Official Lang : English & Swahili\n");
            printf("  President     : Yoweri Museveni (since 1986)\n");
            printf("  National Bird : Grey Crowned Crane\n");
            printf("  National Dish : Matooke (steamed bananas)\n");
            printf("  Nickname      : Pearl of Africa\n");
            printf("  Ethnic Groups : 65+ tribes\n");
            printf("  Religion      : Majority Christian\n");
            printf("  Neighbours    : Kenya, Tanzania, Rwanda,\n");
            printf("                  DRC, South Sudan\n");
            print_line('-', 60);
            pause_screen();
        } else if (choice == 0) {
            clear_screen();
            printf("\n  Thank you for exploring Uganda's heritage!\n");
            printf("  Webale nyo! (Thank you in Luganda) 🇺🇬\n\n");
            break;
        } else {
            printf("\n  Invalid option. Please try again.\n");
            pause_screen();
        }
    }

    return 0;
}
