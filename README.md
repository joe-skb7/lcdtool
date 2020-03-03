# lcdtool

## Description

OLED and LCD displays have vertical byte alignment, so image bits must be
transformed from regular row-major order to OLED order, for image to be
displayed correctly.

In Windows world there is a longstanding "LCD Assistant" tool [1] for converting
images to C arrays, suitable for OLED/LCD displaying. In Linux, we have
ImageMagick `convert` tool, which can generate MONO bitmap binary from image
files, and `xxd` tool, which further converts that bitmap binary to C array. But
OLED has vertical byte alignment, so all bits in that bitmap binary must be
transformed accordingly. This tool provides functionality to do so. Furthermore,
it's able to transform the image bitmap for two OLED addressing modes:

  1. Horizontal addressing: consequent byte is on the right
  2. Vertical addressing: consequent byte is on the bottom

Please consult with SSD1306 datasheet [2] for details. Figure 10-2 shows
vertical byte alignment. Figure 10-3 explains horizontal addressing mode, and
Figure 10-4 explains vertical addressing mode.

## Usage

`lcdtool` is written in UNIX way [3] and only transforms bits ordering. For
preparing the image bitmap binary, consider using `convert` tool from
ImageMagick. For further generating of C array from resulting binary, one can
use `xxd` tool.

Providing image binary bitmap file to convert as an argument:

```
$ lcdtool <mono-image> <width> {v|h}
```

Passing image binary bitmap data via pipe:

```
$ cat <mono-image> | lcdtool <width> {v|h}
```

Please refer to `lcdtool --help` for details.

## Examples

Prepare C array for OLED display from `image.png` image file; array's content
will be printed to console `stdout`; image width is `128` pixels; use OLED
horizontal addressing mode:

```
$ convert image.png -colorspace gray -colors 2 -type bilevel mono:- | \
  lcdtool 128 h | xxd -i
```

Prepare C array for OLED display from `image.png` image file; array's content
(along with variable definitions) will be printed to `image.c` file; image width
is `128` pixels; use OLED vertical addressing mode:

```
$ convert image.png -colorspace gray -colors 2 -type bilevel image.mono
$ lcdtool image.mono 128 v > image.lcd
$ xxd -i image.lcd image.c
```

## Authors

**Sam Protsenko**

## License

The project is licensed under the GPLv3.

## References

[1] http://en.radzio.dxp.pl/bitmap_converter/

[2] https://cdn-shop.adafruit.com/datasheets/SSD1306.pdf

[3] https://homepage.cs.uri.edu/~thenry/resources/unix_art/ch01s06.html
