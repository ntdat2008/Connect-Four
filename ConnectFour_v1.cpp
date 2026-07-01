#ifdef _WIN32
    #include <conio.h>
#else
    #include <termios.h>
    #include <unistd.h>
#endif

#include "BitboardEngine.h"

#include <bits/stdc++.h>
#include <thread>
#include <atomic>
#include <random>
#include <chrono>

using namespace std;

random_device rd;
mt19937 gen(rd() ^ chrono::high_resolution_clock::now().time_since_epoch().count());

int random(int low, int high) {
    uniform_int_distribution<> range(low, high);
    return range(gen);
}

class ConnectFour {
private:
    Bitboard engine;

    const int PLAYER_X = 0;
    const int PLAYER_O = 1;
    const int RANDOM_PLAYER = 2;

    char boardUI[6][7];

    int gamemode;
    int firstPlayer;
    int difficulty;

    int currentPlayer;
    int lastMove;

    float score1;
    float score2;

    const vector<string> difficultyName = {
        "Baby",
        "Easy",
        "Normal",
        "Hard",
        "Insane",
        "Master"
    };
    const int BABY = 1;
    const int EASY = 2;
    const int NORMAL = 3;
    const int HARD = 4;
    const int INSANE = 5;
    const int MASTER = 6;

    const vector<pair<string,int>> invalidInputWarningText = {
        {"Invalid input!\n", 40},
        {"That's not an option!\n", 10},
        {"Invalid. Try again!\n", 10},
        {"No.\n", 10},
        {"Nuh uh.\n", 10},
        {"What are you doing?\n", 10},
        {"Stop it.\n", 10},
    };

    const vector<pair<string,int>> loadingScreenText = {
        {"Loading", 45},
        {"Please wait", 25},
        {"Almost there", 10},
        {"Playing animation", 8},
        {"Waiting for eternity", 8},
        {"Pro tip: Pressing [Ctrl] + [C] may do something", 5},
        {"This text has 2% chance to show up", 2},
    };

    const vector<pair<string,int>> exitingScreenText = {
        {"Exiting", 25},
        {"Quitting", 21},
        {"Closing the game", 21},
        {"Goodbye", 16},
        {"Until next time", 16},
        {"Did you know that you can just click the 'X' button to close the game?", 1},
    };

    const vector<pair<string,int>> computerMakingMoveAnimationText = {
        {"The computer is thinking", 40},
        {"Calculating", 15},
        {"Running algorithms", 15},
        {"This will take a while", 15},
        {"Processing", 15},
    };

public:
    string input() {
        #ifdef _WIN32
            while (_kbhit()) {
                _getch(); 
            }
        #else
            tcflush(STDIN_FILENO, TCIFLUSH); 
        #endif
        cin.clear();
        
        string data;
        getline(cin, data);
        return data;
    }

    void clearRow(int num) {
        while (num--) {
            cout << "\033[A\033[2K\r";
        } 
    }

    void clearScreen() {
        #ifdef _WIN32
            system("cls");
        #else
            system("clear");
        #endif
    }

    string getRandomText(const vector<pair<string,int>>& texts) {
        int num = random(1, 100);

        for (pair<string,int> text : texts) {
            if (num <= text.second) {
                return text.first;
            }
            num -= text.second;
        }

        return texts[0].first;
    }

    void invalidInputWarning() {
        cout << getRandomText(invalidInputWarningText);
        this_thread::sleep_for(chrono::milliseconds(500));
    }

    void loadingScreen(int loopTime) {
        string text = getRandomText(loadingScreenText);
        string text1 = "   " + text + ".  \r";
        string text2 = "   " + text + ".. \r";
        string text3 = "   " + text + "...\r";

        
        clearScreen();
        while (loopTime--) {
            cout << text1 << flush;
            this_thread::sleep_for(chrono::milliseconds(400));
            cout << text2 << flush;
            this_thread::sleep_for(chrono::milliseconds(400));
            cout << text3 << flush;
            this_thread::sleep_for(chrono::milliseconds(400));
        }
        clearScreen();
    }

