#include "minesweeper.h"
#include <iostream>   // For std::cerr (optional, for debugging)
#include <algorithm>  // For std::shuffle, std::remove_if, std::find, std::min
#include <chrono>     // For seeding random number generator


// --- MinesweeperGame Implementation ---
MinesweeperGame::MinesweeperGame(int r, int c, int m)
    : rows(r), cols(c), numMines(m), gameOver(false), firstClick(true) {
    // Seed the random number generator
    std::random_device rd;
    rng = std::mt19937(rd());
    reset();
}

void MinesweeperGame::reset() {
    board.assign(rows, std::vector<Cell>(cols, Cell{})); // Initialize with default Cells
    status = GameStatus::PLAYING;
    minesFlagged = 0;
    cellsRevealed = 0;
    gameOver = false;
    firstClick = true;
    // Mines are not placed yet; they will be placed on the first click
    // to ensure the first click is safe.
    // We can pre-calculate adjacencies if we place dummy mines or handle first click specially.
    // For now, placeMines and calculateAdjacentMines will be called on first reveal.
}

void MinesweeperGame::placeMines(int safeRow, int safeCol) {
    for (auto& rowVec : board) {
        for (auto& cell : rowVec) {
            cell.isMine = false; // Clear existing mines
        }
    }

    std::vector<std::pair<int, int>> potentialMineLocations;
    for (int r = 0; r < rows; ++r) {
        for (int c = 0; c < cols; ++c) {
            // Exclude the safe cell and its immediate neighbors
            bool isSafeZone = false;
            for (int dr = -1; dr <= 1; ++dr) {
                for (int dc = -1; dc <= 1; ++dc) {
                    if (r == safeRow + dr && c == safeCol + dc) {
                        isSafeZone = true;
                        break;
                    }
                }
                if (isSafeZone) break;
            }
            if (!isSafeZone) {
                potentialMineLocations.push_back({r, c});
            }
        }
    }

    std::shuffle(potentialMineLocations.begin(), potentialMineLocations.end(), rng);

    int minesToPlace = std::min(numMines, (int)potentialMineLocations.size());
    for (int i = 0; i < minesToPlace; ++i) {
        board[potentialMineLocations[i].first][potentialMineLocations[i].second].isMine = true;
    }
}

void MinesweeperGame::calculateAdjacentMines() {
    for (int r = 0; r < rows; ++r) {
        for (int c = 0; c < cols; ++c) {
            if (board[r][c].isMine) {
                board[r][c].adjacentMines = -1; // Convention for mine
                continue;
            }
            int count = 0;
            for (int dr = -1; dr <= 1; ++dr) {
                for (int dc = -1; dc <= 1; ++dc) {
                    if (dr == 0 && dc == 0) continue;
                    int nr = r + dr;
                    int nc = c + dc;
                    if (isValid(nr, nc) && board[nr][nc].isMine) {
                        count++;
                    }
                }
            }
            board[r][c].adjacentMines = count;
        }
    }
}

bool MinesweeperGame::revealCell(int r, int c) {
    if (gameOver || !isValid(r, c) || board[r][c].isRevealed || board[r][c].isFlagged) {
        return false;
    }

    if (firstClick) {
        placeMines(r, c);
        calculateAdjacentMines();
        firstClick = false;
    }

    board[r][c].isRevealed = true;
    cellsRevealed++;

    if (board[r][c].isMine) {
        status = GameStatus::LOST;
        gameOver = true;
        return true;
    }

    if (board[r][c].adjacentMines == 0) {
        for (int dr = -1; dr <= 1; ++dr) {
            for (int dc = -1; dc <= 1; ++dc) {
                if (dr == 0 && dc == 0) continue;
                // revealCell will handle its own boundary and state checks
                revealCell(r + dr, c + dc);
            }
        }
    }
    checkWinCondition();
    return true;
}

bool MinesweeperGame::toggleFlag(int r, int c) {
    if (gameOver || !isValid(r, c) || board[r][c].isRevealed) {
        return false;
    }
    board[r][c].isFlagged = !board[r][c].isFlagged;
    if (board[r][c].isFlagged) minesFlagged++;
    else minesFlagged--;
    // Win condition check is not strictly necessary here unless you allow winning by flagging,
    // but it's good practice for consistency if game rules change.
    // checkWinCondition(); // Current win is by revealing all non-mines
    return true;
}

