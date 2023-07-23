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

#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <ctype.h>

#ifdef TTT_TESTS
#include <assert.h>
#endif

const char AVAILABLE_PLAYER_TOKENS[] = {'O', 'X'};
const char EMPTY_TOKEN = ' ';

char *rand_player_tokens()
{
  static char toks[2];
  toks[0] = AVAILABLE_PLAYER_TOKENS[0];
  toks[1] = AVAILABLE_PLAYER_TOKENS[1];
  return toks;
}

#define BOARD_WIDTH 3
#define BOARD_HEIGHT 3
#define BOARD_MIN_SIDE_SZ (BOARD_WIDTH < BOARD_HEIGHT ? BOARD_WIDTH : BOARD_HEIGHT)
#define BOARD_MAX_SIDE_SZ (BOARD_WIDTH > BOARD_HEIGHT ? BOARD_WIDTH : BOARD_HEIGHT)

typedef char TTTBoard[BOARD_HEIGHT][BOARD_WIDTH];

struct ttt
{
  char PLAYER1_TOKEN;
  char PLAYER2_TOKEN;
  uint8_t side_to_play;
  uint8_t board[BOARD_HEIGHT][BOARD_WIDTH];
};
typedef struct ttt TTT;

TTT *ttt_init(TTT *ttt)
{
  int i, j;
  char *toks;

  toks = rand_player_tokens();
  ttt->PLAYER1_TOKEN = toks[0];
  ttt->PLAYER2_TOKEN = toks[1];
  ttt->side_to_play = 0;
  for (i = 0; i < BOARD_HEIGHT; i++) {
    for (j = 0; j < BOARD_WIDTH; j++) {
      ttt->board[i][j] = EMPTY_TOKEN;
    }
  }

  return ttt;
}

TTT *ttt_clone(TTT *src, TTT *dest)
{
  *dest = *src;
  return dest;
}

int ttt_place_token(TTT *ttt, uint8_t row, uint8_t col)
{
  if (row >= BOARD_HEIGHT || col >= BOARD_WIDTH) {
    return 1;
  }
  if (ttt->board[row][col] != EMPTY_TOKEN) {
    return 1;
  }
  ttt->board[row][col] = ttt->side_to_play
    ? ttt->PLAYER2_TOKEN : ttt->PLAYER1_TOKEN;
  ttt->side_to_play = ~ttt->side_to_play + 2;
  return 0;
}

enum ttt_state {
  TTT_STATE_CONTINUE = 0,
  TTT_STATE_PLAYER1_WINS = 1 << 0,
  TTT_STATE_PLAYER2_WINS = 1 << 1,
  TTT_STATE_DRAW = 1 << 2
};
typedef enum ttt_state TTTState;

