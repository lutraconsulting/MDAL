/*
 MDAL - Mesh Data Abstraction Library (MIT License)
 Copyright (C) 2018 Peter Petrik (zilolv at gmail dot com)
*/

#include "mdal_2dm.hpp"
#include "mdal.h"
#include "mdal_utils.hpp"

//#include <QFile>
//#include <QTextStream>

#include <iosfwd>
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <map>
#include <cassert>

MDAL::Loader2dm::Loader2dm( const std::string &meshFile ):
  mMeshFile( meshFile )
{
}

MDAL::Mesh *MDAL::Loader2dm::load( Status *status )
{
  //if (status) status->clear();
  if ( status ) *status = Status::None;

  //std::cerr << "CF: opening 2DM: " << twoDMFileName.toAscii().data() << std::endl;
  //QString twoDMFileName(mMeshFile);

  /*
  QFile file(twoDMFileName);
  if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
  {
    //if (status) status->mLastError = LoadStatus::Err_FileNotFound;
    if (status) *status = Status::Err_FileNotFound;
    return 0;
  }
  */

  if ( !MDAL::fileExists( mMeshFile ) )
  {
    if ( status ) *status = Status::Err_FileNotFound;
    return 0;
  }


  //QTextStream in(&file);
  std::ifstream in( mMeshFile, std::ifstream::in );
  //if (!in.readLine().startsWith("MESH2D"))
  std::string line;
  if ( !std::getline( in, line ) || !startsWith( line, "MESH2D" ) )
  {
    //if (status) status->mLastError = LoadStatus::Err_UnknownFormat;
    if ( status ) *status = Status::Err_UnknownFormat;
    return 0;
  }

  int elemCount = 0;
  int nodeCount = 0;

  // Find out how many nodes and elements are contained in the .2dm mesh file
  //while (!in.atEnd())
  while ( std::getline( in, line ) )
  {
    //QString line = in.readLine();
    //if( line.startsWith("E4Q") ||
    //    line.startsWith("E3T"))
    if ( startsWith( line, "E4Q" ) ||
         startsWith( line, "E3T" ) )
    {
      elemCount++;
    }
    else if ( startsWith( line, "ND" ) )
    {
      nodeCount++;
    }
    else if ( startsWith( line, "E2L" ) ||
              startsWith( line, "E3L" ) ||
              startsWith( line, "E6T" ) ||
              startsWith( line, "E8Q" ) ||
              startsWith( line, "E9Q" ) )
    {
      //if (status) status->mLastWarning = LoadStatus::Warn_UnsupportedElement;
      if ( status ) *status = Status::Warn_UnsupportedElement;
      elemCount += 1; // We still count them as elements
    }
  }

  // Allocate memory
  //Mesh::Nodes nodes(nodeCount);
  //Mesh::Elements elements(elemCount);
  std::vector<Vertex> vertices( nodeCount );
  std::vector<Face> faces( elemCount );

  /*
  // create output for bed elevation
  NodeOutput* o = new NodeOutput;
  o->init(nodeCount, elemCount, false);
  o->time = 0.0;
  memset(o->getActive().data(), 1, elemCount); // All cells active
  */

  //in.seek(0);
  in.clear();
  in.seekg( 0, std::ios::beg );

  //QStringList chunks = QStringList();
  std::vector<std::string> chunks;

  int elemIndex = 0;
  int nodeIndex = 0;
  //QMap<int, int> elemIDtoIndex;
  //QMap<int, int> nodeIDtoIndex;
  std::map<int, int> elemIDtoIndex;
  std::map<int, int> nodeIDtoIndex;
  //int maxElemID = 0;
  //int maxNodeID = 0;

  //while (!in.atEnd())
  while ( std::getline( in, line ) )
  {
    //QString line = in.readLine();
    if ( startsWith( line, "E4Q" ) )
    {
      //chunks = line.split(" ", QString::SkipEmptyParts);
      chunks = split( line,  " ", SplitBehaviour::SkipEmptyParts );
      assert( elemIndex < elemCount );

      int elemID = toInt(chunks[1]);

      std::map<int, int>::iterator search = elemIDtoIndex.find( elemID );
      //if (elemIDtoIndex.contains(elemID))
      if ( search != elemIDtoIndex.end() )
      {
        //if (status) status->mLastWarning = LoadStatus::Warn_ElementNotUnique;
        if ( status ) *status = Status::Warn_ElementNotUnique;
        continue;
      }
      elemIDtoIndex[elemID] = elemIndex;
      //if (elemID > maxElemID)
      //  maxElemID = elemID;

      //Element& elem = elements[elemIndex];
      //elem.setId(elemID);
      Face &face = faces[elemIndex];
      //elem.setEType(Element::E4Q);
      face.resize( 4 );
      // Right now we just store node IDs here - we will convert them to node indices afterwards
      for ( int i = 0; i < 4; ++i )
        //elem.setP(i, chunks[i+2].toInt());
        face[i] = toInt(chunks[i + 2]);

      elemIndex++;
    }
    else if ( startsWith( line, "E3T" ) )
    {
      //chunks = line.split(" ", QString::SkipEmptyParts);
      chunks = split( line,  " ", SplitBehaviour::SkipEmptyParts );
      assert( elemIndex < elemCount );

      uint elemID = toInt(chunks[1]);

      std::map<int, int>::iterator search = elemIDtoIndex.find( elemID );
      //if (elemIDtoIndex.contains(elemID))
      if ( search != elemIDtoIndex.end() )
      {
        //if (status) status->mLastWarning = LoadStatus::Warn_ElementNotUnique;
        if ( status ) *status = Status::Warn_ElementNotUnique;
        continue;
      }
      elemIDtoIndex[elemID] = elemIndex;
      //if (elemID > maxElemID)
      //  maxElemID = elemID;

      //Element& elem = elements[elemIndex];
      //elem.setId(elemID);
      Face &face = faces[elemIndex];
      // elem.setEType(Element::E3T);
      face.resize( 3 );
      // Right now we just store node IDs here - we will convert them to node indices afterwards
      for ( int i = 0; i < 3; ++i )
      {
        //elem.setP(i, chunks[i+2].toInt());
        face[i] = toInt(chunks[i + 2]);
      }

      elemIndex++;
    }
    else if ( startsWith( line, "E2L" ) ||
              startsWith( line, "E3L" ) ||
              startsWith( line, "E6T" ) ||
              startsWith( line, "E8Q" ) ||
              startsWith( line, "E9Q" ) )
    {
      // We do not yet support these elements
      //chunks = line.split(" ", QString::SkipEmptyParts);
      chunks = split( line,  " ", SplitBehaviour::SkipEmptyParts );
      assert( elemIndex < elemCount );

      uint elemID = toInt(chunks[1]);

      std::map<int, int>::iterator search = elemIDtoIndex.find( elemID );
      //if (elemIDtoIndex.contains(elemID))
      if ( search != elemIDtoIndex.end() )
      {
        //if (status) status->mLastWarning = LoadStatus::Warn_ElementNotUnique;
        if ( status ) *status = Status::Warn_ElementNotUnique;
        continue;
      }
      elemIDtoIndex[elemID] = elemIndex;
      //if (elemID > maxElemID)
      //  maxElemID = elemID;

      assert( false ); //WHAT TO DO?
      //elements[elemIndex].setEType(Element::Undefined);

      elemIndex++;
    }
    else if ( startsWith( line, "ND" ) )
    {
      //chunks = line.split(" ", QString::SkipEmptyParts);
      chunks = split( line,  " ", SplitBehaviour::SkipEmptyParts );
      int nodeID = toInt(chunks[1]);

      std::map<int, int>::iterator search = nodeIDtoIndex.find( nodeID );
      // if (nodeIDtoIndex.contains(nodeID))
      if ( search != nodeIDtoIndex.end() )
      {
        //if (status) status->mLastWarning = LoadStatus::Warn_NodeNotUnique;
        if ( status ) *status = Status::Warn_NodeNotUnique;
        continue;
      }
      nodeIDtoIndex[nodeID] = nodeIndex;
      //if (nodeID > maxNodeID)
      //  maxNodeID = nodeID;

      assert( nodeIndex < nodeCount );

      //Node& n = nodes[nodeIndex];
      //n.setId(nodeID);
      Vertex &vertex = vertices[nodeIndex];
      vertex.x = toDouble(chunks[2]);
      vertex.y = toDouble(chunks[3]);
      //o->getValues()[nodeIndex] = chunks[4].toFloat();

      nodeIndex++;
    }
  }


  //for (Mesh::Elements::iterator it = elements.begin(); it != elements.end(); ++it)
  //for (QVector<Face>::iterator it = faces.begin(); it != faces.end(); ++it)
  for ( std::vector<Face>::iterator it = faces.begin(); it != faces.end(); ++it )
  {
    //if( it->isDummy() )
    //  continue;

    //Element& elem = *it;
    Face &face = *it;

    // Resolve node IDs in elements to node indices
    //for (int nd = 0; nd < elem.nodeCount(); ++nd)
    for ( Face::size_type nd = 0; nd < face.size(); ++nd )
    {
      //int nodeID = elem.p(nd);
      int nodeID = face[nd];

      //QMap<int, int>::const_iterator ni2i = nodeIDtoIndex.constFind(nodeID);
      std::map<int, int>::iterator ni2i = nodeIDtoIndex.find( nodeID );
      if ( ni2i != nodeIDtoIndex.end() )
      {
        //elem.setP(nd, *ni2i); // convert from ID to index
        face[nd] = ni2i->second;
      }
      else
      {
        //elem.setEType(Element::Undefined); // mark element as unusable
        assert( false ); //TODO what to do here?

        //if (status) status->mLastWarning = LoadStatus::Warn_ElementWithInvalidNode;
        if ( status ) *status = Status::Warn_ElementWithInvalidNode;
      }
    }

    //TODO what to do here?
    /*
    // check validity of the triangle
    // for now just checking if we have three distinct nodes
    if (elem.eType() == Element::E3T)
    {
      const Node& n1 = nodes[elem.p(0)];
      const Node& n2 = nodes[elem.p(1)];
      const Node& n3 = nodes[elem.p(2)];
      if (n1 == n2 || n1 == n3 || n2 == n3)
      {
        elem.setEType(Element::Undefined); // mark element as unusable

        //if (status) status->mLastWarning = LoadStatus::Warn_InvalidElements;
        if (status) *status = Status::Warn_InvalidElements;
      }
    }
    */
  }

  //Mesh* mesh = new Mesh(nodes, elements);
  Mesh *mesh = new Mesh;
  mesh->faces = faces;
  mesh->vertices = vertices;

  /*
  // Create a dataset for the bed elevation
  DataSet* bedDs = new DataSet(twoDMFileName);
  bedDs->setType(DataSet::Bed);
  bedDs->setName("Bed Elevation");
  bedDs->setIsTimeVarying(false);
  bedDs->addOutput(o);  // takes ownership of the Output
  bedDs->updateZRange();
  mesh->addDataSet(bedDs);
  */

  return mesh;
}
