#include <algorithm>
#include <cmath>
#include <cstdint>
#include <filesystem>
#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>

#include "lodepng.h"

namespace fs = std::filesystem;

struct Image {
    unsigned width = 0;
    unsigned height = 0;
    std::vector<unsigned char> rgba;
};

struct Pixel {
    float r = 0.0f;
    float g = 0.0f;
    float b = 0.0f;
    float a = 0.0f;
};

static Pixel get_pixel(const Image &image, unsigned x, unsigned y) {
    const std::size_t index = (static_cast<std::size_t>(y) * image.width + x) * 4;
    return {
        image.rgba[index + 0] / 255.0f,
        image.rgba[index + 1] / 255.0f,
        image.rgba[index + 2] / 255.0f,
        image.rgba[index + 3] / 255.0f,
    };
}

static void set_pixel(Image &image, unsigned x, unsigned y, const Pixel &pixel) {
    const std::size_t index = (static_cast<std::size_t>(y) * image.width + x) * 4;
    image.rgba[index + 0] = static_cast<unsigned char>(std::clamp(std::lround(pixel.r * 255.0f), 0l, 255l));
    image.rgba[index + 1] = static_cast<unsigned char>(std::clamp(std::lround(pixel.g * 255.0f), 0l, 255l));
    image.rgba[index + 2] = static_cast<unsigned char>(std::clamp(std::lround(pixel.b * 255.0f), 0l, 255l));
    image.rgba[index + 3] = static_cast<unsigned char>(std::clamp(std::lround(pixel.a * 255.0f), 0l, 255l));
}

static Image load_png(const fs::path &path) {
    Image image;
    unsigned error = lodepng::decode(image.rgba, image.width, image.height, path.string());
    if (error != 0) {
        throw std::runtime_error("Failed to load " + path.string() + ": " + lodepng_error_text(error));
    }
    return image;
}

static void write_png(const fs::path &path, const Image &image) {
    unsigned error = lodepng::encode(path.string(), image.rgba, image.width, image.height);
    if (error != 0) {
        throw std::runtime_error("Failed to write " + path.string() + ": " + lodepng_error_text(error));
    }
}

static Pixel sample_bilinear(const Image &image, float x, float y) {
    const auto clamp_x = [&](int value) {
        return std::clamp(value, 0, static_cast<int>(image.width) - 1);
    };
    const auto clamp_y = [&](int value) {
        return std::clamp(value, 0, static_cast<int>(image.height) - 1);
    };

    const int x0 = static_cast<int>(std::floor(x));
    const int y0 = static_cast<int>(std::floor(y));
    const int x1 = x0 + 1;
    const int y1 = y0 + 1;
    const float tx = x - static_cast<float>(x0);
    const float ty = y - static_cast<float>(y0);

    const Pixel p00 = get_pixel(image, clamp_x(x0), clamp_y(y0));
    const Pixel p10 = get_pixel(image, clamp_x(x1), clamp_y(y0));
    const Pixel p01 = get_pixel(image, clamp_x(x0), clamp_y(y1));
    const Pixel p11 = get_pixel(image, clamp_x(x1), clamp_y(y1));

    const auto lerp = [](float a, float b, float t) {
        return a + (b - a) * t;
    };

    const auto premultiply = [](const Pixel &p) {
        return Pixel{p.r * p.a, p.g * p.a, p.b * p.a, p.a};
    };

    const auto unpremultiply = [](const Pixel &p) {
        if (p.a <= 0.0f) {
            return Pixel{};
        }
        return Pixel{p.r / p.a, p.g / p.a, p.b / p.a, p.a};
    };

    const Pixel a = premultiply(p00);
    const Pixel b = premultiply(p10);
    const Pixel c = premultiply(p01);
    const Pixel d = premultiply(p11);

    Pixel top{lerp(a.r, b.r, tx), lerp(a.g, b.g, tx), lerp(a.b, b.b, tx), lerp(a.a, b.a, tx)};
    Pixel bottom{lerp(c.r, d.r, tx), lerp(c.g, d.g, tx), lerp(c.b, d.b, tx), lerp(c.a, d.a, tx)};
    Pixel out{lerp(top.r, bottom.r, ty), lerp(top.g, bottom.g, ty), lerp(top.b, bottom.b, ty), lerp(top.a, bottom.a, ty)};
    return unpremultiply(out);
}

