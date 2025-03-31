#include <Novice.h>
#include "YNet/NetworkManager.h"
#include <vector>
#include <iostream>

constexpr int BOARD_SIZE = 8;
constexpr int BLACK = 1;
constexpr int WHITE = 2;

class OthelloServer {
public:
    OthelloServer(int port) : networkManager(port) {}
    void Run();

private:
    NetworkManager networkManager;
    int board[BOARD_SIZE][BOARD_SIZE] = {};
    int currentPlayer = BLACK;

    void InitializeBoard();
    void ProcessClientRequest(int clientID, const std::string& request);
    std::string GetBoardState();
    bool MakeMove(int x, int y, int player);
};

void OthelloServer::InitializeBoard() {
    memset(board, 0, sizeof(board));
    board[3][3] = WHITE;
    board[3][4] = BLACK;
    board[4][3] = BLACK;
    board[4][4] = WHITE;
}

void OthelloServer::ProcessClientRequest(int clientID, const std::string& request) {
    if (request.empty()) return;

    int x, y;
    sscanf(request.c_str(), "%d %d", &x, &y);
    
    if (MakeMove(x, y, currentPlayer)) {
        currentPlayer = (currentPlayer == BLACK) ? WHITE : BLACK;
    }
    
    std::string boardState = GetBoardState();
    networkManager.Send(clientID, boardState);
}

std::string OthelloServer::GetBoardState() {
    std::string state;
    for (int i = 0; i < BOARD_SIZE; ++i) {
        for (int j = 0; j < BOARD_SIZE; ++j) {
            state += std::to_string(board[i][j]) + " ";
        }
    }
    return state;
}

bool OthelloServer::MakeMove(int x, int y, int player) {
    if (board[x][y] != 0) return false;
    board[x][y] = player;
    return true;
}

void OthelloServer::Run() {
    if (!networkManager.StartServer()) {
        std::cerr << "Failed to start server." << std::endl;
        return;
    }
    
    InitializeBoard();
    std::cout << "Othello Server started on port " << networkManager.GetPort() << std::endl;

    while (true) {
        int clientID;
        std::string request = networkManager.Receive(clientID);
        ProcessClientRequest(clientID, request);
    }
}

int main() {
    OthelloServer server(12345);
    server.Run();
    return 0;
}
