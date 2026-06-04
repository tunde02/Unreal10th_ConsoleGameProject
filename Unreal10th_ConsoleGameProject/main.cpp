#include <iostream>
#include <vector>
#include <memory>
#include <algorithm>
#include <thread>
#include <chrono>
#include <conio.h>
#include <windows.h>

struct Vector2
{
    int X = 0;
    int Y = 0;

    Vector2() = default;
    Vector2(int InX, int InY) : X(InX), Y(InY) {}
    Vector2 operator+(const Vector2& other) const
    {
        Vector2 Result{};

        Result.X = this->X + other.X;
        Result.Y = this->Y + other.Y;

        return Result;
    }
    Vector2 operator-(const Vector2& other) const
    {
        Vector2 Result{};

        Result.X = this->X - other.X;
        Result.Y = this->Y - other.Y;

        return Result;
    }
    Vector2& operator=(const Vector2& other)
    {
        if (this == &other)
        {
            return *this;
        }

        this->X = other.X;
        this->Y = other.Y;

        return *this;
    }
};

struct Transform
{
    Vector2 Position{};
    Vector2 Delta{};
    int Width = 0;
    int Height = 0;

    Transform() = default;
};

// ==========================================
// 1. 최상위 게임 오브젝트 추상 클래스
// ==========================================
class GameObject
{
protected:
    Transform Transform_{};
    bool IsDestroyed_ = false; // 지연 삭제용 플래그
    float UpdatePeriod_ = 0.016f;
    float UpdateTimer_ = 0.0f;

public:
    std::vector<std::string> RenderString;

    virtual ~GameObject() = default;
    virtual void Update() = 0;
    virtual void Render() {}

    // 지연 삭제 상태 확인 및 설정 함수
    inline bool IsDestroyed() const { return IsDestroyed_; }
    inline void Destroy() { IsDestroyed_ = true; }
    inline Vector2 GetPosition() const { return Transform_.Position; }
    inline int GetWidth() const { return Transform_.Width; }
    inline int GetHeight() const { return Transform_.Height; }
};

// 샘플 오브젝트: 몬스터
class Monster : public GameObject
{
public:
    Monster()
    {
        //RenderString = "👾";
        //RenderString = "X";
        Transform_.Width = 3;
        Transform_.Height = 3;
        RenderString.reserve(Transform_.Width * Transform_.Height);
        for (int i = 0; i < Transform_.Height; i++)
        {
            for (int j = 0; j < Transform_.Width; j++)
            {
                RenderString.push_back("X");
            }
        }
        Transform_.Delta = Vector2{ 1, 0 };
    }

    void Update() override
    {
        UpdateTimer_ += 0.016f;
        if (UpdateTimer_ >= UpdatePeriod_)
        {
            TurnAround();
            UpdateTimer_ -= UpdatePeriod_;
        }
    }

    void TurnAround()
    {
        Vector2 NextPosition{ Transform_.Position + Transform_.Delta };
        if (NextPosition.X >= 120)
        {
            Transform_.Delta.X = -1;
            NextPosition = Vector2{ Transform_.Position + Transform_.Delta };
        }
        else if (NextPosition.X < 0)
        {
            Transform_.Delta.X = 1;
            NextPosition = Vector2{ Transform_.Position + Transform_.Delta };
        }

        Transform_.Position = NextPosition;
    }
};

class Player : public GameObject
{
public:
    Player()
    {
        //RenderString[0] = "OOO";
        Transform_.Width = 3;
        Transform_.Height = 3;
        RenderString.reserve(Transform_.Width * Transform_.Height);
        for (int i = 0; i < Transform_.Height; i++)
        {
            for (int j = 0; j < Transform_.Width; j++)
            {
                RenderString.push_back("O");
            }
        }
        Transform_.Position = Vector2{ 5, 5 };
    }

    void Update() override
    {
        Transform_.Delta.X = 0;
        Transform_.Delta.Y = 0;

        if (GetAsyncKeyState(VK_UP)) // ↑
        {
            Transform_.Delta.Y = -1;
        }
        else if (GetAsyncKeyState(VK_DOWN)) // ↓
        {
            Transform_.Delta.Y = 1;
        }

        if (GetAsyncKeyState(VK_LEFT)) // ←
        {
            Transform_.Delta.X = -1;
        }
        else if (GetAsyncKeyState(VK_RIGHT)) // →
        {
            Transform_.Delta.X = 1;
        }

        Vector2 NextPosition{ Transform_.Position + Transform_.Delta };
        if (0 <= NextPosition.X && NextPosition.X < 120
            && 0 <= NextPosition.Y && NextPosition.Y < 29)
        {
            NextPosition = Vector2{ Transform_.Position + Transform_.Delta };
            Transform_.Position = NextPosition;
        }
    }
};


// ==========================================
// 2. 씬 / 스테이지 관리를 위한 부모 추상 클래스
// ==========================================
class BaseScene
{
protected:
    // 각 씬은 자신만의 독립된 오브젝트 리스트를 관리함
    std::vector<GameObject*> SceneObjects;
    int Width = 120;
    int Height = 29;

public:
    virtual ~BaseScene()
    {
        printf("신 오브젝트 메모리 해제\n");
        for (GameObject* SceneObject : SceneObjects)
        {
            delete SceneObject;
            SceneObject = nullptr;
        }
        SceneObjects.clear();
    }

