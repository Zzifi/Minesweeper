#include "minesweeper.h"

#include <ctime>
#include <queue>
#include <stdexcept>
#include <tuple>

bool Minesweeper::Cell::operator==(const Cell &other) const noexcept {
    return std::tie(x, y) == std::tie(other.x, other.y);
}

size_t Minesweeper::CellHash::operator()(const Cell &cell) const noexcept {
    const size_t val = 1000000;
    return cell.x * val + cell.y;
}

void Minesweeper::FieldDefinition(size_t mines_count) {
    if (mines_count > height_ * width_) {
        throw std::runtime_error("Too many mines");
    }
    FillMines(mines_count);
    FillClosed();
}

void Minesweeper::FieldDefinition(const std::vector<Cell> &cells_with_mines) {
    if (cells_with_mines.size() > height_ * width_) {
        throw std::runtime_error("Too many mines");
    }
    for (const auto &cell: cells_with_mines) {
        if (!IsCorrectBoundary(cell)) {
            throw std::runtime_error("Incorrect mine position");
        }
    }
    std::copy(cells_with_mines.begin(), cells_with_mines.end(),
              std::inserter(cells_with_mines_, cells_with_mines_.begin()));
    FillClosed();
}

Minesweeper::Minesweeper(size_t width, size_t height, const std::vector<Cell> &cells_with_mines)
        : width_(width), height_(height),
          cells_with_mines_(cells_with_mines.begin(), cells_with_mines.end()) {
    FieldDefinition(cells_with_mines);
}

Minesweeper::Minesweeper(size_t width, size_t height, size_t mines_count) : width_(width),
                                                                            height_(height) {
    FieldDefinition(mines_count);
}

void Minesweeper::NewGame(size_t width, size_t height, const std::vector<Cell> &cells_with_mines) {
    ResetValues();
    SetNewBoundary(width, height);
    FieldDefinition(cells_with_mines);
}

void Minesweeper::NewGame(size_t width, size_t height, size_t mines_count) {
    ResetValues();
    SetNewBoundary(width, height);
    FieldDefinition(mines_count);
}

void Minesweeper::ResetValues() noexcept {
    width_ = 0;
    height_ = 0;
    start_time_ = 0;
    finish_time_ = 0;
    status_ = GameStatus::NOT_STARTED;
    marked_cells_.clear();
    marked_cells_.rehash(0);
    closed_cells_.clear();
    closed_cells_.rehash(0);
    cells_with_mines_.clear();
    cells_with_mines_.rehash(0);
}

void Minesweeper::SetNewBoundary(size_t width, size_t height) noexcept {
    width_ = width;
    height_ = height;
}

void Minesweeper::StartGame() noexcept {
    status_ = GameStatus::IN_PROGRESS;
    start_time_ = std::time(NULL);
}

void Minesweeper::Defeat() noexcept {
    status_ = GameStatus::DEFEAT;
    finish_time_ = std::time(NULL);
}

void Minesweeper::VictoryCheck() noexcept {
    if (closed_cells_.size() != cells_with_mines_.size()) {
        return;
    }
    status_ = GameStatus::VICTORY;
    finish_time_ = std::time(NULL);
}

bool Minesweeper::IsCorrectBoundary(const Cell &cell) const noexcept {
    return (0 <= cell.x && cell.x < width_ && 0 <= cell.y && cell.y < height_);
}

bool Minesweeper::IsMine(const Cell &cell) const noexcept {
    return cells_with_mines_.contains(cell);
}

bool Minesweeper::IsMarked(const Cell &cell) const noexcept {
    return marked_cells_.contains(cell);
}

bool Minesweeper::IsClosed(const Cell &cell) const noexcept {
    return closed_cells_.contains(cell);
}

bool Minesweeper::IsOpened(const Cell &cell) const noexcept {
    return !IsClosed(cell);
}

bool Minesweeper::IsFinishedGame() const noexcept {
    return (status_ == GameStatus::VICTORY || status_ == GameStatus::DEFEAT);
}

