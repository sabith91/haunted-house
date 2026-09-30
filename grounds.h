#ifndef GROUNDS_H
#define GROUNDS_H

#include "common.h"
#include "primitives.h"
#include "train_railway.h"

// ------------------------------------------------------------------ vegetation generators
void branch(float len, float r, int depth);
void deadTree(unsigned seed, float s);
void pineTree(float s);
bool inHouseZone(float x, float z, float m);
void genTrees();

// ------------------------------------------------------------------ static display list builders
void buildGround();
void buildPath();
void pillar(float x, float z);
void gargoyle(float x, float y, float z, float rotY);
void woodSection(float x0, float z0, float x1, float z1);
void ironFront(float xa, float xb);
void buildFence();
void buildLeaf();
void tombstone(float x, float z, int type, float ry);
void buildGrave();
void buildProps();
void buildTrees();

#endif // GROUNDS_H
