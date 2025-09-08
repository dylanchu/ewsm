#pragma once
#include <wx/bitmap.h>
#include <wx/mstream.h>


namespace utils
{
    static wxBitmap _load_embedded_png(const unsigned char* data, size_t size)
    {
        // 创建内存流并加载为 wxImage
        wxMemoryInputStream stream(data, size);
        wxImage image;
        if (!image.LoadFile(stream, wxBITMAP_TYPE_PNG)) {
            return wxNullBitmap;
        }

        // 24位PNG：应用品红色透明掩码
        if (!image.HasAlpha()) {
            wxBitmap bitmap(image);
            const wxColour mask_color(255, 0, 255); // 品红色 #FF00FF
            bitmap.SetMask(new wxMask(bitmap, mask_color));
            return bitmap;
        }
        // 32位PNG：直接使用原有透明度
        return { image };
    }

#define load_embedded_png(name) _load_embedded_png(EmbeddedImages::name, EmbeddedImages::name##_len)
    // usage:
    // #include "res/icons_canvas.h"
    // auto bitmap = load_embedded_png(EmbeddedImages::pencil);

    ///////////////////////////////////////////////////////////////////////////////

    static wxBitmap load_png_with_mask(const wxString& res_name, const wxColour& mask_color)
    {
        auto bitmap = wxBitmap(res_name, wxBITMAP_TYPE_PNG_RESOURCE);
        bitmap.SetMask(new wxMask(bitmap, mask_color));
        return bitmap;
    }

    static wxImage apply_box_blur(const wxImage& image, const int radius)
    {
        if (radius <= 0) return image;

        const int width = image.GetWidth();
        const int height = image.GetHeight();
        const bool has_alpha = image.HasAlpha();

        // 创建结果图像（保留原始格式）
        wxImage blurred = image.Copy();

        // 处理 RGB 数据
        unsigned char* src_data = image.GetData();
        unsigned char* dst_data = blurred.GetData();

        // 处理 Alpha 数据（如果有）
        unsigned char* src_alpha = has_alpha ? image.GetAlpha() : nullptr;
        unsigned char* dst_alpha = has_alpha ? blurred.GetAlpha() : nullptr;

        // 临时缓冲区（用于分离通道处理）
        std::vector<unsigned char> temp_rgb(width * height * 3);
        std::vector<unsigned char> temp_alpha;
        if (has_alpha) temp_alpha.resize(width * height);

        // 水平模糊 (RGB)
        for (int y = 0; y < height; ++y) {
            for (int x = 0; x < width; ++x) {
                int r = 0, g = 0, b = 0;
                int count = 0;

                for (int dx = -radius; dx <= radius; ++dx) {
                    int px = std::clamp(x + dx, 0, width - 1);
                    int idx = (y * width + px) * 3;
                    r += src_data[idx];
                    g += src_data[idx + 1];
                    b += src_data[idx + 2];
                    count++;
                }

                int idx = (y * width + x) * 3;
                temp_rgb[idx] = r / count;
                temp_rgb[idx + 1] = g / count;
                temp_rgb[idx + 2] = b / count;
            }
        }

        // 水平模糊 (Alpha)
        if (has_alpha) {
            for (int y = 0; y < height; ++y) {
                for (int x = 0; x < width; ++x) {
                    int a = 0;
                    int count = 0;

                    for (int dx = -radius; dx <= radius; ++dx) {
                        int px = std::clamp(x + dx, 0, width - 1);
                        int idx = y * width + px;
                        a += src_alpha[idx];
                        count++;
                    }

                    temp_alpha[y * width + x] = a / count;
                }
            }
        }

        // 垂直模糊 (RGB)
        for (int y = 0; y < height; ++y) {
            for (int x = 0; x < width; ++x) {
                int r = 0, g = 0, b = 0;
                int count = 0;

                for (int dy = -radius; dy <= radius; ++dy) {
                    int py = std::clamp(y + dy, 0, height - 1);
                    int idx = (py * width + x) * 3;
                    r += temp_rgb[idx];
                    g += temp_rgb[idx + 1];
                    b += temp_rgb[idx + 2];
                    count++;
                }

                int idx = (y * width + x) * 3;
                dst_data[idx] = r / count;
                dst_data[idx + 1] = g / count;
                dst_data[idx + 2] = b / count;
            }
        }

        // 垂直模糊 (Alpha)
        if (has_alpha) {
            for (int y = 0; y < height; ++y) {
                for (int x = 0; x < width; ++x) {
                    int a = 0;
                    int count = 0;

                    for (int dy = -radius; dy <= radius; ++dy) {
                        int py = std::clamp(y + dy, 0, height - 1);
                        int idx = py * width + x;
                        a += temp_alpha[idx];
                        count++;
                    }

                    dst_alpha[y * width + x] = a / count;
                }
            }
        }

        return blurred;
    }

    static wxBitmap create_color_bitmap(const wxColour& color, const int size=24)
    {
        wxBitmap bmp(size, size);
        wxMemoryDC dc;
        dc.SelectObject(bmp);
        // 绘制背景
        dc.SetBrush(wxBrush(color));
        dc.SetPen(*wxBLACK_PEN);
        dc.DrawRectangle(0, 0, size, size);
        // 添加边框效果
        dc.SetPen(wxPen(wxSystemSettings::GetColour(wxSYS_COLOUR_3DDKSHADOW)));
        dc.DrawLine(0, 0, size, 0);
        dc.DrawLine(0, 0, 0, size);
        dc.SetPen(wxPen(wxSystemSettings::GetColour(wxSYS_COLOUR_3DLIGHT)));
        dc.DrawLine(size-1, 1, size-1, size-1);
        dc.DrawLine(1, size-1, size-1, size-1);
        dc.SelectObject(wxNullBitmap);
        return bmp;
    }

} // namespace utils
