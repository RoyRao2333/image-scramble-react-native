#include "ImageScramble.hpp"
#include <cassert>
#include <iostream>
#include <vector>

using namespace PicEncrypt;

void testException() {
  std::cout << "Testing exception throw..." << std::endl;
  // An artificially broken pixel count / dimension to test out-of-range safety
  std::vector<uint32_t> px(4, 0x12345678); // 2x2 image
  try {
    // Force width 10, height 10 on a vector of size 4
    PerPixelScramble scramble(px, 10, 10, "test_key");
    scramble.encrypt();
    std::cerr << "Fail: Expected exception but not thrown!" << std::endl;
    assert(false);
  } catch (const std::out_of_range &e) {
    std::cout << "Pass: std::out_of_range safely caught out-of-bounds error "
                 "and rescued the App!"
              << std::endl;
  }
}

void testTomato() {
  std::cout << "Testing TomatoScramble..." << std::endl;
  std::vector<uint32_t> px = {1, 2, 3, 4, 5, 6, 7, 8, 9}; // 3x3 image
  TomatoScramble enc(px, 3, 3, 4.2);
  Image encImg = enc.encrypt();
  TomatoScramble dec(encImg.pixels, 3, 3, 4.2);
  Image decImg = dec.decrypt();

  assert(encImg.pixels != px);
  assert(decImg.pixels == px);
  std::cout << "TomatoScramble Pass!" << std::endl;
}

void testBlock() {
  std::cout << "Testing BlockScramble..." << std::endl;
  std::vector<uint32_t> px(400); // 20x20
  for (int i = 0; i < 400; ++i)
    px[i] = i;

  BlockScramble enc(px, 20, 20, "1234");
  Image encImg = enc.encrypt();
  BlockScramble dec(encImg.pixels, 20, 20, "1234");
  Image decImg = dec.decrypt();

  assert(encImg.pixels != px);
  assert(decImg.pixels == px);
  std::cout << "BlockScramble Pass!" << std::endl;
}

void testRowPixel() {
  std::cout << "Testing RowPixelScramble..." << std::endl;
  std::vector<uint32_t> px(100); // 10x10
  for (int i = 0; i < 100; ++i)
    px[i] = i;

  RowPixelScramble enc(px, 10, 10, "key");
  Image encImg = enc.encrypt();
  RowPixelScramble dec(encImg.pixels, 10, 10, "key");
  Image decImg = dec.decrypt();
  assert(encImg.pixels != px);
  assert(decImg.pixels == px);
  std::cout << "RowPixelScramble Pass!" << std::endl;
}

int main() {
  testException();
  testTomato();
  testBlock();
  testRowPixel();
  std::cout << "All C++ Core Algorithms generated successfully and "
               "successfully passed semantic testing setup."
            << std::endl;
  return 0;
}
