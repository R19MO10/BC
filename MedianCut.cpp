#include <vector>
#include <algorithm>
#include <cstdint>
#include <iostream>
#include <functional>

struct Color {
    uint8_t r, g, b;

    Color(uint8_t red = 0, uint8_t green = 0, uint8_t blue = 0)
        : r(red), g(green), b(blue) {
    }

    // 輝度（明るさ）を計算（ソート用）
    int brightness() const {
        return r + g + b;
    }
};

// メディアンカット減色アルゴリズム
std::vector<Color> medianCut(std::vector<Color>& colorData, int maxColorGroupCount = 64) {
    using PixelIndex = size_t;

    // インデックス配列の初期化（全ピクセルが対象）
    std::vector<PixelIndex> allIndices(colorData.size());
    for (PixelIndex i = 0; i < colorData.size(); ++i) {
        allIndices[i] = i;
    }

    if (allIndices.empty()) return {};

    // 一時的なグループ配列（再帰的に分割）
    std::vector<std::vector<PixelIndex>> tmpGroups{ allIndices };
    std::vector<std::vector<PixelIndex>> finalGroups;

    // 最大グループ数になるまで分割処理を繰り返す
    while (tmpGroups.size() + finalGroups.size() < static_cast<size_t>(maxColorGroupCount)) {
        // 一番要素数が多いグループを探す
        auto maxIt = std::max_element(tmpGroups.begin(), tmpGroups.end(),
            [](const auto& a, const auto& b) { return a.size() < b.size(); });

        if (maxIt == tmpGroups.end() || maxIt->empty()) break;

        auto group = std::move(*maxIt);
        tmpGroups.erase(maxIt);

        // RGB成分の最小・最大を求める
        uint8_t rMin = 255, rMax = 0;
        uint8_t gMin = 255, gMax = 0;
        uint8_t bMin = 255, bMax = 0;

        for (PixelIndex idx : group) {
            const Color& c = colorData[idx];
            rMin = std::min(rMin, c.r); rMax = std::max(rMax, c.r);
            gMin = std::min(gMin, c.g); gMax = std::max(gMax, c.g);
            bMin = std::min(bMin, c.b); bMax = std::max(bMax, c.b);
        }

        // RGBそれぞれの範囲
        int rRange = rMax - rMin;
        int gRange = gMax - gMin;
        int bRange = bMax - bMin;

        // 単一色しかない場合はこれ以上分割不要
        if (rRange == 0 && gRange == 0 && bRange == 0) {
            finalGroups.push_back(std::move(group));
            continue;
        }

        // 分割軸と中心値を決定（最大範囲を持つ成分）
        std::function<uint8_t(const Color&)> getComponent;
        float center = 0.0f;

        if (rRange >= gRange && rRange >= bRange) {
            getComponent = [](const Color& c) { return c.r; };
            center = (rMin + rMax) / 2.0f;
        }
        else if (gRange >= rRange && gRange >= bRange) {
            getComponent = [](const Color& c) { return c.g; };
            center = (gMin + gMax) / 2.0f;
        }
        else {
            getComponent = [](const Color& c) { return c.b; };
            center = (bMin + bMax) / 2.0f;
        }

        // グループを中心値で分割
        std::vector<PixelIndex> lowerGroup, upperGroup;
        lowerGroup.reserve(group.size() / 2);
        upperGroup.reserve(group.size() / 2);

        for (PixelIndex idx : group) {
            if (getComponent(colorData[idx]) < center) {
                lowerGroup.push_back(idx);
            }
            else {
                upperGroup.push_back(idx);
            }
        }

        // 空でないグループだけ追加（空を避けることで分割効率アップ）
        if (!lowerGroup.empty()) tmpGroups.push_back(std::move(lowerGroup));
        if (!upperGroup.empty()) tmpGroups.push_back(std::move(upperGroup));
    }

    // 最終グループに残ったものを合流
    finalGroups.insert(finalGroups.end(), tmpGroups.begin(), tmpGroups.end());

    // 平均色計算＆データ書き換え
    std::vector<Color> palette;
    palette.reserve(finalGroups.size());

    for (const auto& group : finalGroups) {
        uint64_t sumR = 0, sumG = 0, sumB = 0;

        for (PixelIndex idx : group) {
            const Color& c = colorData[idx];
            sumR += c.r;
            sumG += c.g;
            sumB += c.b;
        }

        Color avgColor(
            static_cast<uint8_t>(sumR / group.size()),
            static_cast<uint8_t>(sumG / group.size()),
            static_cast<uint8_t>(sumB / group.size())
        );

        // グループ内ピクセルをすべて平均色で置換
        for (PixelIndex idx : group) {
            colorData[idx] = avgColor;
        }

        palette.push_back(avgColor);
    }

    // パレットは明るさ順にソート
    std::sort(palette.begin(), palette.end(), [](const Color& a, const Color& b) {
        return a.brightness() < b.brightness();
        });

    return palette;
}
