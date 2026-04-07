# PhysicsBoard

Interactive whiteboard application for physics tutoring — built with C++17, Dear ImGui, ImPlot, and GLFW.

## Features

| Feature | Description |
|---------|-------------|
| **5 Boards** | Switch between 5 independent whiteboards via tabs |
| **Drawing Tools** | Pencil, Line, Rectangle, Circle, Arrow, Eraser, Text, Ruler |
| **Function Plotter** | Type math expressions (`sin(x)`, `x^2 + 3*x - 1`, etc.) and see live plots — up to 8 functions simultaneously |
| **Collaboration** | Host a session (teacher) or join (student) via TCP — draw on the same board in real-time |
| **Pan & Zoom** | Middle-mouse drag to pan, scroll wheel to zoom |
| **Undo / Redo** | Ctrl+Z / Ctrl+Y with 50-level undo history per board |
| **Color & Thickness** | Full color picker and thickness slider for all tools |

## Supported Math Functions

The expression parser supports:  `sin`, `cos`, `tan`, `asin`, `acos`, `atan`, `exp`, `log`, `ln`, `log10`, `sqrt`, `abs`, `floor`, `ceil`, `sinh`, `cosh`, `tanh`, and the constants `pi` and `e`.

Variable: `x` — e.g., `sin(2*pi*x) + 0.5*cos(3*x)`

## Building

### Prerequisites
- CMake 3.15+
- C++17 compiler (MSVC 2022 / GCC / Clang)
- Internet (first build downloads GLFW, ImGui, ImPlot via FetchContent)

### Build Steps
```bash
cmake -G Ninja -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build
```

Or open the folder in Visual Studio 2022 — CMake support will auto-configure.

## Collaboration (Network)

1. **Teacher**: Open *View → Network / Collaborate*, set a port (default 7777), click *Start Hosting*.
2. **Student**: Enter the teacher's IP and port, click *Connect*.
3. Both sides see each other's drawings in real-time on the active board.
4. Use *Sync Board to Peer* to send the full board state at any time.

> **Note**: Both teacher and student must be on the same network (or use port forwarding). The protocol is simple TCP — no external server required.

## Project Structure

```
PhysicsBoard/
├── CMakeLists.txt          # Build configuration
├── main.cpp                # GLFW + ImGui bootstrap
├── BoardApp.h/.cpp         # Main app: toolbar, tabs, canvas, rendering
├── Board.h/.cpp            # Stroke data, undo/redo, serialization
├── FunctionPlotter.h/.cpp  # Expression input → ImPlot graphs
├── ExpressionParser.h/.cpp # Recursive-descent math parser
├── NetworkSession.h/.cpp   # TCP host/join collaboration
└── README.md
```

## License

MIT