void MinesweeperGame::revealAllMines() {
    for (int r = 0; r < rows; ++r) {
        for (int c = 0; c < cols; ++c) {
            if (board[r][c].isMine && !board[r][c].isFlagged) { // Reveal unflagged mines
                board[r][c].isRevealed = true;
            }
            // Also, if a cell was incorrectly flagged as a mine, reveal it
            if (!board[r][c].isMine && board[r][c].isFlagged) {
                // Mark as incorrectly flagged (UI can show 'X' over flag)
                // For now, just unflag and reveal if it wasn't already
                board[r][c].isFlagged = false; // Visually show it's not a mine
                // board[r][c].isRevealed = true; // Or keep it hidden but show X
            }
        }
    }
}


GameStatus MinesweeperGame::getStatus() const { return status; }
const std::vector<std::vector<Cell>>& MinesweeperGame::getBoard() const { return board; }
int MinesweeperGame::getMinesRemaining() const { return numMines - minesFlagged; }
int MinesweeperGame::getRows() const { return rows; }
int MinesweeperGame::getCols() const { return cols; }
bool MinesweeperGame::isGameOver() const { return gameOver; }

bool MinesweeperGame::isValid(int r, int c) const {
    return r >= 0 && r < rows && c >= 0 && c < cols;
}

void MinesweeperGame::checkWinCondition() {
    if (status == GameStatus::PLAYING) {
        // Win if all non-mine cells are revealed
        if (cellsRevealed == (rows * cols - numMines)) {
            status = GameStatus::WON;
            gameOver = true;
            // Optionally flag all remaining mines automatically
            for (auto& rowVec : board) {
                for (auto& cell : rowVec) {
                    if (cell.isMine && !cell.isRevealed) { // isRevealed check for already exploded mines
                        cell.isFlagged = true;
                    }
                }
            }
            minesFlagged = numMines; // Update count
        }
    }
}


// --- MinesweeperWindow Implementation ---
MinesweeperWindow::MinesweeperWindow(int r, int c, int m)
    : game(r, c, m), initial_rows(r), initial_cols(c), initial_numMines(m),
      window(nullptr), mainBox(nullptr), grid(nullptr), minesLabel(nullptr), statusLabel(nullptr), resetButton(nullptr) {}

MinesweeperWindow::~MinesweeperWindow() {
    // GTK will handle destroying widgets.
    // If CellButtonData was allocated with g_new, g_object_set_data_full could handle freeing.
    // With C++ new, the "destroy" signal handler is the best place.
}

void MinesweeperWindow::run(int argc, char *argv[]) {
    // gtk_init is called in main.cpp now
    // gtk_init(&argc, &argv);

    window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(window), "Minesweeper GTK");
    gtk_window_set_resizable(GTK_WINDOW(window), FALSE);
    // Set a default size, or allow it to size to content
    gtk_window_set_default_size(GTK_WINDOW(window), 200, 200); // Minimum sensible size
    g_signal_connect(window, "destroy", G_CALLBACK(gtk_main_quit), NULL);

    mainBox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);
    gtk_container_set_border_width(GTK_CONTAINER(mainBox), 5);
    gtk_container_add(GTK_CONTAINER(window), mainBox);

    // Status Bar
    GtkWidget *statusBox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 5);
    gtk_box_pack_start(GTK_BOX(mainBox), statusBox, FALSE, FALSE, 5);

    minesLabel = gtk_label_new("");
    gtk_box_pack_start(GTK_BOX(statusBox), minesLabel, TRUE, TRUE, 0);

    resetButton = gtk_button_new_with_label("🙂"); // Smiley reset
    g_signal_connect(resetButton, "clicked", G_CALLBACK(MinesweeperWindow::onResetClicked), this);
    gtk_box_pack_start(GTK_BOX(statusBox), resetButton, FALSE, FALSE, 0);

    statusLabel = gtk_label_new(""); // Was game status, could be timer
    gtk_box_pack_start(GTK_BOX(statusBox), statusLabel, TRUE, TRUE, 0);

    // Game Grid
    grid = gtk_grid_new();
    gtk_grid_set_row_spacing(GTK_GRID(grid), 1);
    gtk_grid_set_column_spacing(GTK_GRID(grid), 1);
    gtk_box_pack_start(GTK_BOX(mainBox), grid, TRUE, TRUE, 0);

    createBoardUI();
    updateUI();

    gtk_widget_show_all(window);
    // gtk_main is called in main.cpp
}