    void exitingScreen(int loopTime) {
        string text = getRandomText(exitingScreenText);
        string text1 = "   " + text + ".  \r";
        string text2 = "   " + text + ".. \r";
        string text3 = "   " + text + "...\r";
        
        clearScreen();
        while (loopTime--) {
            cout << text1 << flush;
            this_thread::sleep_for(chrono::milliseconds(400));
            cout << text2 << flush;
            this_thread::sleep_for(chrono::milliseconds(400));
            cout << text3 << flush;
            this_thread::sleep_for(chrono::milliseconds(400));
        }
        clearScreen();
    }

    void computerMakingMoveAnimation(atomic<bool>& flag) {
    string text = getRandomText(computerMakingMoveAnimationText);
    string text1 = "  " + text + ".  \r";
    string text2 = "  " + text + ".. \r";
    string text3 = "  " + text + "...\r";
    string blank = string(text3.size(), ' ') + "\r";

    while (!flag) {
        cout << text1 << flush;
        this_thread::sleep_for(chrono::milliseconds(400));
        if (flag) {
            break;
        }

        cout << text2 << flush;
        this_thread::sleep_for(chrono::milliseconds(400));
        if (flag) {
            break;
        }

        cout << text3 << flush;
        this_thread::sleep_for(chrono::milliseconds(400));
        if (flag) {
            break;
        }
    }

    cout << blank << flush;
}

    void startMenu() {
        loadingScreen(3);

        cout << "=====================================================================\n";
        cout << "     ______   ______   __      _ __      _______   _______________\n";
        cout << "    /  ____/ /  __  \\ |  \\    | |  \\    | |  ___| /  ____\\__   __/\n";
        cout << "   |  /     |  /  \\  || |\\\\   | | |\\\\   | | |___ |  /       | |\n";
        cout << "   | |      | |    | || | \\\\  | | | \\\\  | |  ___|| |        | |\n";
        cout << "   | |      | |    | || |  \\\\ | | |  \\\\ | | |    | |        | |\n";
        cout << "   |  \\____ |  \\__/  || |   \\\\| | |   \\\\| | |___ |  \\____   | |\n";
        cout << "    \\______\\ \\______/ |_|    \\__|_|    \\__|_____| \\______\\  |_|\n";
        cout << "               ______   ______    __       ________\n";
        cout << "              |  ____| /  __  \\  |  |     |   ___  \\\n";
        cout << "              | |____ |  /  \\  | |  |     |  |___\\  \\\n";
        cout << "              |  ____|| |    | | |  |     |   ___   /\n";
        cout << "              | |     | |    | | |  |     |  |   \\  \\\n";
        cout << "              | |     |  \\__/  | |   \\___/   |    \\  \\\n";
        cout << "              |_|      \\______/   \\_______/\\_|     \\__\\\n\n";
        cout << "============================ Version 1 ==============================\n\n";
        cout << "                           [1] New game\n\n";
        cout << "                           [2] Credit\n\n";
        cout << "                           [3] Exit\n\n";

        string playerInput;
        while (true) {
            cout << "Input: ";
            playerInput = input();
            if (playerInput == "1" || playerInput == "2" || playerInput == "3") {
                break;
            }
            invalidInputWarning();
            clearRow(2);
        }

        if (playerInput == "1") {
            selectionMenu();
        }
        else if (playerInput == "2") {
            creditMenu();
        }
        else {
            exitingScreen(random(1, 2));
        }
    }

    void creditMenu() {
        loadingScreen(random(1, 2));

        cout << "{========================[ CREDIT ]========================}\n";
        cout << "                 Lead Developer: ntdat2008\n";
        cout << "                   Programmer: ntdat2008\n";
        cout << "                   UI Designer: ntdat2008\n";
        cout << "                  Game Designer: ntdat2008\n";
        cout << "                     Tester: ntdat2008\n";
        cout << "                     Writer: ntdat2008\n\n";
        cout << "       Inspired by the original board game Connect Four\n";
        cout << "{==========================================================}\n\n";
        cout << "Press [Enter] to go back.\n\n";

        string playerInput;
        playerInput = input();

        startMenu();
    }

