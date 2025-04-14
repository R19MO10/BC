#include <vector>
#include <cstdint>
#include <cmath>
#include <algorithm>

// 単一ピクセルのRGBカラー構造体
struct Color {
    uint8_t r, g, b;

    Color(uint8_t red = 0, uint8_t green = 0, uint8_t blue = 0)
        : r(red), g(green), b(blue) {
    }
};

// 線形補間関数：aとbをt(0~1)で内分
float lerp(float a, float b, float t) {
    return a + (b - a) * t;
}

// 値を[minVal, maxVal]の範囲に制限（画素外参照防止）
int clamp(int value, int minVal, int maxVal) {
    return std::max(minVal, std::min(value, maxVal));
}













// バイリニア補間による画像リサイズ
std::vector<Color> resizeImageBilinear(
    const std::vector<Color>& colors,
    int originalW, int originalH,
    int newW, int newH
) {
    // 出力画像用のピクセルバッファを確保（newW × newH）
    std::vector<Color> result(newW * newH);

    // 元画像に対する新画像の拡大・縮小率
    float scaleX = static_cast<float>(originalW) / newW;
    float scaleY = static_cast<float>(originalH) / newH;

    // 各出力ピクセルを順に計算
    for (int y = 0; y < newH; ++y) {
        for (int x = 0; x < newW; ++x) {
            // 拡大前画像上での対応座標（浮動小数点）
            float gx = x * scaleX;
            float gy = y * scaleY;

            // 対応座標の左上整数ピクセル座標
            int x0 = static_cast<int>(gx);
            int y0 = static_cast<int>(gy);

            // 補間対象の右下ピクセル（範囲外を防ぐ）
            int x1 = clamp(x0 + 1, 0, originalW - 1);
            int y1 = clamp(y0 + 1, 0, originalH - 1);

            // 補間に使用する比率
            float dx = gx - x0;
            float dy = gy - y0;

            // 対象となる4ピクセルを取得
            const Color& c00 = colors[y0 * originalW + x0]; // 左上
            const Color& c10 = colors[y0 * originalW + x1]; // 右上
            const Color& c01 = colors[y1 * originalW + x0]; // 左下
            const Color& c11 = colors[y1 * originalW + x1]; // 右下

            // 各チャンネルごとにバイリニア補間
            auto interp = [&](uint8_t c00, uint8_t c10, uint8_t c01, uint8_t c11) {
                float top = lerp(c00, c10, dx);
                float bottom = lerp(c01, c11, dx);
                return static_cast<uint8_t>(lerp(top, bottom, dy));
                };

            // 最終カラーを設定
            result[y * newW + x] = {
                interp(c00.r, c10.r, c01.r, c11.r),
                interp(c00.g, c10.g, c01.g, c11.g),
                interp(c00.b, c10.b, c01.b, c11.b)
            };
        }
    }
    return result;
}

// キュービック補間カーネル関数（Catmull-Rom Splineに類似）
float cubic(float x) {
    x = std::fabs(x);
    if (x <= 1.0f)
        return (1.5f * x - 2.5f) * x * x + 1.0f;
    else if (x < 2.0f)
        return ((-0.5f * x + 2.5f) * x - 4.0f) * x + 2.0f;
    else
        return 0.0f;
}










// バイキュービック補間による画像リサイズ
std::vector<Color> resizeImageBicubic(
    const std::vector<Color>& colors,
    int originalW, int originalH,
    int newW, int newH
) {
    std::vector<Color> result(newW * newH);

    float scaleX = static_cast<float>(originalW) / newW;
    float scaleY = static_cast<float>(originalH) / newH;

    for (int y = 0; y < newH; ++y) {
        for (int x = 0; x < newW; ++x) {
            // 対応する元画像の座標（浮動小数点）
            float gx = x * scaleX;
            float gy = y * scaleY;

            int ix = static_cast<int>(gx);  // 対応画素の整数部
            int iy = static_cast<int>(gy);

            float sumR = 0.0f, sumG = 0.0f, sumB = 0.0f;
            float totalWeight = 0.0f;

            // 周囲4x4ピクセルから補間（−1〜+2の範囲）
            for (int m = -1; m <= 2; ++m) {
                for (int n = -1; n <= 2; ++n) {
                    int px = clamp(ix + n, 0, originalW - 1);
                    int py = clamp(iy + m, 0, originalH - 1);
                    const Color& c = colors[py * originalW + px];

                    // 距離に基づく重み
                    float weight = cubic(n - (gx - ix)) * cubic(m - (gy - iy));

                    sumR += c.r * weight;
                    sumG += c.g * weight;
                    sumB += c.b * weight;
                    totalWeight += weight;
                }
            }

            // 最終RGB値（重み正規化＋クランプ）
            auto normalize = [&](float v) {
                return static_cast<uint8_t>(clamp(static_cast<int>(v / totalWeight + 0.5f), 0, 255));
                };

            result[y * newW + x] = {
                normalize(sumR),
                normalize(sumG),
                normalize(sumB)
            };
        }
    }

    return result;
}
