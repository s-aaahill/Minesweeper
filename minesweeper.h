#ifndef MINESWEEPER_H
#define MINESWEEPER_H

#include <gtk/gtk.h>
#include <vector>
#include <string>
#include <random> // For std::mt19937, std::random_device, std::shuffle

// --- Game Logic Enums and Structs ---
enum class GameStatus { PLAYING, WON, LOST };

struct Cell {
    bool isMine = false;
    bool isRevealed = false;
    bool isFlagged = false;
    int adjacentMines = 0;
    mutable GtkButton* button = nullptr; // Link to the GTK button, mutable for UI updates
};

// --- Game Logic Class ---
class MinesweeperGame {
public:
    MinesweeperGame(int r, int c, int m);
    void reset();
    bool revealCell(int r, int c);
    bool toggleFlag(int r, int c);
    void revealAllMines(); // Used on game over (loss)

    GameStatus getStatus() const;
    const std::vector<std::vector<Cell>>& getBoard() const;
    int getMinesRemaining() const;
    int getRows() const;
    int getCols() const;
    bool isGameOver() const;

private:
    void placeMines(int safeRow, int safeCol);
    void calculateAdjacentMines();
    bool isValid(int r, int c) const;
    void checkWinCondition();

    int rows, cols, numMines;
    std::vector<std::vector<Cell>> board;
    GameStatus status;
    int minesFlagged;
    int cellsRevealed;
    bool gameOver;
    bool firstClick;
    std::mt19937 rng; // Random number generator
};

// --- GTK UI Class ---
class MinesweeperWindow {
public:
    MinesweeperWindow(int r, int c, int m);
    ~MinesweeperWindow(); // For potential cleanup
    void run(int argc, char *argv[]);

private:
    // Helper struct for passing data to callbacks
    struct CellButtonData {
        MinesweeperWindow* self;
        int r, c;
    };

    GtkWidget *window;
    GtkWidget *mainBox;
    GtkWidget *grid;
    GtkWidget *minesLabel;
    GtkWidget *statusLabel;
    GtkWidget *resetButton;

    MinesweeperGame game;
    int initial_rows, initial_cols, initial_numMines; // Store initial dimensions for reset

    void createBoardUI();
    void updateUI();
    void disableAllButtons();

    // --- GTK Callbacks (static member functions) ---
    static gboolean onCellClicked(GtkWidget *widget, GdkEventButton *event, gpointer data);
    static void onResetClicked(GtkButton *button, gpointer data);
    static void freeCellButtonData(gpointer data, GObject* /* GObject* where_the_object_was */);
};

#endif // MINESWEEPER_H