size_t Minesweeper::CalcMinesNear(const Cell &cell) const noexcept {
    size_t result = 0;
    for (int i = -1; i <= 1; ++i) {
        for (int j = -1; j <= 1; ++j) {
            Cell neighbor_cell{.x = cell.x + i, .y = cell.y + j};
            if (IsCorrectBoundary(neighbor_cell) && IsMine(neighbor_cell)) {
                ++result;
            }
        }
    }
    return result;
}

void Minesweeper::FillClosed() noexcept {
    closed_cells_.reserve(height_ * width_);
    for (size_t y = 0; y < height_; ++y) {
        for (size_t x = 0; x < width_; ++x) {
            closed_cells_.insert(Cell{.x = x, .y = y});
        }
    }
}

void Minesweeper::FillMines(size_t mines_count) noexcept {
    if (!mines_count) {
        return;
    }
    std::vector<Cell> possible_mine;
    possible_mine.reserve(height_ * width_);
    for (size_t x = 0; x < width_; ++x) {
        for (size_t y = 0; y < height_; ++y) {
            possible_mine.push_back(Cell{.x = x, .y = y});
        }
    }
    std::shuffle(possible_mine.begin(), possible_mine.end(), gen_);
    std::copy(possible_mine.begin(), possible_mine.begin() + static_cast<int64_t>(mines_count),
              std::inserter(cells_with_mines_, cells_with_mines_.begin()));
}

void Minesweeper::MarkCell(const Cell &cell) {
    if (!IsCorrectBoundary(cell)) {
        throw std::runtime_error("A cell outside the field boundary");
    }
    if (IsFinishedGame()) {
        return;
    }
    if (status_ == GameStatus::NOT_STARTED) {
        StartGame();
    }
    if (Minesweeper::IsMarked(cell)) {
        marked_cells_.erase(cell);
    } else {
        marked_cells_.insert(cell);
    }
}

void Minesweeper::OpenCell(const Cell &cell) {
    if (!IsCorrectBoundary(cell)) {
        throw std::runtime_error("A cell outside the field boundary");
    }
    if (IsFinishedGame() || IsMarked(cell) || IsOpened(cell)) {
        return;
    }
    if (status_ == GameStatus::NOT_STARTED) {
        StartGame();
    }
    if (IsMine(cell)) {
        Defeat();
        return;
    }
    std::queue<Cell> q;
    q.push(cell);
    closed_cells_.erase(cell);
    while (!q.empty()) {
        Cell cur_cell = q.front();
        q.pop();
        size_t mines_near = CalcMinesNear(cur_cell);
        if (!mines_near) {
            for (int i = -1; i <= 1; ++i) {
                for (int j = -1; j <= 1; ++j) {
                    Cell neighbor_cell{.x = cur_cell.x + i, .y = cur_cell.y + j};
                    if (IsCorrectBoundary(neighbor_cell) && IsClosed(neighbor_cell) &&
                        !IsMarked(neighbor_cell)) {
                        q.push(neighbor_cell);
                        closed_cells_.erase(neighbor_cell);
                    }
                }
            }
        }
    }
    VictoryCheck();
}

Minesweeper::GameStatus Minesweeper::GetGameStatus() const noexcept {
    return status_;
}

time_t Minesweeper::GetGameTime() const noexcept {
    if (status_ == GameStatus::NOT_STARTED) {
        return 0;
    }
    if (status_ == GameStatus::IN_PROGRESS) {
        return std::time(NULL) - start_time_;
    }
    return finish_time_ - start_time_;
}

Minesweeper::RenderedField Minesweeper::RenderField() const {
    RenderedField result(height_);
    for (size_t y = 0; y < height_; ++y) {
        result[y].reserve(width_);
        for (size_t x = 0; x < width_; ++x) {
            Cell cell{.x = x, .y = y};
            if (IsMine(cell) && status_ == GameStatus::DEFEAT) {
                result[y] += "*";
            } else if (IsMarked(cell)) {
                result[y] += "?";
            } else if (IsClosed(cell)) {
                result[y] += "-";
            } else {
                size_t mines = CalcMinesNear(cell);
                result[y] += (mines ? (std::to_string(mines)) : ".");
            }
        }
    }
    return result;
}
