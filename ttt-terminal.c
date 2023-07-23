/*
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "ttt.h"

TTTState game(void);

int main(void)
{
  char again;

  do {
    game();
    do {
      printf("Do you want to play again?[y/n] ");
      again = tolower(getchar());
      if (again == '\n') {
        again = tolower(getchar());
      }
    } while (again != 'y' && again != 'n');
  } while (again == 'y');

  return 0;
}

int read_game_input(uint8_t *row, uint8_t *col, uint8_t *resign)
{
  int ch;

  *resign = 0;
  *row = *col = 0;

  ch = getchar();
  if (ch == '\n') {
    ch = getchar();
  }
  if (ch == 'q') {
    *resign = 1;
    return 0;
  }
  while (isdigit(ch)) {
    *row = (*row) * 10 + (ch - '0');
    ch = getchar();
  }
  if (ch != ',') {
    while (ch != '\n') ch = getchar();
    return 1;
  }
  ch = getchar();
  while (isdigit(ch)) {
    *col = (*col) * 10 + (ch - '0');
    ch = getchar();
  }
  if (ch != '\n') {
    while (ch != '\n') ch = getchar();
    return 1;
  }
  return 0;
}

TTTState game(void)
{
  TTT ttt;
  TTTState state;
  uint8_t row, col, resign;

  ttt_init(&ttt);
  state = ttt_eval_state(&ttt, NULL);
  do {
    printf("\nTic Tac Toe\n===========\n\n");
    print_board(&ttt);
    printf("\nPlayer %d moves.\n\n(row,col)=>", ttt.side_to_play + 1);
    while (read_game_input(&row, &col, &resign)) {
      printf("ERROR: Invalid coordinates.\n(row,col)=>");
    }
    if (resign) {
      state = TTT_STATE_PLAYER2_WINS;
      break;
    }

    if (ttt_place_token(&ttt, row-1, col-1)) {
      printf("You cannot play: (%d,%d)\n\n", row, col);
    } else {
      state = ttt_eval_state(&ttt, NULL);
      if (state == TTT_STATE_CONTINUE) {
        ai_play(&ttt);
        state = ttt_eval_state(&ttt, NULL);
      }
    }
  } while (state == TTT_STATE_CONTINUE);
  printf("\nTic Tac Toe\n===========\n\n");
  print_board(&ttt);printf("\n");

  switch (state) {
  case TTT_STATE_DRAW:
    printf("Draw.\n");
    break;
  case TTT_STATE_PLAYER1_WINS:
    printf("You won.\n");
    break;
  case TTT_STATE_PLAYER2_WINS:
    printf("You lost.\n");
    break;
  case TTT_STATE_CONTINUE:
    printf("Why are you giving up?\n");
    break;
  default:
    printf("Unknown game state.\n");
    abort();
  }

  return state;
}
