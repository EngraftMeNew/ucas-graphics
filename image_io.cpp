#include "image_io.h"

#include <algorithm>
#include <cmath>

static uint32_t crcTable[256];
static bool crcTableInit = false;

static void makeCrcTable()
{
    for (uint32_t n = 0; n < 256; ++n)
    {
        uint32_t c = n;
        for (int k = 0; k < 8; ++k)
        {
            if (c & 1)
            {
                c = 0xedb88320u ^ (c >> 1);
            }
            else
            {
                c = c >> 1;
            }
        }
        crcTable[n] = c;
    }
    crcTableInit = true;
}

static uint32_t updateCrc(uint32_t crc, const unsigned char *buf, size_t len)
{
    uint32_t c = crc;
    for (size_t n = 0; n < len; ++n)
    {
        c = crcTable[(c ^ buf[n]) & 0xffu] ^ (c >> 8);
    }
    return c;
}

static uint32_t crc(const unsigned char *buf, size_t len)
{
    if (!crcTableInit)
    {
        makeCrcTable();
    }
    return updateCrc(0xffffffffu, buf, len) ^ 0xffffffffu;
}

static void writeU32(std::ofstream &ofs, uint32_t v)
{
    unsigned char bytes[4] = {static_cast<unsigned char>((v >> 24) & 0xFF),
                              static_cast<unsigned char>((v >> 16) & 0xFF),
                              static_cast<unsigned char>((v >> 8) & 0xFF),
                              static_cast<unsigned char>(v & 0xFF)};
    ofs.write(reinterpret_cast<char *>(bytes), 4);
}

static void writeChunk(std::ofstream &ofs, const char *type, const std::vector<unsigned char> &data)
{
    writeU32(ofs, static_cast<uint32_t>(data.size()));
    ofs.write(type, 4);
    if (!data.empty())
    {
        ofs.write(reinterpret_cast<const char *>(data.data()), static_cast<std::streamsize>(data.size()));
    }
    std::vector<unsigned char> crcData(4 + data.size());
    std::copy(type, type + 4, crcData.begin());
    if (!data.empty())
    {
        std::copy(data.begin(), data.end(), crcData.begin() + 4);
    }
    uint32_t c = crc(crcData.data(), crcData.size());
    writeU32(ofs, c);
}

