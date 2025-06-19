#include <gtest/gtest.h>
#include "../include/game_logic.h"

class TicTacToeTest : public ::testing::Test {
protected:
    void SetUp() override {
        board = std::vector<std::vector<char>>(3, std::vector<char>(3, ' '));
    }
};

TEST_F(TicTacToeTest, RowWinX) {
    board[0] = {'X', 'X', 'X'};
    EXPECT_EQ(checkWinner(), 'X');
}

TEST_F(TicTacToeTest, EasyAIMoveIsValid) {
    auto move = easyAIMove();
    EXPECT_TRUE(board[move.first][move.second] == ' ');
}
int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
