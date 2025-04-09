#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <cstdint>
#include <algorithm> // std::swap
#include <cmath>     // std::round

#pragma pack(push, 1) // 構造体のパディングを無効化

struct BMPFileHeader {
    uint16_t bfType;      // ファイルタイプ (必ず 'BM' = 0x4D42)
    uint32_t bfSize;      // ファイルサイズ
    uint16_t bfReserved1; // 予約領域（未使用）
    uint16_t bfReserved2; // 予約領域（未使用）
    uint32_t bfOffBits;   // ピクセルデータのオフセット
};

struct BMPInfoHeader {
    uint32_t biSize;          // このヘッダのサイズ（40バイト）
    int32_t  biWidth;         // 幅
    int32_t  biHeight;        // 高さ
    uint16_t biPlanes;        // プレーン数（1固定）
    uint16_t biBitCount;      // ビット数（24 or 32）
    uint32_t biCompression;   // 圧縮方式（0 = 非圧縮）
    uint32_t biSizeImage;     // 画像データサイズ（0でも可）
    int32_t  biXPelsPerMeter; // 水平方向の解像度
    int32_t  biYPelsPerMeter; // 垂直方向の解像度
    uint32_t biClrUsed;       // 使用色数
    uint32_t biClrImportant;  // 重要な色数
};

#pragma pack(pop)

// 白黒変換（RGBを平均してグレースケールにする）
void convertToGrayscale(std::vector<uint8_t>& pixelData, int width, int height) {
    int rowSize = ((width * 3 + 3) / 4) * 4;
    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            int idx = y * rowSize + x * 3;
            uint8_t r = pixelData[idx + 2];
            uint8_t g = pixelData[idx + 1];
            uint8_t b = pixelData[idx];
            uint8_t gray = static_cast<uint8_t>(std::round((r + g + b) / 3.0));
            pixelData[idx + 2] = pixelData[idx + 1] = pixelData[idx] = gray;
        }
    }
}

// 上下反転
void flipVertically(std::vector<uint8_t>& pixelData, int width, int height) {
    int rowSize = ((width * 3 + 3) / 4) * 4;
    for (int y = 0; y < height / 2; ++y) {
        int topIdx = y * rowSize;
        int bottomIdx = (height - 1 - y) * rowSize;
        for (int x = 0; x < rowSize; ++x) {
            std::swap(pixelData[topIdx + x], pixelData[bottomIdx + x]);
        }
    }
}

// 左右反転
void flipHorizontally(std::vector<uint8_t>& pixelData, int width, int height) {
    int rowSize = ((width * 3 + 3) / 4) * 4;
    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width / 2; ++x) {
            int leftIdx = y * rowSize + x * 3;
            int rightIdx = y * rowSize + (width - 1 - x) * 3;
            for (int i = 0; i < 3; ++i) {
                std::swap(pixelData[leftIdx + i], pixelData[rightIdx + i]);
            }
        }
    }
}

// 拡大縮小（線形補間）
std::vector<uint8_t> resizeImage(const std::vector<uint8_t>& originalData, int originalWidth, int originalHeight, int newWidth, int newHeight) {
    int originalRowSize = ((originalWidth * 3 + 3) / 4) * 4;
    int newRowSize = ((newWidth * 3 + 3) / 4) * 4;
    std::vector<uint8_t> resizedData(newRowSize * newHeight);

    for (int y = 0; y < newHeight; ++y) {
        for (int x = 0; x < newWidth; ++x) {
            float srcX = static_cast<float>(x) / newWidth * originalWidth;
            float srcY = static_cast<float>(y) / newHeight * originalHeight;

            int baseX = static_cast<int>(srcX);
            int baseY = static_cast<int>(srcY);

            int idx = y * newRowSize + x * 3;
            int srcIdx = baseY * originalRowSize + baseX * 3;

            for (int i = 0; i < 3; ++i) {
                resizedData[idx + i] = originalData[srcIdx + i];
            }
        }
    }

    return resizedData;
}

