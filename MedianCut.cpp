#include <vector>
#include <cstdint>
#include <algorithm>
#include <cmath>
#include <limits>

struct Color32 {
    uint8_t r, g, b, a; // 赤・緑・青・アルファ値（透過）
};

// 各色ボックスを保持する構造体
struct ColorBox {
    std::vector<Color32> colors; // このボックス内の色
    int rMin, rMax, gMin, gMax, bMin, bMax; // RGBの最小・最大値（ボックスのサイズを決める）
};

// 2色間のユークリッド距離の2乗を計算（比較用）
int colorDistanceSquared(const Color32& c1, const Color32& c2) {
    int dr = int(c1.r) - int(c2.r);
    int dg = int(c1.g) - int(c2.g);
    int db = int(c1.b) - int(c2.b);
    return dr * dr + dg * dg + db * db;
}

// 与えられた色の集合の平均色を求める（各成分の合計÷総数）
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

// ボックスのRGB範囲を更新（最大/最小の再計算）
void updateBox(ColorBox& box) {
    // 初期値に1つ目の色を設定
    box.rMin = box.rMax = box.colors[0].r;
    box.gMin = box.gMax = box.colors[0].g;
    box.bMin = box.bMax = box.colors[0].b;

    // 色ごとに最小・最大を更新
    for (const auto& c : box.colors) {
        box.rMin = std::min(box.rMin, (int)c.r);
        box.rMax = std::max(box.rMax, (int)c.r);
        box.gMin = std::min(box.gMin, (int)c.g);
        box.gMax = std::max(box.gMax, (int)c.g);
        box.bMin = std::min(box.bMin, (int)c.b);
        box.bMax = std::max(box.bMax, (int)c.b);
    }
}

// メディアンカット減色アルゴリズム本体
void medianCut(std::vector<Color32>& pix, int w, int h, uint8_t maxColor) {
    std::vector<ColorBox> boxes;

    // 初期ボックスにはすべての色を入れる
    ColorBox initialBox{ pix };
    updateBox(initialBox); // 初期ボックスの範囲を計算
    boxes.push_back(initialBox);

    // 色数がmaxColorに達するまで分割を繰り返す
    while (boxes.size() < maxColor) {
        // 分割すべき最も「広い」ボックスを探す
        auto it = std::max_element(boxes.begin(), boxes.end(), [](const ColorBox& a, const ColorBox& b) {
            int rangeA = std::max({ a.rMax - a.rMin, a.gMax - a.gMin, a.bMax - a.bMin });
            int rangeB = std::max({ b.rMax - b.rMin, b.gMax - b.gMin, b.bMax - b.bMin });
            return rangeA < rangeB; // 範囲が広いほうを優先
            });

        // 分割不能なら終了（色が1つしかない等）
        if (it == boxes.end() || it->colors.size() <= 1)
            break;

        ColorBox box = *it;         // 分割対象のボックスを取り出す
        boxes.erase(it);            // 元のリストから削除

        // RGB成分のどれを軸に分割するか決める（最大範囲の成分）
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

        // ソートした色を2分割（中央値で分ける）
        size_t mid = box.colors.size() / 2;
        ColorBox box1{ std::vector<Color32>(box.colors.begin(), box.colors.begin() + mid) };
        ColorBox box2{ std::vector<Color32>(box.colors.begin() + mid, box.colors.end()) };

        updateBox(box1); // 分割後のボックスの範囲を更新
        updateBox(box2);

        // 新しい2つのボックスを追加
        boxes.push_back(box1);
        boxes.push_back(box2);
    }

    // ボックスごとの平均色を使ってパレット作成
    std::vector<Color32> palette;
    for (const auto& box : boxes) {
        palette.push_back(averageColor(box.colors));
    }

    // 全ピクセルをパレットの最近傍色に変換
    for (auto& p : pix) {
        int minDist = std::numeric_limits<int>::max();
        Color32 nearest = palette[0];
        for (const auto& c : palette) {
            int dist = colorDistanceSquared(p, c); // 色距離の2乗を使って比較
            if (dist < minDist) {
                minDist = dist;
                nearest = c;
            }
        }
        p = nearest; // 最も近い色に置き換え
    }
}
