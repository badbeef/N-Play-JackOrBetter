import random, sys
from collections import Counter, OrderedDict

suits = ('Spade', 'Heart', 'Club', 'Diamond')
numbers = range(1,14)
labels = {1: 'Ace', 2: '2', 3: '3', 4: '4', 5: '5', 6: '6', 7: '7', 8: '8',
          9: '9', 10: '10', 11: 'Jack', 12: 'Queen', 13: 'King'}

pay_table = {
    'Royal Flush' : 800,
    'Straight Flush' : 50,
    'Four of a Kind' : 25,
    'Full House' : 9,
    'Flush' : 6,
    'Straight' : 4,
    'Three of a Kind' : 3,
    'Two Pairs' : 2,
    'Jack or Better' : 1,
    'Nothing' : 0}

# cards = []
# for suit in suits:
#    for number in numbers:
#        cards.append((suit, number))
deck = [(number, suit) for suit in suits for number in numbers]

def jack_or_better_pay_of(cards):
    same_nums = Counter()
    same_suits = Counter()
    for (n, s) in cards:
        same_nums[n] += 1
        same_suits[s] += 1
    same_cards = max(same_nums.values())
    max_suits = max(same_suits.values())
    sorted_cards = sorted(cards)
	# e.g. 4, 5, 6, 7, 8
    straight = True
    royal = False
    for i in [4, 3, 2, 1]:
        if sorted_cards[i][0] - sorted_cards[i - 1][0] != 1:
            if i == 1:
                if sorted_cards[0][0] == 1 and sorted_cards[1][0] == 10:
                    royal = True
                    break
            straight = False
            break
    # Straight Flush
    if (max_suits == 5 and straight):
        if royal:
            return 'Royal Flush'
        else:
            return 'Straight Flush'
    # Four of a Kind
    if (same_cards == 4):
        return 'Four of a Kind'
    # Full House
    if (same_cards == 3 and len(same_nums) == 2):
        return 'Full House'
    # Flush
    if (max_suits == 5):
        return 'Flush'
    if straight:
        return 'Straight'
    # Three of a Kind
    if (same_cards == 3):
        return 'Three of a Kind'
    # Two Pairs
    pairs = 0
    jack_or_better = False
    for n in dict(same_nums):
        if same_nums[n] == 2:
            pairs += 1
            if (n == 1 or n > 10):
                jack_or_better = True
    if (pairs == 2):
        return 'Two Pairs'
    # Jack or Better
    if jack_or_better:
        return 'Jack or Better'

    return 'Nothing'

def show_cards(cards):
    for card in cards:
        print '%s' % labels[card[0]].center(7),
    print '\n' + ('%s ' % 'of'.center(7)) * len(cards)
    for card in cards:
        print '%s' % card[1].center(7),
    print '\n' + (('-' * 5).center(7) + ' ') * len(cards)
    for i in range(len(cards)):
        print '%s' % str(i + 1).center(7),
    print '\n'

def replace_cards(cards, deck, keeps):
    for i in range(5):
        if (i not in keeps):
            cards[i] = deck[i]  # easier than deck.pop()


## main program

random.shuffle(deck)

for p, i in Counter(pay_table).most_common():
    if i == 0:
        print
    else:
        print p.ljust(15), i

cards = []
for i in range(5):
    cards.append(deck.pop())

show_cards(cards)
pay = jack_or_better_pay_of(cards)
if pay != 'Nothing':
    print "Your have %s!!!" % pay
# r = '1, 2 , 4'
while True:
    try:
        r = raw_input('Cards you want to KEEP (e.g. 1,3,5 or 135): ')
        keeps = r.replace(',','').replace(' ','')
        keeps = map(int, keeps)
        keeps = map((-1).__add__, keeps)
    except KeyboardInterrupt:
        quit()
    except:
        print 'Invalid input "%s"' % r
        continue
    break
for i in keeps:
    print 'Keeping card %i' % (i + 1)
backup = deck[:]
replace_cards(cards, deck, keeps)
show_cards(cards)
payout = jack_or_better_pay_of(cards)
print 'You got %s.\nYour payout is %d' % (payout, pay_table[payout])

netpay = 0
if len(sys.argv) == 2:
    trials = int(sys.argv[1])
else:
    trials = 10000
print 'Simulating %i hands of play using same hand...' % trials
stats = OrderedDict(sorted(pay_table.items(), key=lambda x: x[1],
                    reverse=True))
for i in stats:
    stats[i] = 0
for i in xrange(trials):
    random.shuffle(backup)
    replace_cards(cards, backup, keeps)
    pay = jack_or_better_pay_of(cards)
    stats[pay] += 1
    netpay += pay_table[pay]
print 'Statistics:'
for i in stats:
    if stats[i] and i != 'Nothing':
        print '%i %s (%f)' % (stats[i], i, float(stats[i]) / trials)
print 'Your expected payout is $%f' % (float(netpay) / trials)