    void selectionMenu() {
        loadingScreen(random(1, 3));

        cout << "{========================[ NEW GAME ]========================}\n\n";
        cout << "                      Select a gamemode.\n\n";
        cout << "                   [1] Player VS Computer\n";
        cout << "                    Natural VS Artificial\n\n";
        cout << "                    [2] Player VS Player\n";
        cout << "  *This mode requires 2 players to play in the same device.\n";
        cout << "                 *Or you can play alone...\n\n";
        cout << "{============================================================}\n\n";
        
        string gamemodeInput;
        while (true) {
            cout << "Your choice: ";
            gamemodeInput = input();
            if (gamemodeInput == "1" || gamemodeInput == "2") {
                break;
            }
            invalidInputWarning();
            clearRow(2);
        }
        gamemode = stoi(gamemodeInput);
        
        loadingScreen(1);

        if (gamemode == 1) {
            cout << "{========================[ NEW GAME ]========================}\n\n";
            cout << "                        Who goes first?\n\n";
            cout << "                         [1] Player\n";
            cout << "                      You always go first.\n\n";
            cout << "                         [2] Computer\n";
            cout << "                The computer always goes first.\n\n";
            cout << "                         [3] Random\n";
            cout << "                 Random starter for EACH match.\n\n";
            cout << "{============================================================}\n\n";
        }
        else {
            cout << "{========================[ NEW GAME ]========================}\n\n";
            cout << "                      Who goes first?\n\n";
            cout << "                        [1] Player 1\n";
            cout << "                 Player 1 always goes first.\n\n";
            cout << "                        [2] Player 2\n";
            cout << "                 Player 2 always goes first.\n\n";
            cout << "                        [3] Random\n";
            cout << "               Random starter for EACH match.\n\n";
            cout << "{============================================================}\n\n";
        }

        string firstPlayerInput;
        while (true) {
            cout << "Your choice: ";
            firstPlayerInput = input();
            if (firstPlayerInput == "1" || firstPlayerInput == "2" || firstPlayerInput == "3") {
                break;
            }
            invalidInputWarning();
            clearRow(2);
        }
        firstPlayer = stoi(firstPlayerInput) - 1;

        if (gamemode == 1) {
            loadingScreen(1);

            cout << "{========================[ NEW GAME ]========================}\n\n";
            cout << "                     Select a difficulty.\n\n";
            cout << "                          [1] Baby\n";
            cout << "                  The computer plays randomly.\n\n";
            cout << "                          [2] Easy\n";
            cout << "      The computer tends to play sub-optimal defensive moves.\n\n";
            cout << "                          [3] Normal\n";
            cout << "                 The computer plays decently.\n\n";
            cout << "                          [4] Hard\n";
            cout << "                 The computer plays very well.\n\n";
            cout << "                          [5] Insane\n";
            cout << "            The computer plays almost flawlessly.\n\n";
            cout << "                          [6] Master\n";
            cout << "                The computer plays perfectly.\n\n";
            cout << "{============================================================}\n\n";

            string difficultyInput;
            while (true) {
                cout << "Your choice: ";
                difficultyInput = input();
                if (difficultyInput == "1" || difficultyInput == "2" || difficultyInput == "3" || difficultyInput == "4" || difficultyInput == "5" || difficultyInput == "6") {
                    break;
                }
                invalidInputWarning();
                clearRow(2);
            }
            difficulty = stoi(difficultyInput);
        }

        score1 = score2 = 0;

        if (gamemodeInput == "1") {
            initGamePlayerVSComputer();
        }
        else {
            initGamePlayerVSPlayer();
        }
    }

