#include <bmp_image.h>
#include <img_lib.h>
#include <jpeg_image.h>
#include <ppm_image.h>

#include <filesystem>
#include <fstream>
#include <iostream>
#include <string_view>

using namespace std;

namespace format {

enum Format { UNKNOWN, PPM, JPEG, BMP };

class ImageFormatInterface {
 public:
  virtual bool SaveImage(const img_lib::Path& file, const img_lib::Image& image) const = 0;
  virtual img_lib::Image LoadImage(const img_lib::Path& file) const = 0;
};

class PPMInterface : public ImageFormatInterface {
  bool SaveImage(const img_lib::Path& file, const img_lib::Image& image) const override {
    return img_lib::SavePPM(file, image);
  }
  img_lib::Image LoadImage(const img_lib::Path& file) const override {
    return img_lib::LoadPPM(file);
  }
};

class JPEGInterface : public ImageFormatInterface {
  bool SaveImage(const img_lib::Path& file, const img_lib::Image& image) const override {
    return img_lib::SaveJPEG(file, image);
  }
  img_lib::Image LoadImage(const img_lib::Path& file) const override {
    return img_lib::LoadJPEG(file);
  }
};

class BMPInterface : public ImageFormatInterface {
  bool SaveImage(const img_lib::Path& file, const img_lib::Image& image) const override {
    return img_lib::SaveBMP(file, image);
  }
  img_lib::Image LoadImage(const img_lib::Path& file) const override {
    return img_lib::LoadBMP(file);
  }
};

Format GetFormatByExtension(const img_lib::Path& input_file) {
  const string ext = input_file.extension().string();
  if (ext == ".jpg"sv || ext == ".jpeg"sv) {
    return Format::JPEG;
  }
  if (ext == ".ppm"sv) {
    return Format::PPM;
  }
  if (ext == ".bmp"sv) {
    return Format::BMP;
  }
  return Format::UNKNOWN;
}

ImageFormatInterface* GetFormatInterface(const img_lib::Path& path) {
  switch (GetFormatByExtension(path)) {
    case Format::JPEG:
      return new JPEGInterface{};
      break;
    case Format::PPM:
      return new PPMInterface{};
      break;
    case Format::BMP:
      return new BMPInterface{};
      break;
  }
  return nullptr;
}
}  // namespace format

int main(int argc, const char** argv) {
  if (argc != 3) {
    cerr << "Usage: "sv << argv[0] << " <in_file> <out_file>"sv << endl;
    return 1;
  }

  img_lib::Path in_path = argv[1];
  img_lib::Path out_path = argv[2];

  format::ImageFormatInterface* src_interface = format::GetFormatInterface(in_path);
  format::ImageFormatInterface* dest_interface = format::GetFormatInterface(out_path);

  if (!src_interface) {
    cerr << "Unknown format of the input file" << endl;
    return 2;
  }
  if (!dest_interface) {
    cerr << "Unknown format of the output file" << endl;
    return 3;
  }

  img_lib::Image image = src_interface->LoadImage(in_path);

  if (!image) {
    cerr << "Loading failed"sv << endl;
    return 4;
  }

  if (!dest_interface->SaveImage(out_path, image)) {
    cerr << "Saving failed"sv << endl;
    return 5;
  }

  cout << "Successfully converted"sv << endl;
}