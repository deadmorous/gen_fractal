#include "batch.hpp"

#include "anim_param.hpp"
#include "interp_curves.hpp"
#include "render_fractal.hpp"
#include "throw.hpp"

#include <QImage>
#include <QPainter>

#include <iomanip>
#include <iostream>
#include <filesystem>
#include <fstream>
#include <optional>
#include <sstream>


using namespace std::string_view_literals;

namespace {

auto renderFractalImage(std::span<const Vec2d> base,
                        std::span<const Vec2d> gen,
                        const FractalViewParam& param,
                        const QSize& size)
    -> QImage
{
    auto img = QImage{ size, QImage::Format_ARGB32 };
    auto p = QPainter{ &img };
    renderFractal(p, {QPoint{}, size}, base, gen, param);
    return img;
}

struct BatchLine
{
    std::vector<Vec2d> base;
    std::vector<Vec2d> gen;
    FractalViewParam viewParam;
    QSize size;
    AnimParam animParam;
};

inline auto field_names_of(TypeTag<QSize>)
    -> std::array<std::string_view, 2>
{ return { "width", "height" }; }

inline auto fields_of(QSize& p)
    -> std::tuple<int&, int&>
{ return std::tie(p.rwidth(), p.rheight()); }

inline auto fields_of(const QSize& p)
    -> std::tuple<int, int>
{ return {p.width(), p.height()}; }

auto lerp(double x0, double x1, double p)
    -> double
{ return x0*(1-p) + x1*p; }

auto lerp(size_t x0, size_t x1, double p)
    -> size_t
{ return static_cast<size_t>(x0*(1-p) + x1*p); }

auto lerp(int x0, int x1, double p)
    -> int
{ return static_cast<int>(x0*(1-p) + x1*p); }

auto lerp(bool x0, bool x1, double p)
    -> bool
{ return lerp(x0? 1.: 0., x1? 1.:0, p) >= 0.5; }

template <typename T>
auto lerpStruct(const T& x0, const T& x1, double p)
    -> T
{
    auto result = T{};
    auto f0 = fields_of(x0);
    auto f1 = fields_of(x1);
    auto f = fields_of(result);
    constexpr auto fieldCount = std::tuple_size_v<decltype(f)>;
    [&]<size_t... I>(std::index_sequence<I...>)
    {
        ((std::get<I>(f) = lerp(std::get<I>(f0), std::get<I>(f1), p)), ...);
    }(std::make_index_sequence<fieldCount>());
    return result;
}


auto interpolateBatchLines(std::span<const BatchLine> batchLines)
    -> std::vector<BatchLine>
{
    auto getBase = []( const BatchLine& batchLine )
        -> const std::vector< Vec2d >&
    { return batchLine.base; };

    auto getGen = []( const BatchLine& batchLine )
        -> const std::vector< Vec2d >
    {
        auto result = std::vector< Vec2d >( batchLine.gen );
        for(auto& v: result)
        {
            if (batchLine.animParam.reflectX)
                v[0] = -v[0];
            if (batchLine.animParam.reflectY)
                v[1] = -v[1];
        }
        return result;
    };

    auto getSubdiv = []( const BatchLine& batchLine )
        -> size_t
    { return batchLine.animParam.frameCountAfter; };

    auto getSlopeF = []( const BatchLine& batchLine )
        -> double
    { return batchLine.animParam.slopeFactor; };

    auto baseCurves =
        interpCurves(batchLines, getBase, getSubdiv, getSlopeF);

    auto genCurves =
        interpCurves(batchLines, getGen, getSubdiv, getSlopeF);

    auto result = std::vector<BatchLine>{};

    auto curvesAtFrame =
        [](const std::vector<std::vector<Vec2d>>& curves, size_t iframe)
            -> std::vector<Vec2d>
    {
        auto ncurves = curves.size();
        auto result = std::vector<Vec2d>(ncurves);
        for (auto icurve=0; icurve<ncurves; ++icurve)
            result[icurve] = curves[icurve][iframe];
        return result;
    };

    result.push_back(batchLines.front());
    size_t iframe = 1;
    auto nframes = baseCurves.front().size();
    for (size_t iline=0, nlines=batchLines.size(); iline+1<nlines; ++iline)
    {
        const auto& bl0 = batchLines[iline];
        const auto& bl1 = batchLines[iline+1];
        auto subdiv = getSubdiv(bl0);
        auto h = 1. / subdiv;
        for (size_t isub=0; isub<subdiv; ++isub, ++iframe)
        {
            auto p = (isub+1) * h;
            result.push_back(
                {
                    .base = curvesAtFrame(baseCurves, iframe),
                    .gen = curvesAtFrame(genCurves, iframe),
                    .viewParam = lerpStruct(bl0.viewParam, bl1.viewParam, p),
                    .size = lerpStruct(bl0.size, bl1.size, p),
                    .animParam = {}
                });
        }
    }
    assert(iframe == nframes);

    // deBUG: Print interpolated generator curves
    // auto ncurves = result[0].gen.size();
    // for (iframe=0; iframe<nframes; ++iframe)
    // {
    //     std::cout << iframe;
    //     for (size_t icurve=0; icurve<ncurves; ++icurve)
    //     {
    //         const auto& v = result[iframe].gen[icurve];
    //         std::cout << '\t' << v[0] << '\t' << v[1];
    //     }
    //     std::cout << std::endl;
    // }

    return result;
}

} // anonymous namespace

