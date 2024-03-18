int main() {
  /*NCURSES START */
  initscr();
  noecho();
  cbreak();

  // get screen size
  int yMax, xMax;
  getmaxyx(stdscr, yMax, xMax);

  // Create window for password prompt
  WINDOW *passwordwin = newwin(3, xMax - 10, yMax / 2 - 2, 5);
  box(passwordwin, 0, 0);
  mvwprintw(passwordwin, 1, 1, "Enter password:");
  wrefresh(passwordwin);

  // Get password input
  char password[50]; // Adjust size as needed
  mvwgetnstr(passwordwin, 1, 17, password, 50);

  // Clear password prompt window
  werase(passwordwin);
  wrefresh(passwordwin);
  delwin(passwordwin);

  // create window for our input
  curs_set(0);
  WINDOW *menuwin = newwin(8, xMax - 12, yMax / 2 - 4, 5);
  box(menuwin, 0, 0);
  refresh();
  wrefresh(menuwin);

  keypad(menuwin, true);
  vector<string> choices = {
      "Show credentials",   "Add credentials",          "Edit credentials",
      "Delete credentials", "Change database password", "Generate password",
      "Backup database",    "Erase database",           "Exit"};
  int choice;
  int highlight = 0;

  while (1) {
    int row = 1;
    int col = 1;
    for (int i = 0; i < static_cast<int>(choices.size()); i++) {
      if (i == highlight) {
        wattron(menuwin, A_REVERSE);
      }
      mvwprintw(menuwin, row, col, "[%d] %s", i + 1, choices[i].c_str());
      wattroff(menuwin, A_REVERSE);
      row++;
      if (row > 3) {
        row = 1;
        col += xMax / 3;
      }
    }
    choice = wgetch(menuwin);

    switch (choice) {
    case KEY_UP:
      highlight--;
      if (highlight == -1)
        highlight = static_cast<int>(choices.size()) - 1;
      break;
    case KEY_DOWN:
      highlight++;
      if (highlight == static_cast<int>(choices.size()))
        highlight = 0;
      break;
    case KEY_LEFT:
      highlight -= 3;
      if (highlight < 0)
        highlight += static_cast<int>(choices.size());
      break;
    case KEY_RIGHT:
      highlight += 3;
      if (highlight >= static_cast<int>(choices.size()))
        highlight -= static_cast<int>(choices.size());
      break;
    default:
      // Check if the pressed key is a number shortcut
      if (choice >= '1' && choice <= '9') {
        int index = choice - '1';
        if (index < static_cast<int>(choices.size())) {
          highlight = index;
        }
      }
      break;
    }
    if (choice == 10)
      break;
  }
  getch();
  endwin();
  return 0;
}
