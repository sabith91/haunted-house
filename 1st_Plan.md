# Implementation Plan: Interactive 3D Haunted House Simulation

**Project:** Simple OpenGL Project | Roll 2107091 | Student: MD.SABITH
**Stack:** C++, OpenGL 1.x (fixed-function), GLUT/FreeGLUT, GLU quadrics
**Deliverable:** one source file (`haunted_house.cpp`)

---

## 1. Goal and Scope

Build a walkable, first-person horror estate inspired by *Granny* and *Alan Wake 2*:

- A fenced compound (80 x 80 units) with a working gate, cobblestone path, dead trees, graveyard, wagon and lamp posts.
- A stone mansion with a round tower, timber turret, porch, entrance hall, staircase, gallery and three bedrooms.
- A curved railway with an animated train in the forest outside the east fence.
- Eight fixed-function lights, zone-based fog, lightning, and continuous animation.

**Constraints:** primitives only (cube, prism, cylinder, sphere, cone, torus, quads), OpenGL 1.x, must run smoothly on ordinary lab PCs.

---

## 2. Coordinate System and Layout

X points right, Z points toward the viewer, Y is up. 1 unit is about 1 metre.

| Zone | Position | Contents |
|---|---|---|
| Fenced compound | X, Z in [-40, 40] | Fence, pillars, gargoyles, gate at Z = +40 |
| Path | X in [-1.5, 1.5], Z = 60 to -11.5 | Cobblestone quads, five porch steps |
| Mansion | X in [-12, 12], Z in [-30, -14] | Hall, gallery, three bedrooms, roof |
| Tower / turret | (-13.5, -11.8) / (9, -27) | Round stone tower, leaning timber turret |
| Graveyard | X in [-38, -22], Z in [12, 34] | Tombstones, crosses, ghost |
| Railway | X = 55 + 8 sin(Z/25) | Rails, sleepers, train |
| Forest | Outside the fence | Pine trees, thicker fog |

Key levels: ground floor at Y = 0.3, upstairs floor at Y = 4.5, wall height 9, eye height 1.7.

---

## 3. Architecture

```
main()      -> glutInit, window, callbacks, init(), glutMainLoop
init()      -> GL state, fog, quadric, star field, generate tree spots,
               door setup, build all display lists
display()   -> flashlight (eye space) -> gluLookAt -> world lights -> sky
               -> static lists -> dynamic objects -> transparent objects -> HUD
idle()      -> dt from elapsed time -> update() -> redisplay
update(dt)  -> look, doors/gate, walking + collision, floor height,
               fog zone, flicker, lightning
input       -> keyboard up/down, arrows, passive mouse motion
```

**Design rules**

1. Static geometry goes into **display lists** (ground, path, fence, trees, pines, graveyard, props, house exterior, house interior, rails, gate leaf).
2. Anything that moves or flickers is drawn **each frame** (doors, gate, chandelier, flames, drapes, rocking chair, train, ghost, bats, crows).
3. All motion uses **delta time**, so speed does not depend on the computer.
4. Transparent objects are drawn **last** with depth writing off.

---

## 4. Build Order (Milestones)

Each step produces something you can run and check before moving on.

| Step | Work | Check |
|---|---|---|
| 1 | Window, projection, first-person camera, ground, sky, moon | You can look around a dark field |
| 2 | Primitive helpers: box, cylinder, cone, sphere, prism, subdivided box | Test shapes render with correct lighting |
| 3 | Fence posts and rails, iron pickets, pillars, gargoyles, gate, path | Complete fence around the estate |
| 4 | Mansion walls, roof, gables, dormers, chimneys, tower, turret, porch | House looks right from the gate |
| 5 | Dead trees, graveyard, wagon, pumpkins, lamp posts | Yard looks abandoned |
| 6 | Hall floor, staircase, balustrade, gallery, chandelier, portraits | Stairs are visually correct |
| 7 | Three bedrooms and furniture | Each room matches its reference |
| 8 | Railway, sleepers, locomotive, carriages, pine forest | Train loops along the curved track |
| 9 | All eight lights, materials, per-zone light enabling | Each light can be identified on its own |
| 10 | Doors, gate, ghost, bats, crows, drapes, flicker animations | Everything animates continuously |
| 11 | Collision, stair height, fog by zone, lightning, HUD | Cannot leave the estate or pass through walls |
| 12 | Testing, tuning, performance pass | Smooth on lab hardware |

