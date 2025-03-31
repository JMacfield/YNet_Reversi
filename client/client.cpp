#include <Novice.h>
#include <string>
#include "YNet/NetworkManager.h"

int board[8][8];
std::string msg;
int msgWait;

const char kWindowTitle[] = "学籍番号";

YNet::NetworkManager networkManager;
bool isServer = false;
int playerID = 0;

// 盤面をネットワーク同期
void SyncBoard()
{
    networkManager.SendData("BOARD", board, sizeof(board));
}

// 受信処理
void ReceiveData()
{
    YNet::Packet packet;
    while (networkManager.ReceiveData(packet)) {
        if (packet.tag == "BOARD") {
            memcpy(board, packet.data, sizeof(board));
        }
    }
}

bool PutPiece(int x, int y, int turn, bool putFlag)
{
    int sum = 0;
    if (board[y][x] > 0)
        return false;

    for (int dy = -1; dy <= 1; dy++)
        for (int dx = -1; dx <= 1; dx++) {
            int wx[8], wy[8];
            for (int wn = 0;; wn++) {
                int kx = x + dx * (wn + 1);
                int ky = y + dy * (wn + 1);
                if (kx < 0 || kx > 7 || ky < 0 || ky > 7 || board[ky][kx] == 0)
                    break;
                if (board[ky][kx] == turn) {
                    if (putFlag)
                        for (int i = 0; i < wn; i++)
                            board[wy[i]][wx[i]] = turn;
                    sum += wn;
                    break;
                }
                wx[wn] = kx;
                wy[wn] = ky;
            }
        }

    if (sum > 0 && putFlag) {
        board[y][x] = turn;
        SyncBoard();
    }
    return sum > 0;
}

bool GameLogicRoutine1(int turn)
{
    static bool mouse_flag = false;
    if (Novice::IsTriggerMouse(0)) {
        if (!mouse_flag) {
            mouse_flag = true;
            int mx, my;
            Novice::GetMousePosition(&mx, &my);
            if (PutPiece(mx / 48, my / 48, turn, true)) {
                networkManager.SendData("MOVE", &mx, sizeof(int));
                networkManager.SendData("MOVE", &my, sizeof(int));
                return true;
            }
        }
    }
    else
        mouse_flag = false;
    return false;
}

// メイン処理
int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int)
{
    Novice::Initialize(kWindowTitle, 384, 444);
    networkManager.Initialize(isServer);
    
    int pieces[2];
    pieces[0] = Novice::LoadTexture("piece.png");
    pieces[1] = Novice::LoadTexture("piece2.png");

    board[3][3] = board[4][4] = 1;
    board[4][3] = board[3][4] = 2;
    SyncBoard();

    while (Novice::ProcessMessage() == 0) {
        Novice::BeginFrame();
        ReceiveData();
        GameLogicRoutine1(playerID);

        for (int y = 0; y < 8; y++)
            for (int x = 0; x < 8; x++) {
                if (board[y][x])
                    Novice::DrawSprite(x * 48, y * 48, pieces[board[y][x] - 1], 1.0f, 1.0f, 0.0f, WHITE);
            }
        
        Novice::EndFrame();
    }
    Novice::Finalize();
    return 0;
}
