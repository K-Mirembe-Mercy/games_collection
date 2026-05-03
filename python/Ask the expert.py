from tkinter import Tk, simpledialog, messagebox, ttk
import tkinter as tk
import os
import json
import random
import datetime

# ── Constants ────────────────────────────────────────────────────────────────
DATA_FILE    = 'capital_data.txt'
STATS_FILE   = 'stats.json'
WINDOW_TITLE = 'Ask the Expert – Capital Cities of the World'

# ── Global State ─────────────────────────────────────────────────────────────
the_world      = {}
score_correct  = 0
score_wrong    = 0
streak         = 0
best_streak    = 0
session_start  = datetime.datetime.now()
history        = []   # list of (country, guessed, actual, correct)

# ── File I/O ─────────────────────────────────────────────────────────────────
def read_from_file():
    if not os.path.exists(DATA_FILE):
        return
    with open(DATA_FILE, 'r') as f:
        for line in f:
            line = line.strip()
            if '/' in line:
                country, city = line.split('/', 1)
                the_world[country.strip().title()] = city.strip().title()

def write_to_file(country, city):
    with open(DATA_FILE, 'a') as f:
        f.write(country + '/' + city + '\n')

def rewrite_file():
    """Rewrite entire file (used after edits/deletes)."""
    with open(DATA_FILE, 'w') as f:
        for country, city in the_world.items():
            f.write(country + '/' + city + '\n')

def load_stats():
    global best_streak
    if os.path.exists(STATS_FILE):
        with open(STATS_FILE, 'r') as f:
            data = json.load(f)
            best_streak = data.get('best_streak', 0)

def save_stats():
    with open(STATS_FILE, 'w') as f:
        json.dump({'best_streak': best_streak}, f)

# ── Dialog Helpers ────────────────────────────────────────────────────────────
def ask(title, prompt):
    return simpledialog.askstring(title, prompt)

def info(title, msg):
    messagebox.showinfo(title, msg)

def warn(title, msg):
    messagebox.showwarning(title, msg)

def error(title, msg):
    messagebox.showerror(title, msg)

def confirm(title, msg):
    return messagebox.askyesno(title, msg)

# ── Status & Formatting ───────────────────────────────────────────────────────
def status():
    pct = accuracy()
    streak_str = f'🔥 Streak: {streak}' if streak >= 3 else ''
    return (f'\n✅ Correct: {score_correct}   ❌ Wrong: {score_wrong}'
            f'   🎯 Accuracy: {pct}%   📚 Known: {len(the_world)}'
            f'   {streak_str}')

def accuracy():
    total = score_correct + score_wrong
    return round((score_correct / total) * 100) if total > 0 else 0

