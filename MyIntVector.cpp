#include <iostream>
#include <vector>

class MyIntVector {
private:
    std::vector<int> vec;

public:
    // コンストラクタ
    MyIntVector() = default;
    MyIntVector(std::initializer_list<int> list) : vec(list) {}

    // コピー代入演算子（MyIntVector = MyIntVector）
    MyIntVector& operator=(const MyIntVector& other) {
        if (this != &other) {
            vec = other.vec;
        }
        return *this;
    }

    // ムーブ代入演算子（MyIntVector = std::move(MyIntVector)）
    MyIntVector& operator=(MyIntVector&& other) noexcept {
        if (this != &other) {
            vec = std::move(other.vec);
        }
        return *this;
    }

    // std::vector<int> からの代入
    MyIntVector& operator=(const std::vector<int>& other) {
        vec = other;
        return *this;
    }

    // ラッパー関数
    size_t size() const { return vec.size(); }
    int* data() { return vec.data(); }
    const int* data() const { return vec.data(); }

    bool empty() const { return vec.empty(); }
    void push_back(int value) { vec.push_back(value); }
    void pop_back() { vec.pop_back(); }

    int& operator[](size_t index) { return vec[index]; }
    const int& operator[](size_t index) const { return vec[index]; }

    auto begin() { return vec.begin(); }
    auto end() { return vec.end(); }
    auto begin() const { return vec.begin(); }
    auto end() const { return vec.end(); }

    // 暗黙変換
    operator std::vector<int>& () { return vec; }
    operator const std::vector<int>& () const { return vec; }
};






int main() {
    MyIntVector a = { 1, 2, 3 };
    MyIntVector b;
    std::vector<int> c = { 10, 20, 30 };

    b = a;       // MyIntVector = MyIntVector
    b = c;       // MyIntVector = std::vector<int>

    for (int x : b) {
        std::cout << x << " ";
    }
    std::cout << std::endl;
}
