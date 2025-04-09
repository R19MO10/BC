#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <cstdint>
#include <algorithm> // std::swap
#include <cmath>     // std::round

#pragma pack(push, 1) // �\���̂̃p�f�B���O�𖳌���

struct BMPFileHeader {
    uint16_t bfType;      // �t�@�C���^�C�v (�K�� 'BM' = 0x4D42)
    uint32_t bfSize;      // �t�@�C���T�C�Y
    uint16_t bfReserved1; // �\��̈�i���g�p�j
    uint16_t bfReserved2; // �\��̈�i���g�p�j
    uint32_t bfOffBits;   // �s�N�Z���f�[�^�̃I�t�Z�b�g
};

struct BMPInfoHeader {
    uint32_t biSize;          // ���̃w�b�_�̃T�C�Y�i40�o�C�g�j
    int32_t  biWidth;         // ��
    int32_t  biHeight;        // ����
    uint16_t biPlanes;        // �v���[�����i1�Œ�j
    uint16_t biBitCount;      // �r�b�g���i24 or 32�j
    uint32_t biCompression;   // ���k�����i0 = �񈳏k�j
    uint32_t biSizeImage;     // �摜�f�[�^�T�C�Y�i0�ł��j
    int32_t  biXPelsPerMeter; // ���������̉𑜓x
    int32_t  biYPelsPerMeter; // ���������̉𑜓x
    uint32_t biClrUsed;       // �g�p�F��
    uint32_t biClrImportant;  // �d�v�ȐF��
};

#pragma pack(pop)

// �����ϊ��iRGB�𕽋ς��ăO���[�X�P�[���ɂ���j
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

// �㉺���]
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

// ���E���]
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

// �g��k���i���`��ԁj
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
    std::cout << "�摜�������j���[:\n";
    std::cout << "1: �㉺���]\n";
    std::cout << "2: ���E���]\n";
    std::cout << "3: �����ϊ�\n";
    std::cout << "4: �g��k��\n";
    std::cout << "5: �����Ȃ�\n";
    std::cout << "�I��������͂��Ă��������i1-5�j: ";
}

int main() {
    std::string inputFilename, outputFilename;
    std::cout << "�ǂݍ���BMP�t�@�C����: ";
    std::getline(std::cin, inputFilename);

    std::cout << "�ۑ�����BMP�t�@�C����: ";
    std::getline(std::cin, outputFilename);

    std::ifstream inputFile(inputFilename, std::ios::binary);
    if (!inputFile) {
        std::cerr << "���̓t�@�C�����J���܂���: " << inputFilename << std::endl;
        return 1;
    }

    // �w�b�_�[�ǂݍ���
    BMPFileHeader fileHeader;
    BMPInfoHeader infoHeader;

    inputFile.read(reinterpret_cast<char*>(&fileHeader), sizeof(fileHeader));
    inputFile.read(reinterpret_cast<char*>(&infoHeader), sizeof(infoHeader));

    // BMP�`�����m�F
    if (fileHeader.bfType != 0x4D42 || infoHeader.biBitCount != 24 || infoHeader.biCompression != 0) {
        std::cerr << "�Ή����Ă��Ȃ�BMP�`���ł��i24bit�񈳏k�̂ݑΉ��j" << std::endl;
        return 1;
    }

    // �s�N�Z���s���Ƃ̃p�f�B���O���v�Z�i4�o�C�g�A���C�����g�j
    int rowSize = ((infoHeader.biWidth * 3 + 3) / 4) * 4;
    int imageSize = rowSize * std::abs(infoHeader.biHeight);

    std::vector<uint8_t> pixelData(imageSize);

    inputFile.seekg(fileHeader.bfOffBits, std::ios::beg);
    inputFile.read(reinterpret_cast<char*>(pixelData.data()), imageSize);
    inputFile.close();

    // ���[�U�[�ɂ�鏈���̑I��
    displayMenu();
    int choice;
    std::cin >> choice;

    switch (choice) {
    case 1:
        flipVertically(pixelData, infoHeader.biWidth, std::abs(infoHeader.biHeight));
        std::cout << "�㉺���]�����s���܂����B\n";
        break;
    case 2:
        flipHorizontally(pixelData, infoHeader.biWidth, std::abs(infoHeader.biHeight));
        std::cout << "���E���]�����s���܂����B\n";
        break;
    case 3:
        convertToGrayscale(pixelData, infoHeader.biWidth, std::abs(infoHeader.biHeight));
        std::cout << "�����ϊ������s���܂����B\n";
        break;
    case 4:
    {
        // �g��k���i��Ƃ��Ĕ����ɏk���j
        int newWidth = infoHeader.biWidth / 2;
        int newHeight = std::abs(infoHeader.biHeight) / 2;
        pixelData = resizeImage(pixelData, infoHeader.biWidth, std::abs(infoHeader.biHeight), newWidth, newHeight);
        infoHeader.biWidth = newWidth;
        infoHeader.biHeight = newHeight;
        std::cout << "�摜�𔼕��ɏk�����܂����B\n";
        break;
    }
    case 5:
        std::cout << "�����Ȃ��B\n";
        break;
    default:
        std::cerr << "�����ȑI���ł��B\n";
        return 1;
    }

    // �V�����摜�T�C�Y���X�V
    int newImageSize = ((infoHeader.biWidth * 3 + 3) / 4) * 4 * std::abs(infoHeader.biHeight);
    infoHeader.biSizeImage = newImageSize;
    fileHeader.bfSize = sizeof(fileHeader) + sizeof(infoHeader) + newImageSize;

    // �����o��
    std::ofstream outputFile(outputFilename, std::ios::binary);
    if (!outputFile) {
        std::cerr << "�o�̓t�@�C�����J���܂���: " << outputFilename << std::endl;
        return 1;
    }

    outputFile.write(reinterpret_cast<char*>(&fileHeader), sizeof(fileHeader));
    outputFile.write(reinterpret_cast<char*>(&infoHeader), sizeof(infoHeader));
    outputFile.write(reinterpret_cast<char*>(pixelData.data()), newImageSize);
    outputFile.close();

    std::cout << "�摜�������������܂���: " << outputFilename << std::endl;
    return 0;
}
