#include <vector>
#include <cstdint>
#include <algorithm>
#include <cmath>
#include <limits>

struct Color32 {
    uint8_t r, g, b, a; // �ԁE�΁E�E�A���t�@�l�i���߁j
};

// �e�F�{�b�N�X��ێ�����\����
struct ColorBox {
    std::vector<Color32> colors; // ���̃{�b�N�X���̐F
    int rMin, rMax, gMin, gMax, bMin, bMax; // RGB�̍ŏ��E�ő�l�i�{�b�N�X�̃T�C�Y�����߂�j
};

// 2�F�Ԃ̃��[�N���b�h������2����v�Z�i��r�p�j
int colorDistanceSquared(const Color32& c1, const Color32& c2) {
    int dr = int(c1.r) - int(c2.r);
    int dg = int(c1.g) - int(c2.g);
    int db = int(c1.b) - int(c2.b);
    return dr * dr + dg * dg + db * db;
}

// �^����ꂽ�F�̏W���̕��ϐF�����߂�i�e�����̍��v�������j
Color32 averageColor(const std::vector<Color32>& colors) {
    uint64_t r = 0, g = 0, b = 0, a = 0;
    for (const auto& c : colors) {
        r += c.r;
        g += c.g;
        b += c.b;
        a += c.a;
    }
    size_t count = colors.size();
    return {
        static_cast<uint8_t>(r / count),
        static_cast<uint8_t>(g / count),
        static_cast<uint8_t>(b / count),
        static_cast<uint8_t>(a / count)
    };
}

// �{�b�N�X��RGB�͈͂��X�V�i�ő�/�ŏ��̍Čv�Z�j
void updateBox(ColorBox& box) {
    // �����l��1�ڂ̐F��ݒ�
    box.rMin = box.rMax = box.colors[0].r;
    box.gMin = box.gMax = box.colors[0].g;
    box.bMin = box.bMax = box.colors[0].b;

    // �F���Ƃɍŏ��E�ő���X�V
    for (const auto& c : box.colors) {
        box.rMin = std::min(box.rMin, (int)c.r);
        box.rMax = std::max(box.rMax, (int)c.r);
        box.gMin = std::min(box.gMin, (int)c.g);
        box.gMax = std::max(box.gMax, (int)c.g);
        box.bMin = std::min(box.bMin, (int)c.b);
        box.bMax = std::max(box.bMax, (int)c.b);
    }
}

// ���f�B�A���J�b�g���F�A���S���Y���{��
void medianCut(std::vector<Color32>& pix, int w, int h, uint8_t maxColor) {
    std::vector<ColorBox> boxes;

    // �����{�b�N�X�ɂ͂��ׂĂ̐F������
    ColorBox initialBox{ pix };
    updateBox(initialBox); // �����{�b�N�X�͈̔͂��v�Z
    boxes.push_back(initialBox);

    // �F����maxColor�ɒB����܂ŕ������J��Ԃ�
    while (boxes.size() < maxColor) {
        // �������ׂ��ł��u�L���v�{�b�N�X��T��
        auto it = std::max_element(boxes.begin(), boxes.end(), [](const ColorBox& a, const ColorBox& b) {
            int rangeA = std::max({ a.rMax - a.rMin, a.gMax - a.gMin, a.bMax - a.bMin });
            int rangeB = std::max({ b.rMax - b.rMin, b.gMax - b.gMin, b.bMax - b.bMin });
            return rangeA < rangeB; // �͈͂��L���ق���D��
            });

        // �����s�\�Ȃ�I���i�F��1�����Ȃ����j
        if (it == boxes.end() || it->colors.size() <= 1)
            break;

        ColorBox box = *it;         // �����Ώۂ̃{�b�N�X�����o��
        boxes.erase(it);            // ���̃��X�g����폜

        // RGB�����̂ǂ�����ɕ������邩���߂�i�ő�͈͂̐����j
        int rRange = box.rMax - box.rMin;
        int gRange = box.gMax - box.gMin;
        int bRange = box.bMax - box.bMin;

        if (rRange >= gRange && rRange >= bRange) {
            std::sort(box.colors.begin(), box.colors.end(),
                [](const Color32& a, const Color32& b) { return a.r < b.r; });
        }
        else if (gRange >= rRange && gRange >= bRange) {
            std::sort(box.colors.begin(), box.colors.end(),
                [](const Color32& a, const Color32& b) { return a.g < b.g; });
        }
        else {
            std::sort(box.colors.begin(), box.colors.end(),
                [](const Color32& a, const Color32& b) { return a.b < b.b; });
        }

        // �\�[�g�����F��2�����i�����l�ŕ�����j
        size_t mid = box.colors.size() / 2;
        ColorBox box1{ std::vector<Color32>(box.colors.begin(), box.colors.begin() + mid) };
        ColorBox box2{ std::vector<Color32>(box.colors.begin() + mid, box.colors.end()) };

        updateBox(box1); // ������̃{�b�N�X�͈̔͂��X�V
        updateBox(box2);

        // �V����2�̃{�b�N�X��ǉ�
        boxes.push_back(box1);
        boxes.push_back(box2);
    }

    // �{�b�N�X���Ƃ̕��ϐF���g���ăp���b�g�쐬
    std::vector<Color32> palette;
    for (const auto& box : boxes) {
        palette.push_back(averageColor(box.colors));
    }

    // �S�s�N�Z�����p���b�g�̍ŋߖT�F�ɕϊ�
    for (auto& p : pix) {
        int minDist = std::numeric_limits<int>::max();
        Color32 nearest = palette[0];
        for (const auto& c : palette) {
            int dist = colorDistanceSquared(p, c); // �F������2����g���Ĕ�r
            if (dist < minDist) {
                minDist = dist;
                nearest = c;
            }
        }
        p = nearest; // �ł��߂��F�ɒu������
    }
}
