#include <iostream>
#include <vector>
#include <set>
#include <string>

// 盤面を二次元ベクトルで表す
using Field = std::vector<std::vector<int>>;

// すべての値
const std::set<int> all_numbers({1, 2, 3, 4, 5, 6, 7, 8, 9});

// 数独を解くためのクラス
class Sudoku {
 private:
    // 数独盤面
    Field field;

    // nums[x][y][v] ← マス x, y) を含む行・列・ブロックに値 v+1 が何個あるか
    std::vector<std::vector<std::vector<int>>> nums;

    // choices[x][y] ← マス (x, y) に入れることのできる選択肢
    std::vector<std::vector<std::set<int>>> choices;
    
 public:
    // コンストラクタ (未確定マスの値を -1 で表す)
    Sudoku() : field(9, std::vector<int>(9, -1)),
               nums(9, std::vector<std::vector<int>>(9, std::vector<int>(9, 0))),
               choices(9, std::vector<std::set<int>>(9, all_numbers)) {}

    // filed を返す
    const Field& get() {
        return field;
    }

    // 空きマスのうち、選択肢が最も少ないマスを探す
    bool find_empty(int &x, int &y);

    // マス (x, y) に入れられる選択肢を返す
    std::set<int> find_choices(int x, int y) {
        return choices[x][y];
    }

    // マス (x, y) に数値 val を入れることによるマス (x2, y2) への影響
    void put_detail(int x, int y, int val, int x2, int y2);

    // マス (x, y) から数値 val を除去したことによるマス (x2, y2) への影響
    void reset_detail(int x, int y, int val, int x2, int y2);
    
    // マス (x, y) に数値 val を入れる
    void put(int x, int y, int val);

    // マス (x, y) の数値を削除する
    void reset(int x, int y);

    // 一意に決まるマスを埋めていく
    void process();

    void print() {
        for (int x = 0; x < 9; ++x) {
            for (int y = 0; y < 9; ++y) {
                if (field[x][y] == -1) std::cout << "*";
                else std::cout << field[x][y];
                std::cout << " ";
            }
            std::cout << std::endl;
        }
    }
};

// 空きマスのうち、選択肢が最も少ないマスを探す
bool Sudoku::find_empty(int &x, int &y) {
    size_t min_num_choices = 10;
    for (int i = 0; i < 9; ++i) {
        for (int j = 0; j < 9; ++j) {
            if (field[i][j] != -1) continue;
            if (min_num_choices > choices[i][j].size()) {
                min_num_choices = choices[i][j].size();
                x = i;
                y = j;
            }
        }
    }
    // 存在しない場合は false
    if (min_num_choices == 10) return false;
    else return true;

}

// マス (x, y) に数値 val を入れることによるマス (x2, y2) への影響
void Sudoku::put_detail(int x, int y, int val, int x2, int y2) {
    if (x == x2 && y == y2) return;
    
    // (x2, y2) にすでに値が入っている場合は何もしない
    if (field[x2][y2] != -1) return;
    
    // それまで (x2, y2) の影響範囲に値 val がなかった場合は選択肢から除く
    if (nums[x2][y2][val - 1] == 0) choices[x2][y2].erase(val);
    
    // nums を更新
    ++nums[x2][y2][val - 1];
}

// マス (x, y) に数値 val を入れる
void Sudoku::put(int x, int y, int val) {
    // 数値を入れる
    field[x][y] = val;

    // マス (x, y) を含む行・列・ブロックへの影響を更新する
    for (int i = 0; i < 9; ++i) put_detail(x, y, val, x, i);
    for (int i = 0; i < 9; ++i) put_detail(x, y, val, i, y);
    int cx = x / 3 * 3 + 1, cy = y / 3 * 3 + 1;
    for (int i = cx - 1; i <= cx + 1; ++i)
        for (int j = cy - 1; j <= cy + 1; ++j)
            put_detail(x, y, val, i, j);      
}

// マス (x, y) から数値 val を除去したことによるマス (x2, y2) への影響
void Sudoku::reset_detail(int x, int y, int val, int x2, int y2) {
    if (x == x2 && y == y2) return;
    
    // (x2, y2) にすでに値が入っている場合は何もしない
    if (field[x2][y2] != -1) return;
    
    // nums を更新
    --nums[x2][y2][val - 1];
    
    // nums が 0 になる場合は選択肢に復活する
    if (nums[x2][y2][val - 1] == 0) choices[x2][y2].insert(val);
}

