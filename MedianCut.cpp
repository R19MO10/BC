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

    // �P�x�i���邳�j���v�Z�i�\�[�g�p�j
    int brightness() const {
        return r + g + b;
    }
};

// ���f�B�A���J�b�g���F�A���S���Y��
std::vector<Color> medianCut(std::vector<Color>& colorData, int maxColorGroupCount = 64) {
    using PixelIndex = size_t;

    // �C���f�b�N�X�z��̏������i�S�s�N�Z�����Ώہj
    std::vector<PixelIndex> allIndices(colorData.size());
    for (PixelIndex i = 0; i < colorData.size(); ++i) {
        allIndices[i] = i;
    }

    if (allIndices.empty()) return {};

    // �ꎞ�I�ȃO���[�v�z��i�ċA�I�ɕ����j
    std::vector<std::vector<PixelIndex>> tmpGroups{ allIndices };
    std::vector<std::vector<PixelIndex>> finalGroups;

    // �ő�O���[�v���ɂȂ�܂ŕ����������J��Ԃ�
    while (tmpGroups.size() + finalGroups.size() < static_cast<size_t>(maxColorGroupCount)) {
        // ��ԗv�f���������O���[�v��T��
        auto maxIt = std::max_element(tmpGroups.begin(), tmpGroups.end(),
            [](const auto& a, const auto& b) { return a.size() < b.size(); });

        if (maxIt == tmpGroups.end() || maxIt->empty()) break;

        auto group = std::move(*maxIt);
        tmpGroups.erase(maxIt);

        // RGB�����̍ŏ��E�ő�����߂�
        uint8_t rMin = 255, rMax = 0;
        uint8_t gMin = 255, gMax = 0;
        uint8_t bMin = 255, bMax = 0;

        for (PixelIndex idx : group) {
            const Color& c = colorData[idx];
            rMin = std::min(rMin, c.r); rMax = std::max(rMax, c.r);
            gMin = std::min(gMin, c.g); gMax = std::max(gMax, c.g);
            bMin = std::min(bMin, c.b); bMax = std::max(bMax, c.b);
        }

        // RGB���ꂼ��͈̔�
        int rRange = rMax - rMin;
        int gRange = gMax - gMin;
        int bRange = bMax - bMin;

        // �P��F�����Ȃ��ꍇ�͂���ȏ㕪���s�v
        if (rRange == 0 && gRange == 0 && bRange == 0) {
            finalGroups.push_back(std::move(group));
            continue;
        }

        // �������ƒ��S�l������i�ő�͈͂��������j
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

        // �O���[�v�𒆐S�l�ŕ���
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

        // ��łȂ��O���[�v�����ǉ��i�������邱�Ƃŕ��������A�b�v�j
        if (!lowerGroup.empty()) tmpGroups.push_back(std::move(lowerGroup));
        if (!upperGroup.empty()) tmpGroups.push_back(std::move(upperGroup));
    }

    // �ŏI�O���[�v�Ɏc�������̂�����
    finalGroups.insert(finalGroups.end(), tmpGroups.begin(), tmpGroups.end());

    // ���ϐF�v�Z���f�[�^��������
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

        // �O���[�v���s�N�Z�������ׂĕ��ϐF�Œu��
        for (PixelIndex idx : group) {
            colorData[idx] = avgColor;
        }

        palette.push_back(avgColor);
    }

    // �p���b�g�͖��邳���Ƀ\�[�g
    std::sort(palette.begin(), palette.end(), [](const Color& a, const Color& b) {
        return a.brightness() < b.brightness();
        });

    return palette;
}