def session_duration():
    delta = datetime.datetime.now() - session_start
    mins  = int(delta.total_seconds() // 60)
    secs  = int(delta.total_seconds() % 60)
    return f'{mins}m {secs}s'

def streak_message():
    if streak == 3:  return '🔥 3 in a row!'
    if streak == 5:  return '🔥🔥 5-streak – on fire!'
    if streak == 10: return '🏆 10-streak – legendary!'
    return ''

# ── Core Game Logic ───────────────────────────────────────────────────────────
def handle_known(country):
    global score_correct, score_wrong, streak, best_streak
    answer  = the_world[country]
    hint    = answer[0] + ('_' * (len(answer) - 1))   # first-letter hint
    prompt  = (f'What is the capital of {country}?\n'
               f'Hint – starts with: {hint}\n'
               f'{status()}')
    guess = ask('Your Guess', prompt)
    if guess is None:
        return
    guess_norm = guess.strip().title()
    if guess_norm == answer:
        score_correct += 1
        streak        += 1
        best_streak    = max(best_streak, streak)
        bonus          = streak_message()
        info('✅ Correct!',
             f'{answer} is right! Well done! 🎉\n{bonus}\n{status()}')
        history.append((country, guess_norm, answer, True))
    else:
        score_wrong += 1
        streak       = 0
        error('❌ Wrong!',
              f'Not quite!\nThe capital of {country} is {answer}, not {guess_norm}.\n{status()}')
        history.append((country, guess_norm, answer, False))

def handle_unknown(country):
    city = ask('Teach Me! 🎓',
               f"I don't know {country} yet!\nWhat is its capital city?")
    if city is None:
        return
    city = city.strip().title()
    if not city:
        warn('Oops', 'Capital city cannot be blank.')
        return
    the_world[country] = city
    write_to_file(country, city)
    info('Thanks! 🌍', f'Got it! {city} is the capital of {country}.\nDatabase now has {len(the_world)} countries.')

# ── Extra Modes ───────────────────────────────────────────────────────────────
def quiz_mode():
    """Pick a random country from the database and quiz the player."""
    if len(the_world) < 2:
        warn('Quiz Mode', 'Need at least 2 countries in the database to quiz!')
        return
    country = random.choice(list(the_world.keys()))
    handle_known(country)

def reverse_quiz_mode():
    """Give the capital, ask the player to name the country."""
    global score_correct, score_wrong, streak, best_streak
    if len(the_world) < 2:
        warn('Reverse Quiz', 'Need at least 2 countries to play!')
        return
    country = random.choice(list(the_world.keys()))
    capital = the_world[country]
    guess   = ask('Reverse Quiz 🔄',
                  f'Which country has {capital} as its capital?\n{status()}')
    if guess is None:
        return
    guess_norm = guess.strip().title()
    if guess_norm == country:
        score_correct += 1
        streak        += 1
        best_streak    = max(best_streak, streak)
        info('✅ Correct!', f'{country} is right! 🎉\n{streak_message()}\n{status()}')
        history.append((country, guess_norm, country, True))
    else:
        score_wrong += 1
        streak       = 0
        error('❌ Wrong!', f'The answer was {country}, not {guess_norm}.\n{status()}')
        history.append((country, guess_norm, country, False))

def multiple_choice_mode():
    """4-option multiple choice quiz."""
    global score_correct, score_wrong, streak, best_streak
    if len(the_world) < 4:
        warn('Multiple Choice', 'Need at least 4 countries in the database!')
        return
    country  = random.choice(list(the_world.keys()))
    correct  = the_world[country]
    options  = [correct]
    others   = [c for c in the_world.values() if c != correct]
    options += random.sample(others, 3)
    random.shuffle(options)
    labels   = ['A', 'B', 'C', 'D']
    choices  = '\n'.join(f'  {labels[i]}) {options[i]}' for i in range(4))
    guess    = ask('Multiple Choice 🔠',
                   f'What is the capital of {country}?\n\n{choices}\n\nType A, B, C or D:\n{status()}')
    if guess is None:
        return
    guess = guess.strip().upper()
    if guess in labels:
        chosen = options[labels.index(guess)]
        if chosen == correct:
            score_correct += 1
            streak        += 1
            best_streak    = max(best_streak, streak)
            info('✅ Correct!', f'{correct} is right! 🎉\n{streak_message()}\n{status()}')
        else:
            score_wrong += 1
            streak       = 0
            error('❌ Wrong!', f'You chose {chosen}.\nThe answer was {correct}.\n{status()}')
    else:
        warn('Invalid', 'Please enter A, B, C, or D.')

def edit_entry():
    """Let the user correct a wrong entry in the database."""
    country = ask('Edit Entry ✏️', 'Which country do you want to edit?')
    if country is None:
        return
    country = country.strip().title()
    if country not in the_world:
        warn('Not Found', f'{country} is not in the database.')
        return
    new_city = ask('Edit Entry ✏️',
                   f'Current capital of {country}: {the_world[country]}\nEnter the correct capital:')
    if new_city is None:
        return
    new_city = new_city.strip().title()
    if not new_city:
        warn('Oops', 'Capital cannot be blank.')
        return
    old = the_world[country]
    the_world[country] = new_city
    rewrite_file()
    info('Updated ✅', f'{country}: {old} → {new_city}')

def delete_entry():
    """Remove a country from the database."""
    country = ask('Delete Entry 🗑️', 'Which country do you want to delete?')
    if country is None:
        return
    country = country.strip().title()
    if country not in the_world:
        warn('Not Found', f'{country} is not in the database.')
        return
    if confirm('Confirm Delete', f'Delete {country} ({the_world[country]}) from the database?'):
        del the_world[country]
        rewrite_file()
        info('Deleted 🗑️', f'{country} has been removed. Database now has {len(the_world)} countries.')

def show_history():
    """Show last 10 questions answered this session."""
    if not history:
        info('History 📜', 'No questions answered yet this session.')
        return
    lines = []
    for country, guessed, actual, correct in history[-10:]:
        icon = '✅' if correct else '❌'
        lines.append(f'{icon} {country}: guessed {guessed}, answer {actual}')
    info('Last 10 Answers 📜', '\n'.join(lines))

def show_all_countries():
    """List all countries in the database."""
    if not the_world:
        info('Database 📚', 'No countries in the database yet.')
        return
    lines = [f'{c}: {the_world[c]}' for c in sorted(the_world)]
    chunks = [lines[i:i+15] for i in range(0, len(lines), 15)]
    for i, chunk in enumerate(chunks):
        info(f'Database 📚 (page {i+1}/{len(chunks)})', '\n'.join(chunk))

def show_summary():
    info('Session Summary 📊',
         f'Duration:      {session_duration()}\n'
         f'✅ Correct:    {score_correct}\n'
         f'❌ Wrong:      {score_wrong}\n'
         f'🎯 Accuracy:   {accuracy()}%\n'
         f'🔥 Best Streak:{best_streak}\n'
         f'📚 DB Size:    {len(the_world)} countries')

# ── Main Menu ─────────────────────────────────────────────────────────────────
def main_menu():
    return ask('Main Menu 🌍',
               'What would you like to do?\n\n'
               '  1) Look up / guess a country\n'
               '  2) Random quiz\n'
               '  3) Reverse quiz (capital → country)\n'
               '  4) Multiple choice\n'
               '  5) Edit an entry\n'
               '  6) Delete an entry\n'
               '  7) View answer history\n'
               '  8) View all countries\n'
               '  9) Session summary\n'
               '  0) Quit\n')

# ── Entry Point ───────────────────────────────────────────────────────────────
def main():
    print(WINDOW_TITLE)
    root = Tk()
    root.withdraw()
    read_from_file()
    load_stats()

    while True:
        choice = main_menu()
        if choice is None or choice.strip() == '0':
            save_stats()
            show_summary()
            info('Goodbye 👋', 'Thanks for playing! See you next time.')
            break
        elif choice.strip() == '1':
            query = ask('Country', f'Enter a country name:{status()}')
            if query is None: continue
            query = query.strip().title()
            if not query: warn('Oops', 'Please enter a country name.'); continue
            if query in the_world: handle_known(query)
            else:                  handle_unknown(query)
        elif choice.strip() == '2': quiz_mode()
        elif choice.strip() == '3': reverse_quiz_mode()
        elif choice.strip() == '4': multiple_choice_mode()
        elif choice.strip() == '5': edit_entry()
        elif choice.strip() == '6': delete_entry()
        elif choice.strip() == '7': show_history()
        elif choice.strip() == '8': show_all_countries()
        elif choice.strip() == '9': show_summary()
        else: warn('Invalid', 'Please enter a number from 0 to 9.')

    root.mainloop()

if __name__ == '__main__':
    main()
