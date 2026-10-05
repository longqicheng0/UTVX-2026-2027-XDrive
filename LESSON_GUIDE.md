# X-drive C++ lesson guide

Build the drivetrain yourself, one small step at a time. This guide explains the physical ideas, the mathematics, and the C++ needed to turn a requested motion into wheel commands, estimate the robot's pose, and move toward a target.

**Prepared:** October 5, 2026. **Status:** learning plan only. Creating this guide has not created application source, a desktop harness, a PROS project, or hardware results.

## How to use this guide

Read the architecture overview once, then work through one numbered lesson at a time. Each lesson gives one exercise. Attempt it before opening its hints. A milestone finishes when its completion criteria have evidence; reading the section does not complete it.

The derivations have deliberate learner checkpoints. Small C++ examples teach syntax; they do not supply a finished drivetrain implementation. During interactive lessons, your tutor presents exactly one conceptual question **or** coding task, waits for your attempt, and gives one targeted correction when you say `check`.

The teaching workflow is Research → Plan → Implement → Review → Follow-up: inspect the current evidence, explain the next behaviour and check, let you implement, inspect your saved work, then choose one next step. Source edits remain yours unless you explicitly authorize otherwise. Understanding checks use the same one-question rule and hints before answers.

The [teaching contract](AGENTS.md) and the [X-drive tutor instructions](../.agents/skills/xdrive-hve-tutor/SKILL.md) inform this guide. The `AGENTS.md` reference path and its relative skill links are older than the current layout. The paths below and the parent workspace's `.agents/skills/` are the locations used here; generating this document did not repair those other files.

## Project roots and current evidence

| Item | Current evidence |
| --- | --- |
| Learning root | `/Users/elong/Desktop/UTVX-2026-2027-X-Drive/UTVX-2026-2027-XDrive-Basic` |
| Read-only reference | `/Users/elong/Desktop/UTVX-2026-2027-X-Drive/5225A-2024-2025-X-Drive` |
| Existing learning files | `README.md`, `AGENTS.md`, `LICENSE`, `.gitignore`, and setup records in `tooling/`; no drivetrain source was present when this guide was prepared. |
| Available desktop tools | `/usr/bin/clang++`: Apple Clang 21.0.0, ARM64 macOS; `/usr/bin/g++` is also on PATH. GNU Make 3.81 is available. |
| Tools not found on PATH | `pros` and `cmake`. This does not establish whether they are installed somewhere else. |
| Actual robot | Availability, motor ports, wheel geometry, wiring, and sensor mounting are unknown. |
| Learning progress | No coordinate convention has been accepted, no learner application has been compiled, and no desktop mathematics or robot motion has been verified. |

The [reference project metadata](../5225A-2024-2025-X-Drive/main/project.pros) records VEX V5, PROS kernel 3.8.3, OkapiLib 4.8.0, and a radio template. Its [build configuration](../5225A-2024-2025-X-Drive/main/common.mk) uses GNU C++20 and an ARM cross-compiler. These describe the old project, not selected dependencies for yours. Okapi is installed there, but its include is commented out in [main.h](../5225A-2024-2025-X-Drive/main/include/main.h); the relevant drive, tracking, and control code is custom.

## The architecture you will build

A module is a small group of files with a defined job. Separating jobs makes a wrong result easier to locate. A mathematical function should accept ordinary values and return ordinary values; reading a physical device or writing to a motor belongs at the hardware boundary.

| Reference module | What the source does | Beginner version |
| --- | --- | --- |
| [config.hpp / config.cpp](../5225A-2024-2025-X-Drive/main/src/config.cpp) | Declares and defines the controller, motor, and sensor objects. Eight drive motors form two-motor pairs at FL, FR, BL, and BR. | One hardware definition per device; an independently checked configuration. |
| [drive.hpp / drive.cpp](../5225A-2024-2025-X-Drive/main/src/drive.cpp) | `moveDrive()` mixes strafe, forward, and turn demands; `moveWheels()` writes corner demands. Driver input passes through `driveHandleInput()`. | Pure mixing and limits plus a small hardware output layer. |
| [Libraries/controller.cpp](../5225A-2024-2025-X-Drive/main/src/Libraries/controller.cpp) | Maps left X to strafe, left Y to forward, and right X to turning; adds custom controller services. | Use ordinary PROS controller reads initially. A custom wrapper is unnecessary for the first manual-control lesson. |
| [Libraries/util.hpp / util.cpp](../5225A-2024-2025-X-Drive/main/src/Libraries/util.cpp) | Converts angle units and represents/rotates position and vector data. | Plain `Pose` and `Vec2` structs, named conversion and rotation functions. |
| [Libraries/pid.hpp / pid.cpp](../5225A-2024-2025-X-Drive/main/src/Libraries/pid.cpp) | Stores feedback state and computes timed P, I, and D terms. | Start with P; add the other terms only when a measured problem calls for them. |
| [Chassis/tracking.hpp / tracking.cpp](../5225A-2024-2025-X-Drive/main/src/Chassis/tracking.cpp) | Reads two rotation sensors and an IMU, compensates geometry, integrates pose, and runs a background task. | One initialized tracking state, one pure update function, one hardware capture function. |
| [Chassis/chassis.hpp / chassis.cpp](../5225A-2024-2025-X-Drive/main/src/Chassis/chassis.cpp) | Coordinates pose error, motion control, completion, timeouts, and state-machine requests such as `moveToTarget()`. | Pure target-error calculation and a simple synchronous `moveToPose()` loop. |
| [main.cpp](../5225A-2024-2025-X-Drive/main/src/main.cpp) | `initialize()` starts services; `opcontrol()` enters driver control; `autonomous()` runs competition routines. | Minimal PROS callbacks connect the drivetrain modules. |

### Planned files — create these only as their lessons need them

This tree is a proposal. Everything below except this guide and the existing documentation is a **future file**, not a generated project.

```text
UTVX-2026-2027-XDrive-Basic/
├── LESSON_GUIDE.md                 this document
├── desktop/
│   └── lesson_main.cpp             learner-written desktop entry point
└── main/                          future PROS project root
    ├── project.pros                future PROS-generated metadata
    ├── Makefile / common.mk        future PROS-generated build configuration
    ├── include/main.h              future PROS callback declarations
    └── src/
        ├── main.cpp                PROS callbacks and loop ownership
        ├── config.hpp / config.cpp hardware declarations / single definitions
        ├── drive.hpp / drive.cpp   command structs, mixing, input math, limits
        ├── drive_io.cpp            controller reads and motor output
        ├── Libraries/
        │   ├── util.hpp / util.cpp  pose, vectors, units, angles, transforms
        │   └── pid.hpp / pid.cpp   feedback state and calculations
        └── Chassis/
            ├── tracking.hpp / tracking.cpp  pure pose-update math
            ├── tracking_io.cpp              physical sensor capture
            ├── chassis.hpp / chassis.cpp    pure target-command math
            └── chassis_io.cpp               synchronous motion loop
```

The extra `*_io.cpp` files keep the reference's module responsibilities while letting the desktop build use the same mathematics. `io` means input/output. Pure headers and pure `.cpp` files do not include `config.hpp`, `main.h`, or PROS headers. A hardware function declaration can use your ordinary structs without exposing a PROS type; its implementation belongs in an `*_io.cpp` file. The desktop executable links only the pure functions that its current exercise uses.

A `.hpp` file declares what other files may use. A `.cpp` file defines how a function behaves or where an object is stored. Including a header makes declarations visible; it does not compile or link the matching implementation automatically. `#pragma once` prevents repeated inclusion within one translation unit. It does not make repeated global definitions safe across separate `.cpp` files. Hardware objects use an `extern` declaration in `config.hpp` and one definition in `config.cpp`.

```text
Manual control:
controller → raw input check → normalized DriveCommand
           → dead zone → mixDrive → normalizeWheelCommands
           → applyWheelCommands → physical motors

Tracking:
physical sensors → captureSensors → validated SensorSample
                 → updateOdometry + TrackingState → Pose

Target motion:
target + current Pose → computePoseCommand + controller state
                      → DriveCommand → same mixing/output path
                      → new physical motion → new sensor sample
```

`DriveCommand` holds `cmdStrafe`, `cmdForward`, and `cmdTurn`, all dimensionless demands. `WheelCommands` holds four corner demands `fl`, `fr`, `bl`, `br`. `Pose` holds position in metres and heading in radians. `SensorSample` contains a timestamp and converted, signed tracking distances/heading plus validity. These are planned names; agree on their exact fields during the relevant lesson rather than creating all of them now.

One loop owns motor writes at a time. Start with one tracking update per active loop iteration. Do not also start the reference's background tracker and state-machine stack: duplicate owners make samples and motor commands difficult to reason about.

### Proposed coordinate convention

The guide uses the following **proposal** to keep its examples consistent. Accept or revise it in lesson 1.2 before implementing equations; a revision must propagate through mixing, IMU conversion, odometry, and feedback.

```text
Top view, zero heading:

                    +y body forward / +Y field
                              ↑
                    FL        |        FR
                              O ─────────→ +x body right / +X field
                    BL                 BR

                 positive heading: counterclockwise ↺
```

The origin `O` is your chosen robot reference point. `FL/FR/BL/BR` name locations from the robot's perspective. `x,y` are body coordinates; `X,Y` are field coordinates. Heading `θ` describes the orientation of the body axes relative to the field axes. At `θ = 0`, robot forward points along field `+Y`; a positive heading change turns robot forward toward field `−X`. Positive `cmdTurn` requests that same counterclockwise turn, once hardware signs have been verified. A generic vector's polar angle from `+X` is a different reference; do not silently treat it as robot heading.

Use metres (`m`) for length, radians (`rad`) for mathematical angles, and seconds (`s`) for mathematical time. `Δ` means change over an interval. Raw device readings and joystick values are converted once at their boundary. The reference uses its own signs, inches, and timing choices; matching variable names does not establish matching meaning.

## Development paths

### Computer-only path

Use the desktop executable to pass chosen inputs to your pure functions, print the results, and compare them with a hand calculation or an invariant such as preserved length. Begin with one source file. Add a module only when a lesson introduces it. A printed wheel demand is mathematical evidence; it is not a motor measurement.

The compiler was inspected, but **none of these example commands was executed while creating the guide**. They become useful after you have written the named files. Run from the learning root:

```sh
cd /Users/elong/Desktop/UTVX-2026-2027-X-Drive/UTVX-2026-2027-XDrive-Basic
clang++ -std=c++20 -Wall -Wextra -Wpedantic -g desktop/lesson_main.cpp -o desktop/lesson_runner
./desktop/lesson_runner
```

`clang++` compiles C++; `-std=c++20` selects the language standard; the warning options ask for likely mistakes; `-g` includes debugging information; `-o` names the executable. Compilation and running are separate commands so a failed compile cannot silently run an older executable. Inspect the compiler's exit status before running. These commands require the `desktop/` folder and learner-written entry point to exist.

When a later exercise uses `mixDrive()` and utilities, its build can become:

```sh
clang++ -std=c++20 -Wall -Wextra -Wpedantic -g -I main/src desktop/lesson_main.cpp main/src/drive.cpp main/src/Libraries/util.cpp -o desktop/lesson_runner
```

