/*
 MDAL - Mesh Data Abstraction Library (MIT License)
 Copyright (C) 2018 Peter Petrik (zilolv at gmail dot com)
*/

#ifndef MDAL_DEFINES_HPP
#define MDAL_DEFINES_HPP

#include <QVector>

namespace MDAL {

typedef struct {
    double x;
    double y;
} Vertex;

typedef QVector<int> Face;


struct Mesh {
    QVector<Vertex> vertices;
    QVector<Face> faces;
};

} // namespace MDAL
#endif //MDAL_DEFINES_HPP

