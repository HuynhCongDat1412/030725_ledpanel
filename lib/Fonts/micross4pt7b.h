const uint8_t micross4pt7bBitmaps[] PROGMEM = {
  0x69, 0x99, 0x97, 0x75, 0x50, 0x69, 0x12, 0x4F, 0x69, 0x31, 0x97, 0x22,
  0x6A, 0xF2, 0x74, 0x61, 0x97, 0x69, 0xE9, 0x97, 0xF1, 0x12, 0x24, 0x69,
  0x57, 0x97, 0x69, 0x97, 0x96 };

const GFXglyph micross4pt7bGlyphs[] PROGMEM = {
  {     0,   4,   6,   4,    0,   -5 },   // 0x30 '0'
  {     3,   2,   6,   4,    1,   -5 },   // 0x31 '1'
  {     5,   4,   6,   4,    0,   -5 },   // 0x32 '2'
  {     8,   4,   6,   4,    0,   -5 },   // 0x33 '3'
  {    11,   4,   6,   4,    0,   -5 },   // 0x34 '4'
  {    14,   4,   6,   4,    0,   -5 },   // 0x35 '5'
  {    17,   4,   6,   4,    0,   -5 },   // 0x36 '6'
  {    20,   4,   6,   4,    0,   -5 },   // 0x37 '7'
  {    23,   4,   6,   4,    0,   -5 },   // 0x38 '8'
  {    26,   4,   6,   4,    0,   -5 } }; // 0x39 '9'

const GFXfont micross4pt7b PROGMEM = {
  (uint8_t  *)micross4pt7bBitmaps,
  (GFXglyph *)micross4pt7bGlyphs,
  0x30, 0x39, 9 };