void MinesweeperWindow::createBoardUI() {
    GList *children, *iter;
    children = gtk_container_get_children(GTK_CONTAINER(grid));
    for(iter = children; iter != NULL; iter = g_list_next(iter)) {
      gtk_widget_destroy(GTK_WIDGET(iter->data));
    }
    g_list_free(children);

    const auto& board_data = game.getBoard(); // getBoard returns const&
    for (int r = 0; r < game.getRows(); ++r) {
        for (int c = 0; c < game.getCols(); ++c) {
            GtkButton* button = GTK_BUTTON(gtk_button_new());
            gtk_widget_set_size_request(GTK_WIDGET(button), 30, 30);
            gtk_button_set_relief(button, GTK_RELIEF_NORMAL); // Standard button look

            board_data[r][c].button = button; // This works because Cell::button is mutable

            CellButtonData* data = new CellButtonData{this, r, c}; // Use C++ new
            g_signal_connect(button, "button-press-event", G_CALLBACK(MinesweeperWindow::onCellClicked), data);
            // Connect "destroy" signal to free CellButtonData
            g_signal_connect_object(button, "destroy", G_CALLBACK(MinesweeperWindow::freeCellButtonData), data, G_CONNECT_SWAPPED);


            gtk_grid_attach(GTK_GRID(grid), GTK_WIDGET(button), c, r, 1, 1);
        }
    }
    gtk_widget_show_all(grid);
}

void MinesweeperWindow::updateUI() {
    const auto& board_data = game.getBoard();
    for (int r = 0; r < game.getRows(); ++r) {
        for (int c = 0; c < game.getCols(); ++c) {
            const Cell& cell_logic = board_data[r][c];
            GtkButton* button = cell_logic.button;
            if (!button) continue;

            // Reset any custom styling first
            GtkStyleContext *context = gtk_widget_get_style_context(GTK_WIDGET(button));
            gtk_style_context_remove_provider(context,
                reinterpret_cast<GtkStyleProvider*>(g_object_get_data(G_OBJECT(button), "custom-css")));
            g_object_set_data(G_OBJECT(button), "custom-css", NULL); // Clear stored provider

            gtk_button_set_relief(button, GTK_RELIEF_NORMAL);


            if (cell_logic.isFlagged) {
                gtk_button_set_label(button, "🚩");
                gtk_widget_set_sensitive(GTK_WIDGET(button), TRUE);
            } else if (cell_logic.isRevealed) {
                gtk_widget_set_sensitive(GTK_WIDGET(button), FALSE);
                 gtk_button_set_relief(button, GTK_RELIEF_NONE); // Flat appearance for revealed

                if (cell_logic.isMine) {
                    gtk_button_set_label(button, "💣");
                    GtkCssProvider *provider = gtk_css_provider_new();
                    gtk_css_provider_load_from_data(provider, "button { background-image: none; background-color: red; color: black; }", -1, NULL);
                    gtk_style_context_add_provider(context, GTK_STYLE_PROVIDER(provider), GTK_STYLE_PROVIDER_PRIORITY_USER);
                    g_object_set_data(G_OBJECT(button), "custom-css", provider); // Store to remove later
                    // g_object_unref(provider); // context holds a ref
                } else if (cell_logic.adjacentMines > 0) {
                    gtk_button_set_label(button, std::to_string(cell_logic.adjacentMines).c_str());
                    // Optional: Color numbers
                    const char* colors[] = {"blue", "green", "red", "darkblue", "brown", "cyan", "black", "gray"};
                    if (cell_logic.adjacentMines <= 8) {
                         GtkCssProvider *provider = gtk_css_provider_new();
                         std::string css = "button { color: " + std::string(colors[cell_logic.adjacentMines -1]) + "; font-weight: bold; }";
                         gtk_css_provider_load_from_data(provider, css.c_str() , -1, NULL);
                         gtk_style_context_add_provider(context, GTK_STYLE_PROVIDER(provider), GTK_STYLE_PROVIDER_PRIORITY_USER);
                         g_object_set_data(G_OBJECT(button), "custom-css", provider); // Store to remove later
                        // g_object_unref(provider);
                    }
                } else {
                    gtk_button_set_label(button, ""); // Empty revealed cell
                }
            } else { // Hidden
                gtk_button_set_label(button, "");
                gtk_widget_set_sensitive(GTK_WIDGET(button), TRUE);
            }
        }
    }

    gtk_label_set_text(GTK_LABEL(minesLabel), ("Mines: " + std::to_string(game.getMinesRemaining())).c_str());
    
    std::string resetFace = "🙂"; // Default happy face

    switch (game.getStatus()) {
        case GameStatus::PLAYING:
            gtk_label_set_text(GTK_LABEL(statusLabel), "Playing...");
            break;
        case GameStatus::WON:
            gtk_label_set_text(GTK_LABEL(statusLabel), "You Won!");
            resetFace = "😎"; // Cool face for win
            // disableAllButtons(); // Game logic should prevent further moves already
            break;
        case GameStatus::LOST:
            gtk_label_set_text(GTK_LABEL(statusLabel), "Game Over!");
            resetFace = "😵"; // Dead face for loss
            game.revealAllMines(); // Make sure game logic reveals them
             // A second updateUI call might be needed if revealAllMines changes states
             // that this first pass of updateUI didn't catch for game over.
             // Let's call it again just to be safe, or ensure revealAllMines updates cell.button directly.
             // For now, this function iterates all cells and updates, so one call should be enough
             // after game.revealAllMines() has set the cell states.
            // disableAllButtons();
            break;
    }
    gtk_button_set_label(GTK_BUTTON(resetButton), resetFace.c_str());

    if (game.isGameOver()) {
        disableAllButtons(); // After all UI updates reflecting the final state.
    }
}