    void initGamePlayerVSComputer() {
        loadingScreen(random(1, 3));
        
        cout << "{================================================}\n\n";
        cout << "            Player (X) vs Computer (O)\n\n";
        cout << "Difficulty: " << difficultyName[difficulty - 1] << "\n";
        cout << "Match: #" << (int)(score1 + score2 + 1) << "\n";
        cout << "Score: " << score1 << " - " << score2 << "\n";
        cout << "--------------------------------------------------\n\n";

        resetBoard();

        currentPlayer = (firstPlayer == RANDOM_PLAYER ? random(PLAYER_X, PLAYER_O) : firstPlayer);
        string colInput;
        lastMove = -1;
        int result;

        while (true) {
            displayBoardUI();
            cout << "{================================================}\n\n";

            if (engine.isWin(PLAYER_X)) {
                result = PLAYER_X;
                break;
            }
            if (engine.isWin(PLAYER_O)) {
                result = PLAYER_O;
                break;
            }
            if (engine.isDraw()) {
                result = -1;
                break;
            }
            
            if (currentPlayer == PLAYER_X) {
                if (lastMove != -1) {
                    cout << "Computer put an 'O' into the ";
                    if (lastMove == 1) {
                        cout << "1st ";
                    }
                    else if (lastMove == 2) {
                        cout << "2nd ";
                    }
                    else if (lastMove == 3) {
                        cout << "3rd ";
                    }
                    else {
                        cout << lastMove << "th ";
                    }
                    cout << "column.\n\n";
                }

                cout << "It is your turn.\n\n";

                while (true) {
                    cout << "Select a column: ";

                    colInput = input();
                    if (isValidMoveInput(colInput)) {
                        break;
                    }

                    invalidInputWarning();
                    clearRow(2);
                }

                applyMove(stoi(colInput) - 1, PLAYER_X, 'X');

                if (lastMove == -1) {
                    clearRow(13);
                }
                else {
                    clearRow(15);
                }
            }
            else {
                atomic<bool> flag(false);

                thread thread2([this, &flag]() {
                    this -> computerMakeMove();

                    this_thread::sleep_for(chrono::seconds(2));

                    flag = true;
                });

                computerMakingMoveAnimation(flag);

                if (thread2.joinable()) {
                    thread2.join();
                }

                clearRow(10);
            }

            currentPlayer = (currentPlayer == PLAYER_X ? PLAYER_O : PLAYER_X);
        }

        cout << "                   Match ended.\n\n";
        cout << "            Press [Enter] to continue.\n\n";

        string playerInput = input();

        resultScreen(result);
    }

    void initGamePlayerVSPlayer() {
        loadingScreen(random(1, 2));
        
        cout << "{================================================}\n\n";
        cout << "           Player 1 (X) vs Player 2 (O)\n\n";
        cout << "Match: #" << (int)(score1 + score2 + 1) << "\n";
        cout << "Score: " << score1 << " - " << score2 << "\n";
        cout << "--------------------------------------------------\n\n";

        resetBoard();

        currentPlayer = (firstPlayer == RANDOM_PLAYER ? random(PLAYER_X, PLAYER_O) : firstPlayer);
        string colInput;
        int result;
        lastMove = -1;

        while (true) {
            displayBoardUI();
            cout << "{================================================}\n\n";

            if (engine.isWin(PLAYER_X)) {
                result = PLAYER_X;
                break;
            }
            if (engine.isWin(PLAYER_O)) {
                result = PLAYER_O;
                break;
            }
            if (engine.isDraw()) {
                result = -1;
                break;
            }
            
            if (currentPlayer == PLAYER_X) {
                if (lastMove != -1) {
                    cout << "Player 2 put an 'O' into the ";
                    if (lastMove == 1) {
                        cout << "1st ";
                    }
                    else if (lastMove == 2) {
                        cout << "2nd ";
                    }
                    else if (lastMove == 3) {
                        cout << "3rd ";
                    }
                    else {
                        cout << lastMove << "th ";
                    }
                    cout << "column.\n\n";
                }

                cout << "It is Player 1's turn.\n\n";

                while (true) {
                    cout << "Select a column: ";
                    colInput = input();
                    if (isValidMoveInput(colInput)) {
                        break;
                    }
                    invalidInputWarning();
                    clearRow(2);
                }

                applyMove(stoi(colInput) - 1, PLAYER_X, 'X');

                if (lastMove == -1) {
                    clearRow(13);
                }
                else {
                    clearRow(15);
                }
            }
            else {
                if (lastMove != -1) {
                    cout << "Player 1 put an 'X' into the ";
                    if (lastMove == 1) {
                        cout << "1st ";
                    }
                    else if (lastMove == 2) {
                        cout << "2nd ";
                    }
                    else if (lastMove == 3) {
                        cout << "3rd ";
                    }
                    else {
                        cout << lastMove << "th ";
                    }
                    cout << "column.\n\n";
                }

                cout << "It is Player 2's turn.\n\n";

                while (true) {
                    cout << "Select a column: ";
                    colInput = input();
                    if (isValidMoveInput(colInput)) {
                        break;
                    }
                    invalidInputWarning();
                    clearRow(2);
                }

                applyMove(stoi(colInput) - 1, PLAYER_O, 'O');

                if (lastMove == -1) {
                    clearRow(13);
                }
                else {
                    clearRow(15);
                }
            }

            lastMove = stoi(colInput);
            currentPlayer = (currentPlayer == PLAYER_X ? PLAYER_O : PLAYER_X);
        }

        cout << "                   Match ended.\n\n";
        cout << "            Press [Enter] to continue.\n\n";

        string playerInput = input();

        resultScreen(result);
    }