TTTState ttt_eval_state(TTT *ttt, uint8_t *win_pos)
{
  int i, j;
  uint8_t count;
  uint8_t empty_spaces;

  empty_spaces = 0;
  for (i = 0; i < BOARD_HEIGHT; i++) {
    for (j = 0; j < BOARD_WIDTH; j++) {
      if (ttt->board[i][j] == EMPTY_TOKEN) {
        empty_spaces = 1;
      }
    }
  }

  for (i = 0; i < BOARD_HEIGHT; i++) {
    if (win_pos != NULL) {
      win_pos[0] = i;
      win_pos[1] = 0;
    }
    count = 1;
    for (j = 1; j < BOARD_WIDTH; j++) {
      if (ttt->board[i][j] == ttt->board[i][0]) {
        if (win_pos != NULL) {
          win_pos[count*2+0] = i;
          win_pos[count*2+1] = j;
        }
        ++count;
        if (count == BOARD_WIDTH && ttt->board[i][0] != EMPTY_TOKEN) {
          return (ttt->board[i][0] == ttt->PLAYER1_TOKEN)
            ? TTT_STATE_PLAYER1_WINS
            : TTT_STATE_PLAYER2_WINS;
        }
      }
    }
  }

  for (j = 0; j < BOARD_WIDTH; j++) {
    if (win_pos != NULL) {
      win_pos[0] = 0;
      win_pos[1] = j;
    }
    count = 1;
    for (i = 1; i < BOARD_HEIGHT; i++) {
      if (ttt->board[i][j] == ttt->board[0][j]) {
        if (win_pos != NULL) {
          win_pos[count*2+0] = i;
          win_pos[count*2+1] = j;
        }
        ++count;
        if (count == BOARD_WIDTH && ttt->board[0][j] != EMPTY_TOKEN) {
          return (ttt->board[0][j] == ttt->PLAYER1_TOKEN)
            ? TTT_STATE_PLAYER1_WINS
            : TTT_STATE_PLAYER2_WINS;
        }
      }
    }
  }

  if (win_pos != NULL) {
    win_pos[0] = 0;
    win_pos[1] = 0;
  }
  count = 1;
  for (i = 1, j = 1; i < BOARD_HEIGHT || j < BOARD_WIDTH; i++, j++) {
    if (ttt->board[i][j] == ttt->board[0][0]) {
      if (win_pos != NULL) {
        win_pos[count*2+0] = i;
        win_pos[count*2+1] = j;
      }
      ++count;
      if (count == BOARD_MIN_SIDE_SZ && ttt->board[0][0] != EMPTY_TOKEN) {
        return (ttt->board[i][j] == ttt->PLAYER1_TOKEN)
          ? TTT_STATE_PLAYER1_WINS
          : TTT_STATE_PLAYER2_WINS;
      }
    }
  }

  if (win_pos != NULL) {
    win_pos[0] = 0;
    win_pos[1] = BOARD_WIDTH - 1;
  }
  count = 1;
  for (i = 1, j = BOARD_WIDTH - 2; i < BOARD_HEIGHT || j >= 0; i++, j--) {
    if (ttt->board[i][j] == ttt->board[0][BOARD_WIDTH - 1]) {
      if (win_pos != NULL) {
        win_pos[count*2+0] = i;
        win_pos[count*2+1] = j;
      }
      ++count;
      if (count == BOARD_MIN_SIDE_SZ && ttt->board[0][BOARD_WIDTH - 1]
          != EMPTY_TOKEN) {
        return (ttt->board[i][j] == ttt->PLAYER1_TOKEN)
          ? TTT_STATE_PLAYER1_WINS
          : TTT_STATE_PLAYER2_WINS;
      }
    }
  }

  return empty_spaces ? TTT_STATE_CONTINUE : TTT_STATE_DRAW;
}

int ai_outcome_cmp(TTT *ttt, int outcome1, int outcome2)
{
  int favourable_outcome, unfavourable_outcome;

  favourable_outcome = ttt->side_to_play ? TTT_STATE_PLAYER2_WINS
    : TTT_STATE_PLAYER1_WINS;
  unfavourable_outcome = ttt->side_to_play ? TTT_STATE_PLAYER1_WINS
    : TTT_STATE_PLAYER2_WINS;

  if (!(outcome1 & unfavourable_outcome) && (outcome2 & unfavourable_outcome)) {
    return 1;
  }
  if ((outcome1 & unfavourable_outcome) && !(outcome2 & unfavourable_outcome)) {
    return -1;
  }
  if ((outcome1 & favourable_outcome) && !(outcome2 & favourable_outcome)) {
    return 1;
  }
  if (!(outcome1 & favourable_outcome) && (outcome2 & favourable_outcome)) {
    return -1;
  }

  return 0;
}

TTTState ttt_eval_position(TTT *ttt, uint8_t *out_best_row, uint8_t *out_best_col)
{
  uint8_t i, j;
  TTTState state;
  TTTState possible_states;

  possible_states = 0;
  state = ttt_eval_state(ttt, NULL);
  if (state != TTT_STATE_CONTINUE) {
    return state;
  }

  for (i = 0; i < BOARD_HEIGHT; i++) {
    for (j = 0; j < BOARD_WIDTH; j++) {
      if (ttt->board[i][j] == EMPTY_TOKEN) {
        TTTState eval;
        TTT ttt_copy;
        ttt_clone(ttt, &ttt_copy);
        ttt_place_token(&ttt_copy, i, j);
        eval = ttt_eval_position(&ttt_copy, NULL, NULL);
        if (!possible_states
            || (ai_outcome_cmp(ttt, eval, possible_states)) > 0) {
          possible_states = eval;
          if (out_best_row != NULL && out_best_col != NULL) {
            *out_best_row = i;
            *out_best_col = j;
          }
        }
      }
    }
  }

  return possible_states;
}

void ai_play(TTT *ttt)
{
  uint8_t row, col;
  ttt_eval_position(ttt, &row, &col);
  /* printf("AI plays: %d, %d\n\n", row+1, col+1); */
  ttt_place_token(ttt, row, col);
}

void print_board(TTT *ttt)
{
  int i, j;
  for (i = 0; i < BOARD_HEIGHT; i++) {
    for (j = 0; j < BOARD_WIDTH; j++) {
      printf((j > 0) ? " | %c" : " %c", ttt->board[i][j]);
    }
    printf("\n");
  }
}

