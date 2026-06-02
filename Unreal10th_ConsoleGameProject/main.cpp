#include <iostream>
#include <vector>
#include <memory>
#include <algorithm>
#include <thread>
#include <chrono>
#include <conio.h>
#include <windows.h>

// ==========================================
// 1. 최상위 게임 오브젝트 추상 클래스
// ==========================================
class GameObject {
protected:
    bool isDestroyed = false; // 지연 삭제용 플래그

public:
    virtual ~GameObject() = default;
    virtual void Update() = 0;
    virtual void Render() = 0;

    // 지연 삭제 상태 확인 및 설정 함수
    bool IsDestroyed() const { return isDestroyed; }
    void Destroy() { isDestroyed = true; }
};

// 샘플 오브젝트: 몬스터
class Monster : public GameObject {
public:
    void Update() override {
        // 게임 로직 처리... (예: 체력이 0이 되면 스스로 파괴 요청)
        // Destroy(); 
    }
    void Render() override {
        std::cout << "  👾 [Monster] 화면에 그려짐" << std::endl;
    }
};


// ==========================================
// 2. 씬 / 스테이지 관리를 위한 부모 추상 클래스
// ==========================================
class BaseScene {
protected:
    // 각 씬은 자신만의 독립된 오브젝트 리스트를 관리함
    std::vector<GameObject*> sceneObjects;

public:
    virtual ~BaseScene()
    {
        for (GameObject* SceneObject : sceneObjects)
        {
            delete SceneObject;
            SceneObject = nullptr;
        }
        sceneObjects.clear();
    }

    virtual void Enter() = 0;  // 씬 시작 시 (데이터 로드, 배치)
    virtual void Exit() = 0;   // 씬 종료 시 (자원 해제)

    // 현재 씬에 속한 오브젝트들 업데이트 및 지연 삭제 처리
    virtual void Update() {
        // 1. 모든 오브젝트 로직 업데이트
        for (auto& obj : sceneObjects) {
            if (!obj->IsDestroyed()) {
                obj->Update();
            }
        }

        // 2. 지연 삭제 (Update 루프가 완전히 끝난 후 플래그가 켜진 오브젝트 일괄 제거)
        sceneObjects.erase(
            std::remove_if(sceneObjects.begin(), sceneObjects.end(),
                [](const GameObject* obj) {
                    return obj->IsDestroyed();
                }),
            sceneObjects.end()
        );
    }

    // 현재 씬의 오브젝트들 렌더링
    virtual void Render() {
        for (auto& obj : sceneObjects) {
            obj->Render();
        }
    }
};


// ==========================================
// 3. 구체적인 씬 구현 (메뉴 씬 & Stage 1 씬)
// ==========================================
class MenuScene : public BaseScene {
private:
    int CurrentMenuIndex = 0;

public:
    void Enter() override { std::cout << "▶ [MenuScene] 진입: 타이틀 메뉴 생성" << std::endl; }
    void Exit() override { std::cout << "◀ [MenuScene] 탈출: 메뉴 리소스 해제" << std::endl; }

    void Update() override
    {
        if (_kbhit())
        {
            int Key = _getch();
            if (Key == 72) // ↑
            {
                CurrentMenuIndex--;
                if (CurrentMenuIndex < 0)
                {
                    CurrentMenuIndex = 2;
                }
            }
            else if (Key == 80) // ↓
            {
                CurrentMenuIndex++;
                if (CurrentMenuIndex > 2)
                {
                    CurrentMenuIndex = 0;
                }
            }
        }
    }

    void Render() override
    {
        std::cout << "==[MenuScene]==" << std::endl;
        for (int i = 0; i < 3; i++)
        {
            if (CurrentMenuIndex == i)
            {
                printf("[MenuScene] 메뉴 %d  ◀\n", i + 1);
            }
            else
            {
                printf("[MenuScene] 메뉴 %d----\n", i + 1);
            }
        }
    }
};

class Stage1Scene : public BaseScene {
public:
    void Enter() override {
        std::cout << "▶ [Stage1Scene] 진입: 1스테이지 맵 및 몬스터 생성" << std::endl;
        // Stage 1 전용 오브젝트 배치
        //sceneObjects.push_back(std::make_shared<Monster>());
        sceneObjects.push_back(new Monster());
    }
    void Exit() override {
        std::cout << "◀ [Stage1Scene] 탈출: 1스테이지 데이터 메모리 자동 소멸" << std::endl;
    }
};


// ==========================================
// 4. 싱글톤 기반 게임 엔진 클래스
// ==========================================
class GameEngine {
private:
    BaseScene* currentScene = nullptr; // 현재 활성화된 씬 포인터

    // 싱글톤 핵심: 생성자를 private으로 감춰 외부 선언 및 new 차단
    GameEngine() = default;

    void SetCursorPosition(int X, int Y)
    {
        COORD Coord = { (SHORT)X, (SHORT)Y };
        SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), Coord);
    }

    void HideCursor()
    {
        HANDLE ConsoleHandle = GetStdHandle(STD_OUTPUT_HANDLE);
        CONSOLE_CURSOR_INFO Info{};
        Info.dwSize = 100;
        Info.bVisible = FALSE;
        SetConsoleCursorInfo(ConsoleHandle, &Info);
    }

public:
    ~GameEngine()
    {
        delete currentScene;
        currentScene = nullptr;
    }

    // 외부 복사 및 대입 원천 차단
    GameEngine(const GameEngine&) = delete;
    GameEngine& operator=(const GameEngine&) = delete;

    // 전역 어디서나 단 하나의 원본 객체에 접근할 수 있는 통로 (Meyers' Singleton)
    static GameEngine& Instance() {
        static GameEngine instance; // 함수 내부 정적 변수: 프로그램 라이프사이클 동안 딱 '한 번'만 생성
        return instance;
    }

    void Run()
    {
        HideCursor();
        system("cls");

        while (true)
        {
            Update();
            Render();

            std::this_thread::sleep_for(std::chrono::milliseconds(16));
        }
    }

    // 씬 전환 핵심 함수 (이전 씬 안전하게 해제 후 새 씬 교체)
    void ChangeScene(BaseScene* newScene) {
        if (currentScene) {
            currentScene->Exit();
        }
        currentScene = newScene;
        if (currentScene) {
            currentScene->Enter();
        }
    }

    // 메인 게임 루프에서 호출되는 Update/Render 대행
    void Update() {
        if (currentScene) currentScene->Update();
    }

    void Render() {
        if (currentScene)
        {
            SetCursorPosition(0, 0);
            currentScene->Render();
        }
    }
};


// ==========================================
// 5. 엔트리 포인트 (실행 흐름)
// ==========================================
int main() {
    GameEngine::Instance().ChangeScene(new MenuScene());

    GameEngine::Instance().Run();

    // 1. 엔진 원본을 가져와 최초 메뉴 씬으로 시작
    //GameEngine::Instance().ChangeScene(new MenuScene());
    //GameEngine::Instance().Update();
    //GameEngine::Instance().Render();

    //std::cout << "\n--- 유저가 게임 시작 버튼을 누름 ---\n" << std::endl;

    //// 2. 플레이 중 스테이지 1 씬으로 전격 전환
    //GameEngine::Instance().ChangeScene(new Stage1Scene());
    //GameEngine::Instance().Update();
    //GameEngine::Instance().Render();

    //std::cout << "\n=== 게임 프로그램 종료 ===" << std::endl;

    return 0;
}
