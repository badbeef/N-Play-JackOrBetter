#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>
#include <string.h>

/*
 * Each card is represended with a byte,
 * 2-bit suit (0..3) + 4 bit number (1..13). 'Ace' is 1, 'Jack' is 11
 */

#define SUITS_MASK 0x03
#define NUMBER_MASK 0xF0
#define CARDS_PER_SUIT 13
#define SUITS_PER_DECK 4
#define CARDS_PER_DECK (CARDS_PER_SUIT * SUITS_PER_DECK)
#define CARDS_PER_HAND 5

#define NUM(card) ((card) & NUMBER_MASK)

typedef enum {
    spade = 0,
    heart,
    club,
    diamond
} suit;

char * num_str(uint8_t card) {
    switch ((card & NUMBER_MASK) >> 4) {
    case 1: return "Ace";
    case 2: return "2";
    case 3: return "3";
    case 4: return "4";
    case 5: return "5";
    case 6: return "6";
    case 7: return "7";
    case 8: return "8";
    case 9: return "9";
    case 10: return "10";
    case 11: return "Jack";
    case 12: return "Queen";
    case 13: return "King";
    default: return "NULL";
    }
}

char * suit_str(uint8_t card) {
    switch (card & SUITS_MASK) {
    case 0: return "Spade";
    case 1: return "Heart";
    case 2: return "Club";
    case 3: return "Diamond";
    default: return "NULL";
    }
}

typedef struct {
    uint8_t cards;
    uint8_t *card;
} cards;

/*
 * Use Fisher-Yates Shuffle
 * Assumes there are at least 2 cards in deck
 */
void shuffle (cards *deck)
{
    int i, j, tmp;
    for (i = deck->cards - 1; i > 0; i--) {
        j = random() % i;
        tmp = deck->card[j];
        deck->card[j] = deck->card[i];
        deck->card[i] = tmp;
    }
}

void show_cards(cards *hand)
{
    int i;
    for (i = 0; i < hand->cards; i++) {
        printf(" %7s     ", num_str(hand->card[i]));
    }
    printf("\n");
    for (i = 0; i < hand->cards; i++) {
        printf("      of     ");
    }
    printf("\n");
    for (i = 0; i < hand->cards; i++) {
        printf(" %9s   ", suit_str(hand->card[i]));
    }
    printf("\n");
    for (i = 0; i < hand->cards; i++) {
        printf("   -------   ");
    }
    printf("\n");
    for (i = 0; i < hand->cards; i++) {
        printf("      %d      ", i + 1);
    }
    printf("\n");
}


int sort_by_number (const void *a, const void *b)
{
    return *(uint32_t *)a - *(uint32_t *)b;
}

typedef enum {
    nothing,
    royal_flush,
    straight_flush,
    four_of_a_kind,
    full_house,
    flush,
    straight,
    three_of_a_kind,
    two_pairs,
    jack_or_better,
    total_pay_ids,
} pay_id;

typedef struct {
    pay_id id;
    char *str;
    int payout;
} paytable;

paytable pay_table[] = {
    {nothing, "nothing", 0},
    {royal_flush, "Royal Flush", 800},
    {straight_flush, "Straight Flush", 50},
    {four_of_a_kind, "Four of a Kind", 25},
    {full_house, "Full House", 9},
    {flush, "Flush", 6},
    {straight, "Straight", 4},
    {three_of_a_kind, "Three of a Kind", 3},
    {two_pairs, "Two Pairs", 2},
    {jack_or_better, "Jack or Better", 1}
};