    virtual void Enter() = 0;  // 씬 시작 시 (데이터 로드, 배치)
    virtual void Exit() = 0;   // 씬 종료 시 (자원 해제)

    // 현재 씬에 속한 오브젝트들 업데이트 및 지연 삭제 처리
    virtual void Update() {
        // 1. 모든 오브젝트 로직 업데이트
        for (auto& obj : SceneObjects) {
            if (!obj->IsDestroyed()) {
                obj->Update();
            }
        }

        // 2. 지연 삭제 (Update 루프가 완전히 끝난 후 플래그가 켜진 오브젝트 일괄 제거)
        SceneObjects.erase(
            std::remove_if(SceneObjects.begin(), SceneObjects.end(),
                           [](const GameObject* obj) {
                               return obj->IsDestroyed();
                           }),
            SceneObjects.end()
        );
    }

    // 현재 씬의 오브젝트들 렌더링
    virtual void Render()
    {
        COORD Coord = { 0, 0 };
        SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), Coord);

        //std::string Screen{};
        //std::string* Screen = new std::string[Width * Height];
        std::vector<std::string> Screen(Width * Height, "□");
        for (size_t i = 0; i < Height; i++)
        {
            for (size_t j = 0; j < Width; j++)
            {
                Screen[i * Width + j] = "□";
            }
        }

        for (auto& obj : SceneObjects)
        {
            for (int i = 0; i < obj->GetHeight(); i++)
            {
                for (int j = 0; j < obj->GetWidth(); j++)
                {
                    int ScreenIndex = (obj->GetPosition().Y + i) * Width + (obj->GetPosition().Y + j);
                    int RenderStringIndex = i * obj->GetWidth() + j;
                    Screen[ScreenIndex] = obj->RenderString[RenderStringIndex];
                }
            }

            //int Index = obj->GetPosition().Y * Width + obj->GetPosition().X;
            //Screen[Index] = obj->RenderString.at(0);
        }

        std::string Buffer{};
        for (size_t i = 0; i < Height; i++)
        {
            for (size_t j = 0; j < Width; j++)
            {
                Buffer += Screen[i * Width + j];
                //printf("%s", Screen[i * Width + j].c_str());
            }
            Buffer += "\n";
        }

        // TODO: Buffer 문자열에 잘못된 문자가 있습니다 오류
        //printf("%s", Buffer.c_str()); 
        //delete[] Screen;
        //Screen = nullptr;
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
        //SceneObjects.push_back(new Player());
        SceneObjects.push_back(new Monster());
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

    void SetConsoleSize(int Width, int Height)
    {
        HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
        HWND hwnd = GetConsoleWindow();

        // 1. 창 크기를 1920x1080 픽셀로 설정 (모니터 좌상단 0,0 위치)
        // SWP_NOZORDER: 순서 유지, SWP_NOMOVE: 위치 유지(0,0으로 설정했으므로 생략가능)
        SetWindowPos(hwnd, NULL, 0, 0, Width, Height, SWP_NOZORDER);

        // 2. 사용자가 창 크기를 조절하지 못하도록 고정
        LONG style = GetWindowLong(hwnd, GWL_STYLE);
        style &= ~WS_MAXIMIZEBOX; // 최대화 금지
        style &= ~WS_THICKFRAME;  // 드래그로 크기 조절 금지
        SetWindowLong(hwnd, GWL_STYLE, style);

        // 3. 현재 적용된 폰트 정보 가져오기
        CONSOLE_FONT_INFO cfi;
        GetCurrentConsoleFont(hOut, FALSE, &cfi);

        // 4. 실제 출력 가능한 내부 영역(Client Area) 크기 계산
        // 테두리와 타이틀 바를 제외한 실제 '글자가 찍히는 공간'의 픽셀 크기를 구합니다.
        RECT rect;
        GetClientRect(hwnd, &rect);
        int clientWidth = rect.right - rect.left;
        int clientHeight = rect.bottom - rect.top;

        // 5. 글자 수 계산 (픽셀 / 폰트 크기)
        int cols = clientWidth / cfi.dwFontSize.Y;
        int lines = clientHeight / cfi.dwFontSize.Y;

        // 6. 결과 출력
        std::cout << "===============================" << std::endl;
        std::cout << "창 크기: 1920 x 1080 px (고정됨)" << std::endl;
        std::cout << "폰트 크기: " << cfi.dwFontSize.X << " x " << cfi.dwFontSize.Y << " px" << std::endl;
        std::cout << "가로 최대 글자 수 (Cols): " << cols << std::endl;
        std::cout << "세로 최대 줄 수 (Lines): " << lines << std::endl;
        std::cout << "===============================" << std::endl;
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
        SetConsoleSize(1920, 1080);

        HideCursor();
        system("cls");

        while (true)
        {
            Update();
            Render();

            std::this_thread::sleep_for(std::chrono::milliseconds(66));
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
    //GameEngine::Instance().ChangeScene(new MenuScene());
    GameEngine::Instance().ChangeScene(new Stage1Scene());

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