void displayMenu() {
    std::cout << "画像処理メニュー:\n";
    std::cout << "1: 上下反転\n";
    std::cout << "2: 左右反転\n";
    std::cout << "3: 白黒変換\n";
    std::cout << "4: 拡大縮小\n";
    std::cout << "5: 処理なし\n";
    std::cout << "選択肢を入力してください（1-5）: ";
}

int main() {
    std::string inputFilename, outputFilename;
    std::cout << "読み込むBMPファイル名: ";
    std::getline(std::cin, inputFilename);

    std::cout << "保存するBMPファイル名: ";
    std::getline(std::cin, outputFilename);

    std::ifstream inputFile(inputFilename, std::ios::binary);
    if (!inputFile) {
        std::cerr << "入力ファイルを開けません: " << inputFilename << std::endl;
        return 1;
    }

    // ヘッダー読み込み
    BMPFileHeader fileHeader;
    BMPInfoHeader infoHeader;

    inputFile.read(reinterpret_cast<char*>(&fileHeader), sizeof(fileHeader));
    inputFile.read(reinterpret_cast<char*>(&infoHeader), sizeof(infoHeader));

    // BMP形式か確認
    if (fileHeader.bfType != 0x4D42 || infoHeader.biBitCount != 24 || infoHeader.biCompression != 0) {
        std::cerr << "対応していないBMP形式です（24bit非圧縮のみ対応）" << std::endl;
        return 1;
    }

    // ピクセル行ごとのパディングを計算（4バイトアライメント）
    int rowSize = ((infoHeader.biWidth * 3 + 3) / 4) * 4;
    int imageSize = rowSize * std::abs(infoHeader.biHeight);

    std::vector<uint8_t> pixelData(imageSize);

    inputFile.seekg(fileHeader.bfOffBits, std::ios::beg);
    inputFile.read(reinterpret_cast<char*>(pixelData.data()), imageSize);
    inputFile.close();

    // ユーザーによる処理の選択
    displayMenu();
    int choice;
    std::cin >> choice;

    switch (choice) {
    case 1:
        flipVertically(pixelData, infoHeader.biWidth, std::abs(infoHeader.biHeight));
        std::cout << "上下反転を実行しました。\n";
        break;
    case 2:
        flipHorizontally(pixelData, infoHeader.biWidth, std::abs(infoHeader.biHeight));
        std::cout << "左右反転を実行しました。\n";
        break;
    case 3:
        convertToGrayscale(pixelData, infoHeader.biWidth, std::abs(infoHeader.biHeight));
        std::cout << "白黒変換を実行しました。\n";
        break;
    case 4:
    {
        // 拡大縮小（例として半分に縮小）
        int newWidth = infoHeader.biWidth / 2;
        int newHeight = std::abs(infoHeader.biHeight) / 2;
        pixelData = resizeImage(pixelData, infoHeader.biWidth, std::abs(infoHeader.biHeight), newWidth, newHeight);
        infoHeader.biWidth = newWidth;
        infoHeader.biHeight = newHeight;
        std::cout << "画像を半分に縮小しました。\n";
        break;
    }
    case 5:
        std::cout << "処理なし。\n";
        break;
    default:
        std::cerr << "無効な選択です。\n";
        return 1;
    }

    // 新しい画像サイズを更新
    int newImageSize = ((infoHeader.biWidth * 3 + 3) / 4) * 4 * std::abs(infoHeader.biHeight);
    infoHeader.biSizeImage = newImageSize;
    fileHeader.bfSize = sizeof(fileHeader) + sizeof(infoHeader) + newImageSize;

    // 書き出し
    std::ofstream outputFile(outputFilename, std::ios::binary);
    if (!outputFile) {
        std::cerr << "出力ファイルを開けません: " << outputFilename << std::endl;
        return 1;
    }

    outputFile.write(reinterpret_cast<char*>(&fileHeader), sizeof(fileHeader));
    outputFile.write(reinterpret_cast<char*>(&infoHeader), sizeof(infoHeader));
    outputFile.write(reinterpret_cast<char*>(pixelData.data()), newImageSize);
    outputFile.close();

    std::cout << "画像処理が完了しました: " << outputFilename << std::endl;
    return 0;
}