void MinesweeperWindow::disableAllButtons() {
    const auto& board_data = game.getBoard();
    for (int r = 0; r < game.getRows(); ++r) {
        for (int c = 0; c < game.getCols(); ++c) {
            if (board_data[r][c].button) {
                // Keep flagged buttons somewhat active in appearance if desired,
                // but they should not respond to clicks if game is over.
                gtk_widget_set_sensitive(GTK_WIDGET(board_data[r][c].button), FALSE);
            }
        }
    }
}

// --- GTK Static Callbacks ---
gboolean MinesweeperWindow::onCellClicked(GtkWidget *widget, GdkEventButton *event, gpointer data) {
    CellButtonData* button_data = static_cast<CellButtonData*>(data);
    MinesweeperWindow* self = button_data->self;

    if (self->game.isGameOver()) return TRUE;

    bool changed = false;
    // GdkEventButton->button contains the button number (1 for left, 2 for middle, 3 for right)
    if (event->button == 1) { // Left click
        changed = self->game.revealCell(button_data->r, button_data->c);
    } else if (event->button == 2) { // Middle click
        // Implement chording logic if desired (reveal neighbors if cell is revealed and flags match adjacent mines)
        // For now, do nothing or treat as flag for simplicity in this example.
    } else if (event->button == 3) { // Right click
        changed = self->game.toggleFlag(button_data->r, button_data->c);
    }

    if (changed || self->game.isGameOver()) { // Update UI if state changed or game just ended
        self->updateUI();
    }
    return TRUE; // Event handled
}


void MinesweeperWindow::onResetClicked(GtkButton* /*button*/, gpointer data) {
    MinesweeperWindow* self = static_cast<MinesweeperWindow*>(data);
    self->game.reset();
    // createBoardUI will clear old buttons and their associated CellButtonData via "destroy" signal.
    self->createBoardUI(); // Rebuilds grid, assigns new self->game.board[r][c].button
    self->updateUI();
    gtk_widget_set_sensitive(GTK_WIDGET(self->resetButton), TRUE); // Ensure reset button is active
}

void MinesweeperWindow::freeCellButtonData(gpointer data, GObject* /* GObject* where_the_object_was */) {
    CellButtonData* button_data = static_cast<CellButtonData*>(data);
    delete button_data; // Match C++ new with delete
}