    void resultScreen(int winner) {
        loadingScreen(random(1, 2));

        if (winner == PLAYER_X) {
            if (gamemode == 1) {
                cout << "{====================[ RESULT ]====================}\n\n";
                cout << "                    Player WIN!\n\n";
                cout << "               Player gets 1 point.\n\n";
                cout << "----------------------------------------------------\n\n";
                cout << "                 [1] Rematch\n";
                cout << "                 [2] New game\n";
                cout << "                 [3] Back to Start\n\n";
                cout << "{=================================================}\n\n";
            }
            else {
                cout << "{====================[ RESULT ]====================}\n\n";
                cout << "                   Player 1 WIN!\n\n";
                cout << "              Player 1 gets 1 point.\n\n";
                cout << "----------------------------------------------------\n\n";
                cout << "                 [1] Rematch\n";
                cout << "                 [2] New game\n";
                cout << "                 [3] Back to Start\n\n";
                cout << "{=================================================}\n\n";
            }

            ++score1;
        }

        else if (winner == PLAYER_O) {
            if (gamemode == 1) {
                cout << "{====================[ RESULT ]====================}\n\n";
                cout << "                   Computer WIN!\n\n";
                cout << "               Computer gets 1 point.\n\n";
                cout << "----------------------------------------------------\n\n";
                cout << "                 [1] Rematch\n";
                cout << "                 [2] New game\n";
                cout << "                 [3] Back to Start\n\n";
                cout << "{=================================================}\n\n";
            }
            else {
                cout << "{====================[ RESULT ]====================}\n\n";
                cout << "                   Player 2 WIN!\n\n";
                cout << "              Player 2 gets 1 point.\n\n";
                cout << "----------------------------------------------------\n\n";
                cout << "                 [1] Rematch\n";
                cout << "                 [2] New game\n";
                cout << "                 [3] Back to Start\n\n";
                cout << "{=================================================}\n\n";
            }
            

            ++score2;
        }
        
        else {
            if (gamemode == 1) {
                cout << "{====================[ RESULT ]====================}\n\n";
                cout << "                    It's a DRAW!\n\n";
                cout << "      Player and Computer both get 0.5 points.\n\n";
                cout << "----------------------------------------------------\n\n";
                cout << "                 [1] Rematch\n";
                cout << "                 [2] New game\n";
                cout << "                 [3] Back to Start\n\n";
                cout << "{=================================================}\n\n";
            }
            else {
                cout << "{====================[ RESULT ]====================}\n\n";
                cout << "                    It's a DRAW!\n\n";
                cout << "            Both players get 0.5 points.\n\n";
                cout << "----------------------------------------------------\n\n";
                cout << "                 [1] Rematch\n";
                cout << "                 [2] New game\n";
                cout << "                 [3] Back to Start\n\n";
                cout << "{=================================================}\n\n";
            }

            score1 += 0.5;
            score2 += 0.5;
        }

        string playerInput;
        while (true) {
            cout << "Input: ";
            playerInput = input();

            if (playerInput == "1" || playerInput == "2" || playerInput == "3") {
                break;
            }

            invalidInputWarning();
            clearRow(2);
        }

        if (playerInput == "1") {
            if (gamemode == 1) {
                initGamePlayerVSComputer();
            }
            else {
                initGamePlayerVSPlayer();
            }
        }
        else if (playerInput == "2") {
            selectionMenu();
        }
        else {
            startMenu();
        }
    }

