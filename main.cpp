#include "minesweeper.h"
#include <iostream>
#include <string>
#include <stdexcept> // For std::stoi, std::invalid_argument, std::out_of_range

int main(int argc, char *argv[]) {
    // Initialize GTK
    gtk_init(&argc, &argv);

    int rows = 10;
    int cols = 10;
    int mines = 15;

    if (argc == 4) {
        try {
            int arg_rows = std::stoi(argv[1]);
            int arg_cols = std::stoi(argv[2]);
            int arg_mines = std::stoi(argv[3]);

            if (arg_rows > 0 && arg_cols > 0 && arg_mines >= 0 && arg_mines < arg_rows * arg_cols) {
                rows = arg_rows;
                cols = arg_cols;
                mines = arg_mines;
            } else {
                std::cerr << "Invalid arguments values. Using defaults." << std::endl;
                std::cerr << "Rows/Cols > 0, Mines >= 0 and < Rows*Cols." << std::endl;
            }
        } catch (const std::invalid_argument& ia) {
            std::cerr << "Invalid argument type (not an integer): " << ia.what() << std::endl;
            std::cerr << "Using default values." << std::endl;
        } catch (const std::out_of_range& oor) {
            std::cerr << "Argument out of range: " << oor.what() << std::endl;
            std::cerr << "Using default values." << std::endl;
        }
    } else if (argc != 1) {
        std::cerr << "Usage: " << argv[0] << " [rows cols mines]" << std::endl;
        std::cerr << "Using default values." << std::endl;
    }


    MinesweeperWindow gameWindow(rows, cols, mines);
    gameWindow.run(argc, argv); // Pass argc, argv though gtk_init already took them

    gtk_main(); // Start the GTK event loop

    return 0;
}