// マス (x, y) の数値を削除する
void Sudoku::reset(int x, int y) {
    // マス (x, y) を含む行・列・ブロックへの影響を更新する
    int val = field[x][y]; 
    for (int i = 0; i < 9; ++i) reset_detail(x, y, val, x, i);
    for (int i = 0; i < 9; ++i) reset_detail(x, y, val, i, y);
    int cx = x / 3 * 3 + 1, cy = y / 3 * 3 + 1;
    for (int i = cx - 1; i <= cx + 1; ++i)
        for (int j = cy - 1; j <= cy + 1; ++j)
            reset_detail(x, y, val, i, j);

    // 数値を除去する
    field[x][y] = -1;
}

// 一意に決まるマスを埋めていく
void Sudoku::process() {
    // 数値 1, 2, ..., 9 について順に処理する
    for (int val = 1; val <= 9; ++val) {
        // x 行目について
        for (int x = 0; x < 9; ++x) {
            bool exist = false;
            std::vector<int> can_enter;
            for (int y = 0; y < 9; ++y) {
                if (field[x][y] == val) exist = true;
                if (field[x][y] == -1
                    && choices[x][y].count(val)) {
                    can_enter.push_back(y);
                }
            }
            // val を入れられるマス目がただ一つならば入れる
            if (!exist && can_enter.size() == 1) {
                int y = can_enter[0];
                put(x, y, val);
            }
        }

        // y 列目について
        for (int y = 0; y < 9; ++y) {
            bool exist = false;
            std::vector<int> can_enter;
            for (int x = 0; x < 9; ++x) {
                if (field[x][y] == val) exist = true;
                if (field[x][y] == -1
                    && choices[x][y].count(val)) {
                    can_enter.push_back(x);
                }
            }
            // val を入れられるマス目がただ一つならば入れる
            if (!exist && can_enter.size() == 1) {
                int x = can_enter[0];
                put(x, y, val);
            }
        }

        // 各ブロックについて
        for (int bx = 0; bx < 3; ++bx) {
            for (int by = 0; by < 3; ++by) {
                bool exist = false;
                std::vector<std::pair<int,int>> can_enter;
                for (int x = bx * 3; x < (bx + 1) * 3; ++x) {
                    for (int y = by * 3; y < (by + 1) * 3; ++y) {
                        if (field[x][y] == val) exist = true;
                        if (field[x][y] == -1
                            && choices[x][y].count(val)) {
                            can_enter.emplace_back(x, y);
                        }
                    }
                }
                // val を入れられるマス目がただ一つならば入れる
                if (!exist && can_enter.size() == 1) {
                    int x = can_enter[0].first;
                    int y = can_enter[0].second;
                    put(x, y, val);
                }
            }
        }
    }
}

// 数独を解く
void dfs(Sudoku &board, std::vector<Field> &res) {
    // 数独の盤面状態を保持しておく
    Sudoku board_prev = board;
    
    // 一意に自動的に決まるマスを埋める
    board.process();
    
    // 空きマスの座標を表す
    int x, y;

    // 終端条件を処理し、同時に空きマスを探す
    if (!board.find_empty(x, y)) {
        // 解に追加
        res.push_back(board.get());

        // リターンする前に一回元に戻す
        board = board_prev;
        return;
    }

    // マス (x, y) に入れられる数値の集合を求める
    const auto &can_use = board.find_choices(x, y);

    // バックトラッキング
    for (auto val : can_use) {
        board.put(x, y, val);
        dfs(board, res);
        board.reset(x, y);
    }

    // 元に戻す
    board = board_prev;
}

int main() {
    // 数独を入力する
    Sudoku board;
    for (int x = 0; x < 9; ++x) {
        std::string line;
        std::cin >> line;
        for (int y = 0; y < 9; ++y) {
            // 空マスの場合は何もしない
            if (line[y] == '*') continue;

            // 数値情報に変換する
            int val = line[y] - '0';
            board.put(x, y, val);
        }
    }
    
    // 数独を解く
    std::vector<Field> results;
    dfs(board, results);

    // 解を出力する
    if (results.size() == 0) {
        std::cout << "no solutions." << std::endl;
    } else if (results.size() > 1) {
        std::cout << "more than one solutions." << std::endl;
    } else {
        const Field &result = results[0];
        for (int x = 0; x < 9; ++x) {
            for (int y = 0; y < 9; ++y) {
                std::cout << result[x][y] << " ";
            }
            std::cout << std::endl;
        }
    }
}