static Image resize_image(const Image &source, unsigned width, unsigned height) {
    Image resized;
    resized.width = width;
    resized.height = height;
    resized.rgba.resize(static_cast<std::size_t>(width) * height * 4);

    const float scale_x = static_cast<float>(source.width) / static_cast<float>(width);
    const float scale_y = static_cast<float>(source.height) / static_cast<float>(height);

    for (unsigned y = 0; y < height; ++y) {
        for (unsigned x = 0; x < width; ++x) {
            const float src_x = (static_cast<float>(x) + 0.5f) * scale_x - 0.5f;
            const float src_y = (static_cast<float>(y) + 0.5f) * scale_y - 0.5f;
            set_pixel(resized, x, y, sample_bilinear(source, src_x, src_y));
        }
    }

    return resized;
}

static void blend_over(Image &base, const Image &overlay, unsigned dst_x, unsigned dst_y) {
    for (unsigned y = 0; y < overlay.height; ++y) {
        for (unsigned x = 0; x < overlay.width; ++x) {
            const unsigned base_x = dst_x + x;
            const unsigned base_y = dst_y + y;
            if (base_x >= base.width || base_y >= base.height) {
                continue;
            }

            const Pixel src = get_pixel(overlay, x, y);
            Pixel dst = get_pixel(base, base_x, base_y);
            const float out_a = src.a + dst.a * (1.0f - src.a);
            Pixel out;
            if (out_a <= 0.0f) {
                out = Pixel{};
            } else {
                out.r = (src.r * src.a + dst.r * dst.a * (1.0f - src.a)) / out_a;
                out.g = (src.g * src.a + dst.g * dst.a * (1.0f - src.a)) / out_a;
                out.b = (src.b * src.a + dst.b * dst.a * (1.0f - src.a)) / out_a;
                out.a = out_a;
            }
            set_pixel(base, base_x, base_y, out);
        }
    }
}

static Image render_overlay_icon(const Image &icon, const Image &overlay, unsigned icon_size, unsigned overlay_size) {
    Image rendered = resize_image(icon, icon_size, icon_size);
    const Image scaled_overlay = resize_image(overlay, overlay_size, overlay_size);
    blend_over(rendered, scaled_overlay, icon_size - overlay_size, icon_size - overlay_size);
    return rendered;
}

static Image make_contact_sheet(const std::vector<Image> &images) {
    const unsigned padding = 8;
    const unsigned label_height = 0;
    unsigned total_width = padding;
    unsigned max_height = 0;
    for (const auto &image : images) {
        total_width += image.width + padding;
        max_height = std::max(max_height, image.height);
    }

    Image sheet;
    sheet.width = total_width;
    sheet.height = max_height + padding * 2 + label_height;
    sheet.rgba.assign(static_cast<std::size_t>(sheet.width) * sheet.height * 4, 255);

    for (unsigned y = 0; y < sheet.height; ++y) {
        for (unsigned x = 0; x < sheet.width; ++x) {
            set_pixel(sheet, x, y, Pixel{1.0f, 1.0f, 1.0f, 1.0f});
        }
    }

    unsigned cursor_x = padding;
    for (const auto &image : images) {
        const unsigned offset_y = padding + (max_height - image.height) / 2;
        blend_over(sheet, image, cursor_x, offset_y);
        cursor_x += image.width + padding;
    }

    return sheet;
}

int main(int argc, char **argv) {
    try {
        const fs::path repo_root = fs::path(__FILE__).parent_path().parent_path();
        const fs::path default_icon = repo_root / "ksni_overlay_icon_pixmap_bug" / "data" / "default256.png";
        const fs::path default_overlay = repo_root / "ksni_overlay_icon_pixmap_bug" / "data" / "overlay.png";
        const fs::path output_dir = argc >= 4 ? fs::path(argv[3]) : (repo_root / "outputs");
        const fs::path icon_path = argc >= 2 ? fs::path(argv[1]) : default_icon;
        const fs::path overlay_path = argc >= 3 ? fs::path(argv[2]) : default_overlay;

        const Image icon = load_png(icon_path);
        const Image overlay = load_png(overlay_path);
        fs::create_directories(output_dir);

        const std::vector<unsigned> icon_sizes{16, 22, 32, 48};
        std::vector<Image> outputs;
        outputs.reserve(icon_sizes.size());

        for (unsigned size : icon_sizes) {
            const unsigned overlay_size = size == 48 ? 16u : 8u;
            Image rendered = render_overlay_icon(icon, overlay, size, overlay_size);
            const fs::path out_path = output_dir / (std::to_string(size) + "x" + std::to_string(size) + ".png");
            write_png(out_path, rendered);
            outputs.push_back(std::move(rendered));
            std::cout << out_path << '\n';
        }

        write_png(output_dir / "all.png", make_contact_sheet(outputs));
        std::cout << (output_dir / "all.png") << '\n';
        return 0;
    } catch (const std::exception &error) {
        std::cerr << error.what() << '\n';
        return 1;
    }
}
