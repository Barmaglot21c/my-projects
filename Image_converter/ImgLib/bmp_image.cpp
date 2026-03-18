#include "bmp_image.h"

#include <stdint.h>

#include <array>
#include <fstream>
#include <string_view>

#include "pack_defines.h"

using namespace std;

namespace img_lib {

static const uint16_t BMP_SIG =
    (static_cast<uint16_t>('M') << 8) | static_cast<uint16_t>('B');  // 0x4D42
static const uint32_t FILE_HEADER_SIZE = 14;
static const uint32_t INFO_HEADER_SIZE = 40;
static const unsigned COLOR_COUNT = 3;
static const unsigned PADDING = 4;

PACKED_STRUCT_BEGIN BitmapFileHeader {
  // поля заголовка Bitmap File Header
  uint16_t sig = BMP_SIG;
  uint32_t total_size;
  uint32_t reserve = 0;
  uint32_t data_offset = FILE_HEADER_SIZE + INFO_HEADER_SIZE;
}
PACKED_STRUCT_END

PACKED_STRUCT_BEGIN BitmapInfoHeader {
  // поля заголовка Bitmap Info Header
  uint32_t info_header_size = INFO_HEADER_SIZE;
  int32_t width;
  int32_t height;
  uint16_t flat_num = 1;
  uint16_t bits_per_pixel = 24;
  uint32_t compress_type = 0;
  uint32_t data_size;
  int32_t pixels_per_meter_h = 11811;
  int32_t pixels_per_meter_v = 11811;
  int32_t color_num = 0;
  int32_t significant_color_num = 0x1000000;
}
PACKED_STRUCT_END

// функция вычисления отступа по ширине
static int GetBMPStride(int w) {
  return PADDING * ((w * COLOR_COUNT + COLOR_COUNT) / PADDING);
}

// напишите эту функцию
bool SaveBMP(const Path& file, const Image& image) {
  ofstream ofs(file, ios::binary);

  if (!ofs) {
    return false;
  }

  BitmapFileHeader file_header;
  BitmapInfoHeader info_header;

  int data_size = GetBMPStride(image.GetWidth()) * image.GetHeight();
  file_header.total_size = FILE_HEADER_SIZE + INFO_HEADER_SIZE + data_size;
  info_header.width = image.GetWidth();
  info_header.height = image.GetHeight();
  info_header.data_size = data_size;

  ofs.write(reinterpret_cast<char*>(&file_header), FILE_HEADER_SIZE);
  ofs.write(reinterpret_cast<char*>(&info_header), INFO_HEADER_SIZE);

  const int w = image.GetWidth();
  const int h = image.GetHeight();
  const int stride = GetBMPStride(w);
  std::vector<char> buff(stride);

  for (int y = h - 1; y >= 0; --y) {
    const Color* line = image.GetLine(y);
    for (int x = 0; x < w; ++x) {
      buff[x * COLOR_COUNT + 0] = static_cast<char>(line[x].b);
      buff[x * COLOR_COUNT + 1] = static_cast<char>(line[x].g);
      buff[x * COLOR_COUNT + 2] = static_cast<char>(line[x].r);
    }
    ofs.write(buff.data(), stride);
  }

  return ofs.good();
}

// напишите эту функцию
Image LoadBMP(const Path& file) {
  ifstream ifs(file, ios::binary);

  if (!ifs) {
    return {};
  }

  BitmapFileHeader file_header;
  BitmapInfoHeader info_header;

  ifs.read(reinterpret_cast<char*>(&file_header), FILE_HEADER_SIZE);
  ifs.read(reinterpret_cast<char*>(&info_header), INFO_HEADER_SIZE);
  if (!ifs) {
    return {};
  }

  if (file_header.sig != BMP_SIG || file_header.reserve != 0 ||
      file_header.data_offset != FILE_HEADER_SIZE + INFO_HEADER_SIZE ||
      info_header.info_header_size != INFO_HEADER_SIZE || info_header.flat_num != 1 ||
      info_header.bits_per_pixel != 24 || info_header.compress_type != 0 ||
      info_header.pixels_per_meter_h != 11811 || info_header.pixels_per_meter_v != 11811 ||
      info_header.color_num != 0 || info_header.significant_color_num != 0x1000000) {
    return {};
  }

  int w = info_header.width;
  int h = info_header.height;
  int stride = GetBMPStride(w);

  Image result(w, h, Color::Black());

  vector<char> buff(stride);

  for (int y = 0; y < h; ++y) {
    Color* line = result.GetLine(h - y - 1);
    ifs.read(buff.data(), stride);
    if (!ifs) {
      return {};
    }

    for (int x = 0; x < w; ++x) {
      line[x].b = static_cast<byte>(buff[x * COLOR_COUNT + 0]);
      line[x].g = static_cast<byte>(buff[x * COLOR_COUNT + 1]);
      line[x].r = static_cast<byte>(buff[x * COLOR_COUNT + 2]);
    }
  }
  return result;
}

}  // namespace img_lib