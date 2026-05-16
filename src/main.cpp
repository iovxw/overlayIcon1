#include <filesystem>
#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>

#include <QGuiApplication>
#include <QIcon>
#include <QImage>
#include <QPainter>
#include <QPixmap>

#include "statusnotifieritemsource_overlayicon.h"

namespace fs = std::filesystem;

static QString to_qstring(const fs::path &path)
{
    return QString::fromUtf8(path.string().c_str());
}

static void require_file(const fs::path &path)
{
    if (!fs::exists(path)) {
        throw std::runtime_error(
            "Input file does not exist: " + path.string()
            + ". If you are using the bundled sample inputs, run `git submodule update --init --recursive` first."
        );
    }
}

static void require_pixmap(const QPixmap &pixmap, const fs::path &path)
{
    if (pixmap.isNull()) {
        throw std::runtime_error("Failed to load image: " + path.string());
    }
}

static void require_save(const bool ok, const fs::path &path)
{
    if (!ok) {
        throw std::runtime_error("Failed to write image: " + path.string());
    }
}

static QImage make_contact_sheet(const std::vector<QPixmap> &pixmaps)
{
    constexpr int padding = 8;
    int total_width = padding;
    int max_height = 0;
    for (const QPixmap &pixmap : pixmaps) {
        total_width += pixmap.width() + padding;
        max_height = std::max(max_height, pixmap.height());
    }

    QImage sheet(total_width, max_height + padding * 2, QImage::Format_ARGB32_Premultiplied);
    sheet.fill(Qt::white);

    QPainter painter(&sheet);
    int cursor_x = padding;
    for (const QPixmap &pixmap : pixmaps) {
        const int offset_y = padding + (max_height - pixmap.height()) / 2;
        painter.drawPixmap(cursor_x, offset_y, pixmap);
        cursor_x += pixmap.width() + padding;
    }
    painter.end();

    return sheet;
}

int main(int argc, char **argv)
{
    try {
        qputenv("QT_QPA_PLATFORM", "offscreen");
        QGuiApplication app(argc, argv);

        const fs::path repo_root = fs::path(REPO_ROOT);
        const fs::path default_icon = repo_root / "ksni_overlay_icon_pixmap_bug" / "data" / "default256.png";
        const fs::path default_overlay = repo_root / "ksni_overlay_icon_pixmap_bug" / "data" / "overlay.png";
        const fs::path output_dir = argc >= 4 ? fs::path(argv[3]) : (repo_root / "outputs");
        const fs::path icon_path = argc >= 2 ? fs::path(argv[1]) : default_icon;
        const fs::path overlay_path = argc >= 3 ? fs::path(argv[2]) : default_overlay;

        require_file(icon_path);
        require_file(overlay_path);
        const QPixmap base_pixmap(to_qstring(icon_path));
        const QPixmap overlay_pixmap(to_qstring(overlay_path));
        require_pixmap(base_pixmap, icon_path);
        require_pixmap(overlay_pixmap, overlay_path);

        QIcon icon(base_pixmap);
        QIcon overlay_icon(overlay_pixmap);
        StatusNotifierItemSource source;
        source.overlayIcon(&icon, &overlay_icon);

        fs::create_directories(output_dir);

        const std::vector<int> sizes{16, 22, 32, 48};
        std::vector<QPixmap> generated_pixmaps;
        generated_pixmaps.reserve(sizes.size());

        for (const int size : sizes) {
            QPixmap result = icon.pixmap(size, size);
            const fs::path output_path = output_dir / (std::to_string(size) + "x" + std::to_string(size) + ".png");
            require_save(result.save(to_qstring(output_path)), output_path);
            generated_pixmaps.push_back(result);
            std::cout << output_path << '\n';
        }

        const fs::path sheet_path = output_dir / "all.png";
        require_save(make_contact_sheet(generated_pixmaps).save(to_qstring(sheet_path)), sheet_path);
        std::cout << sheet_path << '\n';
        return 0;
    } catch (const std::exception &error) {
        std::cerr << error.what() << '\n';
        return 1;
    }
}