int jack_or_better_pay_of(cards *hand)
{
    uint32_t sorted_card[CARDS_PER_HAND];
    uint32_t num_count[CARDS_PER_SUIT + 1] = {0};
    int i, j;
    int is_straight = 1;
    int is_royal = 0;
    int is_flush = 1;
    int is_jack_or_better = 0;
    //cards tmp;
    //tmp.cards = CARDS_PER_HAND;
    //tmp.card = sorted_card;
    //show_cards(&tmp);
    for (i = 0; i < 5; i++) {
        sorted_card[i] = hand->card[i];
        j = NUM(sorted_card[i]) >> 4;
        if ((num_count[j]++) && (j == 1 || j > 10)) {
            is_jack_or_better = 1;
        }
    }

    qsort(sorted_card, CARDS_PER_HAND, sizeof(sorted_card[0]),
          sort_by_number);
    qsort(num_count, CARDS_PER_SUIT + 1, sizeof(num_count[0]),
          sort_by_number);

    j = sorted_card[0] & SUITS_MASK;
    for (i = 1; i < CARDS_PER_HAND; i++) {
        if (j != (sorted_card[i] & SUITS_MASK)) {
            is_flush = 0;
            break;
        }
    }

    for (i = 4; i > 0; i--) {
        if (NUM(sorted_card[i]) - NUM(sorted_card[i - 1]) != 0x10) {
            if (i == 1) {
                if (NUM(sorted_card[0]) == 0x10 &&
                    NUM(sorted_card[1]) == 0xa0) {
                    // A, 10, J, Q, K
                    is_royal = 1;
                    break;
                }
            }
            is_straight = 0;
            break;
        }
    }
    if (is_flush) {
        if (is_royal) {
            return royal_flush;
        }
        if (is_straight) {
            return straight_flush;
        }
        return flush;
    }
    if (num_count[CARDS_PER_SUIT] == 4) {
        return four_of_a_kind;
    }
    if (num_count[CARDS_PER_SUIT] == 3) {
        if (num_count[CARDS_PER_SUIT - 1] == 2) {
            return full_house;
        }
        return three_of_a_kind;
    }
    if (is_straight) {
        return straight;
    }
    if (num_count[CARDS_PER_SUIT] == 2) {
        if (num_count[CARDS_PER_SUIT - 1] == 2) {
            return two_pairs;
        }
        if (is_jack_or_better) {
            return jack_or_better;
        }
    }
    return nothing;
}

int main (int argc, char **argv)
{
    int i, j, keep;
    pay_id payid;
    cards deck, orig, hand;
    char keeps[20];

    for (i = 1; i < total_pay_ids; i++) {
        printf("%20s %5d\n", pay_table[i].str, pay_table[i].payout);
    }
    printf("\n");

    /* initialized deck */
    srandomdev();
    deck.cards = 52;
    deck.card = malloc(CARDS_PER_DECK);
    for (i = 0; i < SUITS_PER_DECK; i++) {
        for (j = 0; j < CARDS_PER_SUIT; j++) {
            deck.card[i * CARDS_PER_SUIT + j] = ((j + 1) << 4) | i;
        }
    }

    shuffle(&deck);
    //show_cards(&deck);

    /* take 5 cards out */
    orig.cards = CARDS_PER_HAND;
    orig.card = malloc(CARDS_PER_HAND);
    for (i = 0; i < CARDS_PER_HAND; i++) {
        orig.card[i] = deck.card[deck.cards - 1 - i];
    }
    deck.cards -= 5;
    hand.cards = CARDS_PER_HAND;
    hand.card = malloc(CARDS_PER_HAND);
replay:
    for (i = 0; i < CARDS_PER_HAND; i++) {
        hand.card[i] = orig.card[i];
    }
    show_cards(&hand);

    payid = jack_or_better_pay_of(&hand);
    if (payid) {
        printf("You have %s!!!\n", pay_table[payid].str);
    }
    /* replace cards */
    printf("Cards to keep (e.g. '135'):");
    fgets(keeps, sizeof(keeps), stdin);
    keep = 0;
    for (i = 0; i < strlen(keeps); i++) {
        if (keeps[i] >= '1' && keeps[i] <= '5') {
            keep |= 1 << (keeps[i] - '1');
        }
    }
    for (i = 0; i < CARDS_PER_HAND; i++) {
        if (keep & (1 << i)) {
            printf("Keeping Card %i\n", i + 1);
        } else {
            /* hack */
            hand.card[i] = deck.card[i];
        }
    }
    show_cards(&hand);

    payid = jack_or_better_pay_of(&hand);
    printf("You have %s. ", pay_table[payid].str);
    /* calculate pay */
    printf("Your PAYOUT is $%d\n",
        pay_table[jack_or_better_pay_of(&hand)].payout);

    /* n-play simulation */
    if (argc > 1) {
        uint32_t n;
        uint64_t total_pay = 0;
        int stat[total_pay_ids] = {0};
        n = atoi(argv[1]);
        printf("\nSimulating %u hands of play using same hand...\n", n);
        for (j = 0; j < n; j++) {
            shuffle(&deck);
            for (i = 0; i < CARDS_PER_HAND; i++) {
                if (!(keep & (1 << i))) {
                    hand.card[i] = deck.card[i];
                }
            }
            payid = jack_or_better_pay_of(&hand);
            stat[payid]++;
            total_pay += pay_table[payid].payout;
        }
        printf("Statistics:\n");
        for (i = 1; i < total_pay_ids; i++) {
            if (stat[i]) {
                printf("%d %s (%1.3f%%).\n",
                   stat[i], pay_table[i].str, (float)stat[i] * 100 / n);
            }
        }
        printf("Your expected payout is $%f\n\n", (float)total_pay / n);
        goto replay;
    }
}