#ifdef TTT_TESTS

void test_ttt_state_horz(uint8_t row)
{
  int j;
  TTTState state;
  uint8_t win_pos[BOARD_MAX_SIDE_SZ][2];
  TTT ttt;
  ttt_init(&ttt);
  for (j = 0; j < BOARD_WIDTH; j++) {
    ttt.board[row][j] = ttt.PLAYER2_TOKEN;
  }
  print_board(&ttt);printf("\n");
  state = ttt_eval_state(&ttt, &win_pos[0][0]);
  assert(state == TTT_STATE_PLAYER2_WINS);
  for (j = 0; j < BOARD_WIDTH; j++) {
    assert(win_pos[j][0] == row);
    assert(win_pos[j][1] == j);
  }
}

void test_ttt_state_vert(uint8_t col)
{
  int i;
  TTTState state;
  uint8_t win_pos[BOARD_MAX_SIDE_SZ][2];
  TTT ttt;
  ttt_init(&ttt);
  for (i = 0; i < BOARD_HEIGHT; i++) {
    ttt.board[i][col] = ttt.PLAYER2_TOKEN;
  }
  print_board(&ttt);printf("\n");
  state = ttt_eval_state(&ttt, &win_pos[0][0]);
  assert(state == TTT_STATE_PLAYER2_WINS);
  for (i = 0; i < BOARD_HEIGHT; i++) {
    assert(win_pos[i][0] == i);
    assert(win_pos[i][1] == col);
  }
}

void test_ttt_state_diag1()
{
  int i, j;
  TTTState state;
  uint8_t win_pos[BOARD_MAX_SIDE_SZ][2];
  TTT ttt;
  ttt_init(&ttt);
  for (i = 0, j = 0; i < BOARD_HEIGHT && j < BOARD_WIDTH; i++, j++) {
    ttt.board[i][j] = ttt.PLAYER2_TOKEN;
  }
  print_board(&ttt);printf("\n");
  state = ttt_eval_state(&ttt, &win_pos[0][0]);
  assert(state == TTT_STATE_PLAYER2_WINS);
  for (i = 0, j = 0; i < BOARD_HEIGHT && j < BOARD_WIDTH; i++, j++) {
    assert(win_pos[i][0] == i);
    assert(win_pos[i][1] == j);
  }
}

void test_ttt_state_diag2()
{
  int i, j;
  TTTState state;
  uint8_t win_pos[BOARD_MAX_SIDE_SZ][2];
  TTT ttt;
  ttt_init(&ttt);
  for (i = 0, j = BOARD_WIDTH - 1; i < BOARD_HEIGHT && j >= 0; i++, j--) {
    ttt.board[i][j] = ttt.PLAYER2_TOKEN;
  }
  print_board(&ttt);printf("\n");
  state = ttt_eval_state(&ttt, &win_pos[0][0]);
  assert(state == TTT_STATE_PLAYER2_WINS);
  for (i = 0, j = BOARD_WIDTH - 1; i < BOARD_HEIGHT && j >= 0; i++, j--) {
    assert(win_pos[i][0] == i);
    assert(win_pos[i][1] == j);
  }
}

void test_ttt_place_token()
{
  TTT ttt;
  ttt_init(&ttt);
  ttt_place_token(&ttt, 1, 2);
  print_board(&ttt);printf("\n");
  assert(ttt.board[1][2] == ttt.PLAYER1_TOKEN);
  assert(ttt.side_to_play == 1);
}

void test_ttt_clone()
{
  TTT ttt, ttt_copy;
  ttt_init(&ttt);
  ttt_place_token(&ttt, 2, 2);
  ttt_clone(&ttt, &ttt_copy);
  print_board(&ttt);printf("\n");
  print_board(&ttt_copy);printf("\n");
  assert(ttt.board[2][2] == ttt.PLAYER1_TOKEN);
  assert(ttt_copy.board[2][2] == ttt.PLAYER1_TOKEN);
  assert(ttt_copy.side_to_play == ttt.side_to_play);
}

void run_tests()
{
  test_ttt_state_horz(0);
  test_ttt_state_horz(1);
  test_ttt_state_horz(2);
  test_ttt_state_vert(0);
  test_ttt_state_vert(1);
  test_ttt_state_vert(2);
  test_ttt_state_diag1();
  test_ttt_state_diag2();
  test_ttt_place_token();
  test_ttt_clone();
}

#endif /* TTT_TESTS */
