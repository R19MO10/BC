#include <iostream>
#include <vector>

class MyIntVector {
private:
    std::vector<int> vec;

public:
    // �R���X�g���N�^
    MyIntVector() = default;
    MyIntVector(std::initializer_list<int> list) : vec(list) {}

    // �R�s�[������Z�q�iMyIntVector = MyIntVector�j
    MyIntVector& operator=(const MyIntVector& other) {
        if (this != &other) {
            vec = other.vec;
        }
        return *this;
    }

    // ���[�u������Z�q�iMyIntVector = std::move(MyIntVector)�j
    MyIntVector& operator=(MyIntVector&& other) noexcept {
        if (this != &other) {
            vec = std::move(other.vec);
        }
        return *this;
    }

    // std::vector<int> ����̑��
    MyIntVector& operator=(const std::vector<int>& other) {
        vec = other;
        return *this;
    }

    // ���b�p�[�֐�
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

    // �Öٕϊ�
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
