#pragma once

class GameEngine
{
private:
    GameEngine()
    {
        std::cout << "⚙️ 엔진 생성자가 호출되었습니다! (딱 한 번만 보여야 함)" << std::endl;
    }
public:
    static GameEngine& GetInstance()
    {
        static GameEngine instance;
        return instance;
    }
    void Run() { std::cout << "🎮 게임 루프 실행 중..." << std::endl; }
};