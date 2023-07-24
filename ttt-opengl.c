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

#include <glad/glad.h>
#include <GL/freeglut.h>
#include <cglm/cglm.h>
#include <stdlib.h>
#include <stdio.h>
#include <ttt.h>

#define TOKEN_O_NUM_LINES 32

vec4 BOARD_COLOR = {0.19f, 0.2f, 0.22f, 1.0f};
vec4 BOARD_LINE_COLOR = {0.5f, 0.5f, 0.8f, 1.0f};
vec4 TOKEN_COLOR = {0.9f, 0.9f, 0.9f, 1.0f};
vec4 TOKEN_WIN_COLOR = {0.3f, 0.3f, 1.0f, 1.0f};
vec4 TOKEN_DRAW_COLOR = {0.4f, 0.4f, 0.4f, 1.0f};
float TOKEN_WIDTH = 0.135;

int SCREEN_WIDTH = 480;
int SCREEN_HEIGHT = 480;
int LINE_WIDTH = 5;

struct ttt_gl_board
{
  unsigned int VAO;
  unsigned int VBO;
};
typedef struct ttt_gl_board TttGlBoard;

struct ttt_gl_token
{
  unsigned int VAO;
  unsigned int VBO;
};
typedef struct ttt_gl_token TttGlToken;

struct ttt_gl_renderer
{
  unsigned int program;
  TttGlBoard board;
  TttGlToken token_x;
  TttGlToken token_o;
  mat4 view;
  mat4 projection;
};
typedef struct ttt_gl_renderer TttGlRenderer;

TttGlRenderer renderer;

void gfx_init();
void gfx_loop();
void setup_renderer();
void render();
void win_reshape_handler(int width, int height);
void mouse_handler(int button, int state, int x, int y);
void keyboard_handler(unsigned char key, int x, int y);

TTT ttt;

int main(int argc, char *argv[])
{
  ttt_init(&ttt);
  gfx_init(argc, argv);
  gfx_loop();
  return 0;
}

void gfx_init(int argc, char *argv[])
{
  glutInit(&argc, argv);
  glutInitContextVersion(2, 1);

  glutInitDisplayMode(GLUT_DOUBLE);
  glutInitWindowSize(SCREEN_WIDTH, SCREEN_HEIGHT);
  glutCreateWindow("Canole Tic Tac Toe");

  if (!gladLoadGLLoader((GLADloadproc)glutGetProcAddress)) {
    fprintf(stderr, "Failed to initialize GLAD");
    exit(EXIT_FAILURE);
  }  

  /* glut calls reshapeFunc on init, so glViewport(0, 0, SCREEN_WIDTH,
     SCREEN_HEIGHT) here is not necessary */
  glutReshapeFunc(win_reshape_handler);

  setup_renderer();
  glutDisplayFunc(render);
  glutKeyboardFunc(keyboard_handler);
  glutMouseFunc(mouse_handler);
}

void gfx_loop()
{
  render();
  glutMainLoop();
}

const char *vertex_shader_source = "#version 330 core\n"
  "layout (location = 0) in vec3 aPos;\n"
  "uniform mat4 model, view, projection;\n"
  "void main()\n"
  "{\n"
  "   gl_Position = projection * view * model * vec4(aPos, 1.0);\n"
  "}\n";

const char *fragment_shader_source = "#version 330 core\n"
  "uniform vec4 color;\n"
  "out vec4 frag_color;\n"
  "void main()\n"
  "{\n"
  /* "  frag_color = vec4(1.0f, 0.5f, 0.2f, 1.0f);\n" */
  "  frag_color = vec4(color);\n"
  "}\n"; 

