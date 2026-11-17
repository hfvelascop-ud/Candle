// This file is a part of "Candle" application.
// Copyright 2015-2021 Hayrullin Denis Ravilevich

#ifndef INTERPOLATION
#define INTERPOLATION

#include <QVector>
#include <QRectF>
#include <QAbstractTableModel>
#include <cmath>

class Interpolation
{
public:
    // These values are persisted in settings and in candle map v3 files.
    enum Type { Bicubic = 0, Bilinear = 1 };

    static double bilinearInterpolate(QRectF borderRect, QAbstractTableModel *basePoints, double x, double y)
    {
        // A partially probed or invalid map must not supply a made-up height.
        if (!basePoints || basePoints->rowCount() < 2) return qQNaN();
        const int columns = basePoints->columnCount();
        const int rows = basePoints->rowCount();
        if (columns < 2 || !qIsFinite(x) || !qIsFinite(y)
                || !qIsFinite(borderRect.x()) || !qIsFinite(borderRect.y())
                || !qIsFinite(borderRect.width()) || !qIsFinite(borderRect.height())
                || borderRect.width() <= 0 || borderRect.height() <= 0) return qQNaN();

        // Hold the nearest edge outside the measured area; do not extrapolate.
        const double gx = qBound(0.0, (x - borderRect.x()) / borderRect.width() * (columns - 1), double(columns - 1));
        const double gy = qBound(0.0, (y - borderRect.y()) / borderRect.height() * (rows - 1), double(rows - 1));
        const int ix = qMin(int(std::floor(gx)), columns - 2);
        const int iy = qMin(int(std::floor(gy)), rows - 2);
        const double u = gx - ix;
        const double v = gy - iy;

        double h[2][2];
        for (int j = 0; j < 2; ++j) {
            for (int i = 0; i < 2; ++i) {
                bool ok = false;
                h[j][i] = basePoints->data(basePoints->index(iy + j, ix + i), Qt::UserRole).toDouble(&ok);
                if (!ok || !qIsFinite(h[j][i])) return qQNaN();
            }
        }

        return (1 - v) * ((1 - u) * h[0][0] + u * h[0][1])
                + v * ((1 - u) * h[1][0] + u * h[1][1]);
    }

    static double interpolate(QRectF borderRect, QAbstractTableModel *basePoints, double x, double y, int type)
    {
        return type == Bilinear ? bilinearInterpolate(borderRect, basePoints, x, y)
                                : bicubicInterpolate(borderRect, basePoints, x, y);
    }

    static double cubicInterpolate(double p[4], double x)
    {
        return p[1] + 0.5 * x * (p[2] - p[0] + x *(2.0 * p[0] - 5.0 * p[1]
                + 4.0 * p[2] - p[3] + x * (3.0 * (p[1] - p[2]) + p[3] - p[0])));
    }

    static double bicubicInterpolate(double p[4][4], double x, double y)
    {
        double arr[4];
        arr[0] = cubicInterpolate(p[0], x);
        arr[1] = cubicInterpolate(p[1], x);
        arr[2] = cubicInterpolate(p[2], x);
        arr[3] = cubicInterpolate(p[3], x);
        return cubicInterpolate(arr, y);
    }

    static double bicubicInterpolate(QRectF borderRect, QAbstractTableModel *basePoints, double x, double y)
    {
        // Setup grid
        int gridPointsX = basePoints->columnCount();
        int gridPointsY = basePoints->rowCount();

        double gridStepX = gridPointsX > 1 ? borderRect.width() / (gridPointsX - 1) : 0;
        double gridStepY = gridPointsY > 1 ? borderRect.height() / (gridPointsY - 1) : 0;

        // Get 16 points
        x -= borderRect.x();
        y -= borderRect.y();

        // int ix = trunc(x / gridStepX);
        // int iy = trunc(y / gridStepY);
        int ix = (x / gridStepX);
        int iy = (y / gridStepY);

        if (ix > basePoints->columnCount() - 2) ix = basePoints->columnCount() - 2;
        if (iy > basePoints->rowCount() - 2) iy = basePoints->rowCount() - 2;

        double p[4][4];

        p[0][0] = basePoints->data(basePoints->index((iy > 0 ? iy - 1 : iy), (ix > 0 ? ix - 1 : ix)), Qt::UserRole).toDouble();
        p[0][1] = basePoints->data(basePoints->index((iy > 0 ? iy - 1 : iy), ix), Qt::UserRole).toDouble();
        p[0][2] = basePoints->data(basePoints->index((iy > 0 ? iy - 1 : iy), ix + 1), Qt::UserRole).toDouble();
        p[0][3] = basePoints->data(basePoints->index((iy > 0 ? iy - 1 : iy), (ix < basePoints->columnCount() - 2 ? ix + 2: ix + 1)), Qt::UserRole).toDouble();

        p[1][0] = basePoints->data(basePoints->index(iy, ix > 0 ? ix - 1 : ix), Qt::UserRole).toDouble();
        p[1][1] = basePoints->data(basePoints->index(iy, ix), Qt::UserRole).toDouble();
        p[1][2] = basePoints->data(basePoints->index(iy, ix + 1), Qt::UserRole).toDouble();
        p[1][3] = basePoints->data(basePoints->index(iy, ix < basePoints->columnCount() - 2 ? ix + 2: ix + 1), Qt::UserRole).toDouble();

        p[2][0] = basePoints->data(basePoints->index(iy + 1, ix > 0 ? ix - 1 : ix), Qt::UserRole).toDouble();
        p[2][1] = basePoints->data(basePoints->index(iy + 1, ix), Qt::UserRole).toDouble();
        p[2][2] = basePoints->data(basePoints->index(iy + 1, ix + 1), Qt::UserRole).toDouble();
        p[2][3] = basePoints->data(basePoints->index(iy + 1, ix < basePoints->columnCount() - 2 ? ix + 2 : ix + 1), Qt::UserRole).toDouble();

        p[3][0] = basePoints->data(basePoints->index(iy < basePoints->rowCount() - 2 ? iy + 2 : iy + 1, ix > 0 ? ix - 1 : ix), Qt::UserRole).toDouble();
        p[3][1] = basePoints->data(basePoints->index(iy < basePoints->rowCount() - 2 ? iy + 2 : iy + 1, ix), Qt::UserRole).toDouble();
        p[3][2] = basePoints->data(basePoints->index(iy < basePoints->rowCount() - 2 ? iy + 2 : iy + 1, ix + 1), Qt::UserRole).toDouble();
        p[3][3] = basePoints->data(basePoints->index(iy < basePoints->rowCount() - 2 ? iy + 2 : iy + 1, ix < basePoints->columnCount() - 2 ? ix + 2 : ix + 1), Qt::UserRole).toDouble();

        // Interpolate
        return Interpolation::bicubicInterpolate(p, x / gridStepX - ix, y / gridStepY - iy);
    }
};

#endif // INTERPOLATION