auto batch(const QString& batchFileName)
    -> int
{
    namespace fs = std::filesystem;

    try
    {
        const auto outputDirName = "gen_fractal.out";

        auto outputFileName = [&](size_t number)
            -> QString
        {
            auto s = std::ostringstream{};
            s << "img_"
              << std::setw(6) << std::setfill('0') << number
              << ".png";
            return QString::fromStdString(fs::path(outputDirName) / s.str());
        };

        auto in = std::ifstream(batchFileName.toStdString());
        if (!in.is_open())
            throw_("Failed to open input file '",
                   batchFileName.toStdString(), "'");

        size_t lineNumber = 0;
        auto batchLines = std::vector<BatchLine>{};
        while (true)
        {
            ++lineNumber;

            std::string line;
            std::getline(in, line);
            if (in.fail())
                break;
            if (line.empty())
                continue;

            if (lineNumber == 1)
                continue;   // Skip header

            size_t pos = 0;
            auto nextToken = [&]()-> std::optional<std::string>
            {
                if (pos == std::string::npos || pos == line.size())
                    return std::nullopt;

                auto pos2 = line.find_first_of(',', pos);
                auto n = (pos2 == std::string::npos ? line.size() : pos2) - pos;
                auto token = line.substr(pos, n);
                pos = pos2 == std::string::npos? std::string::npos: pos2 + 1;
                return token;
            };

            auto nextValue =
                [&]<typename T>(T& value)
                -> bool
            {
                auto tok = nextToken();
                if (!tok || *tok == "*"sv)
                    return false;
                std::istringstream s{ *tok };
                s >> value;
                if (s.fail())
                    throw_("Failed to parse value '", *tok,
                           "', line ", lineNumber);
                return true;
            };

            auto ensureNextValue =
                [&]<typename T>(T& value)
            {
                if (!nextValue(value))
                    throw_("Too few values in line ", lineNumber);
            };

            auto read2dLine = [&]() -> std::vector<Vec2d>
            {
                auto result = std::vector<Vec2d>{};
                while (true)
                {
                    double x;
                    double y;
                    if (!nextValue(x))
                        break;
                    if (!nextValue(y))
                        throw_("Odd number of 2d line coordinates, line ",
                               lineNumber);
                    result.emplace_back(x, y);
                }
                return result;
            };

            auto readStruct = [&]<typename T>(TypeTag<T> tag) -> T
            {
                T result;

                auto fields = fields_of(result);
                constexpr auto fieldCount = std::tuple_size_v<decltype(fields)>;
                [&]<size_t... I>(std::index_sequence<I...>)
                {
                    (ensureNextValue(std::get<I>(fields)), ...);
                }(std::make_index_sequence<fieldCount>());

                return result;
            };

            batchLines.push_back({
                .base         = read2dLine(),
                .gen          = read2dLine(),
                .viewParam    = readStruct(Type<FractalViewParam>),
                .size         = readStruct(Type<QSize>),
                .animParam    = readStruct(Type<AnimParam>)});

            if (nextToken())
                std::cout << "NOTE: Ignoring extra elements in line "
                          << lineNumber << std::endl;
        }

        if (fs::exists(outputDirName))
            throw_("Output directory '", outputDirName, "' already exists");

        if (!fs::create_directory(outputDirName))
            throw_("Failed to create output directory '", outputDirName, "'");

        std::cout << "Generating images in directory '" << outputDirName << "'"
                  << std::endl;

        size_t iframe = 0;
        for (auto& interpolatedLine: interpolateBatchLines(batchLines))
        {
            auto size = interpolatedLine.size;

            // Make image width and height even, because ffmpeg may want it
            size.rwidth() &= ~1;
            size.rheight() &= ~1;

            renderFractalImage(interpolatedLine.base,
                               interpolatedLine.gen,
                               interpolatedLine.viewParam,
                               size)
                .save(outputFileName(++iframe));
        }

        return EXIT_SUCCESS;
    }

    catch (const std::exception& e)
    {
        std::cerr << "ERROR: " << e.what() << std::endl;
        return EXIT_FAILURE;
    }
}