void setup_board()
{
  float board_vertices[] = {
    -0.2f, -0.6f, 0.0f,
    -0.2f,  0.6f, 0.0f,
     0.2f, -0.6f, 0.0f,
     0.2f,  0.6f, 0.0f,
    -0.6f, -0.2f, 0.0f,
     0.6f, -0.2f, 0.0f,
    -0.6f,  0.2f, 0.0f,
     0.6f,  0.2f, 0.0f
  };

  glGenVertexArrays(1, &renderer.board.VAO);
  glBindVertexArray(renderer.board.VAO);
  glGenBuffers(1, &renderer.board.VBO);
  glBindBuffer(GL_ARRAY_BUFFER, renderer.board.VBO);
  glBufferData(GL_ARRAY_BUFFER, sizeof(board_vertices), board_vertices, GL_STATIC_DRAW);
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
  glEnableVertexAttribArray(0); 
  glBindBuffer(GL_ARRAY_BUFFER, 0);
  glBindVertexArray(0);
}

void build_circle(float radius, int v_count, float *points)
{
  int i, point_idx;
  float angle;
  angle = 2.0f * GLM_PI / v_count;

  point_idx = 0;
  for (i = 0; i < v_count; i++) {
    float current_angle;
    float x;
    float y;
    current_angle = angle * i;
    x = radius * cos(current_angle);
    y = radius * sin(current_angle);

    points[point_idx++] = x;
    points[point_idx++] = y;
    points[point_idx++] = 0.0f;
    if (i) {
      points[point_idx++] = x;
      points[point_idx++] = y;
      points[point_idx++] = 0.0f;
    }
  }
  points[point_idx++] = points[0];
  points[point_idx++] = points[1];
  points[point_idx++] = points[2];
}

void setup_token_o()
{
  float token_vertices[TOKEN_O_NUM_LINES * 2 * 3]; // 16 lines * 2 points/line * 3 (vec size)
  build_circle(TOKEN_WIDTH, TOKEN_O_NUM_LINES, token_vertices);
  /* for (int i = 0; i < TOKEN_O_NUM_LINES * 2 * 3; i++) */
    /* printf(((i+1)%3) ? "%.2f," : "%.2f\n", token_vertices[i]); */

  glGenVertexArrays(1, &renderer.token_o.VAO);
  glBindVertexArray(renderer.token_o.VAO);
  glGenBuffers(1, &renderer.token_o.VBO);
  glBindBuffer(GL_ARRAY_BUFFER, renderer.token_o.VBO);
  glBufferData(GL_ARRAY_BUFFER, sizeof(token_vertices), token_vertices, GL_STATIC_DRAW);
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
  glEnableVertexAttribArray(0); 
  glBindBuffer(GL_ARRAY_BUFFER, 0);
  glBindVertexArray(0);
}

void setup_token_x()
{
  float token_vertices[] = {
    -TOKEN_WIDTH, TOKEN_WIDTH, 0.0f,
     TOKEN_WIDTH, -TOKEN_WIDTH, 0.0f,
     TOKEN_WIDTH, TOKEN_WIDTH, 0.0f,
     -TOKEN_WIDTH, -TOKEN_WIDTH, 0.0f
  };

  glGenVertexArrays(1, &renderer.token_x.VAO);
  glBindVertexArray(renderer.token_x.VAO);
  glGenBuffers(1, &renderer.token_x.VBO);
  glBindBuffer(GL_ARRAY_BUFFER, renderer.token_x.VBO);
  glBufferData(GL_ARRAY_BUFFER, sizeof(token_vertices), token_vertices, GL_STATIC_DRAW);
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
  glEnableVertexAttribArray(0); 
  glBindBuffer(GL_ARRAY_BUFFER, 0);
  glBindVertexArray(0);
}

void renderer_recalc_projection()
{
  glm_mat4_identity(renderer.projection);
  glm_perspective(glm_rad(45.0f), (float)SCREEN_WIDTH / (float)SCREEN_HEIGHT, 0.1f, 100.0f, renderer.projection);
}