`-I main/src` adds a header search directory. Only list `.cpp` files that currently exist and are required. Later add pure `pid.cpp`, `tracking.cpp`, or `chassis.cpp` as needed. Do not use a wildcard that pulls in hardware files or a second entry point. Never compile the reference's entire PROS program with the desktop compiler.

### PROS / VEX V5 path

A V5 application needs a cross-compiler and PROS kernel; the desktop compiler alone is insufficient. When hardware integration begins, follow the official [PROS getting-started and installation guides](https://pros.cs.purdue.edu/v5/pros-4/getting-started.html). Installation is a later task; no tools were installed here.

After installation, inspect `pros --version` and `pros --help`. Confirm the CLI's supported project commands before creating `main/`. The documented project-creation form is `pros conduct new-project PATH`; consult its local help first, and use only the new learning project's `main/` location. That command can download and generate files. It was not run here. [PROS Conductor documentation](https://pros.cs.purdue.edu/v5/cli/conductor.html)

If earlier desktop lessons have already created `main/src/`, preserve those learner files when initializing PROS. Inspect the generator's behaviour for an existing directory first. If it cannot preserve them, generate a blank framework separately and bring over only the required framework infrastructure during the later setup task. Do not replace your mathematical modules with generated or reference implementations.

Do not copy the reference's `project.pros`, firmware, or toolchain files into the new project. Keep the kernel generated for the new project and inspect its headers. The old project's 3.x motor-constructor signature and current [PROS 4 motor API](https://pros.cs.purdue.edu/v5/pros-4/group__cpp-motors.html) differ. Write configuration against the installed version, then record that version alongside your build evidence.

Once a learner project exists, PROS compilation happens from its `main/` root. Read the installed CLI's build and upload help for the exact command and device selection. A successful build creates firmware; a successful upload establishes transfer; observing correct wheel direction and robot motion establishes separate hardware evidence.

### Device facts to keep at the boundary

These compact facts were checked against the reference's local headers and official documentation. They do not establish the actual mounting direction of any device.

| Device/function | Units or behaviour | Boundary responsibility |
| --- | --- | --- |
| `Controller::get_analog()` | Integer values in `[-127,127]`; disconnected controller reads return zero. | Validate connection/read status and map to normalized demands. [Controller API](https://pros.cs.purdue.edu/v5/api/cpp/misc.html#get-analog) |
| `Motor::move()` | Signed voltage demand on the `[-127,127]` scale. | Convert a bounded normalized corner demand; this is not an RPM request. [Motor API](https://pros.cs.purdue.edu/v5/api/cpp/motors.html#move) |
| `Motor::move_voltage()` / `move_velocity()` | Millivolts / RPM respectively. | Do not reuse the same numeric value across these APIs. [Motor API](https://pros.cs.purdue.edu/v5/api/cpp/motors.html) |
| `Rotation::get_position()` | Cumulative position in centidegrees; failures have an error return. | Validate, apply mounting sign, and convert through measured wheel geometry. [Rotation API](https://pros.cs.purdue.edu/v5/api/cpp/rotation.html#get-position) |
| `Imu::get_rotation()` / `get_heading()` | Rotation is unbounded degrees; heading wraps in `[0,360)`. Positive rotation is clockwise in the documented device convention. | Validate calibration/read status, convert to your CCW convention, and apply initial heading alignment. [IMU API](https://pros.cs.purdue.edu/v5/api/cpp/imu.html#get-rotation) |
| `pros::millis()` / task delay | Milliseconds; a requested delay does not measure the actual elapsed loop time. | Timestamp samples and convert actual elapsed time to seconds. [RTOS API](https://pros.cs.purdue.edu/v5/api/cpp/rtos.html) |

## Course map and milestone gates

| Milestone | Lessons | Evidence before advancing |
| --- | --- | --- |
| [1. Physical layout](#milestone-1) | 1.1–1.3 | A labelled layout, proposed axes, and explained rolling directions. |
| [2. C++ foundations](#milestone-2) | 2.1–2.3 | A learner-written desktop program builds and its data/function flow is understood. |
| [3. Commands and motors](#milestone-3) | 3.1–3.3 | Corner ordering and command units are explicit; hardware direction remains pending if unavailable. |
| [4. Wheel mixing](#milestone-4) | 4.1–4.3 | Learner-derived signs pass translation, turn, zero, and superposition checks. |
| [5. Manual control](#milestone-5) | 5.1–5.3 | Dead zones, common scaling, output boundaries, and stop behaviour are checked. |
| [6. Pose and units](#milestone-6) | 6.1–6.4 | Angle, time, and coordinate helpers pass boundary and round-trip checks. |
| [7. Odometry](#milestone-7) | 7.1–7.4 | Baselines, signs, offsets, straight motion, rotation, and near-zero-turn cases are checked. |
| [8. Heading feedback](#milestone-8) | 8.1–8.4 | Signed heading error, bounded output, and controller state behave as intended. |
| [9. Target motion](#milestone-9) | 9.1–9.4 | Frame conversion, termination, and output stopping are checked. |
| [10. Complete loop](#milestone-10) | 10.1–10.3 | A trace identifies each layer; software and hardware evidence are recorded separately. |

Intake, lift, pneumatics, dashboards, competition strategy, generic state machines, asynchronous motion, path following, feedforward, and nested velocity control are outside this first course. Add them after the core loop has evidence.

<a id="milestone-1"></a>

## Milestone 1 — Understand the physical X-drive

Intended behavior: explain how four angled omni-wheel corners can translate and turn. Evidence: a labeled drawing and a defensible sign convention. No software or hardware behavior is verified by this milestone.

### Lesson 1.1 — Separate driven rolling from passive rolling

**Objective and prerequisites:** Recognize the two distinct directions of motion at an omni wheel. No programming prerequisites. **Why physically:** A drivetrain equation only makes sense when it describes the direction the motor actually drives.

A conventional wheel resists sideways motion. An omni wheel adds small rollers around its rim. The motor drives motion along the main wheel's rolling direction; the rollers allow motion across that direction with much less resistance. Neither direction should be identified from the motor housing alone. Look at the main wheel plane and roller axes. An X-drive uses four corners whose driven rolling axes are diagonal relative to the chassis; paired motors at a corner still command one rolling direction.

```text
                   FRONT
          FL                  FR
                  robot
          BL                  BR
                    BACK
```

FL means front-left, FR front-right, BL back-left, and BR back-right, viewed from above facing the robot's front. The diagram locates corners; it does not assert their wheel mounting angles. A rolling **axis** is an unoriented line; a positive rolling **direction** adds an arrow to that line. Keep that distinction until motor directions are checked. The [reference config](../5225A-2024-2025-X-Drive/main/src/config.cpp) defines two motors for each corner, but does not prove the mounting geometry.

**File/function:** No file/function yet. **One exercise:** Draw one omni wheel with its driven axis and passive roller direction labeled.

**Check:** Your two directions must explain which motion requires the main wheel to rotate.

<details>
<summary>Hints — open after your attempt</summary>

1. Imagine holding the motor shaft stationary. 2. Identify the small rollers' remaining freedom.

</details>

**Common mistake:** Calling roller motion the motor-driven direction. **Complete when:** You can explain the drawing without referring to a mixer equation.

### Lesson 1.2 — Propose axes and distinguish three commands

**Objective and prerequisites:** Give translation and turning unambiguous signs; prerequisite: 1.1. **Why physically:** “Positive” is meaningless until everyone knows the axis and viewpoint.

This guide proposes body axes with `x` to the robot's right and `y` forward, and positive heading `theta` counterclockwise when viewed from above. A positive heading change therefore turns left. With `z` upward, these axes form a right-handed system. Field axes are `X` right and `Y` up the page; at `theta = 0`, body forward agrees with field `+Y`. These are proposals to accept before implementation, not previously accepted robot settings.

```text
          +y (forward)
               ^
               |     positive theta: CCW
         robot +----> +x (right)
```

A drivetrain command will contain dimensionless `cmdStrafe`, `cmdForward`, and `cmdTurn`, normally within `[-1, 1]`. A value of `0.5` requests an output fraction; it does not mean `0.5 m/s`. Physical body velocities will later use `v_x` and `v_y` in metres per second, and angular velocity `omega` in radians per second. Heading `theta` is an orientation in radians; `cmdTurn` is an instruction to change orientation. The [reference `moveDrive(x, y, a)`](../5225A-2024-2025-X-Drive/main/src/drive.cpp) uses command arguments on a different scale; its `a` sign must be reconciled with this proposal rather than renamed blindly.

**File/function:** No file/function yet. **One exercise:** Label a left-turn arrow on your body-axis drawing.

**Check:** The arrow must agree with the stated top-down counterclockwise convention.

<details>
<summary>Hints — open after your attempt</summary>

1. Track the forward arrow during rotation. 2. Distinguish the robot turning from it sliding left.

</details>

**Common mistake:** Treating heading and turn command as the same quantity. **Complete when:** The drawing states viewpoint, axes, and positive rotation explicitly.

### Lesson 1.3 — Explain combinations before equations

**Objective and prerequisites:** Connect wheel directions to whole-robot motion; prerequisites: 1.1–1.2. **Why physically:** A robot can rotate while its center stays still, so translation alone cannot explain every corner's motion.

Draw each corner's actual driven rolling axis. For an ideal symmetric X-drive, those axes are diagonal, nominally 45 degrees to the body axes. “45 degrees” is a geometry assumption to inspect, not a setting copied from source. Attach positive arrows only after choosing a mathematical rolling direction; later hardware checks will determine how motor output produces that direction.

For pure translation, all chassis points share the same displacement. Each wheel responds to the component of that displacement along its driven axis; rollers accommodate the other component. For pure rotation, each corner instead moves tangent to a circle about the chassis center. Thus a corner's rotational requirement depends on both its location and rolling axis. Translation and rotation can coexist because their velocity contributions add.

The [reference `moveWheels(fl, fr, bl, br)`](../5225A-2024-2025-X-Drive/main/src/drive.cpp) separates four corner outputs from the eight motor objects. That is a useful architectural boundary: reason with four wheel commands, then send each command to its two motors. It does not validate the physical wheel arrangement.

**Symbols:** A corner label identifies location, not output sign; displacement has length units, whereas rolling-axis direction is dimensionless.

**File/function:** No file/function yet. **One exercise:** Add the tangent direction for a positive rotation at one selected corner.

**Check:** The tangent must be perpendicular to the center-to-corner line.

<details>
<summary>Hints — open after your attempt</summary>

1. Draw that line first. 2. Imagine the corner moving around the center counterclockwise.

</details>

**Common mistake:** Giving every corner the same translational arrow during a spin. **Complete when:** Your drawing explains why corner location matters to turning.

<a id="milestone-2"></a>

## Milestone 2 — Learn the minimum C++ and compilation skills

Intended behavior: represent small quantities and call a function through a tiny desktop program. Evidence: your own compiler output and inspected source. Compilation establishes program structure, not correct wheel behavior.

### Lesson 2.1 — Give a number a name and a type

**Objective and prerequisites:** Understand variables, expressions, and return values; prerequisite: coordinate meanings from 1.2. **Why physically:** Code needs to distinguish a wheel label from a continuously adjustable command.

A variable is a named value. Its type describes how C++ represents and uses it. `int` stores whole numbers; `double` stores approximate real numbers, including fractions; `bool` stores `true` or `false`. Prefer `double` for mathematical commands. A statement usually ends in a semicolon, and braces group statements. Assignment `=` stores a value; comparison `==` asks whether values match.

This unrelated example demonstrates function syntax without implementing the drivetrain:

```cpp
double twice(double value) {
    return 2.0 * value;
}
```

`double` before the name is the return type; `value` is a parameter, a local name for the argument passed by the caller. `return` supplies the function's result. A caller such as `twice(0.25)` can use that result in another expression. The decimal in `2.0` makes the intended floating-point arithmetic clear. Integer division can discard fractional information before assignment to a `double`.

**Symbols/units:** `value` in this demonstration is dimensionless; multiplying by dimensionless `2.0` preserves its units. **File/function proposed:** `desktop/lesson_main.cpp`, learner-written `main()`; this is a future exercise, not an existing file.

**One exercise:** Write one declaration storing a fractional example forward command in a suitably named variable.

**Check:** Its type preserves the fraction and its name describes a command.

<details>
<summary>Hints — open after your attempt</summary>

1. Ask whether whole-number storage is enough. 2. Keep units in the name or comment.

</details>

**Common mistake:** Choosing `int` because joystick input arrives as an integer. **Complete when:** You can explain every token in your declaration.

### Lesson 2.2 — Compile, run, and read the first error

**Objective and prerequisites:** Distinguish source code, compilation, linking, and execution; prerequisite: 2.1. **Why physically:** A file saved in an editor has not yet caused a program or motor to do anything.

A `.cpp` file contains text. Compilation checks syntax and types and translates that text into machine instructions. Linking joins separately compiled parts and resolves calls to functions defined elsewhere. The resulting executable runs only when started. A desktop program begins in `main()`; a PROS robot project supplies framework callbacks such as `initialize()` and `opcontrol()` instead. Do not put a desktop `main()` inside the robot source tree.

An entry point has this shape; the exercise supplies the print statement:

```cpp
#include <iostream>

int main() {
    // Declare your example value and write your print statement here.
    return 0;
}
```

`int` is the type of the exit status, and `0` reports successful completion. Empty parentheses mean no explicit parameters in this version. Braces enclose the statements executed when the program starts. `#include <iostream>` declares the standard output facilities. In an unrelated printing example, `std::cout << "example" << '\n';` sends text and then a newline. Your exercise must print your variable instead.

The future harness should be very small: one entry point, one ordinary function call, and a printed result using `<iostream>`. `std::cout` is the standard output stream; `<<` sends a value to that stream. Include a newline so results remain readable. Compiler flags and available tools belong to the setup section; execute only the command appropriate to the verified development path.

When an error appears, read the first diagnostic with a filename and line number. Later diagnostics may follow from that first defect. A syntax fix differs from a numerical correction: successful compilation can still produce the wrong value. The [reference `main.cpp`](../5225A-2024-2025-X-Drive/main/src/main.cpp) illustrates PROS callbacks, not a desktop entry point.

**File/function proposed:** `desktop/lesson_main.cpp`, `main()`. **One exercise:** Create and compile the smallest desktop program that prints your example variable from 2.1.

**Check:** The process exits successfully and the printed fraction matches your declared value.

<details>
<summary>Hints — open after your attempt</summary>

1. Start from the entry-point shape. 2. Check the output header before changing arithmetic.

</details>

**Common mistake:** Treating a saved file as a successful run. **Complete when:** Source, compile evidence, and run output are separately identified.

### Lesson 2.3 — Separate a declaration from its definition

**Objective and prerequisites:** Explain headers, source files, and shared declarations; prerequisites: 2.1–2.2. **Why physically:** Multiple modules must talk about the same drivetrain rather than accidentally creating different copies.

A function declaration states its name, parameter types, and return type, ending in a semicolon. Its definition adds the body. A header communicates declarations to callers; a `.cpp` file usually owns definitions. `#include` makes a header's text available to a compiler, while `#pragma once` prevents that header from being included repeatedly in one compilation unit. It does not make repeated global definitions legal across different `.cpp` files.

A `struct` groups related values under one type, with members accessed using a dot. For example, an unrelated `struct Interval { double start; double end; };` groups two endpoints. The planned `DriveCommand` will group three different command components, and `WheelCommands` four named corner values. Names make ordering mistakes easier to see than a list of unnamed numbers.

The [reference `config.hpp`](../5225A-2024-2025-X-Drive/main/src/config.hpp) declares hardware objects using `extern`, while [config.cpp](../5225A-2024-2025-X-Drive/main/src/config.cpp) defines them once. A definition actually creates the object; an `extern` declaration tells another file that object exists elsewhere. These are architectural examples, not hardware declarations to copy yet.

**Symbols/units:** Types describe structure; they do not enforce metres or radians automatically. **File/function proposed:** `main/src/drive.hpp`, proposed `DriveCommand` type; no function needed yet.

**One exercise:** Declare the proposed `DriveCommand` struct with named dimensionless components.

**Check:** Another source file could include the header without constructing motors.

<details>
<summary>Hints — open after your attempt</summary>

1. Group quantities that travel together. 2. Keep hardware definitions out of this mathematical interface.

</details>

**Common mistake:** Putting motor definitions in a shared header. **Complete when:** You can distinguish a declaration, definition, and instance.

<a id="milestone-3"></a>

## Milestone 3 — Represent commands and establish motor directions

Intended behavior: maintain one four-corner output ordering and explain how each output reaches two motors. Evidence: desktop structure checks or observed individual motor direction checks, explicitly labeled by path.

### Lesson 3.1 — Represent four corner outputs without hardware

**Objective and prerequisites:** Establish wheel ordering and mathematical interfaces; prerequisites: 2.3 and the corner drawing. **Why physically:** A correct number sent to the wrong corner produces incorrect motion.

Use the same ordering everywhere: FL, FR, BL, BR. A proposed `WheelCommands` struct will name these four dimensionless outputs. Before normalization they may exceed magnitude one because several requested motions can combine. After normalization, each output must fit the selected mathematical bound; hardware scaling comes later.

Plan a pure function `mixDrive(command)` that accepts a `DriveCommand` and returns `WheelCommands`. “Pure” here means it calculates values without reading sensors or touching motors. That makes the same mathematics usable on a computer and a robot. Keep pure mathematics in `drive.cpp` and its PROS-free `drive.hpp`. Plan a separate `applyWheelCommands(wheels)` function in `drive_io.cpp` for motor output. The higher-level `moveDrive` adapter will mix, normalize, then apply. Keeping these functions separate lets you inspect calculations without energizing anything. The [reference `moveDrive` and `moveWheels`](../5225A-2024-2025-X-Drive/main/src/drive.cpp) suggest the same responsibility split, although its mixer directly writes motors.

A function interface is a promise about inputs and outputs. It should state corner order, units, and whether values are raw or normalized. It should not claim measured velocity from a command value. Output fractions are requests; friction, battery voltage, load, and motor response affect actual motion.

**File/function proposed:** `main/src/drive.hpp`, `WheelCommands` and declaration of `mixDrive`; no mixer body yet. **One exercise:** Declare `WheelCommands` with four corner members in the agreed order.

**Check:** Match each member to exactly one corner on your drawing.

<details>
<summary>Hints — open after your attempt</summary>

1. Use names instead of implicit positions. 2. Compare front/back before left/right.

</details>

**Common mistake:** Swapping BL and BR during output conversion. **Complete when:** The interface states names, order, and dimensionless units.

### Lesson 3.2 — Define hardware once and record pair mappings

**Objective and prerequisites:** Understand shared motor objects and paired output; prerequisite: 3.1. **Why physically:** Two motors attached to one corner must cooperate in the same wheel direction even if their shafts face opposite ways.

The reference defines these pairs in [config.cpp](../5225A-2024-2025-X-Drive/main/src/config.cpp): FL top/bottom ports 10/9, FR 4/5, BL 19/18, and BR 14/15. It assigns opposite reversal flags within each pair and uses the `E_MOTOR_GEARSET_18` cartridge setting. These are facts about that code; the learning robot's ports, transmissions, cartridges, and mounting remain unverified. A reversal flag converts motor-native direction to the intended mechanism direction. It does not fix an incorrect corner assignment or wheel-axis equation.

The future robot `config.hpp` will contain an `extern pros::Motor` declaration for each required motor; `config.cpp` will contain exactly one corresponding constructor definition. A declaration supplies a name and type without creating a second motor object. The PROS `move` interface accepts integer output commands; the mathematical normalized fraction will be converted and rounded once at the hardware boundary. Avoid mixing that interface with `move_velocity`, whose input describes a motor speed request.

**Symbols/units:** Port numbers are identifiers; reversal is Boolean; wheel commands are dimensionless. **File/function proposed:** `main/src/config.hpp` and `config.cpp`, hardware object declarations/definitions; no function yet.

**One exercise:** Make one proposed FL pair mapping record with fields for both ports, mounting direction, reversal, and evidence status.

**Check:** Unknown values stay explicitly unknown.

<details>
<summary>Hints — open after your attempt</summary>

1. Separate code evidence from measured mounting. 2. Record how positive output should move the wheel.

</details>

**Common mistake:** Copying reference ports as confirmed hardware. **Complete when:** The record can guide later inspection without pretending it already happened.

### Lesson 3.3 — Check one motor sign before checking the mixer

**Objective and prerequisites:** Design an individual direction check; prerequisites: 3.2 and agreed positive rolling arrows. **Why physically:** A reversed motor can imitate a bad drivetrain equation, and paired motors can oppose each other.

Hardware direction verification proceeds at the smallest boundary: one motor, one short low output, one observed wheel direction, then zero output. Establish a clear, supported setup with wheels free to rotate and an immediate stop available. Check each member of a pair separately before commanding the pair together. The required observation is whether positive motor output produces the chosen positive wheel rolling direction. Matching raw shaft directions is insufficient because mounting or gearing may invert one shaft's effect.

In the computer-only path, `desktop/lesson_main.cpp` prints the intended corner and its `WheelCommands` value directly. It does not link the hardware `drive_io.cpp` or claim to run `applyWheelCommands`. This checks routing and sign bookkeeping; it cannot establish actual motor reversal. Keep a table whose status is `proposed`, `desktop checked`, or `hardware observed`, with the evidence written plainly.

The [reference `moveWheels`](../5225A-2024-2025-X-Drive/main/src/drive.cpp) sends a common command to both motors at each corner. Its effectiveness depends on constructor reversal settings and physical assembly agreeing. Do not run its full `driverControl` just to investigate one motor: that function also handles unrelated mechanisms.

**Symbols/units:** The test output is a small dimensionless fraction converted to the selected motor interface scale; duration is a bounded time interval, not a control gain.

**File/function proposed:** Later robot `main/src/drive_io.cpp`, temporary `checkOneMotorDirection()` for an individual motor; normal paired output stays in `applyWheelCommands()`. Desktop `desktop/lesson_main.cpp`, `main()` prints routing values without hardware. The individual check needs its own explicit stop path and must not accidentally command the paired motor.

**One exercise:** Write the predicted observation for one positive FL-top motor check.

**Check:** The prediction names wheel direction and stop condition.

<details>
<summary>Hints — open after your attempt</summary>

1. Refer to the positive arrow, not “clockwise motor.” 2. Account for transmission direction.

</details>

**Common mistake:** Changing mixer signs before checking motor mapping. **Complete when:** Prediction and eventual observation can be compared independently.

<a id="milestone-4"></a>

## Milestone 4 — Derive four-corner mixing

Intended behavior: calculate wheel commands from independently derived geometry. Evidence: hand calculations and learner-written desktop checks for zero input, pure motions, combinations, and reversed commands. Hardware verification follows only after mapping checks.

### Lesson 4.1 — Project translation onto one wheel

**Objective and prerequisites:** Derive one wheel's translation contribution; prerequisites: 1.3, 3.1, multiplication and addition. **Why physically:** An angled wheel can only drive the component of motion along its own rolling direction.

Represent a positive rolling direction as a unit vector `d_i = (d_ix, d_iy)`, where `i` identifies one corner. Unit vector means its length is one: `d_ix² + d_iy² = 1`. Its components are dimensionless. For physical translation `v = (v_x, v_y)` in metres per second, the required signed rolling speed is the dot product `s_i = d_ix v_x + d_iy v_y`. Multiplying matching components and adding measures how much motion lies along that axis. Perpendicular motion contributes zero; reversing translation reverses the projection.

For an ideal diagonal, each component's magnitude follows from a right triangle with equal legs and unit hypotenuse. Derive that magnitude rather than recalling a mixer. Choose signs from your positive arrow. For normalized commands, analogous coefficients multiply `cmdStrafe` and `cmdForward`. A common scale may be absorbed into the command convention; explain that choice instead of confusing normalized output with physical wheel speed.

The [reference `moveDrive`](../5225A-2024-2025-X-Drive/main/src/drive.cpp) combines signed terms. Compare it only after your own projection is defensible.

**File/function proposed:** `main/src/drive.cpp`, future translation part of `mixDrive`; derivation first. **One exercise:** Derive the two translation coefficient signs for one selected corner.

**Check:** Parallel and perpendicular example motions must give compatible projections.

<details>
<summary>Hints — open after your attempt</summary>

1. Draw the positive arrow. 2. Look at each arrow component separately.

</details>

**Common mistake:** Using the roller axis as `d_i`. **Complete when:** Both signs follow from geometry rather than reference text.

### Lesson 4.2 — Add rotation using corner position

**Objective and prerequisites:** Derive the rotational contribution at one corner; prerequisites: 4.1 and positive rotation. **Why physically:** During a spin, corners move in different tangent directions even though the robot center has no translation.

Let `r_i = (r_ix, r_iy)` be the vector from chassis center to corner `i`, measured in metres in body axes. For positive counterclockwise angular velocity `omega` in radians per second, the corner's rotational velocity is `omega(-r_iy, r_ix)`. The perpendicular vector gives tangential direction; its magnitude scales with distance from the center. Radians are dimensionless mathematically, so multiplying angular velocity by a length yields metres per second.

Project this tangent onto the wheel's positive axis. The physical contribution is `omega(-d_ix r_iy + d_iy r_ix)`. Add it to the translation projection because the corner can translate and rotate simultaneously. Derive the signs from `r_i` and `d_i`; do not assume “left corners positive.” In normalized mixing, `cmdTurn` replaces a physical angular velocity request through a chosen dimensionless turning scale. For symmetric geometry, rotational magnitudes may be made common, but measured asymmetry should not be silently erased.

The [reference `moveDrive(x, y, a)`](../5225A-2024-2025-X-Drive/main/src/drive.cpp) assigns its own turning signs. If its positive `a` means the opposite physical rotation from proposed positive `cmdTurn`, convert at the interface explicitly; determine this from geometry and observed direction.

**File/function proposed:** `main/src/drive.cpp`, rotational part of `mixDrive`. **One exercise:** Derive the rotational coefficient sign for the corner selected in 4.1.

**Check:** Its required rolling direction must match the projected counterclockwise tangent.

<details>
<summary>Hints — open after your attempt</summary>

1. Locate the corner relative to center. 2. Take the dot product only after drawing the tangent.

</details>

**Common mistake:** Copying `a` as positive CCW without evidence. **Complete when:** Your rotational sign has a drawn and algebraic explanation.

### Lesson 4.3 — Combine the derived rows and inspect properties

**Objective and prerequisites:** Implement your own four-corner mixer; prerequisites: 4.1–4.2 for all corners and function syntax. **Why physically:** Translation and turning share the same motors, so the final command must account for both contributions.

Each corner is one row of a mapping: a strafe coefficient, a forward coefficient, and a turning coefficient. Your geometric derivation supplies those three coefficients. The code's named inputs correspond directly to the row terms; named outputs correspond to FL, FR, BL, BR. Keep raw mathematical mixing separate from normalization and motor output. A mixer should not include PROS headers, controller reads, or hidden hardware reversals.

Linear combinations have useful checks. Zero input gives zero output. Negating every input should negate every output. Scaling inputs by a small common factor should scale raw outputs by that factor. Adding two input requests should produce the sum of their separately mixed raw outputs. These properties reveal implementation mistakes without requiring a completed reference solution.

Inspect pure forward, pure strafe, pure turn, and one diagonal using your diagram. A diagonal may align with one pair's rolling axes and be perpendicular to the other pair's axes under ideal geometry. Predict that relationship from the axes rather than memorizing the values. Afterward compare your result with [reference `moveDrive`](../5225A-2024-2025-X-Drive/main/src/drive.cpp), documenting convention or scale differences.

**Symbols/units:** All mixer inputs and outputs here are dimensionless commands; physical speed remains outside this function. **File/function proposed:** `main/src/drive.cpp`, `mixDrive`; desktop calls in `desktop/lesson_main.cpp`.

**One exercise:** Implement the FL row from your derivation.

**Check:** Verify zero and sign reversal for that row.

<details>
<summary>Hints — open after your attempt</summary>

1. Translate one term at a time. 2. Compare variable names with axes.

</details>

**Common mistake:** Adding normalization inside one row only. **Complete when:** Its source and hand calculation agree. Repeat lessons 4.1–4.3 for FR, BL, and BR as separate small attempts before proceeding to Milestone 5; the complete mixer must then pass the stated pure-motion and superposition checks.

<a id="milestone-5"></a>

## Milestone 5 — Implement manual control

Intended behavior: controller input becomes bounded four-corner output and released input commands a stop. Evidence: desktop input/output checks; hardware observations only on the hardware path after individual motor signs are established.

### Lesson 5.1 — Map joystick readings into deliberate commands

**Objective and prerequisites:** Convert raw joystick values and remove neutral noise; prerequisite: the completed four-corner mixer from 4.3. **Why physically:** An untouched joystick can report a small nonzero value, causing unwanted motion if every reading reaches the motors.

The reference chooses left-stick horizontal for strafe, left-stick vertical for forward, and right-stick horizontal for turn in [controller.cpp](../5225A-2024-2025-X-Drive/main/src/Libraries/controller.cpp). Treat that as a proposed ergonomic mapping. Raw analog input is an integer in the applicable PROS range. Divide by its positive full-scale magnitude using floating-point arithmetic to create a dimensionless fraction. Decide the turning sign explicitly: a right-stick push commonly requests a right turn, while our mathematical positive `cmdTurn` means CCW/left.

A dead zone defines an interval around zero whose readings become exactly zero. Choose either raw units or normalized units for its threshold and use that unit consistently. An axial dead zone treats each component independently; a radial translation dead zone uses the combined stick magnitude and is a later refinement. Begin with the simpler axial version. Avoid the reference's extra curvature and subsystem-dependent behavior until plain mapping is understood.

An `if (condition) { ... }` statement chooses a branch when its Boolean condition is true; `else { ... }` supplies the other branch. `<` means strictly less than, while `<=` includes equality. For a `double`, `std::abs(value)` from `<cmath>` gives magnitude. A function can return from either branch; state the exact threshold boundary before implementing it.

**File/function proposed:** Pure `main/src/drive.cpp`, `applyDeadzone`; later hardware `main/src/drive_io.cpp`, `readDriveCommand`. **One exercise:** Implement a dimensionless axial dead-zone function.

**Check:** Inputs inside the threshold produce exactly zero; opposite inputs outside remain opposites.

<details>
<summary>Hints — open after your attempt</summary>

1. Compare magnitude, not signed value. 2. State behavior at the threshold boundary.

</details>

**Common mistake:** Comparing normalized input against a raw threshold. **Complete when:** The threshold units and boundary behavior are documented and checked.

### Lesson 5.2 — Preserve wheel ratios when commands exceed limits

**Objective and prerequisites:** Bound combined commands without distorting their proportions; prerequisites: 4.3 and 5.1. **Why physically:** Individually clipping wheel outputs changes the requested motion because different corners lose different amounts.

Raw wheel values may exceed magnitude one even when each input component fits `[-1, 1]`. Find the largest absolute corner value, called `m`. If `m` is already within the permitted bound, leave the four values unchanged. If it exceeds the bound, multiply every corner by the same positive scale that brings the largest magnitude to that bound. Derive the scale by asking what multiplier takes `m` to the chosen limit.

Using one common multiplier preserves ratios between nonzero wheel outputs, signs, and zeros. For intuition, consider two unrelated demands with magnitudes 2 and 1 and a maximum allowed magnitude 1: reducing both proportionally preserves their relationship, whereas clipping only the larger changes it. The complete four-output implementation remains your exercise. With all-zero input, avoid dividing by zero; the intended result remains four zeros.

The [reference `moveDrive`](../5225A-2024-2025-X-Drive/main/src/drive.cpp) directly sends sums to motors without this common normalization. Build your normalization explicitly rather than relying on hardware clipping. Later, converting normalized fractions to the motor interface's integer range can introduce small rounding differences.

**Symbols/units:** `m`, scale, and normalized outputs are dimensionless; permitted normalized magnitude is 1. **File/function proposed:** `main/src/drive.cpp`, `normalizeWheelCommands`.

**One exercise:** Implement common normalization for `WheelCommands`.

**Check:** Largest magnitude never exceeds one; unsaturated input is unchanged; all-zero input remains finite and zero.

<details>
<summary>Hints — open after your attempt</summary>

1. Inspect all four absolute values. 2. Use one scale for all four members.

</details>

**Common mistake:** Applying four independent clamps. **Complete when:** A saturated example preserves a selected nonzero output ratio.

### Lesson 5.3 — Close the manual loop and make zero reach the motors

**Objective and prerequisites:** Explain repeated input-to-output execution and stopping; prerequisites: 5.1–5.2. **Why physically:** Motors retain their last request until another request or framework behavior changes it; releasing a stick must result in a zero command reaching the output boundary.

A loop repeats a sequence. `while (condition) { ... }` evaluates the condition before each iteration; `while (true)` continues until an explicit exit or runtime mode change. The body is the sequence of statements in braces. The minimal operator-control sequence is read controller input, form commands, mix, normalize, apply all four outputs, then yield for the next iteration. Keep each step inspectable. A regular PROS delay yields processor time; it is not proof that every iteration takes exactly the requested interval. When elapsed time becomes mathematically important, measure it separately.

The [reference `driverControl`](../5225A-2024-2025-X-Drive/main/src/drive.cpp) repeatedly calls `driveHandleInput` and delays 10 milliseconds. Its extensive unrelated subsystem handling is outside this course. Start your simpler loop from [main.cpp's `opcontrol`](../5225A-2024-2025-X-Drive/main/src/main.cpp), without copying that broader implementation.

Send the current request every cycle initially so a transition to zero is easy to trace. Separate an output of zero from brake configuration: coast, brake, and hold alter physical stopping behavior, but none establishes target-pose accuracy. Motor-interface limits belong to the final output adapter after common normalization. Every early return or future movement termination also needs an explicit stopping path.

**Symbols/units:** Loop delay uses milliseconds; normalized output remains dimensionless; stopping distance has length units and must be observed. **File/function proposed:** `main/src/main.cpp`, `opcontrol`; `main/src/drive_io.cpp`, `stopDrive` and `moveDrive`.

**One exercise:** Implement one manual-control input-to-output iteration on your chosen path, using the existing helpers. On the desktop, `main()` supplies a chosen input and prints wheels; on hardware, `opcontrol()` calls the hardware read and write boundaries.

**Check:** Identify the cycle in which all four zero commands reach both motors per corner.

<details>
<summary>Hints — open after your attempt</summary>

1. Follow values across each function. 2. Check that dead-zone zero is still applied.

</details>

**Common mistake:** Skipping output when input becomes zero. **Complete when:** A forward-input-to-released-input trace distinguishes commanded stop from observed wheel stopping. Once that one iteration is checked, the tutor guides you to repeat it in the minimal loop with an explicit yield and stopping path before advancing.

<a id="milestone-6"></a>

## Milestone 6 — Represent pose, angles, coordinates, and time

**Intended behavior:** describe where the robot is and distinguish measured state from requested movement. Check angle boundaries, frame changes, and elapsed time on the computer before attaching sensors.

These conventions are **proposed for this course**, not yet accepted or implemented: body `x` points right, body `y` points forward, field `X` points right on the map, field `Y` points up, and positive heading `theta` turns counterclockwise, toward the robot's left. At `theta = 0`, body forward aligns with field `+Y`. Length uses meters, angle radians, and elapsed time seconds.

### Lesson 6.1 — Store a pose without confusing it with a command

**Objective / prerequisites:** understand a three-value pose; requires structs and the coordinate sketch from Milestones 1–2. **Proposed file/function:** `main/src/Libraries/util.hpp`, `Pose` struct; no function yet.

**Why:** a turn request tells motors what to attempt. Heading tells you which way the robot actually faces. Confusing them breaks feedback even when wheel mixing is correct.

Use a proposed struct with `double` members `x`, `y`, and `heading`. For this struct, `x` and `y` are field position in meters; `heading` is orientation in radians. Name body displacement variables explicitly, such as `body_dx`, so the same letter does not silently change frames. The field origin is a chosen starting point, not a GPS location. Heading zero means the chosen forward direction, not inherent compass north.

```text
Field +Y / heading zero
         ^
         |       body +y = robot's nose
 origin  +----> +X
```

The reference [`Position` and `Vector`](../5225A-2024-2025-X-Drive/main/src/Libraries/util.hpp) provide analogous concepts with member `a` for angle. Prefer plain named members initially; operator overloading can wait. Read [`Position::operator+`](../5225A-2024-2025-X-Drive/main/src/Libraries/util.cpp) critically: its `x + p2.y` expression warrants investigation before reuse.

**One exercise:** declare your own `Pose` struct with unit comments; write no operators.

**Check:** the origin with zero heading represents the proposed starting orientation; assigning a turn command never changes pose directly.

<details>
<summary>Hints — open after your attempt</summary>

first identify values describing state; then attach a frame and unit to each member.

</details>

**Common mistakes:** using joystick values as position, copying an unexplained angle convention. **Completion:** all three members have an unambiguous physical meaning.

### Lesson 6.2 — Convert and wrap angles deliberately

**Objective / prerequisites:** distinguish degrees from radians and shortest signed differences; requires functions, comparisons, and `Pose`. **Proposed files/functions:** `main/src/Libraries/util.{hpp,cpp}`, `degreesToRadians()` and `angleWrap()`.

**Why:** trigonometric functions expect radians. An angle boundary can also make a tiny physical turn appear almost a complete revolution.

A full turn is `360` degrees or `2*pi` radians, where `pi` is the circle constant. A degree value `d` becomes `d*pi/180` radians. Define `angleWrap(a)` to return a value in `[-pi, pi)`: `a` is any finite input angle in radians. Wrapping removes whole turns while preserving orientation. State which endpoint includes an exact half-turn and remain consistent.

For shortest heading error, subtract current heading from target heading, then wrap the difference. Wrapping each separately before subtracting does not ensure a short difference. At exactly a half-turn, either turning direction is equally short; the selected endpoint resolves the tie. Reject nonfinite input rather than letting an adjustment loop run forever.

Reference [`degToRad()`, `radToDeg()`, and `nearAngle()`](../5225A-2024-2025-X-Drive/main/src/Libraries/util.cpp) are examples to interpret. `nearAngle(angle, reference)` returns a wrapped difference despite its name; do not assume it returns an adjusted absolute heading.

**One exercise:** implement `angleWrap()` with the stated interval.

**Check:** adding full turns leaves the wrapped result equivalent. Transitioning from `+179` degrees to `-179` degrees yields a small signed change after conversion and differencing; calculate its sign yourself.

<details>
<summary>Hints — open after your attempt</summary>

remove a full turn whenever outside the interval; examine both endpoints independently.

</details>

**Common mistakes:** passing degrees to `sin`, exact equality comparisons. **Completion:** boundary and full-turn checks satisfy the interval.

### Lesson 6.3 — Turn body displacement into field displacement

**Objective / prerequisites:** understand a frame change; requires radians, sine/cosine, and axes. **Proposed files/functions:** `main/src/Libraries/util.{hpp,cpp}`, `bodyToField()`.

**Why:** forward motion after a turn changes field position in a different direction. Adding forward distance directly to field `Y` works only at zero heading.

Let `u` be a small body-right displacement and `v` a small body-forward displacement, both meters. Let `theta` be CCW heading in radians. Trace the basis arrows: body-right has field components `(cos(theta), sin(theta))`; body-forward has components `(-sin(theta), cos(theta))`. Scale each arrow by its displacement and add matching field components. The matrix recording these two columns is

```text
R(theta) = [ cos(theta)  -sin(theta) ]
           [ sin(theta)   cos(theta) ]
```

The output `(field_dx, field_dy)` is an increment in meters, not new absolute position. At zero heading the frames align. At a left quarter-turn, forward points field-left. Rotation preserves vector length: the frame changes, not physical displacement. Later, the inverse conversion uses the matrix transpose, equivalent to rotation by `-theta`.

Compare [`Vector::rotate()`](../5225A-2024-2025-X-Drive/main/src/Libraries/util.cpp). Reference [`Tracking::updatePos()`](../5225A-2024-2025-X-Drive/main/src/Chassis/tracking.cpp) uses different field-update signs; do not transplant them into our convention.

**One exercise:** implement `bodyToField()` by adding the two scaled basis arrows.

**Check:** zero heading preserves components; arbitrary heading preserves length within numerical tolerance. Identify the quarter-turn direction on your own sketch.

<details>
<summary>Hints — open after your attempt</summary>

assemble one field component at a time; draw the arrows at a quarter-turn if uncertain.

</details>

**Common mistakes:** reversing the transform, swapping forward/right. **Completion:** identity and length checks pass, with signs explained physically.

### Lesson 6.4 — Measure elapsed time and handle an invalid interval

**Objective / prerequisites:** convert timestamp differences to seconds and guard division; requires subtraction and conditionals. **Proposed files/functions:** `main/src/Libraries/util.{hpp,cpp}`, `elapsedSeconds()`.

**Why:** speed and derivative calculations divide by elapsed time. A requested loop period is a scheduling intention; actual elapsed time is what measurements experienced.

Let `t_now` and `t_previous` be millisecond timestamps from the same monotonic clock. Their difference `dt_ms` becomes `dt = dt_ms/1000.0` seconds; the decimal divisor prevents integer division from discarding fractions. Displacement `ds` in meters divided by `dt` is meters per second. Heading change in radians divided by `dt` is radians per second. Neither is a motor command.

Initialize the previous timestamp before updating. If readings share a timestamp, `dt` is zero: mark velocity or derivative unavailable; never divide or invent a small denominator. A valid displacement update need not be discarded solely because optional velocity cannot be computed. Detect excessive gaps separately, since the small-increment assumptions may fail. Use an unsigned timestamp difference appropriately for the local clock's rollover behavior.

Reference [`Tracking::updatePos()`](../5225A-2024-2025-X-Drive/main/src/Chassis/tracking.cpp) multiplies by `1000` after dividing by milliseconds. Inspect update order: some velocities use local displacement before that cycle's value is assigned.

**One exercise:** write a helper returning elapsed seconds from two millisecond timestamps, with an explicit zero-interval result.

**Check:** repeated timestamps never cause division by zero or nonfinite velocity.

<details>
<summary>Hints — open after your attempt</summary>

distinguish integer from floating-point division; decide validity before division.

</details>

**Common mistakes:** assuming exactly `10 ms` elapsed, uninitialized time. **Completion:** valid and zero-interval cases behave as documented.

<a id="milestone-7"></a>

## Milestone 7 — Read sensors and derive incremental odometry

**Intended behavior:** estimate field pose from two tracking-wheel rotation readings and IMU heading. Begin with supplied numerical samples on the computer. Hardware checks later establish actual orientation, offsets, diameter, and sensor directions; reference constants do not establish these facts.

Use two **idealized, orthogonal** tracking wheels: one measures body-right travel and sits at signed body-forward offset `b_x`; the other measures body-forward travel and sits at signed body-right offset `b_y`. Offsets are meters from the chosen tracking center. Positive `b_x` is ahead; positive `b_y` is right. Nonorthogonal hardware needs a more general model.

### Lesson 7.1 — Capture a valid baseline before calculating a delta

**Objective / prerequisites:** understand previous/current samples and validity; requires structs, state, and Lesson 6.4. **Proposed files/functions:** `main/src/Chassis/tracking.hpp` defines `SensorSample` and `TrackingState`; `tracking.cpp` contains pure `resetTracking()`; later `tracking_io.cpp` contains hardware `captureSensors()`.

**Why:** sensors report cumulative values. Subtracting an uninitialized previous value invents startup motion.

Hardware capture validates raw device readings before conversion. The proposed PROS-free `SensorSample` stores signed cumulative wheel distances in meters, converted CCW heading in radians, a monotonic millisecond timestamp, and validity. Keep its timestamp unit explicit; only elapsed intervals become seconds. A desktop harness supplies the same standardized sample without device headers.

Start with a chosen pose and a complete valid sample. Save it as baseline without applying motion. Following increments use current minus previous values. Resetting pose also refreshes baseline, preventing earlier travel from being reapplied. Reject device error sentinels and readings during IMU calibration. After a sensor reset, reconnect, or inconsistent history, establish a fresh baseline before continuing.

The reference [`TrackingWheel` constructor](../5225A-2024-2025-X-Drive/main/src/Chassis/tracking.cpp) initializes a previous wheel position, but `Tracking::updatePos()` uses uninitialized `angle` initially. `getGyroAngle()` returns zero during calibration; treating that as a measurement can fabricate heading change. Reference [`config.cpp`](../5225A-2024-2025-X-Drive/main/src/config.cpp) defines the two rotation sensors and IMU; ports remain unverified for our robot.

**One exercise:** write the baseline initialization step for supplied standardized samples.

**Check:** initializing with nonzero cumulative distances leaves pose unchanged; replaying the same sample produces zero increments.

<details>
<summary>Hints — open after your attempt</summary>

initialization is not a motion update; track baseline validity explicitly.

</details>

**Common mistakes:** assuming startup readings are zero, resetting only pose. **Completion:** the first sample establishes history without fabricated motion.

### Lesson 7.2 — Convert sensor increments into wheel travel

**Objective / prerequisites:** turn a shaft-angle change into signed linear travel; requires units, baselines, and circumference. **Proposed files/functions:** `main/src/Chassis/tracking.{hpp,cpp}`, pure `rotationDeltaToMeters()`; hardware measurements later belong in `config`.

**Why:** a sensor measures shaft angle; odometry needs distance rolled by a wheel. Diameter, transmission, and direction connect them.

PROS [`Rotation::get_position()`](https://pros.cs.purdue.edu/v5/api/cpp/rotation.html#get-position) returns centidegrees; the [local header](../5225A-2024-2025-X-Drive/main/include/pros/rotation.hpp) matches. One centidegree is one hundredth of a degree, so a full turn is `36000` centidegrees. Let `dq` be raw position change in centidegrees, `D` measured diameter in meters, `g` wheel revolutions per sensor revolution, and `s` verified sign factor `+1` or `-1`. Sensor revolutions are `dq/36000.0`; multiply by `g` and circumference `pi*D`, then apply `s` once. Direct coupling proposes `g = 1`, subject to inspection. The same linear conversion can standardize cumulative positions before differencing.

Use cumulative position for multi-turn travel. A bounded angle needs wrap handling. Roll each wheel in its proposed positive body direction before selecting reversal or software sign; avoid applying both.

Reference [`TICKS_TO_INCHES`](../5225A-2024-2025-X-Drive/main/src/Chassis/tracking.hpp) assumes a `2.75` inch diameter; tracking also uses empirical multipliers. These do not establish our dimensions.

**One exercise:** implement the numerical conversion helper with supplied diameter, ratio, and sign.

**Check:** direct-coupled full-turn travel magnitude equals circumference; reversing the input reverses output.

<details>
<summary>Hints — open after your attempt</summary>

cancel units in stages; retain fractional revolutions.

</details>

**Common mistakes:** centidegrees as degrees, inches mixed with meters. **Completion:** dimensions and sign checks pass.

### Lesson 7.3 — Remove travel caused by turning around the center

**Objective / prerequisites:** derive offset compensation; requires signed travel, heading increments, and the idealized layout. **Proposed files/functions:** `main/src/Chassis/tracking.{hpp,cpp}`, `compensateTrackingOffsets()`.

**Why:** an off-center wheel travels along an arc during an in-place turn although the tracking center does not translate. Counting all wheel travel as translation creates drift.

Let `dtheta` be a small CCW heading increment in radians. A point at body coordinates `(r_x, r_y)` gains rotational travel components approximately `(-r_y*dtheta, r_x*dtheta)`. Derive the signs physically: an ahead-of-center point moves left during a left turn; a right-of-center point moves forward. This is the planar cross-product direction.

Let `u` and `v` denote center travel integrated along the instantaneous body-right and body-forward axes over the interval. Let `m_x` and `m_y` be signed measured wheel increments. These four quantities use meters. The ideal sensor equations are

```text
m_x = u - b_x*dtheta
m_y = v + b_y*dtheta
```

These are travel integrals, not yet exact finite field displacement. Isolate each center term yourself. Offsets are signed geometry, not merely positive distances from the center. Determine actual measurement directions and tracking center before measuring them.

Reference [`Tracking::updatePos()`](../5225A-2024-2025-X-Drive/main/src/Chassis/tracking.cpp) uses variable effective radii and additional geometry compensation. Those specialized constants are outside our first orthogonal model.

**One exercise:** implement a helper isolating center travel from the two sensor equations.

**Check:** ideal supplied pure-rotation samples yield zero compensated center travel within numerical tolerance, including a negative offset.

<details>
<summary>Hints — open after your attempt</summary>

solve each equation independently; sketch the ahead-of-center wheel during a left turn.

</details>

**Common mistakes:** adding raw travel to position, compensating an offset twice. **Completion:** ideal pure rotation produces no estimated translation.

### Lesson 7.4 — Integrate center travel into field pose

**Objective / prerequisites:** combine sensor deltas, heading, compensation, and transforms; requires Lessons 6.2–7.3. **Proposed files/functions:** `main/src/Chassis/tracking.{hpp,cpp}`, pure `updateOdometry()`.

**Why:** odometry chains small trustworthy increments. A wrong heading sign rotates every later translation into the wrong field direction.

PROS [`Imu::get_rotation()`](https://pros.cs.purdue.edu/v5/api/cpp/imu.html#get-rotation) reports unbounded degrees, positive clockwise; [local documentation](../5225A-2024-2025-X-Drive/main/include/pros/imu.hpp) matches. Convert at capture to our proposed CCW radians, with verified mounting and initial heading offset. Bounded `get_heading()` instead wraps every revolution. For bounded samples, wrap the converted current-minus-previous difference to obtain `dtheta`, assuming less than a half-turn between valid samples. Unbounded rotation permits a continuous difference after validated conversion.

Given old heading `theta_old`, use midpoint heading `theta_mid = theta_old + dtheta/2` to transform compensated body travel into a field increment. Add it to old position, then advance heading. Keep raw history distinct from the pose's starting orientation.

Midpoint integration is an **approximation** because the axes rotate during the interval. It suits small heading changes and smooth travel. Larger turns need subdivision or a separately derived constant-twist arc/chord correction; never apply that correction twice. Near-zero `dtheta` works without dividing by it.

**One exercise:** connect your existing helpers into `updateOdometry()` for one valid supplied sample.

**Check:** unchanged readings leave pose fixed; straight travel at zero heading follows the aligned field axis; pure rotation changes heading only; tiny heading changes remain finite. Desktop evidence checks the ideal model, not slip or mounting.

<details>
<summary>Hints — open after your attempt</summary>

calculate increments before updating history; transform at midpoint orientation.

</details>

**Common mistakes:** stale baselines, clockwise/CCW mismatch, exact-zero branches. **Completion:** these ideal checks pass and approximation limits are recorded.

<a id="milestone-8"></a>

## Milestone 8 — Build heading feedback in small steps

Intended behaviour: turn wrapped heading error into a bounded command with explicit controller state. Evidence: signed P traces, time/reset checks, and output-limit behaviour; I and D stay disabled until justified.

### Lesson 8.1 — Understand proportional feedback before implementing PID

**Objective and prerequisite:** Explain proportional feedback and design its interface after completing the motor, units, and coordinate lessons. Begin with integral and derivative gains at zero.

**Physical reason:** A fixed motor command cannot correct a changing distance or heading error. Feedback recomputes a command from the difference between the desired measurement and the current measurement.

**Guided explanation:** Define the error as $e=r-z$, where $r$ is the target and $z$ is the measurement, both in the same units. Derive $u_P=k_Pe$. For position, $e$ is meters, $u_P$ is a dimensionless normalized command, and $k_P$ therefore has units $1/\mathrm m$. An angular controller instead uses radians and a gain with units $1/\mathrm{rad}$. A positive command must increase the corresponding measurement; otherwise the same equation becomes positive feedback. Doubling a gain doubles an unsaturated command without guaranteeing better behavior.

**Proposed files/functions:** `main/src/Libraries/pid.hpp` and `pid.cpp`, `updateController()`, for controller configuration and state independent of PROS hardware. Read [PID::compute](../5225A-2024-2025-X-Drive/main/src/Libraries/pid.cpp) for its `target - input` structure. Declare its input/output interface before writing the calculation.

**One exercise:** Implement the P-only `updateController()` calculation using an explicitly documented error input in radians and dimensionless output. Keep I and D gains at zero.

**Check:** Zero error produces zero P output; a trace approaching the target from either side gives a restoring sign; doubling an unsaturated error doubles its P output.

<details>
<summary>Hints — open after your attempt</summary>

**Hint 1:** Write input and output units beside each parameter.

**Hint 2:** Ask what motion a positive output should cause before choosing its sign.

</details>

**Common mistake:** Reusing an old robot's tuned gain without checking units or motor polarity.

**Complete when:** You can explain the sign, gain units, and zero-error behavior without hardware.

### Lesson 8.2 — Give the controller explicit state and time

**Objective and prerequisite:** Define controller state, elapsed time, and reset behavior after the P-only trace. Explain I and D with both gains initially zero.

**Proposed files/functions:** `main/src/Libraries/pid.{hpp,cpp}`, `updateController()` and `resetController()`.

**Physical reason:** Integral remembers sustained error; derivative estimates how rapidly error changes. Both depend on time and cannot be implemented correctly as arbitrary per-loop additions.

**Guided explanation:** Let $\Delta t$ be the measured interval in seconds between valid updates. Define an accumulated error $J$ in error-units times seconds and reason toward $J_k=J_{k-1}+e_k\Delta t$. Define an error-rate estimate from two successive errors and the interval separating them. Then identify the units required for $k_IJ$ and $k_D\dot e$ to produce dimensionless commands. Distinguish stored accumulated error from stored gain-weighted integral contribution.

In proposed `main/src/Libraries/pid.{hpp,cpp}`, state should include accumulated error, previous error, and whether a previous valid sample exists. Specify `updateController(...)` and `resetController(...)` behaviour without filling in their bodies. Do not invent a previous measurement; reject or hold on invalid/nonpositive time. Read [PID::compute](../5225A-2024-2025-X-Drive/main/src/Libraries/pid.cpp): its timer is used in milliseconds, so its gains cannot be transferred directly to this seconds-based design.

**One exercise:** Draft a state table for construction, first update, second update, and reset.

**Check:** A 10 ms interval enters this controller as 0.010 s; reset removes previous-motion history.

<details>
<summary>Hints — open after your attempt</summary>

**Hint 1:** Separate configuration from values that change each update.

**Hint 2:** Trace the first sample before deriving a derivative.

</details>

**Common mistake:** Assuming the requested sleep duration equals measured elapsed time.

**Complete when:** Every stored value has a meaning, units, and initialization rule. Use this table to implement `resetController()` as the next tiny step; complete that check before enabling history-dependent terms.

### Lesson 8.3 — Handle wrapped heading error and derivative noise

**Objective and prerequisite:** Compute wrapped heading error and explain derivative limitations after the heading and controller-state lessons.

**Proposed files/functions:** `main/src/Libraries/util.{hpp,cpp}`, existing `angleWrap()`; use its result as the heading error passed to `updateController()`.

**Physical reason:** Heading is periodic. Subtracting two displayed angles near the wrap boundary can request almost a full turn when the target is nearby. Differencing sensor noise can also create large, alternating commands.

**Guided explanation:** Define $e_\theta=\operatorname{wrap}(\theta_t-\theta)$, in radians, with positive heading counterclockwise/left from the initial forward direction. Document a half-open interval, such as $[-\pi,\pi)$, including its half-turn convention. The reference's [nearAngle](../5225A-2024-2025-X-Drive/main/src/Libraries/util.cpp) supplies a wrapped relative angle, but its boundary behavior should be traced before reuse. Use the existing proposed `angleWrap(...)` in `main/src/Libraries/util.{hpp,cpp}` on the target-minus-current difference; a second wrapping implementation is unnecessary.

Now consider derivative: tiny measurement changes divided by short intervals can produce large rates. Target jumps produce derivative kick when differentiating error. Explain these effects before filtering or differentiating measurement. If successive wrapped errors straddle the chosen boundary, naive subtraction creates another artificial jump; any later angular derivative needs a deliberate angular-difference policy. Keep $k_D=0$ while basic heading P control is validated.

**One exercise:** Trace a heading target of $-179^\circ$ from a measured $+179^\circ$, showing the wrapped sign and the requested physical turn.

**Check:** Adding $2\pi$ to either angle does not change the physical error.

<details>
<summary>Hints — open after your attempt</summary>

**Hint 1:** Draw both directions around a circle.

**Hint 2:** Convert degrees to radians before passing values into the planned function.

</details>

**Common mistake:** Treating a display wrap as a sudden physical rotation.

**Complete when:** You can defend wrap boundaries and explain why derivative remains disabled initially.

### Lesson 8.4 — Understand saturation, integral windup, and reset

**Objective and prerequisite:** Describe integral windup and propose a conservative controller policy after the P, time, and output-limits lessons. Enable I only for a justified problem.

**Proposed files/functions:** `main/src/Libraries/pid.{hpp,cpp}`, `updateController()` and `resetController()`; this lesson proposes policies while I remains disabled.

**Physical reason:** Motors have finite command limits. A large error can demand more output than the drivetrain can deliver; accumulating that error can keep pushing after the target has been reached.

**Guided explanation:** Distinguish requested normalized command $u_{raw}$ from applied limited command. At the upper bound, positive error can grow integral without additional actuation. Reason through what happens after crossing the target.

In proposed `main/src/Libraries/pid.{hpp,cpp}`, compare integral limits with conditional integration: a candidate integral update is rejected when it would drive an already saturated output farther into saturation, but integration that helps leave saturation may remain useful. A local clamp cannot know what the wheel mixer applied. If combined translation and rotation are scaled downstream, keep $k_I=0$ initially; later integration must account for applied-output limits. The reference [PID::compute](../5225A-2024-2025-X-Drive/main/src/Libraries/pid.cpp) resets integral under selected error conditions, but this does not establish actuator-aware antiwindup.

**One exercise:** Draw a time trace where a command saturates, the error later reverses, and an unchecked integral delays reversal.

**Check:** Resetting between motion requests clears accumulated error and prior derivative history.

<details>
<summary>Hints — open after your attempt</summary>

**Hint 1:** Draw requested and applied commands on separate lines.

**Hint 2:** Ask whether an integral update increases or decreases the saturated demand.

</details>

**Common mistake:** Clamping the final output while allowing stored integral to grow forever.

**Complete when:** You explain when I is useful and names its saturation and reset policies.

<a id="milestone-9"></a>

## Milestone 9 — Move toward a position and heading

Intended behaviour: derive body commands from field-pose error, then run a small synchronous motion with explicit termination. Evidence: frame, output, ownership, timeout, and stop checks; arrival requires valid pose evidence.

### Lesson 9.1 — Convert a field position error into body axes

**Objective and prerequisite:** Design a deterministic `computePoseCommand(...)` calculation after pose, vector, heading, and P-controller lessons. It reads supplied data and computes commands without reading sensors or moving motors.

**Proposed files/functions:** `main/src/Libraries/util.{hpp,cpp}`, `fieldToBody()`; `main/src/Chassis/chassis.{hpp,cpp}`, `computePoseCommand()`.

**Physical reason:** A target is expressed on the field, while the wheels respond relative to the robot. The same field target needs different strafe and forward commands as the robot turns.

**Guided explanation:** Define field $X$ right, field $Y$ up, body $x$ right, body $y$ forward, and heading $\theta$ counterclockwise from forward along field $+Y$. At zero heading the frames coincide. Let $e_X=X_t-X$ and $e_Y=Y_t-Y$, both meters. Starting from the body-to-field rotation matrix, derive its inverse by transposing it. Fill the gaps in:

$$
e_x=(\_\_)e_X+(\_\_)e_Y,\qquad
e_y=(\_\_)e_X+(\_\_)e_Y.
$$

Apply separate P gains to body strafe and forward errors. Compute heading error independently from the target's heading, not from the direction to its position. Proposed `main/src/Chassis/chassis.{hpp,cpp}` owns this calculation; `Libraries/util.{hpp,cpp}` owns `Pose` and frame transforms. Read [DriveMoveToTargetParams::handle](../5225A-2024-2025-X-Drive/main/src/Chassis/chassis.cpp) for separation of position and angular errors, without adopting its advanced control stack.

**One exercise:** Derive the inverse transform and trace a target one meter along field $+X$ when heading is $90^\circ$.

**Check:** At $0^\circ$, the transformed error equals the original components; rotation preserves its length.

<details>
<summary>Hints — open after your attempt</summary>

**Hint 1:** Ask where body forward points at a quarter-turn left.

**Hint 2:** Check a unit basis vector before a general vector.

</details>

**Common mistake:** Rotating with the forward matrix when an inverse transform is required.

**Complete when:** The transform, units, and independent heading target are explained correctly and implemented in `fieldToBody()` and the P-only `computePoseCommand()` as separate tutor-guided micro-edits before 9.2.

### Lesson 9.2 — Bound translation and preserve wheel-command ratios

**Objective and prerequisite:** Connect pose commands to the planned mixer after deriving body errors. Distinguish chosen motion limits from actuator output limits.

**Proposed files/functions:** `main/src/Chassis/chassis.cpp`, limits within `computePoseCommand()`; `main/src/drive.cpp`, existing `mixDrive()` and `normalizeWheelCommands()`.

**Physical reason:** Position and heading errors can demand large simultaneous commands. Independent wheel clipping distorts the intended combination of strafe, forward, and turn.

**Guided explanation:** Name the dimensionless requests `cmdStrafe`, `cmdForward`, and `cmdTurn`. First bound the translation-vector magnitude to a configured limit $L$ using one scalar when necessary, preserving its direction. Bound angular demand using a documented turn limit. Individual axis limits do not guarantee feasible wheels.

Then pass `DriveCommand` to hardware-independent `mixDrive(...)` in `main/src/drive.{hpp,cpp}`. For raw wheel values $w_i$, derive a shared denominator from $\max(1,\max_i|w_i|)$. Use your existing `normalizeWheelCommands(...)`, returning `WheelCommands`. `drive_io.cpp` owns `moveDrive(...)`, `applyWheelCommands(...)`, and `stopDrive(...)`. Read [moveDrive](../5225A-2024-2025-X-Drive/main/src/drive.cpp): its reference sums omit this normalization. Positive turn must produce counterclockwise motion after physical motor polarity is verified.

**One exercise:** Choose a mixed strafe/forward/turn request that exceeds a wheel limit and calculate the shared scaling step.

**Check:** Every final magnitude is at most one; nonzero wheel ratios and signs remain unchanged by shared scaling.

<details>
<summary>Hints — open after your attempt</summary>

**Hint 1:** Compute all raw wheel values before limiting any one of them.

**Hint 2:** A denominator below one would amplify an already valid request.

</details>

**Common mistake:** Claiming wheel normalization preserves independently requested magnitudes or guarantees tracking performance. Common downward scaling preserves upper bounds, while reducing the requested magnitudes.

**Complete when:** You trace both bounding stages and distinguishes math from motor writes.

### Lesson 9.3 — Give one loop ownership of sensing and actuation

**Objective and prerequisite:** Design a synchronous `moveToPose(...)` wrapper after the pure pose-command calculation and motor boundary. Its caller waits for a result.

**Proposed files/functions:** Later hardware `main/src/Chassis/chassis_io.cpp`, `moveToPose()`; computer-only `desktop/lesson_main.cpp`, `main()` supplies samples and captures one iteration without linking hardware.

**Physical reason:** Two loops writing drivetrain commands can override each other; inconsistent pose reads can combine measurements taken at different times. Start with one owner before considering background tasks.

**Guided explanation:** Proposed `main/src/Chassis/chassis.{hpp,cpp}` defines `PoseTarget`/result types and deterministic `computePoseCommand(...)`; `chassis_io.cpp` later owns hardware-facing `moveToPose(...)`. Plan one iteration in order: check expected mode and abort conditions, capture one valid sample and update odometry, obtain that iteration's pose snapshot, measure elapsed seconds, calculate field/body/heading errors, compute bounded commands, apply motors once, update completion bookkeeping, and yield until the next cycle. Reject invalid or nonfinite pose, target, time, or command values before transforms and motor writes. Recheck abort conditions before a motor write if sensing can take significant time.

The wrapper needs a verified pose provider; declaring `Pose` does not create odometry. Do not invent sensors or copy old tracking geometry. A desktop version can use supplied fake pose snapshots, an artificial clock, and captured motor commands. The reference [Chassis::moveToTarget](../5225A-2024-2025-X-Drive/main/src/Chassis/chassis.cpp) supplies a motion entry point; propose a simpler synchronous loop. Do not run tracking or driver tasks alongside it.

**One exercise:** Implement one synchronous motion-loop iteration using your previously checked helpers, marking the single sensor reader and motor writer. Keep termination policy separate until 9.4.

**Check:** A static fake pose produces repeated feedback requests; it does not demonstrate convergence.

<details>
<summary>Hints — open after your attempt</summary>

**Hint 1:** Label each value with the iteration that produced it.

**Hint 2:** Identify every possible second owner before adding a loop.

</details>

**Common mistake:** Calling the driver-control loop and pose-control loop concurrently.

**Complete when:** Ownership, pose-provider requirements, and synchronous return behavior are explicit.

### Lesson 9.4 — Separate reaching the target from terminating a motion

**Objective and prerequisite:** Specify success, timeout, and abort behavior after planning the synchronous loop. Returning does not establish arrival.

**Proposed files/functions:** Later hardware `main/src/Chassis/chassis_io.cpp`, `moveToPose()`, and `main/src/main.cpp`, lifecycle callbacks; desktop `desktop/lesson_main.cpp`, `main()` supplies a clock and records terminal output.

**Physical reason:** A robot may cross a target briefly, stall, lose a usable sensor reading, or become disabled. A controlled motion needs an intentional ending in every case.

**Guided explanation:** Define position tolerance $\epsilon_p$ in meters and heading tolerance $\epsilon_\theta$ in radians. Use the field-error magnitude and absolute wrapped heading error. Require both to remain within tolerance for a continuous settling interval $T_s$, measured in seconds; leaving either tolerance resets that interval. Settling demonstrates sampled proximity, not zero velocity.

Define a separate maximum duration $T_{max}$. Plan distinguishable results such as reached, timed out, disabled, mode changed, and invalid pose. Choose abort precedence deliberately so a disabled robot does not report successful motion. Require zero commands on every cooperative return path, including success, and reset controller history before a new request. Proposed `main/src/Chassis/chassis_io.cpp`/`moveToPose(...)` owns that cleanup; no early return bypasses it. PROS may forcibly stop the calling task on a mode transition, bypassing this cleanup. Therefore planned `main/src/main.cpp`/`disabled()` also stops the drivetrain; each `autonomous()` and `opcontrol()` entry starts with zero output, fresh controller state, and validated tracking history. Check the expected active mode as well as disabled status. Do not rely on an end-of-loop statement or destructor after forced task cancellation. If tracking paused while the robot moved, re-establish a known pose rather than pretending the old pose is current. Read [DriveMoveToTargetParams::handle](../5225A-2024-2025-X-Drive/main/src/Chassis/chassis.cpp) for its error and timeout checks, without its advanced control stack.

**One exercise:** Implement the termination decision for `moveToPose()` with distinct result reasons and cooperative zero-output cleanup. Plan the separate callback stop/reset hooks described above; implement those as the next small hardware step.

**Check:** A never-changing out-of-tolerance pose returns timeout and ends with four zero wheel commands. A scripted enter-exit-enter tolerance trace resets settling correctly. Mode-transition stop/reset is a separate lifecycle check; a desktop cooperative return does not verify runtime task cancellation.

<details>
<summary>Hints — open after your attempt</summary>

**Hint 1:** Draw separate motion-duration and settling timers.

**Hint 2:** List return reasons before arranging branches.

</details>

**Common mistake:** Treating timeout as success or leaving the last nonzero command active.

**Complete when:** Every result has a precise condition and verified zero-output cleanup.

<a id="milestone-10"></a>

## Milestone 10 — Trace and check the complete loop

Intended behaviour: explain both sensing-to-pose and error-to-command paths, then isolate a wrong result with one useful check. Evidence: independently explained traces with software and hardware outcomes distinguished.

### Lesson 10.1 — Trace the full calculation before a physical trial

**Objective and prerequisite:** Verify the planned chain from pose input to motion result after all prior lessons. Keep calculations, build status, upload status, and measured robot behavior distinct.

**Proposed files/functions:** `desktop/lesson_main.cpp`, `main()` calls pure `computePoseCommand()`, `mixDrive()`, and `normalizeWheelCommands()` for the trace.

**Physical reason:** Equations can use wrong frames, signs, units, motor order, or termination conditions. A trace exposes assumptions before hardware movement.

**Guided explanation:** Use a desktop worksheet or captured fake inputs to record current/target `Pose`, field error, wrapped heading error, body error, P demands, configured limits, raw mixer outputs, normalized outputs, and termination decision. Label meters, radians, seconds, and dimensionless commands. Follow `computePoseCommand(...)` → `mixDrive(...)` → `normalizeWheelCommands(...)` → motor boundary; compare responsibilities with reference [moveDrive](../5225A-2024-2025-X-Drive/main/src/drive.cpp) and [DriveMoveToTargetParams::handle](../5225A-2024-2025-X-Drive/main/src/Chassis/chassis.cpp).

Cover field $+X$ targets at headings $90^\circ$ and $45^\circ$, zero error, heading wrap boundaries, a saturating mixed request, timeout, and a disabled abort. At $45^\circ$, reason about both translation components. Scripted changing fake poses can verify reactions and completion logic, but they do not show that commands physically caused those changes. Static fake sensors cannot show convergence.

**One exercise:** Produce one complete trace at $45^\circ$, with unchanged target heading, justifying every command sign.

**Check:** The transform preserves translation-error magnitude; shared normalization preserves wheel ratios; all terminal cases capture zero output.

<details>
<summary>Hints — open after your attempt</summary>

**Hint 1:** Set heading error to zero to isolate the translation transform.

**Hint 2:** Compare the result with the neighboring zero- and quarter-turn cases.

</details>

**Common mistake:** Reporting a successful fake-input trace as a tested robot motion.

**Complete when:** You explain the complete trace and identify hardware polarity and pose validation still required later.

### Lesson 10.2 — Trace one sensor sample all the way to pose

**Objective and prerequisites:** explain the sensing half of the complete loop after the baseline, conversion, compensation, and field-transform lessons. **Physical reason:** plausible-looking pose values can hide an incorrect sensor sign, repeated increment, or stale heading.

**Proposed files/functions:** `main/src/Chassis/tracking_io.cpp`, `captureSensors()`; pure `tracking.cpp`, `resetTracking()` and `updateOdometry()`; desktop `desktop/lesson_main.cpp`, `main()` for supplied samples.

Choose an ideal stationary baseline followed by one short straight-motion sample. A trace has columns for raw wheel counts in centidegrees, raw heading in degrees, read validity, timestamp in milliseconds, signed cumulative distances in metres, converted heading in radians, wheel increments, `dtheta`, compensated body travel, field increment, and resulting `Pose`. Explain which values are measured or supplied, which are configuration assumptions, and which are computed. Compare the responsibilities with [TrackingWheel::updatePos and Tracking::updatePos](../5225A-2024-2025-X-Drive/main/src/Chassis/tracking.cpp).

Process a sample once. Update previous readings only at the documented acceptance point. Reject an invalid sample without substituting a numeric zero measurement. If measurements recover after a long gap, rebaseline or apply a documented recovery policy; do not conceal the gap with a guessed interval. At each active iteration, use the new valid pose before computing feedback.

**One exercise:** produce one baseline-to-straight-motion trace using your own supplied sample and previously derived conversion.

**Check:** replaying the last cumulative readings does not add the same travel again. A sensor-invalid sample cannot reach the pose controller as a trustworthy new measurement.

<details>
<summary>Hints — open after your attempt</summary>

separate cumulative readings from increments; write the acceptance point before changing history.

</details>

**Common mistake:** subtracting the same old baseline every cycle. **Complete when:** every pose change can be traced to one accepted sample with stated units.

### Lesson 10.3 — Locate a wrong result and record what was verified

**Objective and prerequisites:** choose a useful diagnostic check after completing both command and sensing traces. **Physical reason:** changing several signs or gains at once can hide the original problem and create a second one.

**Proposed files/functions:** desktop `desktop/lesson_main.cpp`, `main()` for a single numerical case; later hardware `main/src/drive_io.cpp`, `checkOneMotorDirection()` or `applyWheelCommands()`, selected according to the failing boundary. This lesson does not authorize a physical trial automatically.

Start with the first value that disagrees with your prediction. Wrong body error suggests a frame or angle problem. Correct body commands but wrong raw corner values suggest mixing or ordering. Correct mathematical corner values but wrong wheel motion suggest routing, reversal, wiring, or mechanism direction. Correct short-direction motion but wrong estimated pose suggests sensor conversion, sign, offset, or baseline. Correct directions with oscillation suggests gains, timing, sensor noise, or saturation. These are hypotheses to test, not diagnoses based on appearance alone.

On the computer, select one case that isolates one function and compare it with an independently explained result. With hardware available, begin from individually verified motor and sensor directions, then use a short bounded pure-motion check with a defined stopping path. Observe both motion and pose before progressing to a mixed or target motion. Compare [drive.cpp](../5225A-2024-2025-X-Drive/main/src/drive.cpp) and [tracking.cpp](../5225A-2024-2025-X-Drive/main/src/Chassis/tracking.cpp) only for responsibilities; old constants do not diagnose this robot.

**One exercise:** choose one hypothetical mismatch and write the smallest check that distinguishes two plausible causes.

**Check:** the proposed check changes one relevant input or boundary and names the distinguishing observation.

<details>
<summary>Hints — open after your attempt</summary>

locate the earliest disagreement; separate numerical output from physical response.

</details>

**Common mistake:** retuning PID to compensate for a reversed motor or wrong coordinate transform. **Complete when:** you can explain what that check would establish and what evidence would remain missing.

## Reference review notes

Use the old source to understand responsibilities and trace behaviour. These specific observations explain why the lessons require independent checks:

- [drive.cpp: `moveDrive()`](../5225A-2024-2025-X-Drive/main/src/drive.cpp) sends summed corner demands directly to motors. Common wheel normalization is absent in that function.
- [util.cpp: `Position::operator+`](../5225A-2024-2025-X-Drive/main/src/Libraries/util.cpp) uses `x + p2.y` for its first component. That is suspicious relative to component-wise vector addition; do not reproduce it.
- [tracking.cpp: `Tracking::updatePos()`](../5225A-2024-2025-X-Drive/main/src/Chassis/tracking.cpp) reads `angle` before its first assignment; `beta` is assigned in the nonzero-turn branch but used afterward; velocity calculations precede assignment of the current `local_x`/`local_y`. Your design must initialize history and use current increments deliberately.
- [tracking.cpp: `getGyroAngle()`](../5225A-2024-2025-X-Drive/main/src/Chassis/tracking.cpp) returns numeric zero while calibrating. Treat calibration as invalid measurement status instead.
- [pid.cpp: `PID::compute()`](../5225A-2024-2025-X-Drive/main/src/Libraries/pid.cpp) uses a millisecond timer. A seconds-based controller needs correspondingly defined gains and state.
- [main.cpp callback comments](../5225A-2024-2025-X-Drive/main/src/main.cpp) describe runtime termination and restart of autonomous/operator tasks. A stopped task may never reach its ordinary cleanup code; mode-boundary stopping and fresh state are separate responsibilities.

These are source-inspection findings. The reference was not built, uploaded, repaired, or tested during guide creation.

## Keep a small evidence record

The tutor keeps a conversational checkpoint unless you authorize saving progress. Use this shape without marking the course complete:

```text
Current milestone / lesson:
Phase: Research / Plan / Implement / Review / Follow-up
File / function: actual saved path, proposed path, or no file/function yet
Intended behaviour:
Implemented work:
Verified evidence: exact calculation, compiler result, or observation
Still assumed / unavailable:
One pending exercise:
```

| Evidence level | What it establishes |
| --- | --- |
| Proposed design | The intended convention, interface, or behaviour has been written down. |
| Saved implementation | Source exists and can be inspected. |
| Successful compilation | Syntax, types, and required symbols pass this build; mathematics may still be wrong. |
| Desktop numerical check | The selected function behaves as checked under supplied inputs and model assumptions. |
| Controlled mathematical simulation | An explicitly modelled plant responds to commands; results depend on that model. Scripted changing poses alone do not establish this. |
| Successful upload | Firmware reached the selected device; physical direction is still unverified. |
| Hardware observation | The named motor, sensor, or motion behaved as recorded under stated conditions. |

A missing hardware path does not prevent learning the pure mathematics. Keep that evidence gap explicit rather than forcing an invented robot setup.

## Current checkpoint and first lesson

**Phase:** Plan. **Lesson:** 1.1, driven rolling versus passive roller motion. **File/function:** no file/function yet. **Implemented:** this guide only. **Verified:** project/reference paths, architecture inspection, local tool availability, and cited API facts. **Not yet verified:** learner code, accepted conventions, numerical drivetrain behaviour, actual hardware configuration, or motion.

When you ask to begin, first establish whether you have a VEX V5 robot and controller available or only your computer. The tutor then presents lesson 1.1's single exercise and waits. Writing this document does not begin the exercises or advance that checkpoint.