    void displayBoardUI() {
        string blank = string(10, ' ');

        cout << blank << "  ";
        for (int num = 1; num <= 7; ++num) {
            cout << num << "   ";
        }
        cout << '\n';

        for (int row = 0; row <= 4; ++row) {
            cout << blank << "| ";
            for (int col = 0; col <= 5; ++col) {
                cout << boardUI[row][col] << " | ";
            }
            cout << boardUI[row][6] << " |\n";
        }

        cout << blank << "|_";
        for (int col = 0; col <= 5; ++col) {
            cout << boardUI[5][col] << "_|_";
        }
        cout << boardUI[5][6] << "_|\n\n";
    }

    void resetBoard() {
        engine.reset();
        for (int row = 0; row < 6; ++row) {
            for (int col = 0; col < 7; ++col) {
                boardUI[row][col] = '-';
            }
        }
    }

    bool isValidMoveInput(string colInput) {
        if (colInput.size() != 1) {
            return false;
        }
        if (colInput[0] < '1' || '7' < colInput[0]) {
            return false;
        }
        if (!engine.isValidMove(stoi(colInput) - 1)) {
            return false;
        }
        return true;
    }

    void applyMove(int col, int player, char symbol) {
        engine.makeMove(col, player);
        
        for (int r = 5; r >= 0; --r) {
            if (boardUI[r][col] == '-') {
                boardUI[r][col] = symbol;
                break;
            }
        }
    }

    void computerMakeMove() {
        int chosenCol = -1;
        vector<int> moves;
        
        if (difficulty != BABY) {
            for (int col = 0; col < 7; ++col) {
                if (engine.isValidMove(col)) {
                    engine.makeMove(col, PLAYER_O);

                    if (engine.isWin(PLAYER_O)) {
                        engine.undoMove(col, PLAYER_O);

                        chosenCol = col;
                        break;
                    }

                    engine.undoMove(col, PLAYER_O);
                }
            }

            if (chosenCol == -1) {
                for (int col = 0; col < 7; ++col) {
                    if (engine.isValidMove(col)) {
                        engine.makeMove(col, PLAYER_X);
                        
                    if (engine.isWin(PLAYER_X)) {
                        engine.undoMove(col, PLAYER_X);

                        chosenCol = col;
                        break;
                    }

                    engine.undoMove(col, PLAYER_X);
                    }
                }
            }
            
        }
        
        if (chosenCol == -1) {
            if (difficulty == BABY) {
                moves = engine.getRandomMoves();
            }

            else if (difficulty == EASY) {
                int dice = random(1, 100);
                int delta = 100;

                // Best: 50%
                if (dice <= 50) {
                    moves = engine.getBestMoves(random(1, 2));
                }
                // Good: 50%
                else {
                    moves = engine.getGoodMoves(random(1, 2), delta);
                }
            }

            else if (difficulty == NORMAL) {
                int dice = random(1, 100);
                int delta = random(20, 30);
                
                // Best: 50% 
                if (dice <= 50) {
                    moves = engine.getBestMoves(random(3, 4));
                }
                // Good: 45%
                else if (dice <= 95) {
                    moves = engine.getGoodMoves(random(3, 4), delta);
                }
                // Bad: 5%
                else {
                    moves = engine.getBadMoves(random(3, 4), delta);
                }
            }

            else if (difficulty == HARD) {
                int dice = random(1, 100);
                int delta = 15;

                // Best: 70% 
                if (dice <= 70) {
                    moves = engine.getBestMoves(random(5, 6));
                }
                // Good: 28%
                else if (dice <= 98) {
                    moves = engine.getGoodMoves(random(5, 6), delta);
                }
                // Bad: 2%
                else {
                    moves = engine.getBadMoves(random(5, 6), delta);
                }
            }

            else if (difficulty == INSANE) {
                int dice = random(1, 100);
                int delta = 5;

                // Best: 98%
                if (dice <= 98) {
                    moves = engine.getBestMoves(random(7, 9));
                }
                // Good: 2%
                else  {
                    moves = engine.getGoodMoves(random(7, 9), delta);
                }
            }

            else if (difficulty == MASTER) {
                moves = engine.getBestMoves(13);
            }

            if (moves.empty()) {
                moves = engine.getRandomMoves();
            }

            chosenCol = moves[random(0, (int)moves.size() - 1)];
        }

        lastMove = chosenCol + 1;
        applyMove(chosenCol, PLAYER_O, 'O');
    }
};

int main() {
    ConnectFour game;
    
    game.startMenu();

    return 0;
}