void setup_renderer()
{
  unsigned int vertex_shader;
  unsigned int fragment_shader;
  int  ret;
  char log[512];

  vertex_shader = glCreateShader(GL_VERTEX_SHADER);
  glShaderSource(vertex_shader, 1, &vertex_shader_source, NULL);
  glCompileShader(vertex_shader);
  glGetShaderiv(vertex_shader, GL_COMPILE_STATUS, &ret);
  if (!ret) {
    glGetShaderInfoLog(vertex_shader, 512, NULL, log);
    fprintf(stderr, "ERROR: vertex shader compilation failed. Reason: %s\n", log);
  }

  fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
  glShaderSource(fragment_shader, 1, &fragment_shader_source, NULL);
  glCompileShader(fragment_shader);
  if (!ret) {
    glGetShaderInfoLog(fragment_shader, 512, NULL, log);
    fprintf(stderr, "ERROR: fragment shader compilation failed. Reason: %s\n", log);
  }

  renderer.program = glCreateProgram();
  glAttachShader(renderer.program, vertex_shader);
  glAttachShader(renderer.program, fragment_shader);
  glLinkProgram(renderer.program);
  glGetProgramiv(renderer.program, GL_LINK_STATUS, &ret);
  if (!ret) {
    glGetProgramInfoLog(renderer.program, 512, NULL, log);
    fprintf(stderr, "ERROR: shader program compilation failed. Reason: %s\n", log);
  }
  glDeleteShader(vertex_shader);
  glDeleteShader(fragment_shader); 

  glLineWidth(LINE_WIDTH);

  renderer_recalc_projection();

  setup_board();
  setup_token_x();
  setup_token_o();
}

void render_token(uint8_t row, uint8_t col, char type, float *color)
{
  mat4 model = GLM_MAT4_IDENTITY_INIT;

  glm_translate(model, (vec3){(col - 1) * 0.4, (1 - row) * 0.4, 0.0f});
  glUniformMatrix4fv(glGetUniformLocation(renderer.program, "model"), 1, GL_FALSE, &model[0][0]);
  glUniform4fv(glGetUniformLocation(renderer.program, "color"), 1, color);

  glUseProgram(renderer.program);
  if (type == 'X') {
    glBindVertexArray(renderer.token_x.VAO);
    glDrawArrays(GL_LINES, 0, 8);
  } else {
    glBindVertexArray(renderer.token_o.VAO);
    glDrawArrays(GL_LINES, 0, TOKEN_O_NUM_LINES * 2);
  }
  glBindVertexArray(0);
}

void render_tokens()
{
  const static int winning_state
    = (TTT_STATE_PLAYER1_WINS | TTT_STATE_PLAYER2_WINS);

  int i, j, tok_i, is_winning_tok;
  TTTState state;
  uint8_t win_pos[BOARD_MAX_SIDE_SZ][2];

  state = ttt_eval_state(&ttt, &win_pos[0][0]);

  tok_i = 0;
  for (i = 0; i < BOARD_HEIGHT; i++) {
    for (j = 0; j < BOARD_WIDTH; j++) {
      if (ttt.board[i][j] != EMPTY_TOKEN) {
        is_winning_tok = ((state & winning_state) &&
                          (win_pos[tok_i][0] == i && win_pos[tok_i][1] == j));
        if (is_winning_tok) {
          ++tok_i;
          glLineWidth(LINE_WIDTH + 10);
          render_token(i, j, ttt.board[i][j], TOKEN_WIN_COLOR);
          glLineWidth(LINE_WIDTH);
        } else if (state == TTT_STATE_DRAW) {
          render_token(i, j, ttt.board[i][j], TOKEN_DRAW_COLOR);
        } else {
          render_token(i, j, ttt.board[i][j], TOKEN_COLOR);
        }
      }
    }
  }
}

void render_board()
{
  mat4 model = GLM_MAT4_IDENTITY_INIT;
  glUniformMatrix4fv(glGetUniformLocation(renderer.program, "model"), 1, GL_FALSE, &model[0][0]);

  glUseProgram(renderer.program);
  glBindVertexArray(renderer.board.VAO);
  glDrawArrays(GL_LINES, 0, 8);
  glBindVertexArray(0);
}