---

## 5. Module Details

### 5.1 Primitive helpers
- `box`, `cyl`, `cone`, `sph`, `ell`: wrappers using `glutSolidCube` with scaling, `gluCylinder`, `glutSolidCone` and `glutSolidSphere`. Cylinders and cones are rotated so they point up (+Y).
- `prism` / `prismZ`: custom triangle and quad prism for roofs, with a ridge along X or Z.
- `bigBox`: a box whose faces are split into cells (about 1 unit). Each cell gets a small random brightness. This gives a stone or plank look, and it lets the fixed-function lights work per cell instead of per huge face (important for the flashlight).
- `wallBox`: draws a wall and registers its collision box in one call.

### 5.2 Boundary and grounds
- Wooden sections use posts every 4 units (about 80 around the compound) with upper rail at 1.6 and lower at 0.7. Some are tilted.
- The front uses iron pickets (spacing 0.25) with rails, stored in a display list.
- Stone pillars at corners and beside the gate. Gargoyles on the gate pillars.
- Two gate leaves hinge at X = +-4 and swing up to 70 degrees.
- Ground is a subdivided quad grid (finer inside the fence) with grass tufts made of thin cones. The path is a grid of quads with random brightness.

### 5.3 Mansion exterior
- Outer walls are thin boxes (0.4 thick) with a door gap in the front wall, plus a darker lower band.
- Roof prism with ridge at height 12, two front cross gables, four dormers, three chimneys of different heights.
- Round tower (cylinder plus cone roof plus spire, arched slits), tilted timber turret (stacked boxes, leaning top block).
- Porch with two columns, lintel, prism roof and five step cubes.
- Windows are emissive quads drawn without lighting: about one third lit amber, the rest dark and cracked.

### 5.4 Interior
- **Hall:** plank floor with missing and raised planks, green-grey lined walls with peeling patches, ceiling beams.
- **Staircase:** 14 stacked cubes (rise 0.3, run 0.45, width 4), balustrade posts every step, sloped handrail, newel posts.
- **Gallery:** upstairs slab with a front rail; three bedroom doors open onto it.
- **Decor:** chandelier (torus ring, six arms, flames), candelabras, six portraits (one crooked).
- **Bedroom A (R9):** plank walls, bed, rocking chair, dresser, radiator, hanging lamp, barred windows.
- **Bedroom B (R10):** dark bed, torn drapes, bedside candle, rags, very low light.
- **Bedroom C (R11):** two iron beds, canopy drape, wardrobe, striped wallpaper, holes in the floor, dust.

### 5.5 Railway and forest
- Track follows `X = 55 + 8 sin(Z/25)`. Rails are built in 2-unit segments rotated to the curve tangent; sleepers every 0.7 units on a gravel strip.
- Train head position loops over 30 seconds. Each car is placed on the curve and rotated to match its own heading.
- Locomotive: chassis, body, nose, cab, roof, chimney, rust patches, emissive headlight. Wheels spin with train speed.
- Pines are stacked cones on cylinder trunks, placed in rows on both sides of the track plus a ring around the fence.

### 5.6 Lighting (8 lights)

| Light | Type | Purpose |
|---|---|---|
| LIGHT0 | Directional (w = 0), cold blue | Moon; dimmed smoothly when indoors |
| LIGHT1 | Spot, cutoff 22, set in eye space | Flashlight on camera (F toggles) |
| LIGHT2 | Point, warm amber, flicker | Chandelier |
| LIGHT3 | Point, orange, strong flicker | Bedroom B candle |
| LIGHT4 | Point, warm yellow, occasional flicker | Lamp post at the gate |
| LIGHT5 | Spot, moves with the train | Train headlight |
| LIGHT6 | Point, low range | Window glow in front of the house |
| LIGHT7 | Point, pale blue, follows the ghost | Ghost glow |

Only lights within range of the player are enabled each frame. World lights are positioned after `gluLookAt`. The flashlight is positioned before `gluLookAt`, while the modelview matrix is identity.

