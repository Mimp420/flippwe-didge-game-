#include <furi.h>
#include <gui/gui.h>
#include <input/input.h>
#include <stdlib.h>
#include <stdio.h>

typedef struct {
    int player_x;
    int obstacle_x;
    int obstacle_y;
    int score;
    bool game_over;
} GameState;

static void draw_callback(Canvas* canvas, void* context) {
    GameState* game = context;

    canvas_clear(canvas);
    canvas_set_font(canvas, FontPrimary);

    if(game->game_over) {
        canvas_draw_str(canvas, 30, 24, "GAME OVER");

        char score_text[32];
        snprintf(score_text, sizeof(score_text), "Score: %d", game->score);
        canvas_draw_str(canvas, 38, 40, score_text);

        canvas_set_font(canvas, FontSecondary);
        canvas_draw_str(canvas, 20, 58, "OK = Restart");

        return;
    }

    // Player
    canvas_draw_box(
        canvas,
        game->player_x,
        55,
        8,
        6
    );

    // Falling obstacle
    canvas_draw_box(
        canvas,
        game->obstacle_x,
        game->obstacle_y,
        6,
        6
    );

    // Score
    char score_text[32];
    snprintf(score_text, sizeof(score_text), "Score: %d", game->score);
    canvas_draw_str(canvas, 2, 10, score_text);
}

static void input_callback(InputEvent* input_event, void* context) {
    FuriMessageQueue* event_queue = context;

    furi_message_queue_put(
        event_queue,
        input_event,
        FuriWaitForever
    );
}

int32_t dodge_game_app(void* p) {
    UNUSED(p);

    GameState game = {
        .player_x = 60,
        .obstacle_x = 40,
        .obstacle_y = 0,
        .score = 0,
        .game_over = false,
    };

    FuriMessageQueue* event_queue =
        furi_message_queue_alloc(8, sizeof(InputEvent));

    ViewPort* viewport = view_port_alloc();

    view_port_draw_callback_set(
        viewport,
        draw_callback,
        &game
    );

    view_port_input_callback_set(
        viewport,
        input_callback,
        event_queue
    );

    Gui* gui = furi_record_open(RECORD_GUI);

    gui_add_view_port(
        gui,
        viewport,
        GuiLayerFullscreen
    );

    bool running = true;

    while(running) {
        InputEvent event;

        if(furi_message_queue_get(
               event_queue,
               &event,
               50
           ) == FuriStatusOk) {

            if(event.type == InputTypePress ||
               event.type == InputTypeRepeat) {

                if(event.key == InputKeyBack) {
                    running = false;
                }

                if(!game.game_over) {
                    if(event.key == InputKeyLeft) {
                        game.player_x -= 5;

                        if(game.player_x < 0) {
                            game.player_x = 0;
                        }
                    }

                    if(event.key == InputKeyRight) {
                        game.player_x += 5;

                        if(game.player_x > 120) {
                            game.player_x = 120;
                        }
                    }
                } else {
                    if(event.key == InputKeyOk) {
                        game.player_x = 60;
                        game.obstacle_x = rand() % 120;
                        game.obstacle_y = 0;
                        game.score = 0;
                        game.game_over = false;
                    }
                }
            }
        }

        if(!game.game_over) {
            game.obstacle_y += 2;

            if(game.obstacle_y > 64) {
                game.obstacle_y = 0;
                game.obstacle_x = rand() % 120;
                game.score++;
            }

            if(
                game.obstacle_y + 6 >= 55 &&
                game.obstacle_y <= 61 &&
                game.obstacle_x + 6 >= game.player_x &&
                game.obstacle_x <= game.player_x + 8
            ) {
                game.game_over = true;
            }
        }

        view_port_update(viewport);
    }

    gui_remove_view_port(gui, viewport);

    view_port_free(viewport);
    furi_message_queue_free(event_queue);

    furi_record_close(RECORD_GUI);

    return 0;
}
