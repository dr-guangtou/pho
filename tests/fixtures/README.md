# Test Fixtures

This directory contains test data for the pho test suite.

## Current Fixtures

Test images are located in `../test-img/` (project root).

## Needed Fixtures

### Image Types
- [ ] JPEG with various EXIF orientations (1, 3, 6, 8)
- [ ] PNG files
- [ ] GIF files  
- [ ] BMP files
- [ ] Very large images (>10MB)
- [ ] Very small images (1x1 pixel)
- [ ] Corrupted/broken image files
- [ ] Non-image files (text, binary)

### EXIF Test Images
- [ ] Image with DateTime EXIF
- [ ] Image with GPS data
- [ ] Image with MakerNote
- [ ] Image with thumbnail
- [ ] Image without EXIF

### Edge Cases
- [ ] Filename with spaces
- [ ] Filename with Unicode
- [ ] Filename at PATH_MAX limit
- [ ] Image with dimensions at INT_MAX

## Using Fixtures in Tests

```c
/* Example test using fixtures */
void test_load_jpeg_with_exif(void) {
    PhoImage* img = NewPhoImage("fixtures/images/exif_orientation_6.jpg");
    int result = LoadImageFromFile(img);
    TEST_ASSERT_EQUAL_INT(0, result);
    TEST_ASSERT_NOT_EQUAL_INT(0, img->trueWidth);
}
```
