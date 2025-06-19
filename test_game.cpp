#define TESTING
#include "../src/tictactoe.cpp"
#include <gtest/gtest.h>
#include <vector>
#include <string>
 

extern std::vector<std::vector<char>> board;

class TicTacToeTest : public ::testing::Test {
protected:
    void SetUp() override {
        board = std::vector<std::vector<char>>(3, std::vector<char>(3, ' '));
    }
};

// === checkWinner() Tests ===

TEST_F(TicTacToeTest, NoWinnerInitially) {
    EXPECT_EQ(checkWinner(), ' ');
}

TEST_F(TicTacToeTest, RowWinX) {
    board[0] = {'X', 'X', 'X'};
    EXPECT_EQ(checkWinner(), 'X');
}

TEST_F(TicTacToeTest, ColumnWinO) {
    board[0][1] = 'O';
    board[1][1] = 'O';
    board[2][1] = 'O';
    EXPECT_EQ(checkWinner(), 'O');
}

TEST_F(TicTacToeTest, DiagonalWinX) {
    board[0][0] = board[1][1] = board[2][2] = 'X';
    EXPECT_EQ(checkWinner(), 'X');
}

TEST_F(TicTacToeTest, AntiDiagonalWinO) {
    board[0][2] = board[1][1] = board[2][0] = 'O';
    EXPECT_EQ(checkWinner(), 'O');
}

// === isDraw() Tests ===

TEST_F(TicTacToeTest, NotDrawIfEmpty) {
    EXPECT_FALSE(isDraw());
}

TEST_F(TicTacToeTest, DrawScenario) {
    board = {
        {'X', 'O', 'X'},
        {'X', 'O', 'O'},
        {'O', 'X', 'X'}
    };
    EXPECT_TRUE(isDraw());
}

// === AI Move Validity ===

TEST_F(TicTacToeTest, EasyAIMoveIsValid) {
    auto move = easyAIMove();
    EXPECT_GE(move.first, 0);
    EXPECT_LE(move.first, 2);
    EXPECT_GE(move.second, 0);
    EXPECT_LE(move.second, 2);
    EXPECT_EQ(board[move.first][move.second], ' ');
}

// === Hashing Tests ===

TEST(UtilityTest, SimpleHashConsistency) {
    EXPECT_EQ(simpleHash("abc"), simpleHash("abc"));
    EXPECT_NE(simpleHash("abc"), simpleHash("def"));
}


int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}