void render()
{
  glm_mat4_identity(renderer.view);
  glm_translate(renderer.view, (vec3){0.0f, 0.0f, -1.5f});
  /* glm_rotate(renderer.view, glm_rad(-55.0f), (vec3){1.0f, 0.0f, 0.0f}); */
  glUniformMatrix4fv(glGetUniformLocation(renderer.program, "view"), 1, GL_FALSE, &renderer.view[0][0]);

  glUniformMatrix4fv(glGetUniformLocation(renderer.program, "projection"), 1, GL_FALSE, &renderer.projection[0][0]);

  glUniform4fv(glGetUniformLocation(renderer.program, "color"), 1, BOARD_LINE_COLOR);

  glClearColor(BOARD_COLOR[0], BOARD_COLOR[1], BOARD_COLOR[2], BOARD_COLOR[3]);
  glClear(GL_COLOR_BUFFER_BIT);
  render_board();
  render_tokens();
  glutSwapBuffers();
}

void win_reshape_handler(int width, int height)
{
  SCREEN_WIDTH = width;
  SCREEN_HEIGHT = height;
  renderer_recalc_projection();
  glViewport(0, 0, width, height);
  /* printf("Screen resized to: %d, %d\n", SCREEN_WIDTH, SCREEN_HEIGHT); */
}

int place_token(uint8_t row, uint8_t col)
{
  if (ttt_eval_state(&ttt, NULL) == TTT_STATE_CONTINUE) {
    if (!ttt_place_token(&ttt, row, col)) {
      if (ttt_eval_state(&ttt, NULL) == TTT_STATE_CONTINUE) {
        ai_play(&ttt);
        /* print_board(&ttt);printf("\n"); */
      }
      return 0;
    }
  }
  return 1;
}

void keyboard_handler(unsigned char key, int x, int y)
{
  int row, col;
  /* printf("Key: %x\n", key); */

  row = col = 0;
  switch (key) {
  case 0x1b:
    glutLeaveMainLoop();
    break;
  case 'q': case 'Q':
    row = 1; col = 1;
    break;
  case 'w': case 'W':
    row = 1; col = 2;
    break;
  case 'e': case 'E':
    row = 1; col = 3;
    break;
  case 'a': case 'A':
    row = 2; col = 1;
    break;
  case 's': case 'S':
    row = 2; col = 2;
    break;
  case 'd': case 'D':
    row = 2; col = 3;
    break;
  case 'z': case 'Z':
    row = 3; col = 1;
    break;
  case 'x': case 'X':
    row = 3; col = 2;
    break;
  case 'c': case 'C':
    row = 3; col = 3;
    break;
  case ' ':
    ttt_init(&ttt);
    render();
    break;
  }
  if (row && col) {
    place_token(row - 1, col - 1);
    render();
  }
}

void mouse_handler(int button, int button_state, int x, int y)
{
  TTTState ttt_state;
  uint8_t row, col;
  int screen_board_pos_x, screen_board_pos_y;
  int screen_board_width, screen_board_height;
  screen_board_width = 0.4f * SCREEN_WIDTH;
  screen_board_height = 0.54f * SCREEN_HEIGHT;
  screen_board_pos_y = SCREEN_HEIGHT / 2.0f;
  screen_board_pos_x = SCREEN_WIDTH / 2.0f;
  screen_board_pos_y = SCREEN_HEIGHT / 2.0f;

  row = col = 0;
  if ((button == GLUT_LEFT_BUTTON) && (button_state == GLUT_UP)) {
    /* printf("left click at %d %d\n", x, y); */

    if (y < (screen_board_pos_y - 0.3 * screen_board_height)) {
      row = 1;
    } else if (y > (screen_board_pos_y + 0.3 * screen_board_height)) {
      row = 3;
    } else {
      row = 2;
    }

    if (x < (screen_board_pos_x - 0.3 * screen_board_width)) {
      col = 1;
    } else if (x > (screen_board_pos_x + 0.3 * screen_board_width)) {
      col = 3;
    } else {
      col = 2;
    }

    ttt_state = ttt_eval_state(&ttt, NULL);
    if (ttt_state != TTT_STATE_CONTINUE) {
      ttt_init(&ttt);
      render();
    } else if (row && col) {
      place_token(row - 1, col - 1);
      render();
    }
  } else if ((button == GLUT_RIGHT_BUTTON) && (button_state == GLUT_UP)) {
    ttt_init(&ttt);
    render();
  }
}

/* https://learnopengl.com/Getting-started/Hello-Window */
/* https://freeglut.sourceforge.net/docs/api.php#GlobalCallback */