bool writePNG(const std::string &path, const std::vector<Vec3> &img, int width, int height)
{
    std::vector<unsigned char> data(static_cast<size_t>(width) * height * 3);
    for (int i = 0; i < width * height; ++i)
    {
        Vec3 c = clamp01(img[static_cast<size_t>(i)]);
        c.x = std::pow(c.x, 1.0f / 2.2f);
        c.y = std::pow(c.y, 1.0f / 2.2f);
        c.z = std::pow(c.z, 1.0f / 2.2f);
        data[static_cast<size_t>(i) * 3 + 0] = static_cast<unsigned char>(c.x * 255.0f);
        data[static_cast<size_t>(i) * 3 + 1] = static_cast<unsigned char>(c.y * 255.0f);
        data[static_cast<size_t>(i) * 3 + 2] = static_cast<unsigned char>(c.z * 255.0f);
    }

    std::vector<unsigned char> raw;
    raw.reserve(static_cast<size_t>(height) * (width * 3 + 1));
    for (int y = 0; y < height; ++y)
    {
        raw.push_back(0);
        const unsigned char *row = data.data() + static_cast<size_t>(y) * width * 3;
        raw.insert(raw.end(), row, row + width * 3);
    }

    std::vector<unsigned char> zlib;
    zlib.push_back(0x78);
    zlib.push_back(0x01);

    size_t offset = 0;
    while (offset < raw.size())
    {
        size_t blockSize = std::min<size_t>(65535, raw.size() - offset);
        bool finalBlock = (offset + blockSize) == raw.size();
        zlib.push_back(static_cast<unsigned char>(finalBlock ? 0x01 : 0x00));
        uint16_t len = static_cast<uint16_t>(blockSize);
        uint16_t nlen = static_cast<uint16_t>(~len);
        zlib.push_back(static_cast<unsigned char>(len & 0xFF));
        zlib.push_back(static_cast<unsigned char>((len >> 8) & 0xFF));
        zlib.push_back(static_cast<unsigned char>(nlen & 0xFF));
        zlib.push_back(static_cast<unsigned char>((nlen >> 8) & 0xFF));
        zlib.insert(zlib.end(), raw.begin() + static_cast<std::ptrdiff_t>(offset), raw.begin() + static_cast<std::ptrdiff_t>(offset + blockSize));
        offset += blockSize;
    }

    uint32_t s1 = 1;
    uint32_t s2 = 0;
    for (unsigned char c : raw)
    {
        s1 = (s1 + c) % 65521u;
        s2 = (s2 + s1) % 65521u;
    }
    uint32_t adler = (s2 << 16) | s1;
    zlib.push_back(static_cast<unsigned char>((adler >> 24) & 0xFF));
    zlib.push_back(static_cast<unsigned char>((adler >> 16) & 0xFF));
    zlib.push_back(static_cast<unsigned char>((adler >> 8) & 0xFF));
    zlib.push_back(static_cast<unsigned char>(adler & 0xFF));

    std::ofstream ofs(path, std::ios::binary);
    if (!ofs)
    {
        return false;
    }

    unsigned char signature[8] = {0x89, 'P', 'N', 'G', 0x0D, 0x0A, 0x1A, 0x0A};
    ofs.write(reinterpret_cast<char *>(signature), 8);

    std::vector<unsigned char> ihdr(13, 0);
    ihdr[0] = static_cast<unsigned char>((width >> 24) & 0xFF);
    ihdr[1] = static_cast<unsigned char>((width >> 16) & 0xFF);
    ihdr[2] = static_cast<unsigned char>((width >> 8) & 0xFF);
    ihdr[3] = static_cast<unsigned char>(width & 0xFF);
    ihdr[4] = static_cast<unsigned char>((height >> 24) & 0xFF);
    ihdr[5] = static_cast<unsigned char>((height >> 16) & 0xFF);
    ihdr[6] = static_cast<unsigned char>((height >> 8) & 0xFF);
    ihdr[7] = static_cast<unsigned char>(height & 0xFF);
    ihdr[8] = 8;
    ihdr[9] = 2;
    ihdr[10] = 0;
    ihdr[11] = 0;
    ihdr[12] = 0;
    writeChunk(ofs, "IHDR", ihdr);
    writeChunk(ofs, "IDAT", zlib);
    writeChunk(ofs, "IEND", {});

    return true;
}

void writeGIFHeader(std::ofstream &ofs, int width, int height)
{
    ofs.write("GIF89a", 6);
    auto writeWord = [&](int v)
    {
        unsigned char lo = static_cast<unsigned char>(v & 0xFF);
        unsigned char hi = static_cast<unsigned char>((v >> 8) & 0xFF);
        ofs.put(lo);
        ofs.put(hi);
    };
    writeWord(width);
    writeWord(height);
    unsigned char packed = 0x87;
    ofs.put(packed);
    ofs.put(0);
    ofs.put(0);

    for (int i = 0; i < 256; ++i)
    {
        int r = ((i >> 5) & 0x7) * 255 / 7;
        int g = ((i >> 2) & 0x7) * 255 / 7;
        int b = (i & 0x3) * 255 / 3;
        ofs.put(static_cast<unsigned char>(r));
        ofs.put(static_cast<unsigned char>(g));
        ofs.put(static_cast<unsigned char>(b));
    }

    unsigned char ns_ext[] = {
        0x21, 0xFF, 0x0B, 'N', 'E', 'T', 'S', 'C', 'A', 'P', 'E', '2', '.', '0',
        0x03, 0x01, 0x00, 0x00, 0x00};
    ofs.write(reinterpret_cast<char *>(ns_ext), sizeof(ns_ext));
}