### 5.7 Atmosphere
- `GL_EXP2` fog with teal colour. Density eases toward 0.006 inside, 0.018 outdoors, 0.03 near the railway. The clear colour matches the fog colour.
- Sky drawn with fog and lighting off: gradient walls, 200 stars, moon with halo, slow cloud quads.
- Lightning: random timer, double flash of about 0.12 s that brightens sky, ambient and fog. L triggers it manually.
- Blob quads under trees stand in for shadows.

### 5.8 Animation table

| Object | Motion |
|---|---|
| Front door, bedroom doors | Rotate about the hinge; E toggles; first bedroom door creaks open when near |
| Gate | Opens when the player is within 7 units; E flips the state |
| Ghost | Drifts on a path, bobs, alpha `0.35 + 0.15 sin(0.7t)` |
| Bats / crows | Orbit paths; wing angle from a sine wave |
| Chandelier | Pendulum swing of 3 sin(1.1t) degrees |
| Candles, lamps | Flicker from sine mixes plus small random values |
| Drapes | Vertex offsets from a sine wave |
| Rocking chair | Rocks about its base |
| Train | Position along the curve, wheels turn, loops every 30 s |

### 5.9 Camera, controls and collision
- Camera at eye height 1.7 with yaw and limited pitch. Mouse look uses `glutWarpPointer`.
- Movement: 3 units/s, 6 with Shift, plus a small head bob.
- **Collision:** the player is a circle (radius 0.35) tested against axis-aligned boxes; each box has a Y range, so ground-floor and upstairs walls do not interfere. The player slides along boxes. Closed doors and the closed gate are temporary boxes each frame.
- **Floor height:** a function returns porch step height, stair height under the player, gallery/bedroom floor upstairs, or ground. Camera height eases toward it.
- Viewpoint keys 1 to 4 teleport to gate, hall, bedroom, railway.

---

## 6. Performance Plan

- Static parts in display lists (one call each).
- Only nearby lights enabled; fog hides the 400-unit far plane.
- Low slice counts for distant or small shapes (pines use 8 slices).
- Immediate-mode drawing kept for a small number of dynamic objects.
- Target: 60 fps at the start position and inside the hall.

---

## 7. Testing and Verification Checklist

- [ ] Program compiles with no errors on the lab compiler.
- [ ] Runs at about 60 fps at start and inside the hall.
- [ ] Player cannot leave the fence, pass through walls or fall through stairs.
- [ ] Gate, front door and bedroom doors open and close with E.
- [ ] All eight lights can be identified by isolating them; F toggles the flashlight.
- [ ] Fog density changes between yard, house interior and railway.
- [ ] Viewpoints 1 to 4 match R3/R2, R6, R9 to R11, and R12/R13.
- [ ] Train loops every 30 seconds and its headlight lights the track.
- [ ] Lightning flashes at random and with L.

---

## 8. Risks and Mitigations

| Risk | Mitigation |
|---|---|
| Fixed-function lighting is per vertex, so spot and point lights look flat on large faces | Subdivide big surfaces into cells (`bigBox`, grid floors) |
| More than eight lights needed | Reuse the eight slots and enable only nearby ones |
| Z-fighting between wall linings, windows and planks | Offset decorations 0.02 to 0.1 units from the surface |
| Player stuck in corners | Circle-vs-box push-out with three iterations per frame |
| Slow on weak GPUs | Display lists, culled lights, low-poly cones for forest |
| No real shadows in OpenGL 1.x | Dark blob quads under trees and props |

---

## 9. Future Work

- Sound (footsteps, door creak, train) with a small audio library.
- A game goal in the style of *Granny*: find a rusty key in Bedroom B to open the gate.
- Procedural or image textures for stone, planks and cobblestones.
- Shaders for real shadows and volumetric fog (outside the OpenGL 1.x scope of this project).

---

## 10. How to Build and Run

```bash
# Linux
sudo apt install g++ freeglut3-dev
g++ -O2 -o haunted_house haunted_house.cpp -lglut -lGLU -lGL -lm
./haunted_house

# Windows (MSYS2 MinGW64)
g++ -O2 -o haunted_house.exe haunted_house.cpp -lfreeglut -lglu32 -lopengl32
```

**Controls:** W A S D move (Shift run), mouse or arrows look, F flashlight, E door/gate, L lightning, 1 to 4 viewpoints, H help, M mouse look on/off, Esc quit.
