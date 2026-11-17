#include <QCoreApplication>
#include <QtMath>
#include <algorithm>
#include <cstdlib>
#include <iostream>
#include "tables/heightmaptablemodel.h"
#include "utils/interpolation.h"

static int checks = 0;

static void require(bool condition, const char *message)
{
    ++checks;
    if (!condition) {
        std::cerr << "FAILED: " << message << '\n';
        std::exit(EXIT_FAILURE);
    }
}

static void near(double actual, double expected, const char *message)
{
    require(qIsFinite(actual) && qAbs(actual - expected) < 1e-10, message);
}

static void put(HeightMapTableModel &grid, int x, int y, double z)
{
    grid.setData(grid.index(y, x), z, Qt::UserRole);
}

int main(int argc, char **argv)
{
    QCoreApplication app(argc, argv);
    HeightMapTableModel grid;
    grid.resize(2, 2);
    const QRectF border(10, -4, 30, 5);
    put(grid, 0, 0, 0.0);
    put(grid, 1, 0, 0.2);
    put(grid, 0, 1, 0.6);
    put(grid, 1, 1, 1.0);

    near(Interpolation::bilinearInterpolate(border, &grid, 10, -4), 0.0, "lower-left measured node");
    near(Interpolation::bilinearInterpolate(border, &grid, 40, -4), 0.2, "lower-right measured node");
    near(Interpolation::bilinearInterpolate(border, &grid, 10, 1), 0.6, "upper-left measured node");
    near(Interpolation::bilinearInterpolate(border, &grid, 40, 1), 1.0, "upper-right measured node");
    near(Interpolation::bilinearInterpolate(border, &grid, 25, -1.5), 0.45, "asymmetric cell midpoint");
    near(Interpolation::bilinearInterpolate(border, &grid, 17.5, -0.25), 0.5375, "noncentral weighted point");
    near(Interpolation::bilinearInterpolate(border, &grid, 100, -1.5), 0.6, "outside area holds edge");
    near(Interpolation::bilinearInterpolate(border, &grid, -100, -100), 0.0, "outside area holds corner");
    near(Interpolation::interpolate(border, &grid, 17.5, -0.25, Interpolation::Bilinear),
         0.5375, "method dispatch uses bilinear");
    near(Interpolation::interpolate(border, &grid, 17.5, -0.25, Interpolation::Bicubic),
         Interpolation::bicubicInterpolate(border, &grid, 17.5, -0.25), "legacy method unchanged");
    near(Interpolation::interpolate(border, &grid, 17.5, -0.25, -1),
         Interpolation::bicubicInterpolate(border, &grid, 17.5, -0.25), "unknown type defaults to legacy");
    require(qAbs(Interpolation::interpolate(border, &grid, 17.5, -0.25, Interpolation::Bilinear)
                 - Interpolation::interpolate(border, &grid, 17.5, -0.25, Interpolation::Bicubic)) > 1e-4,
            "switch changes the computed height on a curved cell");

    // An offset, non-square grid catches transposed X/Y and wrong cell indices.
    const QRectF planeBorder(-3, 7, 30, 5);
    grid.resize(7, 3);
    for (int j = 0; j < 3; ++j)
        for (int i = 0; i < 7; ++i)
            put(grid, i, j, 0.013 * (-3 + i * 5) - 0.021 * (7 + j * 2.5) + 0.12);

    for (int j = 0; j <= 20; ++j) {
        for (int i = 0; i <= 120; ++i) {
            const double x = -3 + i * 0.25;
            const double y = 7 + j * 0.25;
            const double height = 0.013 * x - 0.021 * y + 0.12;
            near(Interpolation::bilinearInterpolate(planeBorder, &grid, x, y), height, "inclined plane reproduction");
            near(-0.05 + Interpolation::interpolate(planeBorder, &grid, x, y, Interpolation::Bilinear),
                 height - 0.05, "nominal cut plus interpolated correction");
        }
    }

    // Strongly alternating heights must stay within the surrounding four nodes.
    for (int j = 0; j < 3; ++j)
        for (int i = 0; i < 7; ++i)
            put(grid, i, j, ((i + j) % 2) ? 0.23 : -0.07);
    for (int j = 0; j <= 20; ++j)
        for (int i = 0; i <= 120; ++i) {
            const double z = Interpolation::bilinearInterpolate(planeBorder, &grid, -3 + i * 0.25, 7 + j * 0.25);
            require(z >= -0.07000000001 && z <= 0.23000000001, "no overshoot");
        }
    for (int j = 0; j < 3; ++j)
        for (int i = 0; i < 7; ++i)
            near(Interpolation::bilinearInterpolate(planeBorder, &grid, -3 + i * 5, 7 + j * 2.5),
                 ((i + j) % 2) ? 0.23 : -0.07, "all measured nodes retained");

    put(grid, 0, 0, qQNaN());
    require(qIsNaN(Interpolation::bilinearInterpolate(planeBorder, &grid, -2, 8)), "unmeasured height stays invalid");
    require(qIsNaN(Interpolation::bilinearInterpolate(QRectF(0, 0, 0, 1), &grid, 0, 0)), "zero-width map rejected");
    require(qIsNaN(Interpolation::bilinearInterpolate(planeBorder, &grid, qQNaN(), 0)), "invalid coordinate rejected");
    require(qIsNaN(Interpolation::bilinearInterpolate(planeBorder, nullptr, 0, 0)), "null map rejected");
    grid.resize(1, 2);
    require(qIsNaN(Interpolation::bilinearInterpolate(planeBorder, &grid, 0, 0)), "single column rejected");
    grid.clear();
    require(qIsNaN(Interpolation::bilinearInterpolate(planeBorder, &grid, 0, 0)), "empty map rejected before column access");

    std::cout << "Passed " << checks << " heightmap checks\n";
    return EXIT_SUCCESS;
}
