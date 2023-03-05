#pragma once

#include <random>
#include <string>
#include <unordered_set>
#include <vector>

class Minesweeper {
public:
    struct Cell {
        size_t x = 0;
        size_t y = 0;

        bool operator==(const Cell &other) const noexcept;
    };

    struct CellHash {
        size_t operator()(const Cell &cell) const noexcept;
    };

    enum class GameStatus {
        NOT_STARTED,
        IN_PROGRESS,
        VICTORY,
        DEFEAT,
    };

    using RenderedField = std::vector<std::string>;

    Minesweeper(size_t width, size_t height, size_t mines_count);

    Minesweeper(size_t width, size_t height, const std::vector<Cell> &cells_with_mines);

    void NewGame(size_t width, size_t height, size_t mines_count);

    void NewGame(size_t width, size_t height, const std::vector<Cell> &cells_with_mines);

    void OpenCell(const Cell &cell);

    void MarkCell(const Cell &cell);

    GameStatus GetGameStatus() const noexcept;

    time_t GetGameTime() const noexcept;

    RenderedField RenderField() const;

private:
    size_t width_{0};
    size_t height_{0};
    time_t start_time_{0};
    time_t finish_time_{0};
    GameStatus status_{GameStatus::NOT_STARTED};
    std::unordered_set<Cell, CellHash> cells_with_mines_;
    std::unordered_set<Cell, CellHash> marked_cells_;
    std::unordered_set<Cell, CellHash> closed_cells_;

    std::mt19937 gen_ = std::mt19937(std::random_device()());

    void FieldDefinition(size_t mines_count);

    void FieldDefinition(const std::vector<Cell> &cells_with_mines);

    void ResetValues() noexcept;

    void SetNewBoundary(size_t width, size_t height) noexcept;

    void FillClosed() noexcept;

    void FillMines(size_t mines_count) noexcept;

    void StartGame() noexcept;

    void Defeat() noexcept;

    void VictoryCheck() noexcept;

    bool IsCorrectBoundary(const Cell &cell) const noexcept;

    bool IsMine(const Cell &cell) const noexcept;

    bool IsMarked(const Cell &cell) const noexcept;

    bool IsClosed(const Cell &cell) const noexcept;

    bool IsOpened(const Cell &cell) const noexcept;

    bool IsFinishedGame() const noexcept;

    size_t CalcMinesNear(const Cell &cell) const noexcept;
};