void writeGIFFrame(std::ofstream &ofs, const std::vector<unsigned char> &rgba, int width, int height, int delayCs)
{
    unsigned char gce[] = {0x21, 0xF9, 0x04, 0x00,
                           static_cast<unsigned char>(delayCs & 0xFF), static_cast<unsigned char>((delayCs >> 8) & 0xFF),
                           0x00, 0x00};
    ofs.write(reinterpret_cast<char *>(gce), sizeof(gce));

    ofs.put(0x2C);
    auto writeWord = [&](int v)
    {
        unsigned char lo = static_cast<unsigned char>(v & 0xFF);
        unsigned char hi = static_cast<unsigned char>((v >> 8) & 0xFF);
        ofs.put(lo);
        ofs.put(hi);
    };
    writeWord(0);
    writeWord(0);
    writeWord(width);
    writeWord(height);
    ofs.put(0x00);

    const int minCodeSize = 8;
    ofs.put(static_cast<unsigned char>(minCodeSize));

    static const int bayer4[4][4] = {
        {0, 8, 2, 10},
        {12, 4, 14, 6},
        {3, 11, 1, 9},
        {15, 7, 13, 5}};
    const float stepRG = 255.0f / 7.0f;
    const float stepB = 255.0f / 3.0f;

    std::vector<unsigned char> indices(static_cast<size_t>(width) * height);
    for (int y = 0; y < height; ++y)
    {
        for (int x = 0; x < width; ++x)
        {
            size_t i = static_cast<size_t>(y) * width + x;
            float threshold = (bayer4[y & 3][x & 3] / 16.0f) - 0.5f;
            float r = rgba[i * 4 + 0] + threshold * stepRG;
            float g = rgba[i * 4 + 1] + threshold * stepRG;
            float b = rgba[i * 4 + 2] + threshold * stepB;
            r = std::max(0.0f, std::min(255.0f, r));
            g = std::max(0.0f, std::min(255.0f, g));
            b = std::max(0.0f, std::min(255.0f, b));

            int ir = std::min(7, std::max(0, int(r * 7.0f / 255.0f + 0.5f)));
            int ig = std::min(7, std::max(0, int(g * 7.0f / 255.0f + 0.5f)));
            int ib = std::min(3, std::max(0, int(b * 3.0f / 255.0f + 0.5f)));
            indices[i] = static_cast<unsigned char>((ir << 5) | (ig << 2) | ib);
        }
    }

    const int clearCode = 1 << minCodeSize;
    const int endCode = clearCode + 1;
    int codeSize = minCodeSize + 1;
    int dictSize = endCode + 1;

    std::vector<unsigned char> lzwBytes;
    lzwBytes.reserve(indices.size() + 10);
    unsigned int bitBuffer = 0;
    int bitCount = 0;

    auto writeCode = [&](int code)
    {
        bitBuffer |= (code << bitCount);
        bitCount += codeSize;
        while (bitCount >= 8)
        {
            lzwBytes.push_back(static_cast<unsigned char>(bitBuffer & 0xFF));
            bitBuffer >>= 8;
            bitCount -= 8;
        }
    };

    auto resetDictionary = [&]()
    {
        codeSize = minCodeSize + 1;
        dictSize = endCode + 1;
    };

    writeCode(clearCode);
    bool first = true;
    for (size_t i = 0; i < indices.size(); ++i)
    {
        if (dictSize >= 4096)
        {
            writeCode(clearCode);
            resetDictionary();
            first = true;
        }

        writeCode(indices[i]);
        if (first)
        {
            first = false;
        }
        else
        {
            dictSize++;
            if (dictSize == (1 << codeSize) && codeSize < 12)
            {
                codeSize++;
            }
        }
    }
    writeCode(endCode);

    if (bitCount > 0)
    {
        lzwBytes.push_back(static_cast<unsigned char>(bitBuffer & 0xFF));
    }

    size_t offset = 0;
    while (offset < lzwBytes.size())
    {
        size_t blockSize = std::min<size_t>(255, lzwBytes.size() - offset);
        ofs.put(static_cast<unsigned char>(blockSize));
        ofs.write(reinterpret_cast<const char *>(lzwBytes.data() + offset), blockSize);
        offset += blockSize;
    }
    ofs.put(0x00);
}

void writeGIFFooter(std::ofstream &ofs)
{
    ofs.put(0x3B);
}

std::vector<unsigned char> toRGBA(const std::vector<Vec3> &img)
{
    std::vector<unsigned char> out(img.size() * 4);
    for (size_t i = 0; i < img.size(); ++i)
    {
        Vec3 c = clamp01(img[i]);
        c.x = std::pow(c.x, 1.0f / 2.2f);
        c.y = std::pow(c.y, 1.0f / 2.2f);
        c.z = std::pow(c.z, 1.0f / 2.2f);
        out[i * 4 + 0] = static_cast<unsigned char>(c.x * 255.0f);
        out[i * 4 + 1] = static_cast<unsigned char>(c.y * 255.0f);
        out[i * 4 + 2] = static_cast<unsigned char>(c.z * 255.0f);
        out[i * 4 + 3] = 255;
    }
    return out;
}
